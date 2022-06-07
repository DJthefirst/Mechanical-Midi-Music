#pragma once

#include <stdint.h>

class InstrumentController{

public: 
    virtual void Tick() = 0;

    virtual void Reset();
    virtual void PlayNote(uint8_t instrument, uint8_t note, uint8_t velocity) = 0;
    virtual void StopNote(uint8_t instrument, uint8_t note, uint8_t velocity) = 0;
    virtual void SetKeyPressure(uint8_t instrument, uint8_t note, uint8_t velocity) = 0;

    virtual uint8_t getNumActiveNotes(uint8_t instrument) = 0;
    virtual bool isNotActive(uint8_t instrument, uint8_t note) = 0;

private:

    //Controller Attributes
    
    //[Instrument][ActiveNote] MSB of note is Active last 7 is Value 
    uint8_t _activeNotes[32][16];
    uint8_t _numActiveNotes[32];

};