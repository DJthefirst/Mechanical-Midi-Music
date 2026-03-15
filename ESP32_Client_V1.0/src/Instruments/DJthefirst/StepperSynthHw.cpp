#include "Config.h"
#if (defined(PLATFORM_ESP32) || defined(PLATFORM_TEENSY41)) && defined(CFG_INSTRUMENT_STEPPERSYNTHHW) && defined(CFG_COMPONENT_PWM) && defined(CFG_COMPONENT_SHIFTREGISTER)

#include "Extras/AddrLED.h"
#include "Instruments/DJthefirst/StepperSynthHw.h"
#include "Instruments/Components/ShiftRegister/ShiftRegisterFactory.h"
#include "Arduino.h"

#include "Device.h"

IShiftRegister<CFG_SHIFTREGISTER_NUM_OUTPUTS>* StepperSynthHw::m_shiftReg = nullptr;

StepperSynthHw::StepperSynthHw() : HwPWM()
{
    m_shiftReg = ShiftRegisterFactory::create<CFG_SHIFTREGISTER_NUM_OUTPUTS>(
        CFG_PINS_INSTRUMENT_ShiftRegister);
    m_shiftReg->setInverted(true);
    m_shiftReg->init();

    //Setup FAST LED
    
    

    delay(500); // Wait a half second for safety
}

void StepperSynthHw::periodic() {
    HwPWM::periodic();
    updateLEDs();
}

void StepperSynthHw::playNote(uint8_t instrument, uint8_t note, uint8_t velocity,  uint8_t channel)
{
    m_shiftReg->setOutputEnabled(instrument, true);
    m_shiftReg->update();
    HwPWM::playNote(instrument, note, velocity, channel);
    setInstrumentLedOn(instrument, channel, note, velocity);
    return;
}

void StepperSynthHw::stopNote(uint8_t instrument, uint8_t note, uint8_t velocity, uint8_t channel)
{
    HwPWM::stopNote(instrument, note, velocity, channel);
    m_shiftReg->setOutputEnabled(instrument, false);
    m_shiftReg->update();
    setInstrumentLedOff(instrument);
}

void StepperSynthHw::reset(uint8_t instrument){
    HwPWM::reset(instrument);
    m_shiftReg->setOutputEnabled(instrument, false);
    m_shiftReg->update();
    setInstrumentLedOff(instrument);
}

void StepperSynthHw::resetAll(){
    stopAll();
}

void StepperSynthHw::stopAll(){
    HwPWM::stopAll();
    m_shiftReg->disableAll();
    m_shiftReg->update();
    resetLEDs();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//FAST LED Helper Functions
////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef CFG_EXTRA_ADDRESSABLE_LEDS

//Set an Instrument Led to on
void StepperSynthHw::setInstrumentLedOn(uint8_t instrument, uint8_t channel, uint8_t note, uint8_t velocity){
    CHSV color = AddrLED::get().getColor(instrument, channel, note, velocity);
    AddrLED::get().turnLedsOn(instrument*5, instrument*5+4, color);
}

//Set an Instrument Led to off
void StepperSynthHw::setInstrumentLedOff(uint8_t instrument){
    AddrLED::get().turnLedsOn(instrument*5, instrument*5+4, CHSV(0,0,0));
}

#else
void StepperSynthHw::setInstrumentLedOn(uint8_t instrument, uint8_t channel, uint8_t note, uint8_t velocity) {};
void StepperSynthHw::setInstrumentLedOff(uint8_t instrument) {};
#endif

#endif // (PLATFORM_ESP32 || PLATFORM_TEENSY41) && CFG_INSTRUMENT_STEPPERSYNTHHW && CFG_COMPONENT_PWM && CFG_COMPONENT_SHIFTREGISTER