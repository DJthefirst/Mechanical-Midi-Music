#pragma once

#include "Instruments/InstrumentController.h"
#include <stdint.h>

class ExampleInstrument : public InstrumentController{

public: 
    void SetUp() override;
    static void Tick();

    void Reset(uint8_t instrument) override;
    void ResetAll() override;
    void PlayNote(uint8_t instrument, uint8_t note, uint8_t velocity) override;
    void StopNote(uint8_t instrument, uint8_t note, uint8_t velocity) override;
    void SetKeyPressure(uint8_t instrument, uint8_t note, uint8_t velocity) override;

    uint8_t getNumActiveNotes(uint8_t instrument) override;
    bool isNoteActive(uint8_t instrument, uint8_t note) override;

private:

    static void togglePin(uint8_t instrument);

};