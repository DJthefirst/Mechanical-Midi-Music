#include "Config.h"

#if (defined(PLATFORM_ESP32) || defined(PLATFORM_TEENSY41)) && defined(CFG_INSTRUMENT_FLOPPYSYNTH2) && defined(CFG_COMPONENT_PWM)

#include "Extras/AddrLED.h"
#include "Instruments/DJthefirst/FloppySynth2.h"
#include "Arduino.h"

#include "Device.h"

FloppySynth2::FloppySynth2() : StepSw()
{
    //Setup FAST LED
    setupLEDs();

    delay(500); // Wait a half second for safety
}

void FloppySynth2::periodic() {
    StepSw::periodic();
    updateLEDs();
}

void FloppySynth2::playNote(uint8_t instrument, uint8_t note, uint8_t velocity,  uint8_t channel)
{
    StepSw::playNote(instrument, note, velocity, channel);
    setInstrumentLedOn(instrument, channel, note, velocity);
    return;
}

void FloppySynth2::stopNote(uint8_t instrument, uint8_t note, uint8_t velocity, uint8_t channel)
{
   StepSw::stopNote(instrument, note, velocity, channel);
    setInstrumentLedOff(instrument);
}

void FloppySynth2::reset(uint8_t instrument){
    StepSw::reset(instrument);
    setInstrumentLedOff(instrument);
}

void FloppySynth2::resetAll(){
    stopAll();
}

void FloppySynth2::stopAll(){
    StepSw::stopAll();
    resetLEDs();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//FAST LED Helper Functions
////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef CFG_EXTRA_ADDRESSABLE_LEDS

//Set an Instrument Led to on
void FloppySynth2::setInstrumentLedOn(uint8_t instrument, uint8_t channel, uint8_t note, uint8_t velocity){
    CHSV color = AddrLED::get().getColor(instrument, channel, note, velocity);
    AddrLED::get().turnLedsOn(instrument*15, instrument*15+15, color);
}

//Set an Instrument Led to off
void FloppySynth2::setInstrumentLedOff(uint8_t instrument){
    AddrLED::get().turnLedsOn(instrument*15, instrument*15+15, CHSV(0, 0, 0));
}



#else
void FloppySynth2::setInstrumentLedOn(uint8_t instrument, uint8_t channel, uint8_t note, uint8_t velocity){}
void FloppySynth2::setInstrumentLedOff(uint8_t instrument){}
#endif

#endif // (PLATFORM_ESP32 || PLATFORM_TEENSY41) && CFG_INSTRUMENT_FLOPPYSYNTH2 && CFG_COMPONENT_PWM
