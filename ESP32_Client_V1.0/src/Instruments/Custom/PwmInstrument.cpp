#include "Arduino.h"
#include "Extras/AddrLED.h"
#include "Instruments/Custom/PwmInstrument.h"
#include "Instruments/Utility/NoteTable.h"
#include <cmath>

////////////////////////////////////////////////////////////////////////////////////////////////////
//FAST LED Helper Functions
////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef EXTRA_ADDRESSABLE_LEDS

    void PwmInstrument::setupLEDs(){
        AddrLED::get().setup();
    }

    //Set an Instrument Led to on
    void PwmInstrument::setInstrumentLedOn(uint8_t instrument, uint8_t channel, uint8_t note, uint8_t velocity){
        CHSV color = AddrLED::get().getColor(instrument, channel, note, velocity);
        AddrLED::get().turnLedOn(instrument, color);
    }

    //Set an Instrument Led to off
    void PwmInstrument::setInstrumentLedOff(uint8_t instrument){
        AddrLED::get().turnLedOff(instrument);
    }

    //Reset Leds
    void PwmInstrument::resetLEDs(){
        AddrLED::get().reset();
    }

#else
void PwmInstrument::setupLEDs(){}
void PwmInstrument::setInstrumentLedOn(uint8_t instrument, uint8_t channel, uint8_t note, uint8_t velocity){}
void PwmInstrument::setInstrumentLedOff(uint8_t instrument){}
void PwmInstrument::resetLEDs(){}
#endif