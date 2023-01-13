#pragma once

#include "Constants.h"
#include "Instruments/InstrumentController.h"
#include <cstdint>
using std::int8_t;

enum SteppingMode
{ 
    WaveDrive = 0,   //One Coil Engaged at a time (One Phase)
    FullStep,        //Two Coils Engaged at a time (Two Phase)
    HalfStep,        //Half Stepping
};

class StepperL298n : public InstrumentController{
public:

private:

    static void stepMotor(uint8_t instrument);

    //Local MIDI Device Atributes
    uint16_t m_pitchBend[MAX_NUM_INSTRUMENTS];

public: 
    StepperL298n();
    static void Tick();

    void reset(uint8_t instrument) override;
    void resetAll() override;
    void playNote(uint8_t instrument, uint8_t note, uint8_t velocity) override;
    void stopNote(uint8_t instrument, uint8_t note, uint8_t velocity) override;
    void stopAll() override;

    void setPitchBend(uint8_t instrument, uint16_t value) override;

    uint8_t getNumActiveNotes(uint8_t instrument) override;
    bool isNoteActive(uint8_t instrument, uint8_t note) override;
};