#pragma once

// Must be at topp of file
#ifndef INSTRUMENT_TYPE
    #define INSTRUMENT_TYPE PwmInstrument
#endif

#include "Constants.h"
#include "Instruments/Base/HwPWM/HwPWM.h"

#include <cstdint>
using std::int8_t;

/* Outputs a PWM signal at the Notes Frequency on each Digital IO Pin using LedC */
class PwmInstrument : public HwPWM{

public: 
    PwmInstrument() : HwPWM(){
        setupLEDs();
    }

    void reset(uint8_t instrument) override {
        HwPWM::reset(instrument);
        resetLEDs();
    };
    void resetAll() override {
        HwPWM::resetAll();
        resetLEDs();
    }
    void playNote(uint8_t instrument, uint8_t note, uint8_t velocity,  uint8_t channel) override{
        HwPWM::playNote(instrument, note, velocity, channel);
        setInstrumentLedOn(instrument, channel, note, velocity);
    }
    void stopNote(uint8_t instrument, uint8_t velocity) override{
        HwPWM::stopNote(instrument, velocity);
        setInstrumentLedOff(instrument);
    }
    void stopAll() override{
        HwPWM::stopAll();
        resetLEDs();
    }

private:
    //LEDs
    void setInstrumentLedOn(uint8_t instrument, uint8_t channel, uint8_t note, uint8_t velocity);
    void setInstrumentLedOff(uint8_t instrument);
    void resetLEDs();
    void setupLEDs();
};