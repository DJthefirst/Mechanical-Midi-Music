#include "InterruptTimer.h"

#ifdef ARDUINO_ARCH_AVR
#include <TimerOne.h>
#endif

#if defined(ARDUINO_ARCH_ESP32) || defined(PLATFORM_TEENSY41)
#include <Arduino.h>
#endif

#if defined(PLATFORM_TEENSY41)
#include <IntervalTimer.h>
#endif

// Single callback storage; the trampoline ISR calls this if non-null.
static void (*s_callback)() = nullptr;

// Timer object (ESP32) kept around so repeated initialize() calls reuse it.
#ifdef ARDUINO_ARCH_ESP32
static hw_timer_t* s_hw_timer = nullptr;
// Place the trampoline in IRAM to be safe for ISR on ESP32
void IRAM_ATTR InterruptTimer_trampoline() {
    if (s_callback) s_callback();
}
#else
// AVR trampoline
void InterruptTimer_trampoline() {
    if (s_callback) s_callback();
}
#endif

void InterruptTimer::initialize(uint32_t microseconds, void (*isr)()) {
    // Set the currently active callback to the provided isr (may be nullptr)
    s_callback = isr;

#ifdef ARDUINO_ARCH_AVR
    Timer1.initialize(microseconds);
    Timer1.attachInterrupt(InterruptTimer_trampoline);
#elif defined(ARDUINO_ARCH_ESP32)
    // ESP32 limited to ~8us interrupts
    if (microseconds < 8) {
        microseconds = 8;
    }

    // Create the hardware timer once and reuse it for subsequent calls.
    if (s_hw_timer == nullptr) {
        s_hw_timer = timerBegin(0, 80, true);
        // Attach our trampoline which will dispatch to the current s_callback
        timerAttachInterrupt(s_hw_timer, InterruptTimer_trampoline, true);
    }

    timerAlarmWrite(s_hw_timer, microseconds, true);
    timerAlarmEnable(s_hw_timer);

#elif defined(PLATFORM_TEENSY41)
    // Teensy IntervalTimer uses microseconds directly
    static IntervalTimer s_intervalTimer;
    // If already attached, end it first
    s_intervalTimer.end();
    s_intervalTimer.begin(InterruptTimer_trampoline, microseconds);
#endif
}

void InterruptTimer::setCallback(void (*isr)()) {
    s_callback = isr;
}