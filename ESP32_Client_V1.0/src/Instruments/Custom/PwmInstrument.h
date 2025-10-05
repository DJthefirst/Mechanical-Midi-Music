#pragma once

#include "Constants.h"
#include "Instruments/Utility/PWM/PwmBase.h"

#include <cstdint>
using std::int8_t;

/* Outputs a PWM signal at the Notes Frequency on each Digital IO Pin using LedC */
class PwmInstrument : public PwmBase{

public: 
    PwmInstrument() : PwmBase(){
        setupLEDs();
    }

    void reset(uint8_t instrument) override {
        PwmBase::reset(instrument);
        resetLEDs();
    };
    void resetAll() override {
        PwmBase::resetAll();
        resetLEDs();
    }
    void playNote(uint8_t instrument, uint8_t note, uint8_t velocity,  uint8_t channel) override{
        PwmBase::playNote(instrument, note, velocity, channel);
        setInstrumentLedOn(instrument, channel, note, velocity);
    }
    void stopNote(uint8_t instrument, uint8_t note, uint8_t velocity) override{
        PwmBase::stopNote(instrument, note, velocity);
        setInstrumentLedOff(instrument);
    }
    void stopAll() override{
        PwmBase::stopAll();
        resetLEDs();
    }

private:
    //LEDs
    void setInstrumentLedOn(uint8_t instrument, uint8_t channel, uint8_t note, uint8_t velocity);
    void setInstrumentLedOff(uint8_t instrument);
    void resetLEDs();
    void setupLEDs();
};