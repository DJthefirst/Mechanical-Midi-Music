/*
 * InterruptTimer.h
 * Attempt at making a high-precision timer that's relatively platform-agnostic
 */
#pragma once

class InterruptTimer {
public:
    static void initialize(unsigned long microseconds, void (*isr)());
};
