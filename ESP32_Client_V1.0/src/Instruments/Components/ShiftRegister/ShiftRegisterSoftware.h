/*
 * ShiftRegisterSoftware.h
 * 
 * Software-based shift register implementation using bit-banging
 * Works on all platforms with GPIO support (ESP32, Teensy, Arduino, etc.)
 * Uses optimized dirty flag to avoid unnecessary hardware updates
 */

#pragma once

#include "IShiftRegister.h"
#include "Config.h"
#include "Arduino.h"
#include <array>
#include <cstdint>

/**
 * ShiftRegisterSoftware - Software bit-banging implementation
 * 
 * Controls 74HC595-style shift registers using direct GPIO manipulation.
 * Optimized with dirty flag to only update hardware when state changes.
 */
class ShiftRegisterSoftware : public IShiftRegister {
private:
    static std::array<bool, HardwareConfig::NUM_INSTRUMENTS> m_outputEnabled;
    static bool m_initialized;
    static bool m_dirty;  // Tracks if m_outputEnabled has changed since last update

public:
    /**
     * Initialize shift register pins and state
     */
    void init() override {
        if (m_initialized) return;
        
        // Setup pins
        pinMode(PIN_SHIFTREG_Data, OUTPUT);
        pinMode(PIN_SHIFTREG_Clock, OUTPUT);
        pinMode(PIN_SHIFTREG_Load, OUTPUT);
        
        // Initialize all outputs as disabled
        m_outputEnabled = {};
        m_dirty = true;  // Force initial update
        
        m_initialized = true;
        
        // Update hardware to reflect initial state
        update();
    }

    /**
     * Set output enable state for a specific instrument
     */
    void setOutputEnabled(uint8_t instrument, bool enabled) override {
        if (instrument >= HardwareConfig::NUM_INSTRUMENTS) return;
        
        // Only mark dirty if value actually changed
        if (m_outputEnabled[instrument] != enabled) {
            m_outputEnabled[instrument] = enabled;
            m_dirty = true;
        }
    }

    /**
     * Get output enable state for a specific instrument
     */
    bool getOutputEnabled(uint8_t instrument) override {
        if (instrument >= HardwareConfig::NUM_INSTRUMENTS) return false;
        return m_outputEnabled[instrument];
    }

    /**
     * Disable all outputs
     */
    void disableAll() override {
        // Check if any are currently enabled
        bool wasEnabled = false;
        for (const auto& enabled : m_outputEnabled) {
            if (enabled) {
                wasEnabled = true;
                break;
            }
        }
        
        m_outputEnabled = {};
        
        // Only mark dirty if we actually changed something
        if (wasEnabled) {
            m_dirty = true;
        }
    }

    /**
     * Update shift register hardware with current output enable states
     * Only performs update if state has changed (dirty flag optimization)
     * 
     * For 74HC595-style shift registers: last bit shifted in appears at Q7
     * Shifts MSB first so highest instrument index ends up at Q7,
     * LSB last (instrument 0) ends up at Q0
     */
    void update() override {
        // Skip update if nothing has changed
        if (!m_dirty) return;
        
        // Write and Shift Data
        // Using NOP instructions for tiny delays without blocking SW PWM
        for(int8_t i = HardwareConfig::NUM_INSTRUMENTS - 1; i >= 0; i--) {
            // Note: Using ! for active-low enable logic (LOW = enabled)
            // If your drivers are active-high, change to: m_outputEnabled[i]
            digitalWriteFast(PIN_SHIFTREG_Data, !m_outputEnabled[i]);
            delayNanoseconds(SHIFTREG_HOLDTIME_NS);
            digitalWriteFast(PIN_SHIFTREG_Clock, HIGH); // Serial Clock
            delayNanoseconds(SHIFTREG_HOLDTIME_NS);
            digitalWriteFast(PIN_SHIFTREG_Clock, LOW);  // Serial Clock (latch data)
            delayNanoseconds(SHIFTREG_HOLDTIME_NS);
        }
        
        // Toggle Load to transfer shift register to output latches
        digitalWriteFast(PIN_SHIFTREG_Load, HIGH); // Register Load
        delayNanoseconds(SHIFTREG_HOLDTIME_NS);
        digitalWriteFast(PIN_SHIFTREG_Load, LOW);  // Register Load (latch output)
        digitalWriteFast(PIN_SHIFTREG_Data, LOW);  // Leave data line low when idle
        
        // Clear dirty flag after successful update
        m_dirty = false;
    }
};

// Static member initialization
inline bool ShiftRegisterSoftware::m_initialized = false;
inline bool ShiftRegisterSoftware::m_dirty = false;
inline std::array<bool, HardwareConfig::NUM_INSTRUMENTS> ShiftRegisterSoftware::m_outputEnabled = {};
