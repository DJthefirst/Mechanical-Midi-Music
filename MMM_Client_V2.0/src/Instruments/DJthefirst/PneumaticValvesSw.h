#pragma once

// Must be at top of file
#ifndef INSTRUMENT_TYPE
    #define INSTRUMENT_TYPE PneumaticValvesSw
#endif

#include "Constants.h"
#include "Instruments/Base/SwPWM/SwPWM.h"

#include <cstdint>
using std::int8_t;

/* Outputs a PWM signal at the Notes Frequency on each Digital IO Pin */
class PneumaticValvesSw : public SwPWM{
public:

private:
    static void togglePin(uint8_t instrument);

public: 
    PneumaticValvesSw();
    void periodic() override;
    void reset(uint8_t instrument) override;
    void resetAll() override;
    void playNote(uint8_t instrument, uint8_t note, uint8_t velocity, uint8_t channel) override;
    void stopNote(uint8_t instrument, uint8_t note, uint8_t velocity, uint8_t channel) override;
    void stopAll() override;

private:
    void setInstrumentLedOn(uint8_t instrument, uint8_t channel, uint8_t note, uint8_t velocity) override;
    void setInstrumentLedOff(uint8_t instrument) override;
};