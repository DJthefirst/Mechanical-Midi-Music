/*
 * Default_SwShift.h
 * 
 * Software-based shift register implementation using bit-banging
 * Works on all platforms with GPIO support (ESP32, Teensy, Arduino, etc.)
 * Uses optimized dirty flag to avoid unnecessary hardware updates
 */

#pragma once

#include "Config.h"
#include "IShiftRegister.h"
#include <bitset>
#include <cstdint>

template<size_t numOutputs>
class Default_SwShift : public IShiftRegister<numOutputs> {
public:
    Default_SwShift(uint8_t PIN_SER, uint8_t PIN_CLK, uint8_t PIN_LD, uint8_t PIN_EN, uint8_t PIN_RST):
        Default_SwShift::IShiftRegister(PIN_SER, PIN_CLK, PIN_LD, PIN_EN, PIN_RST) {}
    void init() override;
    void setOutputEnabled(uint8_t instrument, bool enabled) override;
    bool getOutputEnabled(uint8_t instrument) override;
    void disableAll() override;
    void update() override;
};

