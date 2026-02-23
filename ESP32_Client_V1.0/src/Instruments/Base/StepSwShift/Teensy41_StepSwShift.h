#pragma once

#include "Constants.h"
#include "Instruments/InstrumentControllerBase.h"
#include "Instruments/Base/SwPWM/Teensy41_SwPWM.h"
#include "Instruments/Components/ShiftRegister/IShiftRegister.h"

#include <cstdint>
using std::int8_t;

class Teensy41_StepSwShift : public Teensy41_SwPWM{
public:
    static constexpr Instrument Type = Instrument::StepSW_PWM;
protected:
    static void tick();
    static void togglePin(uint8_t instrument);

    //Static Member Variables
    static std::array<uint16_t,HardwareConfig::MAX_NUM_INSTRUMENTS> m_headPosition;
    static std::bitset<HardwareConfig::MAX_NUM_INSTRUMENTS> m_pinStateDir;
    static IShiftRegister<CFG_SHIFTREGISTER_NUM_OUTPUTS>* m_shiftReg;  // First shift register (C3-B4)


public: 
    Teensy41_StepSwShift();
    Instrument getInstrumentType() const override { return Instrument::StepSW_PWM; }
};