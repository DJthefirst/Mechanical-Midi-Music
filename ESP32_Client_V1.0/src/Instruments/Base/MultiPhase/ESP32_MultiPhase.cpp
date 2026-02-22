#include "Config.h"

#if defined(PLATFORM_ESP32) && defined(CFG_INSTRUMENT_MULTIPHASE) && defined(CFG_COMPONENT_MULTIPHASE) && defined(CFG_COMPONENT_PWM)

#include "Instruments/Base/MultiPhase/ESP32_MultiPhase.h"
#include "Instruments/Components/InterruptTimer.h"
#include "Instruments/Components/NoteTable.h"
#include "Arduino.h"
#include "Distributors/Distributor.h"
#include <bitset>

constexpr uint8_t pwmPins[] = {CFG_PINS_INSTRUMENT_PWM};
constexpr uint8_t numPwmPins = sizeof(pwmPins) / sizeof(pwmPins[0]);
constexpr uint8_t wavetable[CFG_MULTIPHASE_WAVE_TABLE_STEPS][CFG_MULTIPHASE_WAVE_TABLE_OUTPUTS] = CFG_MULTIPHASE_WAVE_TABLE;

// Define static member variables
std::array<uint8_t, CFG_NUM_INSTRUMENTS> ESP32_MultiPhase::m_activeNotes = {};
uint8_t ESP32_MultiPhase::m_numActiveNotes = 0;
std::array<uint16_t, CFG_NUM_INSTRUMENTS> ESP32_MultiPhase::m_notePeriod = {};
std::array<uint16_t, CFG_NUM_INSTRUMENTS> ESP32_MultiPhase::m_activePeriod =  {};
std::array<uint16_t, CFG_NUM_INSTRUMENTS> ESP32_MultiPhase::m_currentTick = {};
std::array<uint8_t, CFG_NUM_INSTRUMENTS> ESP32_MultiPhase::m_currentState = {};
std::array<uint16_t, CFG_NUM_INSTRUMENTS> ESP32_MultiPhase::m_vibratoPhase = {};
std::array<uint8_t,CFG_NUM_INSTRUMENTS> ESP32_MultiPhase::m_vibratoDepth = {};
std::array<uint8_t,CFG_NUM_INSTRUMENTS> ESP32_MultiPhase::m_vibratoRate = {}; //slow default rate

ESP32_MultiPhase::ESP32_MultiPhase() : InstrumentControllerBase()
{
    //Setup pins
    for(uint8_t i=0; i <numPwmPins; i++){
        pinMode(pwmPins[i], OUTPUT);
    }

    delay(500); // Wait a half second for safety

    // Setup timer hardware and register the base tick callback. If a
    // specialized subclass (like StepSwPWM) wants to take ownership it can
    // call InterruptTimer::setCallback() to replace this handler.
    InterruptTimer::initialize(CFG_TIMER_RESOLUTION_US, nullptr);
    InterruptTimer::setCallback(tick);

    //Initalize Default values
    std::fill_n(m_pitchBend, Midi::NUM_CH, Midi::CTRL_CENTER);
}

void ESP32_MultiPhase::reset(uint8_t instrument)
{
    //Not Yet Implemented
}

void ESP32_MultiPhase::resetAll()
{
    stopAll();
}

void ESP32_MultiPhase::playNote(uint8_t instrument, uint8_t note, uint8_t velocity,  uint8_t channel)
{
    // Only increment counter if this instrument wasn't already playing a note
    bool wasActive = (m_activeNotes[instrument] != 0);
    
    m_activeInstruments.set(instrument);
    m_activeNotes[instrument] = (MSB_BITMASK | note);
    m_notePeriod[instrument] = NoteTables::NOTE_TICKS_DOUBLE[note];

    #ifdef PWM_NOTES_DOUBLE
        m_activePeriod[instrument] = NoteTables::applyPitchBendToNoteDouble(note, m_pitchBend[channel]);
    #else
        m_activePeriod[instrument] = NoteTables::applyPitchBendToNote(note, m_pitchBend[channel]);
    #endif

    m_noteStartTime[instrument] = millis(); // Record when note started for timeout tracking

    if(m_lastDistributor[instrument] != nullptr){
        Distributor* distributor = static_cast<Distributor*>(m_lastDistributor[instrument]);
            //m_vibratoDepth[instrument] = distributor->getVibratoEnabled() ? m_modulationWheel[channel] : 0;
            //m_vibratoRate[instrument] = m_vibratoDepth[instrument] >> 3; // Set vibrato rate from modulation wheel
    }

    if (!wasActive) {
        m_numActiveNotes++;
    }
    digitalWrite(pwmPins[instrument], HIGH); // Start pin HIGH to begin waveform
    return;
}

void ESP32_MultiPhase::stopNote(uint8_t instrument, uint8_t note, uint8_t velocity, uint8_t channel)
{
    // Only decrement if there was actually an active note
    bool wasActive = (m_activeNotes[instrument] != 0);
    
    m_activeInstruments.reset(instrument);
    m_lastDistributor[instrument] = nullptr;
    m_lastChannel[instrument] = NONE;
    m_noteStartTime[instrument] = 0;
    m_activeNotes[instrument] = 0;
    m_notePeriod[instrument] = 0;
    m_activePeriod[instrument] = 0;
    m_currentTick[instrument] = 0;  // Reset tick counter to prevent residual state
    m_vibratoPhase[instrument] = 0;  // Reset vibrato phase to prevent carryover
    m_vibratoDepth[instrument] = 0;  // Reset vibrato depth to prevent carryover
    //m_currentState[instrument] = 0;  // Reset state f wavetable

    for(uint8_t output=0; output < CFG_MULTIPHASE_WAVE_TABLE_OUTPUTS+1; output++){
        digitalWrite(pwmPins[instrument + output], LOW);
    }
    
    if (wasActive && m_numActiveNotes > 0) {
        m_numActiveNotes--;
    }
    return;
}

