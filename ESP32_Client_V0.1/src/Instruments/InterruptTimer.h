/*
 * InterruptTimer.h
 * Attempt at making a high-precision timer that's relatively platform-agnostic
 */
#pragma once

#include <stdint.h>

class InterruptTimer {
public:
    static void initialize(uint32_t microseconds, void (*isr)());
};
