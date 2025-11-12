#include "Config.h"
#ifdef SHIFTREG_TYPE_74HC595

#include "Extras/AddrLED.h"
#include "Instruments/DJthefirst/StepperSynthHw.h"
#include "Instruments/Components/ShiftRegister/ShiftRegister.h"
#include "Arduino.h"

#include "Device.h"

StepperSynthHw::StepperSynthHw() : HwPWM()
{

    ShiftRegister::init();

    //Setup FAST LED
    setupLEDs();

    delay(500); // Wait a half second for safety
}

void StepperSynthHw::periodic() {
    HwPWM::periodic();
    updateLEDs();
}

void StepperSynthHw::playNote(uint8_t instrument, uint8_t note, uint8_t velocity,  uint8_t channel)
{
    ShiftRegister::setOutputEnabled(instrument, true);
    ShiftRegister::update();
    HwPWM::playNote(instrument, note, velocity, channel);
    setInstrumentLedOn(instrument, channel, note, velocity);
    return;
}

void StepperSynthHw::stopNote(uint8_t instrument, uint8_t velocity)
{
    HwPWM::stopNote(instrument, velocity);
    ShiftRegister::setOutputEnabled(instrument, false);
    ShiftRegister::update();
    setInstrumentLedOff(instrument);
}

void StepperSynthHw::reset(uint8_t instrument){
    HwPWM::reset(instrument);
    ShiftRegister::setOutputEnabled(instrument, false);
    ShiftRegister::update();
    setInstrumentLedOff(instrument);
}

void StepperSynthHw::resetAll(){
    stopAll();
}

void StepperSynthHw::stopAll(){
    HwPWM::stopAll();
    ShiftRegister::disableAll();
    ShiftRegister::update();
    resetLEDs();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//FAST LED Helper Functions
////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef EXTRA_ADDRESSABLE_LEDS

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

#endif // SHIFTREG_TYPE_74HC595