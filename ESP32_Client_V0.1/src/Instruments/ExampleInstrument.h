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

    //Controller Attributes
    
    //[Instrument][ActiveNote] MSB of note is Active last 7 is Value 
    uint8_t _activeNotes[32][16];
    uint8_t _numActiveNotes[32];

    static void togglePin(uint8_t instrument);

};