void ESP32_MultiPhase::stopAll(){
    std::fill_n(m_pitchBend, Midi::NUM_CH, Midi::CTRL_CENTER);
    m_numActiveNotes = 0;
    m_lastDistributor.fill(nullptr);
    m_lastChannel.fill(NONE);
    m_noteStartTime.fill(0);
    m_activeNotes = {};
    m_notePeriod = {};
    m_activePeriod = {};
    m_currentTick = {};
    //m_currentState = {};
    m_vibratoPhase = {};
    m_vibratoDepth = {};

    for(uint8_t i = 0; i < numPwmPins; i++){
        digitalWrite(pwmPins[i], LOW);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//Tick
////////////////////////////////////////////////////////////////////////////////////////////////////

/*
Called by the timer interrupt at the specified resolution.  Because this is called extremely often,
it's crucial that any computations here be kept to a minimum!
*/
void ICACHE_RAM_ATTR ESP32_MultiPhase::tick()
{
    // Go through every Instrument
    for (int i = 0; i < CFG_NUM_INSTRUMENTS; i++) {
        // Early exit if no notes are active - check inside loop like original
        if(m_numActiveNotes == 0) return;
        
        //If note active increase tick until period reset and toggle pin
        if (m_activePeriod[i] > 0){
            // Apply vibrato if enabled (modulation wheel > 0)
            uint16_t targetPeriod = m_activePeriod[i];
            
            #ifdef VIBRATO_ENABLED
            // Only apply vibrato if depth is non-zero AND the note period is valid
            if (m_vibratoDepth[i] > 0) {
                // Optimized vibrato: update phase less frequently
                if ((m_currentTick[i] & 0x07) == 0) {  // Bit mask check is faster than modulo
                    m_vibratoPhase[i] = NoteTables::updateVibratoPhase(m_vibratoPhase[i], m_vibratoRate[i]);
                }
                
                // Calculate vibrato offset and apply to period
                int16_t vibratoOffset = NoteTables::calculateVibratoOffset(m_vibratoPhase[i], m_vibratoDepth[i]);
                targetPeriod = NoteTables::applyVibratoToPeriod(m_activePeriod[i], vibratoOffset);
            }
            #endif
            
            if (m_currentTick[i] >= targetPeriod) {
                updatePhase(i);
                m_currentTick[i] = 0;
            } else {
                m_currentTick[i]++;
            }
        }
    }
}


#ifdef ARDUINO_ARCH_ESP32
void ICACHE_RAM_ATTR ESP32_MultiPhase::updatePhase(uint8_t instrument)
#else
void ESP32_MultiPhase::updatePhase(uint8_t instrument)
#endif
{
    //Pulse the control pin
    m_currentState[instrument]+=1; if (m_currentState[instrument]>=CFG_MULTIPHASE_WAVE_TABLE_STEPS) m_currentState[instrument]=0;
    for(uint8_t output=0; output < CFG_MULTIPHASE_WAVE_TABLE_OUTPUTS; output++){
        bool pinState = (wavetable[m_currentState[instrument]][output]);
        digitalWrite(pwmPins[instrument + output + 1], pinState);
    }
}
// #pragma GCC pop_options (Legacy)

////////////////////////////////////////////////////////////////////////////////////////////////////
//Getters and Setters
////////////////////////////////////////////////////////////////////////////////////////////////////

uint8_t ESP32_MultiPhase::getNumActiveNotes(uint8_t instrument)
{
    return (m_activeNotes[instrument] != 0) ? 1 : 0;
}
 
bool ESP32_MultiPhase::isNoteActive(uint8_t instrument, uint8_t note)
{
    //Mask lower 7bits and return true if the instument is playing the respective note.
    return ((m_activeNotes[instrument] & (~ MSB_BITMASK)) == note);
}

void ESP32_MultiPhase::setPitchBend(uint8_t channel, uint16_t bend){
    m_pitchBend[channel] = bend; 
    for (uint8_t i = 0; i < CFG_NUM_INSTRUMENTS; i++){
        if(m_lastChannel[i] == channel){
            if(m_notePeriod[i] == 0) continue;
            // Mask off the MSB flag bit to get the actual note value (0-127)
            uint8_t note = m_activeNotes[i] & (~MSB_BITMASK);
            
            #ifdef PWM_NOTES_DOUBLE
                m_activePeriod[i] = NoteTables::applyPitchBendToNoteDouble(note, bend);
            #else
                m_activePeriod[i] = NoteTables::applyPitchBendToNote(note, bend);
            #endif
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Timeout Tracking Functions
////////////////////////////////////////////////////////////////////////////////////////////////////

void ESP32_MultiPhase::checkInstrumentTimeouts() {
    // Only check timeouts if a timeout is configured
    if (CFG_NOTE_TIMEOUT_MS == 0) return;
    
    uint32_t currentTime = millis();
    for (uint8_t i = 0; i < CFG_NUM_INSTRUMENTS; i++) {
        // Check if instrument is active and has timed out
        if (m_activeNotes[i] != 0 && 
            (currentTime - m_noteStartTime[i]) > CFG_NOTE_TIMEOUT_MS) {
            // Stop the note due to timeout
            stopNote(i, 0, 0, 0);
        }
    }
}

#endif // PLATFORM_ESP32 && CFG_INSTRUMENT_MULTIPHASE && CFG_COMPONENT_MULTIPHASE && CFG_COMPONENT_PWM