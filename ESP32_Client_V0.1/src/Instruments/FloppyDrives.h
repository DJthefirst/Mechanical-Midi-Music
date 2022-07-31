#pragma once

#include "Instruments/InstrumentController.h"
#include <stdint.h>

class FloppyDrives : public InstrumentController{
private:
    //Controller Attributes
    
    //[Instrument][ActiveNote] MSB of note is Active last 7 is Value 
    uint8_t _activeNotes[32];

    //Instrument Attributes
    uint8_t floppyHeadPosition[32];
    uint8_t floppyNotePeriod[32];
    uint8_t floppyCurrentPeriod[32];
    uint8_t floppyCurrentTick[32];

public: 
    void SetUp() override;
    static void Tick();

    void Reset();
    void ResetAll() override;
    void PlayNote(uint8_t instrument, uint8_t note, uint8_t velocity) override;
    void StopNote(uint8_t instrument, uint8_t note, uint8_t velocity) override;
    void SetKeyPressure(uint8_t instrument, uint8_t note, uint8_t velocity) override;

    uint8_t getNumActiveNotes(uint8_t instrument) override;
    bool isNoteActive(uint8_t instrument, uint8_t note) override;
};