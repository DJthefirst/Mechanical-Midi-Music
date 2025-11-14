#include "Config.h"
#if defined(PLATFORM_TEENSY41) && defined(COMPONENT_STEP) 

#include "Instruments/Base/StepSw/Teensy41_StepSw.h"
#include "Instruments/Components/InterruptTimer.h"
#include "Instruments/Components/NoteTable.h"
#include "Arduino.h"
#include <bitset>

// Define static member variables
std::array<uint16_t, HardwareConfig::MAX_NUM_INSTRUMENTS> Teensy41_StepSw::m_headPosition = {};
std::bitset<HardwareConfig::MAX_NUM_INSTRUMENTS> Teensy41_StepSw::m_pinStateDir = 0;

Teensy41_StepSw::Teensy41_StepSw() : Teensy41_SwPWM()
{
    //Setup pins
    for(uint8_t i=0; i < HardwareConfig::PINS_INSTRUMENT_DIR.size(); i++){
        pinMode(HardwareConfig::PINS_INSTRUMENT_DIR[i], OUTPUT);
        digitalWrite(HardwareConfig::PINS_INSTRUMENT_DIR[i], LOW);
    }

    delay(500); // Wait a half second for safety

    // Timer hardware is initialized by the base SwPWM. Replace the
    // currently-registered callback so StepSw becomes the periodic
    // handler.
    InterruptTimer::setCallback(tick);
}



////////////////////////////////////////////////////////////////////////////////////////////////////
//Tick
////////////////////////////////////////////////////////////////////////////////////////////////////

/*
Called by the timer interrupt at the specified resolution.  Because this is called extremely often,
it's crucial that any computations here be kept to a minimum!
*/

void Teensy41_StepSw::tick()
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



void Teensy41_StepSw::togglePin(uint8_t instrument)
{
    //Increment/Decrement Head position
    m_pinStateDir[instrument] ? m_headPosition[instrument]-- : m_headPosition[instrument]++;

    //Toggle Direction if the Drive Head is at a limit.
    if ((m_headPosition[instrument] == STEP_MAX_HEAD_POS) || (m_headPosition[instrument] == STEP_MIN_HEAD_POS)){
        m_pinStateDir[instrument] = !m_pinStateDir[instrument];
        digitalWrite(HardwareConfig::PINS_INSTRUMENT_DIR[instrument], m_pinStateDir[instrument]);
    }

    //Pulse the step pin.
    Teensy41_SwPWM::m_currentState.flip(instrument);
    digitalWrite(HardwareConfig::PINS_INSTRUMENT_PWM[instrument], m_currentState[instrument]);
}

#endif //ARDUINO_ARCH_Teensy41