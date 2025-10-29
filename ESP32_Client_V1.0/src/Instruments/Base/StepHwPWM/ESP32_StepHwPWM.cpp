#include "Config.h"
#ifdef ARDUINO_ARCH_ESP32

#include "Instruments/Base/StepHwPWM/ESP32_StepHwPWM.h"
#include "Instruments/Utility/InterruptTimer.h"
#include "Instruments/Utility/NoteTable.h"
#include "Arduino.h"
#include <bitset>

// Define static member variables
std::array<uint8_t, HardwareConfig::MAX_NUM_INSTRUMENTS> ESP32_StepHwPWM::m_activeNotes = {};
uint8_t ESP32_StepHwPWM::m_numActiveNotes = 0;
std::array<uint16_t, HardwareConfig::MAX_NUM_INSTRUMENTS> ESP32_StepHwPWM::m_notePeriod = {};
std::array<uint16_t, HardwareConfig::MAX_NUM_INSTRUMENTS> ESP32_StepHwPWM::m_activePeriod = {};
std::array<uint16_t, HardwareConfig::MAX_NUM_INSTRUMENTS> ESP32_StepHwPWM::m_currentTick = {};
std::bitset<HardwareConfig::MAX_NUM_INSTRUMENTS> ESP32_StepHwPWM::m_currentState = 0;

ESP32_StepHwPWM::ESP32_StepHwPWM() : ESP32_HwPWM()
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

void ESP32_StepHwPWM::reset(uint8_t instrument)
{
    //Not Yet Implemented
}

void ESP32_StepHwPWM::resetAll()
{
    stopAll();
}

void ESP32_StepHwPWM::playNote(uint8_t instrument, uint8_t note, uint8_t velocity,  uint8_t channel)
{
    // Only increment counter if this instrument wasn't already playing a note
    bool wasActive = (m_activeNotes[instrument] != 0);
    
    m_activeInstruments.set(instrument);
    m_activeNotes[instrument] = (MSB_BITMASK | note);
    m_notePeriod[instrument] = NoteTables::NOTE_TICKS_DOUBLE[note];
    m_activePeriod[instrument] = NoteTables::applyPitchBendToNote(note, m_pitchBend[channel]);
    m_noteStartTime[instrument] = millis(); // Record when note started for timeout tracking
    
    if (!wasActive) {
        m_numActiveNotes++;
    }
    return;
}

void ESP32_StepHwPWM::stopNote(uint8_t instrument, uint8_t velocity)
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
    digitalWrite(HardwareConfig::PINS[instrument], 0);
    
    if (wasActive && m_numActiveNotes > 0) {
        m_numActiveNotes--;
    }
    return;
}

void ESP32_StepHwPWM::stopAll(){
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
*/

void ICACHE_RAM_ATTR ESP32_StepHwPWM::Tick()
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
void ICACHE_RAM_ATTR ESP32_StepHwPWM::togglePin(uint8_t instrument)
#else
void ESP32_StepHwPWM::togglePin(uint8_t instrument)
#endif
{
    //Pulse the control pin
    m_currentState.flip(instrument);
    digitalWrite(HardwareConfig::PINS[instrument], m_currentState[instrument]);
        
}

#endif //ARDUINO_ARCH_ESP32