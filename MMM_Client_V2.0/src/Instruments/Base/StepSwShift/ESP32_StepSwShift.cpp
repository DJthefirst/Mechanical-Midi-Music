#include "Config.h"
#if defined(PLATFORM_ESP32) && defined(CFG_INSTRUMENT_STEPSWSHIFT) && defined(CFG_COMPONENT_PWM) && defined(CFG_COMPONENT_SHIFTREGISTER)

#include "Instruments/Base/StepSwShift/ESP32_StepSwShift.h"
#include "Instruments/Components/InterruptTimer.h"
#include "Instruments/Components/ShiftRegister/ShiftRegisterFactory.h"
#include "Arduino.h"
#include <bitset>

// Define constants for PWM configuration
constexpr uint8_t pwmPins[] = {CFG_PINS_INSTRUMENT_PWM};
constexpr uint8_t numPwmPins = sizeof(pwmPins) / sizeof(pwmPins[0]);
static_assert(numPwmPins >= HardwareConfig::MAX_NUM_INSTRUMENTS,
              "CFG_PINS_INSTRUMENT_PWM must provide one step pin per configured instrument");
static_assert(CFG_SHIFTREGISTER_NUM_OUTPUTS >= HardwareConfig::MAX_NUM_INSTRUMENTS,
              "Shift register must provide one direction output per configured instrument");

// Define static member variables
std::array<uint16_t, HardwareConfig::MAX_NUM_INSTRUMENTS> ESP32_StepSw::m_headPosition = {};
std::bitset<HardwareConfig::MAX_NUM_INSTRUMENTS> ESP32_StepSw::m_pinStateDir = 0;
IShiftRegister<CFG_SHIFTREGISTER_NUM_OUTPUTS>* ESP32_StepSw::m_shiftReg = nullptr;
volatile uint32_t ESP32_StepSw::m_pendingDirectionUpdates = 0;

ESP32_StepSw::ESP32_StepSw() : ESP32_SwPWM()
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

void ICACHE_RAM_ATTR ESP32_StepSw::tick()
{
    // Early exit if no notes are active
    if(m_numActiveNotes == 0) return;

    // Go through every Instrument
    for (int i = 0; i < HardwareConfig::MAX_NUM_INSTRUMENTS; i++) {
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



void ICACHE_RAM_ATTR ESP32_StepSw::togglePin(uint8_t instrument)
{
    const uint32_t instrumentMask = uint32_t{1} << instrument;
    if ((m_pendingDirectionUpdates & instrumentMask) != 0) return;

    //Increment/Decrement Head position
    m_pinStateDir[instrument] ? m_headPosition[instrument]-- : m_headPosition[instrument]++;

    //Toggle Direction if the Drive Head is at a limit.
    if ((m_headPosition[instrument] == CFG_LIMITS_POS_MAX) || (m_headPosition[instrument] == CFG_LIMITS_POS_MIN)){
        m_pinStateDir[instrument] = !m_pinStateDir[instrument];
        m_pendingDirectionUpdates |= instrumentMask;
    }

    //Pulse the step pin.
    ESP32_SwPWM::m_currentState.flip(instrument);
    digitalWrite(pwmPins[instrument], m_currentState[instrument]);
}

void ESP32_StepSw::periodic()
{
    ESP32_SwPWM::periodic();

    noInterrupts();
    const uint32_t pending = m_pendingDirectionUpdates;
    interrupts();
    if (pending == 0) return;

    for (uint8_t instrument = 0; instrument < HardwareConfig::MAX_NUM_INSTRUMENTS; ++instrument) {
        if ((pending & (uint32_t{1} << instrument)) != 0) {
            m_shiftReg->setOutputEnabled(instrument, m_pinStateDir[instrument]);
        }
    }
    m_shiftReg->update();

    noInterrupts();
    m_pendingDirectionUpdates &= ~pending;
    interrupts();
}

#endif // PLATFORM_ESP32 && CFG_INSTRUMENT_STEPSWSHIFT && CFG_COMPONENT_PWM && CFG_COMPONENT_SHIFTREGISTER