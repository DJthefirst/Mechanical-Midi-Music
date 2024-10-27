#pragma once

#include "Constants.h"
#include "Instruments/InstrumentController.h"
#include "Device.h"

#include <cstdint>
using std::int8_t;

/* Outputs a PWM signal at the Notes Frequency on each Digital IO Pin */
class Pwm8A04 : public InstrumentController{
public:

private:

    //Local MIDI Device Atributes
    uint8_t m_program = 0;
    uint8_t m_channelPressure = 0;
    uint16_t m_pitchBend[MAX_NUM_INSTRUMENTS];


public: 
    Pwm8A04();

    void reset(uint8_t instrument) override;
    void resetAll() override;
    void playNote(uint8_t instrument, uint8_t note, uint8_t velocity,  uint8_t channel) override;
    void stopNote(uint8_t instrument, uint8_t note, uint8_t velocity) override;
    void stopAll() override;

    void setPitchBend(uint8_t instrument, uint16_t value) override;

    uint8_t getNumActiveNotes(uint8_t instrument) override;
    bool isNoteActive(uint8_t instrument, uint8_t note) override;

private:
    void setModBusInstrument(uint8_t instrument, uint16_t frequency);
    void setInstumentLedOn(uint8_t instrument, uint8_t channel, uint8_t note, uint8_t velocity);
    void setInstumentLedOff(uint8_t instrument);
    
    //LEDs
    void resetLEDs();
    void setupLEDs();
};