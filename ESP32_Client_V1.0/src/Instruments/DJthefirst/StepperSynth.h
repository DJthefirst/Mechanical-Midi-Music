#pragma once

#include "Constants.h"
#include "Instruments/Base/SwPWM/SwPWM.h"

#include <cstdint>
using std::int8_t;

/* Outputs a PWM signal at the Notes Frequency on each Digital IO Pin */
class StepperSynth : public SwPWM{
public:

private:
    static void togglePin(uint8_t instrument);

public: 
    StepperSynth();
    void reset(uint8_t instrument) override;
    void resetAll() override;
    void playNote(uint8_t instrument, uint8_t note, uint8_t velocity,  uint8_t channel) override;
    void stopNote(uint8_t instrument, uint8_t note, uint8_t velocity) override;
    void stopAll() override;

private:
    //LEDs
    void setInstrumentLedOn(uint8_t instrument, uint8_t channel, uint8_t note, uint8_t velocity);
    void setInstrumentLedOff(uint8_t instrument);
    void resetLEDs();
    void setupLEDs();
    void updateShiftRegister();
};