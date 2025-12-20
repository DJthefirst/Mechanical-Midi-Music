#pragma once

#include "Constants.h"
#include "Instruments/InstrumentControllerBase.h"
#include "Instruments/Base/SwPWM/ESP32_SwPWM.h"

#include <cstdint>
using std::int8_t;

class ESP32_StepSw : public ESP32_SwPWM{
public:
    static constexpr Instrument Type = Instrument::StepSW_PWM;
protected:
    static void tick();
    static void togglePin(uint8_t instrument);

    //Static Member Variables
    static std::array<uint16_t,HardwareConfig::MAX_NUM_INSTRUMENTS> m_headPosition;
    static std::bitset<HardwareConfig::MAX_NUM_INSTRUMENTS> m_pinStateDir;


public: 
    ESP32_StepSw();
    Instrument getInstrumentType() const override { return Instrument::StepSW_PWM; }
};