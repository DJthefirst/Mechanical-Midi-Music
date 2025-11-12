/*
 * ShiftRegister.h
 * 
 * Platform-independent shift register control utility with automatic
 * implementation selection based on configuration.
 * 
 * Selects appropriate platform-specific implementation at compile time:
 * - Teensy 4.x with SHIFTREG_USE_FLEXIO: Hardware-accelerated FlexIO
 * - All other platforms: Software bit-banging
 * 
 * Usage:
 *   1. Call ShiftRegister::init() once during setup
 *   2. Use ShiftRegister::setOutputEnabled() to enable/disable individual outputs
 *   3. Call ShiftRegister::update() to push changes to hardware (only updates if changed)
 */

#pragma once

#include "Config.h"

#ifdef SHIFTREG_TYPE_74HC595

// Platform-specific includes - follows same pattern as SwPWM.h
#if defined(PLATFORM_TEENSY41) && defined(SHIFTREG_USE_FLEXIO)
    // Teensy 4.x with FlexIO hardware acceleration
    #include "Instruments/Components/ShiftRegister/ShiftRegisterFlexIO.h"
    #define ShiftRegister ShiftRegisterFlexIO
#else
    // Default: Software bit-banging (works on all platforms)
    #include "Instruments/Components/ShiftRegister/ShiftRegisterSoftware.h"
    #define ShiftRegister ShiftRegisterSoftware
#endif

#endif // SHIFTREG_TYPE_74HC595
