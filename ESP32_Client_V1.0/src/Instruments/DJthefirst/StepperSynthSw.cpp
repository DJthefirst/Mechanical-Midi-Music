#include "Config.h"
#ifdef SHIFTREG_TYPE_74HC595

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

void StepperSynthSw::stopNote(uint8_t instrument, uint8_t velocity)
{
   SwPWM::stopNote(instrument, velocity);
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
#ifdef EXTRA_ADDRESSABLE_LEDS

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

#endif // SHIFTREG_TYPE_74HC595