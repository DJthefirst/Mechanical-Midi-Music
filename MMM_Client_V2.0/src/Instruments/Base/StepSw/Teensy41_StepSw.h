#pragma once

#include "Constants.h"
#include "Instruments/InstrumentControllerBase.h"
#include "Instruments/Base/SwPWM/Teensy41_SwPWM.h"

#include <cstdint>
using std::int8_t;

class Teensy41_StepSw : public Teensy41_SwPWM{
public:
    static constexpr Instrument Type = Instrument::StepSW_PWM;
protected:
    static void tick();
    static void togglePin(uint8_t instrument);

    //Static Member Variables
    static std::array<uint16_t,HardwareConfig::MAX_NUM_INSTRUMENTS> m_headPosition;
    static std::bitset<HardwareConfig::MAX_NUM_INSTRUMENTS> m_pinStateDir;


public: 
    Teensy41_StepSw();
    Instrument getInstrumentType() const override { return Instrument::StepSW_PWM; }
};