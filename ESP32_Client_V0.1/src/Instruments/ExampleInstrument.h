#pragma once

#include "Constants.h"
#include "Instruments/InstrumentController.h"
#include <stdint.h>

class ExampleInstrument : public InstrumentController{
public:

private:
    static void togglePin(uint8_t instrument);

    //Local MIDI Device Atributes
    uint8_t m_program = 0;
    uint8_t m_channelPressure = 0;
    uint16_t m_pitchBend[MAX_NUM_INSTRUMENTS];

    //Local CC Effect Atributes
    uint16_t m_modulationWheel = 0;
    uint16_t m_footPedal = 0;
    uint16_t m_volume = 127;
    uint16_t m_expression = 0;
    uint16_t m_effectCrtl_1 = 0;
    uint16_t m_effectCrtl_2 = 0;

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
};