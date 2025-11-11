#include "Arduino.h"
#include "Instruments/Custom/PwmInstrument.h"
#include "Instruments/Utility/NoteTable.h"
#include <cmath>

////////////////////////////////////////////////////////////////////////////////////////////////////
// Custom LED Implementations
// PwmInstrument uses 1 LED per instrument instead of the default 5
////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef EXTRA_ADDRESSABLE_LEDS
#include "Extras/AddrLED.h"

//Set an Instrument Led to on - CUSTOM: uses single LED per instrument
void PwmInstrument::setInstrumentLedOn(uint8_t instrument, uint8_t channel, uint8_t note, uint8_t velocity){
    CHSV color = AddrLED::get().getColor(instrument, channel, note, velocity);
    AddrLED::get().turnLedOn(instrument, color);
}

//Set an Instrument Led to off - CUSTOM: uses single LED per instrument
void PwmInstrument::setInstrumentLedOff(uint8_t instrument){
    AddrLED::get().turnLedOff(instrument);
}

#else
// Empty implementations when LEDs are disabled
void PwmInstrument::setInstrumentLedOn(uint8_t, uint8_t, uint8_t, uint8_t) {}
void PwmInstrument::setInstrumentLedOff(uint8_t) {}
#endif