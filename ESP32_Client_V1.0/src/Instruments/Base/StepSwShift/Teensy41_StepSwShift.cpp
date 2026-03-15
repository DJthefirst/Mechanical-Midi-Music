#include "Config.h"
#if defined(PLATFORM_TEENSY41) && defined(CFG_INSTRUMENT_STEPSWSHIFT) && defined(CFG_COMPONENT_PWM) && defined(CFG_COMPONENT_SHIFTREGISTER)

#include "Instruments/Base/StepSwShift/Teensy41_StepSwShift.h"
#include "Instruments/Components/InterruptTimer.h"
#include "Instruments/Components/ShiftRegister/ShiftRegisterFactory.h"
#include "Arduino.h"
#include <bitset>

// Define constants for PWM configuration
constexpr uint8_t pwmPins[] = {CFG_PINS_INSTRUMENT_PWM};
constexpr uint8_t numPwmPins = sizeof(pwmPins) / sizeof(pwmPins[0]);

// Define static member variables
std::array<uint16_t, HardwareConfig::MAX_NUM_INSTRUMENTS> Teensy41_StepSwShift::m_headPosition = {};
std::bitset<HardwareConfig::MAX_NUM_INSTRUMENTS> Teensy41_StepSwShift::m_pinStateDir = 0;
IShiftRegister<CFG_SHIFTREGISTER_NUM_OUTPUTS>* Teensy41_StepSwShift::m_shiftReg = nullptr;

Teensy41_StepSwShift::Teensy41_StepSwShift() : Teensy41_SwPWM()
{
    //Setup pins
    m_shiftReg = ShiftRegisterFactory::create<CFG_SHIFTREGISTER_NUM_OUTPUTS>(
        CFG_PINS_INSTRUMENT_ShiftRegister);
    m_shiftReg->init();

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

void Teensy41_StepSwShift::tick()
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

void Teensy41_StepSwShift::togglePin(uint8_t instrument)
{
    //Increment/Decrement Head position
    m_pinStateDir[instrument] ? m_headPosition[instrument]-- : m_headPosition[instrument]++;

    //Toggle Direction if the Drive Head is at a limit.
    if ((m_headPosition[instrument] == CFG_LIMITS_POS_MAX) || (m_headPosition[instrument] == CFG_LIMITS_POS_MIN)){
        m_pinStateDir[instrument] = !m_pinStateDir[instrument];
        m_shiftReg->setOutputEnabled(instrument, m_pinStateDir[instrument]);
        m_shiftReg->update();
    }

    //Pulse the step pin.
    Teensy41_SwPWM::m_currentState.flip(instrument);
    digitalWrite(pwmPins[instrument], m_currentState[instrument]);
}

#endif // PLATFORM_TEENSY41 && CFG_INSTRUMENT_STEPSWSHIFT && CFG_COMPONENT_PWM && CFG_COMPONENT_SHIFTREGISTER