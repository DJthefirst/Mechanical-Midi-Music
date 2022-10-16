#pragma once

#include "Constants.h"
#include "Instruments/InstrumentController.h"
#include <stdint.h>

enum SteppingMode
{ 
    WaveDrive = 0,   //One Coil Engaged at a time (One Phase)
    FullStep,        //Two Coils Engaged at a time (Two Phase)
    HalfStep,        //Half Stepping
};

class StepperMotors : public InstrumentController{
public:

private:

    static void stepMotor(uint8_t instrument);

    //Local MIDI Device Atributes
    uint16_t m_pitchBend[MAX_NUM_INSTRUMENTS];

public: 
    StepperMotors();
    static void Tick();

    void Reset(uint8_t instrument) override;
    void ResetAll() override;
    void PlayNote(uint8_t instrument, uint8_t note, uint8_t velocity) override;
    void StopNote(uint8_t instrument, uint8_t note, uint8_t velocity) override;
    void StopAll() override;

    void SetPitchBend(uint8_t instrument, uint16_t value) override;

    uint8_t getNumActiveNotes(uint8_t instrument) override;
    bool isNoteActive(uint8_t instrument, uint8_t note) override;
};