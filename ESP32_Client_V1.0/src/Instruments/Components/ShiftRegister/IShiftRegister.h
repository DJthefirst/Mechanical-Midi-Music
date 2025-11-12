/*
 * IShiftRegister.h
 * 
 * Interface for shift register implementations
 * Provides a common API for controlling 74HC595-style shift registers
 * across different hardware platforms and acceleration methods.
 */

#pragma once

#include "Config.h"
#include <array>
#include <cstdint>

/**
 * IShiftRegister - Abstract interface for shift register control
 * 
 * Defines the contract that all shift register implementations must follow.
 * Implementations may use software bit-banging, hardware SPI, FlexIO, or
 * other platform-specific acceleration techniques.
 */
class IShiftRegister {
public:
    virtual ~IShiftRegister() = default;

    /**
     * Initialize shift register pins and hardware state
     * Must be called once before using other functions
     */
    virtual void init() = 0;

    /**
     * Set output enable state for a specific instrument
     * @param instrument Index of the instrument (0 to NUM_INSTRUMENTS-1)
     * @param enabled true to enable output, false to disable
     */
    virtual void setOutputEnabled(uint8_t instrument, bool enabled) = 0;

    /**
     * Get output enable state for a specific instrument
     * @param instrument Index of the instrument
     * @return true if output is enabled, false otherwise
     */
    virtual bool getOutputEnabled(uint8_t instrument) = 0;

    /**
     * Disable all outputs
     */
    virtual void disableAll() = 0;

    /**
     * Update shift register hardware with current output enable states
     * Only performs hardware update if state has changed since last update
     */
    virtual void update() = 0;
};
