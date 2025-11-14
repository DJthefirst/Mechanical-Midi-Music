/*
 * InterruptTimer.h
 * Utility class for a high-precision timer that's relatively platform-agnostic
 */
#pragma once

#include "Config.h"
#include <cstdint>
using std::int8_t;


namespace InterruptTimer {
    // Initialize the hardware timer (creates the timer if needed) and register
    // an initial callback. The timer implementation uses a trampoline ISR which
    // calls the currently-registered callback. Callers can later replace the
    // callback using setCallback().
    void initialize(uint32_t microseconds, void (*isr)());

    // Replace the currently registered periodic callback.
    void setCallback(void (*isr)());
};