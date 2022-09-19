#pragma once

#include "Constants.h"
#include "Instruments/InstrumentController.h"
#include <stdint.h>

class ExampleInstrument : public InstrumentController{
public:

private:
    static void togglePin(uint8_t instrument);

    //Local MIDI Device Atributes
    uint8_t m_program = 0;
    uint8_t m_channelPressure = 0;
    uint16_t m_pitchBend[MAX_NUM_INSTRUMENTS];


public: 
    ExampleInstrument();
    static void Tick();

    void Reset(uint8_t instrument);
    void ResetAll();
    void PlayNote(uint8_t instrument, uint8_t note, uint8_t velocity);
    void StopNote(uint8_t instrument, uint8_t note, uint8_t velocity);
    void StopAll();

    void SetPitchBend(uint8_t instrument, uint16_t value);


    uint8_t getNumActiveNotes(uint8_t instrument);
    bool isNoteActive(uint8_t instrument, uint8_t note);
};