#pragma once

//#include "Arduino.h"
#include <stdint.h>

class InstrumentController{

public: 

    void Tick();

    void Reset();
    void PlayNote(uint8_t instrument, uint8_t note, uint8_t velocity);
    void StopNote(uint8_t instrument, uint8_t note, uint8_t velocity);
    void SetKeyPressure(uint8_t instrument, uint8_t note, uint8_t velocity);

    uint8_t getNumActiveNotes(uint8_t instrument);
    bool getIsNoteActive(uint8_t instrument, uint8_t note);

private:

    //Controller Attributes
    
    //[Instrument][ActiveNote] MSB of note is Active last 7 is Value 
    uint8_t _activeNotes[32][16];
    uint8_t _numActiveNotes[32];

    //Instrument Attributes
    //uint8_t floppyHeadPosition[32];
    //uint8_t floppyNotePeriod[32];
    //uint8_t floppyCurrentPeriod[32];
    //uint8_t floppyCurrentTick[32];
};