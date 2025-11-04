#include "InterruptTimer.h"

#ifdef ARDUINO_ARCH_AVR
#include <TimerOne.h>
#elif ARDUINO_ARCH_ESP32
#include <Arduino.h>
#endif

void InterruptTimer::initialize(uint32_t microseconds, void (*isr)()) {
#ifdef ARDUINO_ARCH_AVR
    Timer1.initialize(microseconds);
    Timer1.attachInterrupt(isr);
#elif ARDUINO_ARCH_ESP32
    //Esp32 limitet to 8us interrupts
    if (microseconds < 8) {
        microseconds = 8;
    }

    //Create a timer to update instruments periodicaly every one ms	
    hw_timer_t * timer = nullptr;
    timer = timerBegin(0, 80, true);
    timerAttachInterrupt(timer, isr, true);
    timerAlarmWrite(timer, microseconds, true);
    timerAlarmEnable(timer);

#endif
}