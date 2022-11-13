#pragma once

#include "Constants.h"
#include "Instruments/InstrumentController.h"
#include <stdint.h>

class ShiftRegister : public InstrumentController{
public:

private:
    static void updateShiftRegister();

    //Local MIDI Device Atributes
    uint16_t m_pitchBend;

    const static uint16_t NOTE_ONTIME = 50;
    const static bool SUSTAIN = false;

public: 
    ShiftRegister();
    static void Tick();

    void Reset(uint8_t instrument) override;
    void ResetAll() override;
    void PlayNote(uint8_t instrument, uint8_t note, uint8_t velocity) override;
    void StopNote(uint8_t instrument, uint8_t note, uint8_t velocity) override;
    void StopAll() override;

    void SetPitchBend(uint8_t instrument, uint16_t value) override;

    uint8_t getNumActiveNotes(uint8_t instrument) override;
    bool isNoteActive(uint8_t instrument, uint8_t note) override;
};