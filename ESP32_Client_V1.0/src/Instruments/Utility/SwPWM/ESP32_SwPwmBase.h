#pragma once

#include "Constants.h"
#include "Instruments/InstrumentController.h"


#include <cstdint>
using std::int8_t;

/* Outputs a PWM signal at the Notes Frequency on each Digital IO Pin */
class ESP32_SwPwmBase : public InstrumentController{
public:
    static constexpr Instrument Type = Instrument::PWM;
private:
    static void Tick();
    static void togglePin(uint8_t instrument);

    //Local MIDI Device Atributes
    uint8_t m_program = 0;
    uint8_t m_channelPressure = 0;
    uint16_t m_pitchBend[NUM_MIDI_CH];


public: 
    ESP32_SwPwmBase();
    void reset(uint8_t instrument) override;
    void resetAll() override;
    void playNote(uint8_t instrument, uint8_t note, uint8_t velocity,  uint8_t channel) override;
    void stopNote(uint8_t instrument, uint8_t note, uint8_t velocity) override;
    void stopAll() override;

    void setPitchBend(uint8_t instrument, uint16_t value, uint8_t channel) override;

    uint8_t getNumActiveNotes(uint8_t instrument) override;
    bool isNoteActive(uint8_t instrument, uint8_t note) override;

};