#include "Config.h"
#if (defined(PLATFORM_ESP32) || defined(PLATFORM_TEENSY41)) && defined(CFG_INSTRUMENT_STEPPERSYNTHSW) && defined(CFG_COMPONENT_PWM) && defined(CFG_COMPONENT_SHIFTREGISTER)

#include "Extras/AddrLED.h"
#include "Instruments/DJthefirst/StepperSynthSw.h"
#include "Instruments/Components/ShiftRegister/ShiftRegister.h"
#include "Arduino.h"

#include "Device.h"

StepperSynthSw::StepperSynthSw() : SwPWM()
{

    ShiftRegister::init();

    //Setup FAST LED
    setupLEDs();

    delay(500); // Wait a half second for safety
}

void StepperSynthSw::periodic() {
    SwPWM::periodic();
    updateLEDs();
}

void StepperSynthSw::playNote(uint8_t instrument, uint8_t note, uint8_t velocity,  uint8_t channel)
{
    ShiftRegister::setOutputEnabled(instrument, true);
    ShiftRegister::update();
    SwPWM::playNote(instrument, note, velocity, channel);
    setInstrumentLedOn(instrument, channel, note, velocity);
    return;
}

void StepperSynthSw::stopNote(uint8_t instrument, uint8_t note, uint8_t velocity, uint8_t channel)
{
   SwPWM::stopNote(instrument, note, velocity, channel);
    ShiftRegister::setOutputEnabled(instrument, false);
    ShiftRegister::update();
    setInstrumentLedOff(instrument);
}

void StepperSynthSw::reset(uint8_t instrument){
    SwPWM::reset(instrument);
    ShiftRegister::setOutputEnabled(instrument, false);
    ShiftRegister::update();
    setInstrumentLedOff(instrument);
}

void StepperSynthSw::resetAll(){
    stopAll();
}

void StepperSynthSw::stopAll(){
    SwPWM::stopAll();
    ShiftRegister::disableAll();
    ShiftRegister::update();
    resetLEDs();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//FAST LED Helper Functions
////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef CFG_EXTRA_ADDRESSABLE_LEDS

//Set an Instrument Led to on
void StepperSynthSw::setInstrumentLedOn(uint8_t instrument, uint8_t channel, uint8_t note, uint8_t velocity){
    CHSV color = AddrLED::get().getColor(instrument, channel, note, velocity);
    AddrLED::get().turnLedsOn(instrument*5, instrument*5+4, color);
}

//Set an Instrument Led to off
void StepperSynthSw::setInstrumentLedOff(uint8_t instrument){
    AddrLED::get().turnLedsOn(instrument*5, instrument*5+4, CHSV(0,0,0));
}


#else
void StepperSynthSw::setInstrumentLedOn(uint8_t instrument, uint8_t channel, uint8_t note, uint8_t velocity){}
void StepperSynthSw::setInstrumentLedOff(uint8_t instrument){}
#endif

#endif // (PLATFORM_ESP32 || PLATFORM_TEENSY41) && CFG_INSTRUMENT_STEPPERSYNTHSW && CFG_COMPONENT_PWM && CFG_COMPONENT_SHIFTREGISTER