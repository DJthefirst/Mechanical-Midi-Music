#include "Config.h"

#include "Extras/AddrLED.h"
#include "Instruments/DJthefirst/PneumaticValvesSw.h"
#include "Arduino.h"

#include "Device.h"

PneumaticValvesSw::PneumaticValvesSw() : SwPWM()
{
    //Setup FAST LED
    setupLEDs();

    delay(500); // Wait a half second for safety
}

void PneumaticValvesSw::periodic() {
    SwPWM::periodic();
    updateLEDs();
}

void PneumaticValvesSw::playNote(uint8_t instrument, uint8_t note, uint8_t velocity,  uint8_t channel)
{
    SwPWM::playNote(instrument, note, velocity, channel);
    setInstrumentLedOn(instrument, channel, note, velocity);
    return;
}

void PneumaticValvesSw::stopNote(uint8_t instrument, uint8_t velocity)
{
   SwPWM::stopNote(instrument, velocity);
    setInstrumentLedOff(instrument);
}

void PneumaticValvesSw::reset(uint8_t instrument){
    SwPWM::reset(instrument);
    setInstrumentLedOff(instrument);
}

void PneumaticValvesSw::resetAll(){
    stopAll();
}

void PneumaticValvesSw::stopAll(){
    SwPWM::stopAll();
    resetLEDs();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//FAST LED Helper Functions
////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef EXTRA_ADDRESSABLE_LEDS

//Set an Instrument Led to on
void PneumaticValvesSw::setInstrumentLedOn(uint8_t instrument, uint8_t channel, uint8_t note, uint8_t velocity){
    CHSV color = AddrLED::get().getColor(instrument, channel, note, velocity);
    AddrLED::get().turnLedOn(instrument, color);
}

//Set an Instrument Led to off
void PneumaticValvesSw::setInstrumentLedOff(uint8_t instrument){
    AddrLED::get().turnLedOff(instrument);
}


#else
void PneumaticValvesSw::setInstrumentLedOn(uint8_t instrument, uint8_t channel, uint8_t note, uint8_t velocity){}
void PneumaticValvesSw::setInstrumentLedOff(uint8_t instrument){}
#endif
