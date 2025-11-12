#pragma once

// Must be at topp of file
#ifndef INSTRUMENT_TYPE
    #define INSTRUMENT_TYPE StepperSynthHw
#endif

#include "Constants.h"
#include "Instruments/Base/HwPWM/HwPWM.h"
#include "Instruments/Components/ShiftRegister/ShiftRegister.h"

#include <cstdint>
using std::int8_t;

/* Outputs a PWM signal at the Notes Frequency on each Digital IO Pin */
class StepperSynthHw : public HwPWM{
public:

private:
    static void togglePin(uint8_t instrument);

public: 
    StepperSynthHw();
    void periodic() override;
    void reset(uint8_t instrument) override;
    void resetAll() override;
    void playNote(uint8_t instrument, uint8_t note, uint8_t velocity,  uint8_t channel) override;
    void stopNote(uint8_t instrument, uint8_t velocity) override;
    void stopAll() override;

private:
    void setInstrumentLedOn(uint8_t instrument, uint8_t channel, uint8_t note, uint8_t velocity) override;
    void setInstrumentLedOff(uint8_t instrument) override;
};