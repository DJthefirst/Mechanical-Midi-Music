#include "Config.h"
#if defined(PLATFORM_TEENSY41) && defined(COMPONENT_PWM)

#include "Instruments/Base/SwPWM/Teensy41_SwPWM.h"
#include "Instruments/Components/NoteTable.h"
#include "Arduino.h"
#include "Distributors/Distributor.h"
#include <bitset>
#include "Instruments/Components/InterruptTimer.h"

// Define static member variables
std::array<uint8_t, HardwareConfig::MAX_NUM_INSTRUMENTS> Teensy41_SwPWM::m_activeNotes = {};
uint8_t Teensy41_SwPWM::m_numActiveNotes = 0;
std::array<uint16_t, HardwareConfig::MAX_NUM_INSTRUMENTS> Teensy41_SwPWM::m_notePeriod = {};
std::array<uint16_t, HardwareConfig::MAX_NUM_INSTRUMENTS> Teensy41_SwPWM::m_activePeriod =  {};
std::array<uint16_t, HardwareConfig::MAX_NUM_INSTRUMENTS> Teensy41_SwPWM::m_currentTick = {};
std::bitset<HardwareConfig::MAX_NUM_INSTRUMENTS> Teensy41_SwPWM::m_currentState = 0;
std::array<uint16_t, HardwareConfig::MAX_NUM_INSTRUMENTS> Teensy41_SwPWM::m_vibratoPhase = {};
std::array<uint8_t,HardwareConfig::MAX_NUM_INSTRUMENTS> Teensy41_SwPWM::m_vibratoDepth = {};
std::array<uint8_t,HardwareConfig::MAX_NUM_INSTRUMENTS> Teensy41_SwPWM::m_vibratoRate = {}; //slow default rate

Teensy41_SwPWM::Teensy41_SwPWM() : InstrumentControllerBase()
{
    //Setup pins
    for(uint8_t i=0; i < HardwareConfig::PINS_INSTRUMENT_PWM.size(); i++){
        pinMode(HardwareConfig::PINS_INSTRUMENT_PWM[i], OUTPUT);
    }

    delay(500); // Wait a half second for safety

    // Setup timer hardware and register the base tick callback. If a
    // specialized subclass (like Teensy41_StepSwPWM) wants to take
    // ownership it can call InterruptTimer::setCallback() to replace this
    // handler.
    InterruptTimer::initialize(HardwareConfig::TIMER_RESOLUTION, nullptr);
    InterruptTimer::setCallback(Tick);


    //Initialize Default values
    std::fill_n(m_pitchBend, Midi::NUM_CH, Midi::CTRL_CENTER);
}

void Teensy41_SwPWM::reset(uint8_t instrument)
{
    //Not Yet Implemented
}

void Teensy41_SwPWM::resetAll()
{
    stopAll();
}

void Teensy41_SwPWM::playNote(uint8_t instrument, uint8_t note, uint8_t velocity,  uint8_t channel)
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
        const Distributor* distributor = static_cast<const Distributor*>(m_lastDistributor[instrument]);
            m_vibratoDepth[instrument] = distributor->getVibratoEnabled() ? m_modulationWheel[channel] : 0;
            m_vibratoRate[instrument] = m_vibratoDepth[instrument] >> 3; // Set vibrato rate from modulation wheel
    }

    if (!wasActive) {
        m_numActiveNotes++;
    }
    return;
}

void Teensy41_SwPWM::stopNote(uint8_t instrument, uint8_t velocity)
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
    m_currentState.reset(instrument);  // Reset pin state bit
    digitalWriteFast(HardwareConfig::PINS_INSTRUMENT_PWM[instrument], 0);
    
    if (wasActive && m_numActiveNotes > 0) {
        m_numActiveNotes--;
    }
    return;
}

