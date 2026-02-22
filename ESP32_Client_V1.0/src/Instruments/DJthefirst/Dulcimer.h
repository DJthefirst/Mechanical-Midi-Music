#pragma once

// Must be at topp of file
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
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  //C-1 - B-1
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  //C-0 - B-0
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  //C1 - B1
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  //C2 - B2
    1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12,  //C3 - B3
    13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24,  //C4 - B4
    25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36,  //C5 - B5
    37, 38, 39, 40,  0,  0,  0,  0,  0,  0,  0,  0,  //C6 - B6
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  //C7 - B7
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  //C8 - B8
    0,  0,  0,  0,  0,  0,  0,  0   //C9 - G9
};
constexpr uint8_t NUM_REG1_OUTPUTS = 24;
constexpr uint8_t NUM_REG2_OUTPUTS = 16;
constexpr uint8_t NUM_OUTPUTS = NUM_REG1_OUTPUTS + NUM_REG2_OUTPUTS;

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
    void setInstrumentLedOff(uint8_t instrument) override;
    void checkSolenoidTimeouts();
    void togglePin(uint8_t instrument);
};