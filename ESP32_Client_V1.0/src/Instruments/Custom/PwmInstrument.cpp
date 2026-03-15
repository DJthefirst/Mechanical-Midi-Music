#include "Arduino.h"
#include "Instruments/Custom/PwmInstrument.h"
#include "Instruments/Components/NoteTable.h"
#include <cmath>

#if (defined(PLATFORM_ESP32) || defined(PLATFORM_TEENSY41)) && defined(CFG_INSTRUMENT_PWMINSTRUMENT) && defined(CFG_COMPONENT_PWM)

////////////////////////////////////////////////////////////////////////////////////////////////////
// Custom LED Implementations
// PwmInstrument uses 1 LED per instrument instead of the default 5
////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef CFG_EXTRA_ADDRESSABLE_LEDS
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

#endif // (PLATFORM_ESP32 || PLATFORM_TEENSY41) && CFG_INSTRUMENT_PWMINSTRUMENT && CFG_COMPONENT_PWM