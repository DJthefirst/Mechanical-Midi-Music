#pragma once

// Must be at topp of file
#ifndef INSTRUMENT_TYPE
    #define INSTRUMENT_TYPE StepperSynthSw
#endif

#include "Constants.h"
#include "Instruments/Base/SwPWM/SwPWM.h"

#include <cstdint>
using std::int8_t;

/* Outputs a PWM signal at the Notes Frequency on each Digital IO Pin */
class StepperSynthSw : public SwPWM{
public:

private:
    static std::array<bool,HardwareConfig::NUM_INSTRUMENTS> m_outputenabled; //Output Enabled
    static void togglePin(uint8_t instrument);

public: 
    StepperSynthSw();
    void periodic() override;
    void reset(uint8_t instrument) override;
    void resetAll() override;
    void playNote(uint8_t instrument, uint8_t note, uint8_t velocity,  uint8_t channel) override;
    void stopNote(uint8_t instrument, uint8_t velocity) override;
    void stopAll() override;

private:
    //LEDs
    void setInstrumentLedOn(uint8_t instrument, uint8_t channel, uint8_t note, uint8_t velocity) override;
    void setInstrumentLedOff(uint8_t instrument) override;
    void updateShiftRegister();
};