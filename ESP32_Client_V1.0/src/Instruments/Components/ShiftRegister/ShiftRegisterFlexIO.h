/*
 * ShiftRegisterFlexIO.h
 * 
 * Teensy 4.x FlexIO-based shift register implementation
 * Uses hardware-accelerated FlexIO peripheral for fast, efficient shift register control
 * Optimized with dirty flag to avoid unnecessary hardware updates
 * 
 * Hardware connections:
 *   Data:  Pin 17 (FlexIO1:4)
 *   Clock: Pin 16 (FlexIO1:5)
 *   Load:  Pin 15 (FlexIO1:6)
 */

#pragma once

#include "IShiftRegister.h"
#include "Config.h"

#if defined(PLATFORM_TEENSY41)

#include "Arduino.h"
#include <array>
#include <cstdint>

// FlexIO register access
#define FLEXIO1_CTRL        (*(volatile uint32_t *)0x401AC000)
#define FLEXIO1_SHIFTCTL0   (*(volatile uint32_t *)0x401AC080)
#define FLEXIO1_SHIFTCFG0   (*(volatile uint32_t *)0x401AC100)
#define FLEXIO1_SHIFTBUF0   (*(volatile uint32_t *)0x401AC200)
#define FLEXIO1_TIMCTL0     (*(volatile uint32_t *)0x401AC180)
#define FLEXIO1_TIMCFG0     (*(volatile uint32_t *)0x401AC200)
#define FLEXIO1_TIMCMP0     (*(volatile uint32_t *)0x401AC280)

// FlexIO1 clock gate
#define CCM_CCGR5           (*(volatile uint32_t *)0x400FC07C)

/**
 * ShiftRegisterFlexIO - Hardware-accelerated FlexIO implementation for Teensy 4.x
 * 
 * Uses FlexIO peripheral to generate precise shift register timing with minimal CPU overhead.
 * Significantly faster and more efficient than software bit-banging.
 */
class ShiftRegisterFlexIO : public IShiftRegister {
private:
    static std::array<bool, HardwareConfig::NUM_INSTRUMENTS> m_outputEnabled;
    static bool m_initialized;
    static bool m_dirty;  // Tracks if m_outputEnabled has changed since last update
    
    // Pin configuration (from config file)
    static constexpr uint8_t PIN_DATA = PIN_SHIFTREG_Data;   // Pin 17
    static constexpr uint8_t PIN_CLOCK = PIN_SHIFTREG_Clock; // Pin 16
    static constexpr uint8_t PIN_LOAD = PIN_SHIFTREG_Load;   // Pin 15

    /**
     * Initialize FlexIO hardware for shift register operation
     */
    void initFlexIO() {
        // Enable FlexIO1 clock
        CCM_CCGR5 |= (3 << 30); // Enable FlexIO1 clock gate
        
        // Configure pins as FlexIO (ALT4 mode for these pins)
        *(volatile uint32_t *)(0x401F8000 + 0x1C4) = 0x14; // Pin 17 (GPIO_B1_00) -> FlexIO1:4
        *(volatile uint32_t *)(0x401F8000 + 0x1C0) = 0x14; // Pin 16 (GPIO_B1_01) -> FlexIO1:5
        *(volatile uint32_t *)(0x401F8000 + 0x1BC) = 0x14; // Pin 15 (GPIO_B1_02) -> FlexIO1:6
        
        // Configure pad settings for outputs
        *(volatile uint32_t *)(0x401F8000 + 0x3B4) = 0x10B0; // Pin 17 pad
        *(volatile uint32_t *)(0x401F8000 + 0x3B0) = 0x10B0; // Pin 16 pad
        *(volatile uint32_t *)(0x401F8000 + 0x3AC) = 0x10B0; // Pin 15 pad
        
        // Reset FlexIO module
        FLEXIO1_CTRL = 0x00000002; // Software reset
        delayMicroseconds(1);
        FLEXIO1_CTRL = 0x00000001; // Enable FlexIO
        
        // Configure Shifter 0 for serial output
        // SMODE=2 (Transmit), PINCFG=3 (Shifter pin output), PINSEL=4 (FlexIO1:4 = Data pin)
        FLEXIO1_SHIFTCTL0 = (2 << 0) | (3 << 16) | (4 << 8);
        
        // SSIZE=NUM_INSTRUMENTS-1 (shift count), SSTOP=0, SSTART=0
        FLEXIO1_SHIFTCFG0 = ((HardwareConfig::NUM_INSTRUMENTS - 1) << 16);
        
        // Configure Timer 0 for clock generation
        // TMODE=1 (Dual 8-bit counters baud mode), PINCFG=3 (Timer pin output)
        // PINSEL=5 (FlexIO1:5 = Clock pin)
        FLEXIO1_TIMCTL0 = (1 << 0) | (3 << 16) | (5 << 8);
        
        // Configure timer for clock timing
        // TIMCMP holds both baud divisor and bit count
        FLEXIO1_TIMCMP0 = ((HardwareConfig::NUM_INSTRUMENTS * 2 - 1) << 8) | 0x0F;
        
        // Set Load pin as regular GPIO output (handled separately)
        pinMode(PIN_LOAD, OUTPUT);
        digitalWrite(PIN_LOAD, LOW);
    }

    /**
     * Wait for FlexIO shifter to complete
     */
    void waitShifterComplete() {
        // Wait for shifter flag (bit 0 of SHIFTSTAT)
        while (!(*(volatile uint32_t *)0x401AC020 & 0x01)) {
            // Busy wait
        }
        // Clear flag
        *(volatile uint32_t *)0x401AC020 = 0x01;
    }

public:
    /**
     * Initialize shift register pins and hardware state
     */
    void init() override {
        if (m_initialized) return;
        
        // Initialize FlexIO hardware
        initFlexIO();
        
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
     * Update shift register hardware using FlexIO
     * Only performs update if state has changed (dirty flag optimization)
     * 
     * Uses FlexIO to shift out data much faster than software bit-banging
     * For 74HC595-style shift registers: MSB first, so highest instrument 
     * index ends up at Q7, instrument 0 ends up at Q0
     */
    void update() override {
        // Skip update if nothing has changed
        if (!m_dirty) return;
        
        // Build data word to shift out (MSB first)
        uint32_t shiftData = 0;
        for (int8_t i = HardwareConfig::NUM_INSTRUMENTS - 1; i >= 0; i--) {
            shiftData <<= 1;
            // Note: Using ! for active-low enable logic (LOW = enabled)
            // If your drivers are active-high, remove the !
            shiftData |= (!m_outputEnabled[i]) ? 1 : 0;
        }
        
        // Write data to FlexIO shift buffer (triggers automatic shifting)
        FLEXIO1_SHIFTBUF0 = shiftData;
        
        // Wait for shift operation to complete
        waitShifterComplete();
        
        // Toggle Load pin to latch data to outputs
        digitalWriteFast(PIN_LOAD, HIGH);
        delayNanoseconds(SHIFTREG_HOLDTIME_NS);
        digitalWriteFast(PIN_LOAD, LOW);
        
        // Clear dirty flag after successful update
        m_dirty = false;
    }
};

// Static member initialization
inline bool ShiftRegisterFlexIO::m_initialized = false;
inline bool ShiftRegisterFlexIO::m_dirty = false;
inline std::array<bool, HardwareConfig::NUM_INSTRUMENTS> ShiftRegisterFlexIO::m_outputEnabled = {};

#endif // PLATFORM_TEENSY41
