#pragma once

#include "Config.h"

#ifdef COMPONENT_SHIFTREG_74HC595

    // Platform-specific includes - follows same pattern as SwPWM.h
    #if defined(PLATFORM_TEENSY41) && defined(SHIFTREG_USE_FLEXIO)
        // Teensy 4.x with FlexIO hardware acceleration
        #include "Instruments/Components/ShiftRegister/Teensy41_FlexShift.h"
        #define ShiftRegister Teensy41_FlexShift
    #elif defined(PLATFORM_TEENSY41) && !defined(SHIFTREG_USE_FLEXIO)
        // Teensy 4.x with software bit-banging
        #include "Instruments/Components/ShiftRegister/Teensy41_SwShift.h"
        #define ShiftRegister Teensy41_SwShift
    #else
        // Default: Software bit-banging (works on all platforms)
        #include "Instruments/Components/ShiftRegister/Default_SwShift.h"
        #define ShiftRegister Default_SwShift
    #endif

#endif // COMPONENT_SHIFTREG_74HC595
