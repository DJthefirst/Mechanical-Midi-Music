#pragma once

#include "Constants.h"
#include "Instruments/InstrumentController.h"
#include <stdint.h>

class ShiftRegisterInstrument : public InstrumentController{
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
    void SetUp() override;
    static void Tick();

    void Reset(uint8_t instrument) override;
    void ResetAll() override;
    void PlayNote(uint8_t instrument, uint8_t note, uint8_t velocity) override;
    void StopNote(uint8_t instrument, uint8_t note, uint8_t velocity) override;
    void StopAll() override;
    void SetKeyPressure(uint8_t instrument, uint8_t note, uint8_t velocity) override;

    void SetPitchBend(uint8_t instrument, uint16_t value) override;
    void SetProgramChange(uint8_t value) override;
    void SetChannelPressure(uint8_t value) override;
    void SetModulationWheel(uint8_t value) override;
    void SetFootPedal(uint8_t value) override;
    void SetVolume(uint8_t value) override;
    void SetExpression(uint8_t value) override;
    void SetEffectCrtl_1(uint8_t value) override;
    void SetEffectCrtl_2(uint8_t value) override;

    uint8_t getNumActiveNotes(uint8_t instrument) override;
    bool isNoteActive(uint8_t instrument, uint8_t note) override;

private:
    static void ShiftAllData();
    static void ZeroOutputs();
    static void OutputOn(uint8_t outputNum);
    static void OutputOff(uint8_t outputNum);

};