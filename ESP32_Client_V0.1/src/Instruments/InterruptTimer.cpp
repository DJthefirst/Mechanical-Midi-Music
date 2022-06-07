#include "InterruptTimer.h"

#ifdef ARDUINO_ARCH_AVR
#include <TimerOne.h>
#elif ARDUINO_ARCH_ESP32
#include <Arduino.h>
#endif

void InterruptTimer::initialize(unsigned long microseconds, void (*isr)()) {
#ifdef ARDUINO_ARCH_AVR
    Timer1.initialize(microseconds);
    Timer1.attachInterrupt(isr);
#elif ARDUINO_ARCH_ESP32
    timer1_isr_init();
    timer1_attachInterrupt(isr);
    timer1_enable(TIM_DIV16, TIM_EDGE, TIM_LOOP);
    timer1_write(5 * microseconds);
#endif
}