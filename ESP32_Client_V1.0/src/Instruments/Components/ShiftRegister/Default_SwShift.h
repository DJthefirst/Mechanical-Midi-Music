/*
 * Default_SwShift.h
 * 
 * Software-based shift register implementation using bit-banging
 * Works on all platforms with GPIO support (ESP32, Teensy, Arduino, etc.)
 * Uses optimized dirty flag to avoid unnecessary hardware updates
 */

#pragma once

#include "Config.h"
#include <array>
#include <cstdint>

class Default_SwShift {
private:
    static std::array<bool, HardwareConfig::NUM_INSTRUMENTS> m_outputEnabled;
    static bool m_initialized;
    static bool m_update;  // Tracks if m_outputEnabled has changed since last update

public:

    static void init();
    static void setOutputEnabled(uint8_t instrument, bool enabled);
    static bool getOutputEnabled(uint8_t instrument);
    static void disableAll();
    static void update();
};

