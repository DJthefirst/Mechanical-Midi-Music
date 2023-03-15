/*
 * InterruptTimer.h
 * Attempt at making a high-precision timer that's relatively platform-agnostic
 */
#pragma once

#include <cstdint>
using std::int8_t;


namespace InterruptTimer {
    void initialize(uint32_t microseconds, void (*isr)());
};