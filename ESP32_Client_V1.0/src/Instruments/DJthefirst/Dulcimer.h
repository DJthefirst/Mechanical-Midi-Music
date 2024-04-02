#pragma once
#include "Device.h"
#include "Instruments/InstrumentController.h"
#include "Constants.h"

#include <cstdint>
using std::int8_t;

class Dulcimer : public InstrumentController{
public:

private:
    static void updateShiftRegister();

    //Local MIDI Device Atributes
    const static uint16_t NOTE_ONTIME = 800;
    const static bool SUSTAIN = false;

public: 
    Dulcimer();
    static void Tick();

    void reset(uint8_t instrument) override;
    void resetAll() override;
    void playNote(uint8_t instrument, uint8_t note, uint8_t velocity,  uint8_t channel) override;
    void stopNote(uint8_t instrument, uint8_t note, uint8_t velocity) override;
    void stopAll() override;

    uint8_t getNumActiveNotes(uint8_t instrument) override;
    bool isNoteActive(uint8_t instrument, uint8_t note) override;
    
private:
    //LEDs
    void setInstumentLedOn(uint8_t instrument, uint8_t channel, uint8_t notePos, uint8_t velocity);
    void setInstumentLedOff(uint8_t notePos);
    void resetLEDs();
    void setupLEDs();

};