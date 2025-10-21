#include "Instruments/Base/SwPWM/ESP32_SwPWM.h"
#include "Instruments/Utility/InterruptTimer.h"
#include "Instruments/Utility/NoteTable.h"
#include "Config.h"
#include "Arduino.h"
#include <bitset>

//[Instrument][ActiveNote] MSB is set if note is Active the 7 LSBs are the Notes Value 
static std::array<uint8_t,HardwareConfig::MAX_NUM_INSTRUMENTS> m_activeNotes;
static uint8_t m_numActiveNotes;

//Instrument Attributes
static std::array<uint16_t,HardwareConfig::MAX_NUM_INSTRUMENTS> m_notePeriod;  //Base Note
static std::array<uint16_t,HardwareConfig::MAX_NUM_INSTRUMENTS> m_activePeriod;//Note Played
static std::array<uint16_t,HardwareConfig::MAX_NUM_INSTRUMENTS> m_currentTick; //Timeing
static std::bitset<HardwareConfig::MAX_NUM_INSTRUMENTS> m_currentState; //IO

ESP32_SwPWM::ESP32_SwPWM() : InstrumentControllerBase()
{
    //Setup pins
    for(uint8_t i=0; i < HardwareConfig::PINS.size(); i++){
        pinMode(HardwareConfig::PINS[i], OUTPUT);
    }

    delay(500); // Wait a half second for safety

    // Setup timer to handle interrupts for driving the instrument
    InterruptTimer::initialize(HardwareConfig::TIMER_RESOLUTION, Tick);

    //Initalize Default values
    std::fill_n(m_pitchBend, Midi::NUM_CH, Midi::CTRL_CENTER);
}

void ESP32_SwPWM::reset(uint8_t instrument)
{
    //Not Yet Implemented
}

void ESP32_SwPWM::resetAll()
{
    stopAll();
}

void ESP32_SwPWM::playNote(uint8_t instrument, uint8_t note, uint8_t velocity,  uint8_t channel)
{
    // Only increment counter if this instrument wasn't already playing a note
    bool wasActive = (m_activeNotes[instrument] != 0);
    
    _activeInstruments.set(instrument);
    m_activeNotes[instrument] = (MSB_BITMASK | note);
    m_notePeriod[instrument] = NoteTables::NOTE_TICKS_DOUBLE[note];
    m_activePeriod[instrument] = NoteTables::applyPitchBendToNote(note, m_pitchBend[channel]);
    _noteStartTime[instrument] = millis(); // Record when note started for timeout tracking
    
    if (!wasActive) {
        m_numActiveNotes++;
    }
    return;
}

void ESP32_SwPWM::stopNote(uint8_t instrument, uint8_t velocity)
{
    // Only decrement if there was actually an active note
    bool wasActive = (m_activeNotes[instrument] != 0);
    
    _activeInstruments.reset(instrument);
    _lastDistributor[instrument] = nullptr;
    _lastChannel[instrument] = NONE;
    _noteStartTime[instrument] = 0;
    m_activeNotes[instrument] = 0;
    m_notePeriod[instrument] = 0;
    m_activePeriod[instrument] = 0;
    digitalWrite(HardwareConfig::PINS[instrument], 0);
    
    if (wasActive && m_numActiveNotes > 0) {
        m_numActiveNotes--;
    }
    return;
}

void ESP32_SwPWM::stopAll(){
    std::fill_n(m_pitchBend, Midi::NUM_CH, Midi::CTRL_CENTER);
    m_numActiveNotes = 0;
    _lastDistributor.fill(nullptr);
    _lastChannel.fill(NONE);
    _noteStartTime.fill(0);
    m_activeNotes = {};
    m_notePeriod = {};
    m_activePeriod = {};
    m_currentTick = {};
    m_currentState.reset();

    for(uint8_t i = 0; i < HardwareConfig::PINS.size(); i++){
        digitalWrite(HardwareConfig::PINS[i], LOW);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//Tick
////////////////////////////////////////////////////////////////////////////////////////////////////

/*
Called by the timer interrupt at the specified resolution.  Because this is called extremely often,
it's crucial that any computations here be kept to a minimum!

Additionally, the ICACHE_RAM_ATTR helps avoid crashes with WiFi libraries, but may increase speed generally anyway
 */
// #pragma GCC push_options (Legacy)
// #pragma GCC optimize("Ofast") // Required to unroll this loop, but useful to try to keep this speedy (Legacy)
#ifdef ARDUINO_ARCH_ESP32
void ICACHE_RAM_ATTR ESP32_SwPWM::Tick()
#else
void ESP32_SwPWM::tick()
#endif
{
    // Go through every Instrument
    for (int i = 0; i < HardwareConfig::MAX_NUM_INSTRUMENTS; i++) {
        if(m_numActiveNotes == 0)return;

        //If note active increase tick until period reset and toggle pin
        if (m_activePeriod[i] > 0){
            if (m_currentTick[i] >= m_activePeriod[i]) {
                togglePin(i);

                m_currentTick[i] = 0;
                continue;
            }
            m_currentTick[i]++;
        }
    }
}


#ifdef ARDUINO_ARCH_ESP32
void ICACHE_RAM_ATTR ESP32_SwPWM::togglePin(uint8_t instrument)
#else
void ESP32_SwPWM::togglePin(uint8_t instrument)
#endif
{
    //Pulse the control pin
    m_currentState.flip(instrument);
    digitalWrite(HardwareConfig::PINS[instrument], m_currentState[instrument]);
        
}
// #pragma GCC pop_options (Legacy)

////////////////////////////////////////////////////////////////////////////////////////////////////
//Getters and Setters
////////////////////////////////////////////////////////////////////////////////////////////////////

uint8_t ESP32_SwPWM::getNumActiveNotes(uint8_t instrument)
{
    return (m_activeNotes[instrument] != 0) ? 1 : 0;
}
 
bool ESP32_SwPWM::isNoteActive(uint8_t instrument, uint8_t note)
{
    //Mask lower 7bits and return true if the instument is playing the respective note.
    return ((m_activeNotes[instrument] & (~ MSB_BITMASK)) == note);
}

void ESP32_SwPWM::setPitchBend(uint8_t instrument, uint16_t bend, uint8_t channel){
    m_pitchBend[channel] = bend; 
    if(m_notePeriod[instrument] == 0) return;
    m_activePeriod[instrument] = NoteTables::applyPitchBendToNote(m_activeNotes[instrument], bend);
}