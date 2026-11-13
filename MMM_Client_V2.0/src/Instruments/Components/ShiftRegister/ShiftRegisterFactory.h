#pragma once

#include "Config.h"
#include "IShiftRegister.h"
#include <bitset>
#include <cstdint>

// Include all shift register implementations at file scope
#if defined(PLATFORM_TEENSY41)
    #include "Instruments/Components/ShiftRegister/Teensy41_SwShift.h"
    #include "Instruments/Components/ShiftRegister/Teensy41_FlexShift.h"
#endif
#include "Instruments/Components/ShiftRegister/Default_SwShift.h"

// CFG_SHIFTREGISTER_TYPE accepts DEFAULT (software bit-banging), HW_ACCEL or FLEXIO.
// The names only have values while this check runs; DEFAULT is any other value.
#define HW_ACCEL 0x5A01
#define FLEXIO 0x5A02
#if defined(PLATFORM_TEENSY41) && defined(CFG_SHIFTREGISTER_TYPE) && \
    (CFG_SHIFTREGISTER_TYPE == HW_ACCEL || CFG_SHIFTREGISTER_TYPE == FLEXIO)
    #define SHIFTREGISTER_USE_FLEXIO
#endif
#undef HW_ACCEL
#undef FLEXIO

// Remove this once Teensy41_FlexShift has an implementation
#ifdef SHIFTREGISTER_USE_FLEXIO
    #error "CFG_SHIFTREGISTER_TYPE=HW_ACCEL/FLEXIO selects Teensy41_FlexShift, which is not implemented yet. Use DEFAULT."
#endif

class ShiftRegisterFactory {
public:
    template<size_t numOutputs>
    static IShiftRegister<numOutputs>* create(uint8_t PIN_SER, uint8_t PIN_CLK, uint8_t PIN_LD, uint8_t PIN_EN, uint8_t PIN_RST) {

    // Teensy 4.x with software bit-banging
    #if defined(PLATFORM_TEENSY41) && !defined(SHIFTREGISTER_USE_FLEXIO)
        return new Teensy41_SwShift<numOutputs>(PIN_SER, PIN_CLK, PIN_LD, PIN_EN, PIN_RST);

    // Teensy 4.x with FlexIO hardware acceleration
    #elif defined(PLATFORM_TEENSY41)
        return new Teensy41_FlexShift<numOutputs>(PIN_SER, PIN_CLK, PIN_LD, PIN_EN, PIN_RST);

    // Default: Software bit-banging (works on all platforms)
    #else
        return new Default_SwShift<numOutputs>(PIN_SER, PIN_CLK, PIN_LD, PIN_EN, PIN_RST);

    #endif
    }

    template<size_t numOutputs>
    static IShiftRegister<numOutputs>* create(uint8_t PIN_SER, uint8_t PIN_CLK, uint8_t PIN_LD) {

    // Teensy 4.x with software bit-banging
    #if defined(PLATFORM_TEENSY41) && !defined(SHIFTREGISTER_USE_FLEXIO)
        return new Teensy41_SwShift<numOutputs>(PIN_SER, PIN_CLK, PIN_LD, std::nullopt, std::nullopt);

    // Teensy 4.x with FlexIO hardware acceleration
    #elif defined(PLATFORM_TEENSY41)
        return new Teensy41_FlexShift<numOutputs>(PIN_SER, PIN_CLK, PIN_LD, std::nullopt, std::nullopt);

    // Default: Software bit-banging (works on all platforms)
    #else
        return new Default_SwShift<numOutputs>(PIN_SER, PIN_CLK, PIN_LD, std::nullopt, std::nullopt);

    #endif
    }
};