/*
 * Default_SwShift.cpp
 * 
 * Software-based shift register implementation using bit-banging
 */

#include "Default_SwShift.h"
#include "Arduino.h"
#include <array>

/**
 * Initialize shift register pins and state
 */
template<size_t numOutputs>
void Default_SwShift<numOutputs>::init() {
    if (this->m_initialized) return;
    
    // Setup pins
    pinMode(this->m_PIN_SER, OUTPUT);
    pinMode(this->m_PIN_CLK, OUTPUT);
    pinMode(this->m_PIN_LD, OUTPUT);
    if (this->m_PIN_EN.has_value()) pinMode(this->m_PIN_EN.value(), OUTPUT);
    if (this->m_PIN_RST.has_value()) pinMode(this->m_PIN_RST.value(), OUTPUT);

    if (this->m_PIN_RST.has_value()) digitalWriteFast(this->m_PIN_RST.value(), HIGH);

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
void Default_SwShift<numOutputs>::setOutputEnabled(uint8_t instrument, bool enabled) {
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
bool Default_SwShift<numOutputs>::getOutputEnabled(uint8_t instrument) {
    if (instrument >= numOutputs) return false;
    return this->m_outputEnabled[instrument];
}

/**
 * Disable all outputs
 */
template<size_t numOutputs>
void Default_SwShift<numOutputs>::disableAll() {
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
 * For 74HC595-style shift registers: last bit shifted in appears at Q7
 * Shifts MSB first so highest instrument index ends up at Q7,
 * LSB last (instrument 0) ends up at Q0
 */
template<size_t numOutputs>
void Default_SwShift<numOutputs>::update() {
    // Skip update if nothing has changed
    if (!this->m_update) return;
    
    // Write and Shift Data
    // Using microsecond delays for timing requirements
    for (int32_t i = static_cast<int32_t>(numOutputs) - 1; i >= 0; i--) {
        digitalWrite(this->m_PIN_SER, this->m_outputEnabled[i]);
        delayMicroseconds(1); // Short delay to meet timing requirements
        digitalWrite(this->m_PIN_CLK, HIGH); // Serial Clock
        delayMicroseconds(1); // Short delay to meet timing requirements
        digitalWrite(this->m_PIN_CLK, LOW);  // Serial Clock (latch data)
        delayMicroseconds(1); // Short delay to meet timing requirements
    }
    
    // Toggle Load to transfer shift register to output latches
    digitalWrite(this->m_PIN_LD, HIGH); // Register Load
    delayMicroseconds(1); // Short delay to meet timing requirements
    digitalWrite(this->m_PIN_LD, LOW);  // Register Load (latch output)
    digitalWrite(this->m_PIN_SER, LOW);  // Leave data line low when idle
    
    // Clear update flag after successful update
    this->m_update = false;
}

// Template instantiations for common sizes
template class Default_SwShift<8>;
template class Default_SwShift<12>;
template class Default_SwShift<16>;
template class Default_SwShift<24>;
template class Default_SwShift<32>;