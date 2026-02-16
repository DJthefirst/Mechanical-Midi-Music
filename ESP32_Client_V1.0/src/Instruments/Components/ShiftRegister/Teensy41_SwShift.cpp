/*
 * Teensy41_SwShift.cpp
 * 
 * Software-based shift register implementation using bit-banging
 */
#include "Config.h"
#if defined (PLATFORM_TEENSY41)

#include "Teensy41_SwShift.h"
#include "Arduino.h"
#include <array>

/**
 * Initialize shift register pins and state
 */
template<size_t numOutputs>
void Teensy41_SwShift<numOutputs>::init() {
    if (this->m_initialized) return;
    
    // Setup pins
    pinMode(this->m_PIN_SER, OUTPUT);
    pinMode(this->m_PIN_CLK, OUTPUT);
    pinMode(this->m_PIN_LD, OUTPUT);
    pinMode(this->m_PIN_EN, OUTPUT);
    pinMode(this->m_PIN_RST, OUTPUT);

    digitalWriteFast(this->m_PIN_RST, HIGH);

    // Initialize all outputs as disabled
    this->m_outputEnabled.reset();
    this->m_update = true;  // Force initial update
    
    this->m_initialized = true;
    
    // Update hardware to reflect initial state
    update();
}

/**
 * Set output enable state for a specific instrument
 */
template<size_t numOutputs>
void Teensy41_SwShift<numOutputs>::setOutputEnabled(uint8_t instrument, bool enabled) {
    if (instrument >= numOutputs) return;
    
    // Only mark update if value actually changed
    if (this->m_outputEnabled[instrument] != enabled) {
        this->m_outputEnabled[instrument] = enabled;
        this->m_update = true;
    }
}

/**
 * Get output enable state for a specific instrument
 */
template<size_t numOutputs>
bool Teensy41_SwShift<numOutputs>::getOutputEnabled(uint8_t instrument) {
    if (instrument >= numOutputs) return false;
    return this->m_outputEnabled[instrument];
}

/**
 * Disable all outputs
 */
template<size_t numOutputs>
void Teensy41_SwShift<numOutputs>::disableAll() {
    // Check if any are currently enabled
    bool wasEnabled = false;
    for (size_t i = 0; i < numOutputs; i++) {
        if (this->m_outputEnabled[i]) {
            wasEnabled = true;
            break;
        }
    }
    
    this->m_outputEnabled.reset();
    
    // Only mark update if we actually changed something
    if (wasEnabled) {
        this->m_update = true;
    }
}

/**
 * Update shift register hardware with current output enable states
 * Only performs update if state has changed (update flag optimization)
 * 
 * For 74HC595-style shift registers: first bit shifted in ends up at the
 * highest output position after all bits are shifted. We send LSB first
 * (index 0) so that output 0 ends up at Q0 of the first register.
 */
template<size_t numOutputs>
void Teensy41_SwShift<numOutputs>::update() {
    // Skip update if nothing has changed
    if (!this->m_update) return;
    
    // Write and Shift Data
    // Using nanosecond delays for Teensy's faster timing requirements
    // Send LSB first (output 0 first, which will end up at Q0 after all shifts)
    for (int32_t i = static_cast<int32_t>(numOutputs) - 1; i >= 0; i--) {
        digitalWriteFast(this->m_PIN_SER, this->m_outputEnabled[i]);
        delayNanoseconds(CFG_SHIFTREG_DATA_HOLDTIME_NS);
        digitalWriteFast(this->m_PIN_CLK, HIGH); // Serial Clock
        delayNanoseconds(CFG_SHIFTREG_DATA_HOLDTIME_NS);
        digitalWriteFast(this->m_PIN_CLK, LOW);  // Serial Clock (latch data)
        delayNanoseconds(CFG_SHIFTREG_DATA_HOLDTIME_NS);
    }
    
    // Toggle Load to transfer shift register to output latches
    digitalWriteFast(this->m_PIN_LD, HIGH); // Register Load
    delayNanoseconds(CFG_SHIFTREG_DATA_HOLDTIME_NS);
    digitalWriteFast(this->m_PIN_LD, LOW);  // Register Load (latch output)
    digitalWriteFast(this->m_PIN_SER, LOW);  // Leave data line low when idle
    
    // Clear update flag after successful update
    this->m_update = false;
}

// Template instantiations for common sizes
template class Teensy41_SwShift<8>;
template class Teensy41_SwShift<12>;
template class Teensy41_SwShift<16>;
template class Teensy41_SwShift<24>;
template class Teensy41_SwShift<32>;

#endif // PLATFORM_TEENSY41