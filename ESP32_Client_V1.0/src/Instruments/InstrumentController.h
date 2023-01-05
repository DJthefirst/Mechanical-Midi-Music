#pragma once

#include "Constants.h"
#include "Device.h"
#include "Platform.h"
#include <stdint.h>

class InstrumentController{
public:

protected:

    //Local CC Effect Atributes
    uint16_t _ModulationWheel = 0;
    uint16_t _FootPedal = 0;
    uint16_t _Volume = 127;
    uint16_t _Expression = 0;
    uint16_t _EffectCrtl_1 = 0;
    uint16_t _EffectCrtl_2 = 0;

public:

    //Basic Functions
    virtual void reset(uint8_t instrument);
    virtual void resetAll();
    virtual void playNote(uint8_t instrument, uint8_t note, uint8_t velocity);
    virtual void stopNote(uint8_t instrument, uint8_t note, uint8_t velocity);
    virtual void stopAll();

    //Setters
    virtual void setKeyPressure(uint8_t instrument, uint8_t note, uint8_t velocity);
    virtual void setPitchBend(uint8_t instrument, uint16_t value);
    virtual void setProgramChange(uint8_t value);
    virtual void setChannelPressure(uint8_t value);
    virtual void setModulationWheel(uint8_t value);
    virtual void setFootPedal(uint8_t value);
    virtual void setVolume(uint8_t value);
    virtual void setExpression(uint8_t value);
    virtual void setEffectCrtl_1(uint8_t value);
    virtual void setEffectCrtl_2(uint8_t value);

    //Getters
    virtual uint8_t getNumActiveNotes(uint8_t instrument);
    virtual bool isNoteActive(uint8_t instrument, uint8_t note);
};