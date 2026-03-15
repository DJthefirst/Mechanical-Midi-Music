/*
 * Teensy41_FlexShift.h
 * 
 * Teensy 4.x FlexIO-based shift register implementation
 * Uses hardware-accelerated FlexIO peripheral for fast, efficient shift register control
 * Optimized with dirty flag to avoid unnecessary hardware updates
 * 
 * Hardware connections:
 *   Data:  Pin 17 (FlexIO1:4 / GPIO_B1_00)
 *   Clock: Pin 16 (FlexIO1:5 / GPIO_B1_01)
 *   Load:  Pin 15 (GPIO - handled via digitalWriteFast)
 */

#pragma once

#pragma once

#include "Config.h"
#include "IShiftRegister.h"
#include <bitset>
#include <cstdint>
#include <optional>

template<size_t numOutputs>
class Teensy41_FlexShift : public IShiftRegister<numOutputs> {
public:
    Teensy41_FlexShift(uint8_t PIN_SER, uint8_t PIN_CLK, uint8_t PIN_LD, std::optional<uint8_t> PIN_EN, std::optional<uint8_t> PIN_RST):
        Teensy41_FlexShift::IShiftRegister(PIN_SER, PIN_CLK, PIN_LD, PIN_EN, PIN_RST) {}
    void init() override;
    void setOutputEnabled(uint8_t instrument, bool enabled) override;
    bool getOutputEnabled(uint8_t instrument) override;
    void disableAll() override;
    void update() override;
};