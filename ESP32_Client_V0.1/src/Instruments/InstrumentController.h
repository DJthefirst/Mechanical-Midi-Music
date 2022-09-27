#pragma once

#include "Constants.h"
#include <stdint.h>

class InstrumentController{
public:

private:

    //Local CC Effect Atributes
    uint16_t _ModulationWheel = 0;
    uint16_t _FootPedal = 0;
    uint16_t _Volume = 127;
    uint16_t _Expression = 0;
    uint16_t _EffectCrtl_1 = 0;
    uint16_t _EffectCrtl_2 = 0;

public:

    //Basic Functions
    virtual void Reset(uint8_t instrument);
    virtual void ResetAll();
    virtual void PlayNote(uint8_t instrument, uint8_t note, uint8_t velocity);
    virtual void StopNote(uint8_t instrument, uint8_t note, uint8_t velocity);
    virtual void StopAll();

    //Setters
    virtual void SetKeyPressure(uint8_t instrument, uint8_t note, uint8_t velocity);
    virtual void SetPitchBend(uint8_t instrument, uint16_t value);
    virtual void SetProgramChange(uint8_t value);
    virtual void SetChannelPressure(uint8_t value);
    virtual void SetModulationWheel(uint8_t value);
    virtual void SetFootPedal(uint8_t value);
    virtual void SetVolume(uint8_t value);
    virtual void SetExpression(uint8_t value);
    virtual void SetEffectCrtl_1(uint8_t value);
    virtual void SetEffectCrtl_2(uint8_t value);

    //Getters
    virtual uint8_t getNumActiveNotes(uint8_t instrument);
    virtual bool isNoteActive(uint8_t instrument, uint8_t note);
};