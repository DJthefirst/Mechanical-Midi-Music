/*
 * Teensy41_SwShift.cpp
 * 
 * Software-based shift register implementation using bit-banging
 * Works on all platforms with GPIO support (ESP32, Teensy, Arduino, etc.)
 */

#include "Config.h"

#if defined(PLATFORM_TEENSY41) && !defined(SHIFTREG_USE_FLEXIO) && defined(COMPONENT_SHIFTREG_74HC595)

#include "Teensy41_SwShift.h"
#include "Arduino.h"

// Static member definitions
bool Teensy41_SwShift::m_initialized = false;
bool Teensy41_SwShift::m_update = false;
std::array<bool, HardwareConfig::NUM_INSTRUMENTS> Teensy41_SwShift::m_outputEnabled = {};

/**
 * Initialize shift register pins and state
 */
void Teensy41_SwShift::init() {
    if (m_initialized) return;
    
    // Setup pins
    pinMode(HardwareConfig::PIN_SHIFTREG_Data, OUTPUT);
    pinMode(HardwareConfig::PIN_SHIFTREG_Clock, OUTPUT);
    pinMode(HardwareConfig::PIN_SHIFTREG_Load, OUTPUT);

    // Initialize all outputs as disabled
    m_outputEnabled = {};
    m_update = true;  // Force initial update
    
    m_initialized = true;
    
    // Update hardware to reflect initial state
    update();
}

/**
 * Set output enable state for a specific instrument
 */
void Teensy41_SwShift::setOutputEnabled(uint8_t instrument, bool enabled) {
    if (instrument >= HardwareConfig::NUM_INSTRUMENTS) return;
    
    // Only mark update if value actually changed
    if (m_outputEnabled[instrument] != enabled) {
        m_outputEnabled[instrument] = enabled;
        m_update = true;
    }
}

/**
 * Get output enable state for a specific instrument
 */
bool Teensy41_SwShift::getOutputEnabled(uint8_t instrument) {
    if (instrument >= HardwareConfig::NUM_INSTRUMENTS) return false;
    return m_outputEnabled[instrument];
}

/**
 * Disable all outputs
 */
void Teensy41_SwShift::disableAll() {
    // Check if any are currently enabled
    bool wasEnabled = false;
    for (const auto& enabled : m_outputEnabled) {
        if (enabled) {
            wasEnabled = true;
            break;
        }
    }
    
    m_outputEnabled = {};
    
    // Only mark update if we actually changed something
    if (wasEnabled) {
        m_update = true;
    }
}

/**
 * Update shift register hardware with current output enable states
 * Only performs update if state has changed (update flag optimization)
 * 
 * For 74HC595-style shift registers: last bit shifted in appears at Q7
 * Shifts MSB first so highest instrument index ends up at Q7,
 * LSB last (instrument 0) ends up at Q0
 */
void Teensy41_SwShift::update() {
    // Skip update if nothing has changed
    if (!m_update) return;
    
    // Write and Shift Data
    // Using NOP instructions for tiny delays without blocking SW PWM
    for(int8_t i = HardwareConfig::NUM_INSTRUMENTS - 1; i >= 0; i--) {
        // Note: Using ! for active-low enable logic (LOW = enabled)
        // If your drivers are active-high, change to: m_outputEnabled[i]
        digitalWriteFast(HardwareConfig::PIN_SHIFTREG_Data, !m_outputEnabled[i]);
        delayNanoseconds(SHIFTREG_HOLDTIME_NS);
        digitalWriteFast(HardwareConfig::PIN_SHIFTREG_Clock, HIGH); // Serial Clock
        delayNanoseconds(SHIFTREG_HOLDTIME_NS);
        digitalWriteFast(HardwareConfig::PIN_SHIFTREG_Clock, LOW);  // Serial Clock (latch data)
        delayNanoseconds(SHIFTREG_HOLDTIME_NS);
    }
    
    // Toggle Load to transfer shift register to output latches
    digitalWriteFast(HardwareConfig::PIN_SHIFTREG_Load, HIGH); // Register Load
    delayNanoseconds(SHIFTREG_HOLDTIME_NS);
    digitalWriteFast(HardwareConfig::PIN_SHIFTREG_Load, LOW);  // Register Load (latch output)
    digitalWriteFast(HardwareConfig::PIN_SHIFTREG_Data, LOW);  // Leave data line low when idle
    
    // Clear update flag after successful update
    m_update = false;
}

#endif // PLATFORM_TEENSY41 && !SHIFTREG_USE_FLEXIO