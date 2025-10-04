#pragma once

#include "Constants.h"
#include "Instruments/InstrumentController.h"
#include "Device.h"
#include <cstdint>
using std::int8_t;

class DrumSimple : public InstrumentController{
public:

private:

    //Local MIDI Device Atributes
    const static uint16_t NOTE_ONTIME = 1000;
    const static bool SUSTAIN = false;

public: 
    DrumSimple();
    static void Tick();

    void reset(uint8_t instrument) override;
    void resetAll() override;
    void playNote(uint8_t instrument, uint8_t note, uint8_t velocity,  uint8_t channel) override;
    void stopNote(uint8_t instrument, uint8_t note, uint8_t velocity) override;
    void stopAll() override;

    uint8_t getNumActiveNotes(uint8_t instrument) override;
    bool isNoteActive(uint8_t instrument, uint8_t note) override;
};