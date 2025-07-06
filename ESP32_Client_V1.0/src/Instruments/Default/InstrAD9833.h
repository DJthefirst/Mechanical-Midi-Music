#pragma once
#include "Constants.h"
#include "Instruments/InstrumentController.h"
#include "Device.h"

#include <cstdint>
using std::int8_t;

/* Set the Freq on AD9833 Signal Generator*/
class InstrAD9833 : public InstrumentController{
public:

private:

    //Local MIDI Device Atributes
    uint8_t m_program = 0;
    uint8_t m_channelPressure = 0;
    uint16_t m_pitchBend[NUM_MIDI_CH];


public: 
    InstrAD9833();

    void reset(uint8_t instrument) override;
    void resetAll() override;
    void playNote(uint8_t instrument, uint8_t note, uint8_t velocity,  uint8_t channel) override;
    void stopNote(uint8_t instrument, uint8_t note, uint8_t velocity) override;
    void stopAll() override;

    void setPitchBend(uint8_t instrument, uint16_t value, uint8_t channel) override;

    uint8_t getNumActiveNotes(uint8_t instrument) override;
    bool isNoteActive(uint8_t instrument, uint8_t note) override;

private:
    void setInstrumentFrequency(uint8_t instrument, float frequency);
    void setInstumentLedOn(uint8_t instrument, uint8_t channel, uint8_t note, uint8_t velocity);
    void setInstumentLedOff(uint8_t instrument);
    
    //LEDs
    void resetLEDs();
    void setupLEDs();
};