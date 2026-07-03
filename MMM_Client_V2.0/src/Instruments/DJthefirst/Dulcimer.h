#pragma once

// Must be at top of file
#ifndef INSTRUMENT_TYPE
    #define INSTRUMENT_TYPE Dulcimer
#endif

#include "Constants.h"
#include "Instruments/InstrumentControllerBase.h"
#include "Instruments/Components/ShiftRegister/IShiftRegister.h"
#include "Instruments/Components/ShiftRegister/ShiftRegisterFactory.h"

#include <cstdint>
#include <array>
using std::int8_t;

constexpr std::array<uint16_t, 128> NOTE_TO_SHIFT_REG_OUTPUT = {
//  C   C#  D   D#  E   F   F#  G   G#  A   A#  B
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  //C-1 - B-1
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  //C-0 - B-0
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  //C1 - B1
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  //C2 - B2
    0,  0,  1,  0,  2,  0,  3,  4, 17, 18,  5,  7,  //C3 - B3
    8, 19, 25,  9, 10, 11, 32, 12, 20, 13, 14, 31,  //C4 - B4
   30, 15, 29, 16, 40, 39, 26, 27, 38, 28, 37, 33,  //C5 - B5
   34,  0, 35,  0, 36,  0,  0,  0,  0,  0,  0,  0,  //C6 - B6
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  //C7 - B7
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  //C8 - B8
    0,  0,  0,  0,  0,  0,  0,  0   //C9 - G9
};

constexpr std::array<uint16_t, 128> NOTE_TO_LED_MAP = {
//  C   C#  D   D#  E   F   F#  G   G#  A   A#  B
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  //C-1 - B-1
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  //C-0 - B-0
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  //C1 - B1
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  //C2 - B2
    0,  0,  1,  0,  2,  0,  3,  4, 17, 18,  5,  6,  //C3 - B3
    7, 25,  8, 19,  9, 10, 32, 11, 20, 12, 13, 31,  //C4 - B4
   30, 14, 29, 15, 40, 39, 26, 27, 38, 28, 37, 33,  //C5 - B5
   34,  0, 35,  0, 36,  0,  0,  0,  0,  0,  0,  0,  //C6 - B6
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  //C7 - B7
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  //C8 - B8
    0,  0,  0,  0,  0,  0,  0,  0   //C9 - G9
};

constexpr uint8_t NUM_REG1_OUTPUTS = 16;
constexpr uint8_t NUM_REG2_OUTPUTS = 24;
constexpr uint8_t NUM_OUTPUTS = NUM_REG1_OUTPUTS + NUM_REG2_OUTPUTS;

// Generate inverse map at compile time: shift-register output index (1..NUM_OUTPUTS) -> MIDI note
// Unmapped outputs are set to 0x00.
constexpr std::array<uint8_t, NUM_OUTPUTS> make_output_reg_to_note()
{
    std::array<uint8_t, NUM_OUTPUTS> map{};
    for (size_t i = 0; i < NUM_OUTPUTS; ++i) {
        map[i] = 0x00;
    }
    
    for (size_t note = 0; note < NOTE_TO_SHIFT_REG_OUTPUT.size(); ++note) {
        const uint16_t out = NOTE_TO_SHIFT_REG_OUTPUT[note];
        if (out > 0 && out <= NUM_OUTPUTS) {
            map[out - 1] = static_cast<uint8_t>(note);
        }
    }
    return map;
}

constexpr auto OUTPUT_REG_TO_NOTE = make_output_reg_to_note();

/* Dual ShiftRegister controlled solenoid instrument */
class Dulcimer : public InstrumentControllerBase{
private:
    
    // Shift register objects
    static IShiftRegister<NUM_REG1_OUTPUTS>* m_shiftReg1;  // First shift register (C3-B4)
    static IShiftRegister<NUM_REG2_OUTPUTS>* m_shiftReg2;  // Second shift register (C5-B5)
    
    // Tracking arrays
    static uint8_t m_numActiveNotes;

public: 
    Dulcimer();
    void periodic() override;
    void reset(uint8_t instrument) override;
    void resetAll() override;
    void playNote(uint8_t instrument, uint8_t note, uint8_t velocity, uint8_t channel) override;
    void stopNote(uint8_t instrument, uint8_t note, uint8_t velocity, uint8_t channel) override;
    void stopAll() override;

    // void setPitchBend(uint8_t channel, uint16_t value) override;
    // void setModulationWheel(uint8_t channel, uint8_t value) override;

    Instrument getInstrumentType() const override { return Instrument::ShiftRegister; }
    uint8_t getNumActiveNotes(uint8_t instrument) override;
    bool isNoteActive(uint8_t instrument, uint8_t note) override;
    
private:
    void setInstrumentLedOn(uint8_t instrument, uint8_t channel, uint8_t note, uint8_t velocity) override;
    void setInstrumentLedOff(uint8_t note) override;
    void checkSolenoidTimeouts();
    void togglePin(uint8_t instrument);
};