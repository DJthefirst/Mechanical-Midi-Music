#pragma once

#include "Instruments/InstrumentController.h"
#include <stdint.h>

class FloppyDrives : public InstrumentController{

public: 
    void Tick();

    void Reset();
    void PlayNote(uint8_t instrument, uint8_t note, uint8_t velocity);
    void StopNote(uint8_t instrument, uint8_t note, uint8_t velocity);
    void SetKeyPressure(uint8_t instrument, uint8_t note, uint8_t velocity);

    uint8_t getNumActiveNotes(uint8_t instrument);
    bool isNotActive(uint8_t instrument, uint8_t note);

private:

    //Instrument Attributes
    //uint8_t floppyHeadPosition[32];
    //uint8_t floppyNotePeriod[32];
    //uint8_t floppyCurrentPeriod[32];
    //uint8_t floppyCurrentTick[32];
};