void Teensy41_SwPWM::stopAll(){
    std::fill_n(m_pitchBend, Midi::NUM_CH, Midi::CTRL_CENTER);
    m_numActiveNotes = 0;
    m_lastDistributor.fill(nullptr);
    m_lastChannel.fill(NONE);
    m_noteStartTime.fill(0);
    m_activeNotes = {};
    m_notePeriod = {};
    m_activePeriod = {};
    m_currentTick = {};
    m_currentState.reset();
    m_vibratoPhase = {};
    m_vibratoDepth = {};

    for(uint8_t i = 0; i < HardwareConfig::PINS_INSTRUMENT_PWM.size(); i++){
        digitalWriteFast(HardwareConfig::PINS_INSTRUMENT_PWM[i], LOW);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//Tick
////////////////////////////////////////////////////////////////////////////////////////////////////

/*
Called by the timer interrupt at the specified resolution.  Because this is called extremely often,
it's crucial that any computations here be kept to a minimum!
*/
void Teensy41_SwPWM::Tick()
{
    // Go through every Instrument
    for (int i = 0; i < HardwareConfig::MAX_NUM_INSTRUMENTS; i++) {
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
                togglePin(i);
                m_currentTick[i] = 0;
            } else {
                m_currentTick[i]++;
            }
        }
    }
}

void Teensy41_SwPWM::togglePin(uint8_t instrument)
{
    //Pulse the control pin
    m_currentState.flip(instrument);
    digitalWriteFast(HardwareConfig::PINS_INSTRUMENT_PWM[instrument], m_currentState[instrument]);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//Getters and Setters
////////////////////////////////////////////////////////////////////////////////////////////////////

uint8_t Teensy41_SwPWM::getNumActiveNotes(uint8_t instrument)
{
    return (m_activeNotes[instrument] != 0) ? 1 : 0;
}
 
bool Teensy41_SwPWM::isNoteActive(uint8_t instrument, uint8_t note)
{
    //Mask lower 7bits and return true if the instrument is playing the respective note.
    return ((m_activeNotes[instrument] & (~ MSB_BITMASK)) == note);
}

void Teensy41_SwPWM::setPitchBend(uint8_t channel, uint16_t bend){
    m_pitchBend[channel] = bend; 
    for (uint8_t i = 0; i < HardwareConfig::MAX_NUM_INSTRUMENTS; i++){
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

void Teensy41_SwPWM::setModulationWheel(uint8_t channel, uint8_t value)
{
    #ifdef VIBRATO_ENABLED
        // Map modulation wheel (0-127) to vibrato depth (0-127)
        // The depth will determine how much pitch variation occurs
        
        // Check each instrument's last distributor to see if vibrato is enabled
        m_modulationWheel[channel] = value;
        for (uint8_t i = 0; i < HardwareConfig::MAX_NUM_INSTRUMENTS; i++){
            if(m_lastChannel[i] == channel){
                if(m_notePeriod[i] == 0) continue;
                if(m_lastDistributor[i] == nullptr) continue;
                Distributor* distributor = static_cast<Distributor*>(m_lastDistributor[i]);

                m_vibratoDepth[i] = distributor->getVibratoEnabled() ? value : 0;
                m_vibratoRate[i] = value >> 3; // Higher modulation wheel = faster vibrato
            }
        }
    #endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Timeout Tracking Functions
////////////////////////////////////////////////////////////////////////////////////////////////////

void Teensy41_SwPWM::checkInstrumentTimeouts() {
    // Only check timeouts if a timeout is configured
    if (HardwareConfig::INSTRUMENT_TIMEOUT_MS == 0) return;
    
    uint32_t currentTime = millis();
    for (uint8_t i = 0; i < HardwareConfig::MAX_NUM_INSTRUMENTS; i++) {
        // Check if instrument is active and has timed out
        if (m_activeNotes[i] != 0 && 
            (currentTime - m_noteStartTime[i]) > HardwareConfig::INSTRUMENT_TIMEOUT_MS) {
            // Stop the note due to timeout
            stopNote(i, 0);
        }
    }
}

#endif //PLATFORM_TEENSY41