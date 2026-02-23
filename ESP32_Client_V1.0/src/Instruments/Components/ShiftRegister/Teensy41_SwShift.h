/*
 * Teensy41_SwShift.h
 * 
 * Software-based shift register implementation using bit-banging
 * Uses optimized update flag to avoid unnecessary hardware updates
 */

#pragma once

#include "Config.h"
#include "IShiftRegister.h"
#include <bitset>
#include <cstdint>
#include <optional>

template<size_t numOutputs>
class Teensy41_SwShift : public IShiftRegister<numOutputs> {
public:
    Teensy41_SwShift(uint8_t PIN_SER, uint8_t PIN_CLK, uint8_t PIN_LD, std::optional<uint8_t> PIN_EN, std::optional<uint8_t> PIN_RST):
        Teensy41_SwShift::IShiftRegister(PIN_SER, PIN_CLK, PIN_LD, PIN_EN, PIN_RST) {}
    void init() override;
    void setOutputEnabled(uint8_t instrument, bool enabled) override;
    bool getOutputEnabled(uint8_t instrument) override;
    void disableAll() override;
    void update() override;
};