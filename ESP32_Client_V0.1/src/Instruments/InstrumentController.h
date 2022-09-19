#pragma once

#include "Constants.h"
#include <stdint.h>

class InstrumentController{
public:

private:
    //Local MIDI Device Atributes
    uint8_t _Program = 0;
    uint8_t _ChannelPressure = 0;
    uint16_t _PitchBend[MAX_NUM_INSTRUMENTS];

    //Local CC Effect Atributes
    uint16_t _ModulationWheel = 0;
    uint16_t _FootPedal = 0;
    uint16_t _Volume = 127;
    uint16_t _Expression = 0;
    uint16_t _EffectCrtl_1 = 0;
    uint16_t _EffectCrtl_2 = 0;

public: 
    InstrumentController();
    static void Tick();

    void Reset(uint8_t instrument);
    void ResetAll();
    void PlayNote(uint8_t instrument, uint8_t note, uint8_t velocity);
    void StopNote(uint8_t instrument, uint8_t note, uint8_t velocity);
    void StopAll();
    void SetKeyPressure(uint8_t instrument, uint8_t note, uint8_t velocity);

    void SetPitchBend(uint8_t instrument, uint16_t value);
    void SetProgramChange(uint8_t value);
    void SetChannelPressure(uint8_t value);
    void SetModulationWheel(uint8_t value);
    void SetFootPedal(uint8_t value);
    void SetVolume(uint8_t value);
    void SetExpression(uint8_t value);
    void SetEffectCrtl_1(uint8_t value);
    void SetEffectCrtl_2(uint8_t value);

    uint8_t getNumActiveNotes(uint8_t instrument);
    bool isNoteActive(uint8_t instrument, uint8_t note);
};