//
// An Interface Class For Various Instrument Types
//
#pragma once

#include <stdint.h>

class InstrumentController{

public: 
    virtual void SetUp() = 0;

    virtual void Reset(uint8_t instrument) = 0;
    virtual void ResetAll() = 0;
    virtual void PlayNote(uint8_t instrument, uint8_t note, uint8_t velocity) = 0;
    virtual void StopNote(uint8_t instrument, uint8_t note, uint8_t velocity) = 0;
    virtual void StopAll() = 0;
    virtual void SetKeyPressure(uint8_t instrument, uint8_t note, uint8_t velocity){};

    virtual void SetModulationWheel(uint8_t value) = 0;
    virtual void SetFootPedal(uint8_t value) = 0;
    virtual void SetVolume(uint8_t value) = 0;
    virtual void SetExpression(uint8_t value) = 0;
    virtual void SetEffectCrtl_1(uint8_t value) = 0;
    virtual void SetEffectCrtl_2(uint8_t value) = 0;

    virtual uint8_t getNumActiveNotes(uint8_t instrument) = 0;
    virtual bool isNoteActive(uint8_t instrument, uint8_t note) = 0;

};

