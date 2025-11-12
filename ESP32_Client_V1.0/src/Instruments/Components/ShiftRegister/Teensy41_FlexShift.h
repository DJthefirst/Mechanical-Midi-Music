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

#include "Config.h"

#if defined(PLATFORM_TEENSY41)

#include <array>
#include <cstdint>

/**
 * Teensy41_FlexShift - Hardware-accelerated FlexIO implementation for Teensy 4.x
 * 
 * Uses FlexIO peripheral to generate precise shift register timing with minimal CPU overhead.
 * Significantly faster and more efficient than software bit-banging.
 */

class Teensy41_FlexShift {
private:
    static std::array<bool, HardwareConfig::NUM_INSTRUMENTS> m_outputEnabled;
    static bool m_initialized;
    static bool m_dirty;  // Tracks if m_outputEnabled has changed since last update
    
    // Pin configuration (from config file)
    static constexpr uint8_t PIN_DATA = PIN_SHIFTREG_Data;   // Pin 17
    static constexpr uint8_t PIN_CLOCK = PIN_SHIFTREG_Clock; // Pin 16
    static constexpr uint8_t PIN_LOAD = PIN_SHIFTREG_Load;   // Pin 15
    
    // FlexIO pin assignments
    static constexpr uint8_t FLEXIO_DATA_PIN = 4;   // FlexIO1 pin 4 = physical pin 17
    static constexpr uint8_t FLEXIO_CLOCK_PIN = 5;  // FlexIO1 pin 5 = physical pin 16

    static void initFlexIO();
    static void waitShifterComplete();

public:
    static void init();
    static void setOutputEnabled(uint8_t instrument, bool enabled);
    static bool getOutputEnabled(uint8_t instrument);
    static void disableAll();
    static void update();
};

#endif // PLATFORM_TEENSY41