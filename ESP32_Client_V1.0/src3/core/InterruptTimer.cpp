/*
 * InterruptTimer.cpp - V2.0 Platform-Abstracted Timer Implementation
 *
 * Part of the Mechanical MIDI Music V2.0 redesign
 */

#include "InterruptTimer.h"
#include <Arduino.h>

////////////////////////////////////////////////////////////////////////////////////////////////////
// Static Member Definitions
////////////////////////////////////////////////////////////////////////////////////////////////////

std::function<void()> InterruptTimer::s_callback = nullptr;
uint32_t InterruptTimer::s_resolutionMicros = 0;
bool InterruptTimer::s_initialized = false;
bool InterruptTimer::s_running = false;

#ifdef ARDUINO_ARCH_ESP32
    hw_timer_t* InterruptTimer::s_timer = nullptr;
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////
// Public Interface Implementation
////////////////////////////////////////////////////////////////////////////////////////////////////

bool InterruptTimer::initialize(uint32_t resolutionMicros, std::function<void()> callback) {
    if (!isValidResolution(resolutionMicros)) {
        Serial.printf("ERROR: Invalid timer resolution %d μs\n", resolutionMicros);
        return false;
    }
    
    s_callback = callback;
    s_resolutionMicros = resolutionMicros;
    
    #ifdef ARDUINO_ARCH_ESP32
        return initializeESP32();
    #elif defined(ARDUINO_AVR_UNO)
        setupTimer1();
        s_initialized = true;
        return true;
    #else
        Serial.println("ERROR: Timer not implemented for this platform");
        return false;
    #endif
}

void InterruptTimer::start() {
    if (!s_initialized) {
        Serial.println("ERROR: Timer not initialized");
        return;
    }
    
    #ifdef ARDUINO_ARCH_ESP32
        if (s_timer) {
            timerAlarmEnable(s_timer);
            s_running = true;
        }
    #elif defined(ARDUINO_AVR_UNO)
        // Enable Timer1 interrupt
        TIMSK1 |= (1 << OCIE1A);
        s_running = true;
    #endif
}

void InterruptTimer::stop() {
    if (!s_initialized) return;
    
    #ifdef ARDUINO_ARCH_ESP32
        if (s_timer) {
            timerAlarmDisable(s_timer);
            s_running = false;
        }
    #elif defined(ARDUINO_AVR_UNO)
        // Disable Timer1 interrupt
        TIMSK1 &= ~(1 << OCIE1A);
        s_running = false;
    #endif
}

bool InterruptTimer::setResolution(uint32_t resolutionMicros) {
    if (!isValidResolution(resolutionMicros)) {
        return false;
    }
    
    bool wasRunning = s_running;
    if (wasRunning) {
        stop();
    }
    
    s_resolutionMicros = resolutionMicros;
    
    // Reinitialize with new resolution
    #ifdef ARDUINO_ARCH_ESP32
        if (s_timer) {
            timerAlarmWrite(s_timer, resolutionMicros, true);
        }
    #elif defined(ARDUINO_AVR_UNO)
        setupTimer1();  // Reconfigure timer
    #endif
    
    if (wasRunning) {
        start();
    }
    
    return true;
}

void InterruptTimer::setCallback(std::function<void()> callback) {
    s_callback = callback;
}

// void InterruptTimer::printStatus() {
//     Serial.println("=== Timer Status ===");
//     Serial.printf("Initialized: %s\n", s_initialized ? "Yes" : "No");
//     Serial.printf("Running: %s\n", s_running ? "Yes" : "No");
//     Serial.printf("Resolution: %d μs\n", s_resolutionMicros);
//     Serial.printf("Frequency: %.1f Hz\n", getFrequency());
//     Serial.printf("Platform: %s\n", Platform::getName());
// }

////////////////////////////////////////////////////////////////////////////////////////////////////
// Platform-Specific Implementations
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef ARDUINO_ARCH_ESP32

bool InterruptTimer::initializeESP32() {
    // ESP32 timer: 80MHz APB clock, prescaler 80 = 1MHz (1μs ticks)
    s_timer = timerBegin(0, 80, true);
    if (!s_timer) {
        Serial.println("ERROR: Failed to initialize ESP32 timer");
        return false;
    }
    
    timerAttachInterrupt(s_timer, &timerISR, true);
    timerAlarmWrite(s_timer, s_resolutionMicros, true);
    
    s_initialized = true;
    Serial.printf("ESP32 timer initialized: %d μs resolution\n", s_resolutionMicros);
    return true;
}

void IRAM_ATTR InterruptTimer::timerISR() {
    if (s_callback) {
        s_callback();
    }
}

#elif defined(ARDUINO_AVR_UNO)

void InterruptTimer::setupTimer1() {
    // Arduino Uno Timer1 setup for custom resolution
    // This is a simplified implementation
    
    noInterrupts();
    
    // Clear Timer1 registers
    TCCR1A = 0;
    TCCR1B = 0;
    TCNT1 = 0;
    
    // Calculate compare match register value
    // 16MHz clock, prescaler will be determined based on resolution
    uint32_t compareValue;
    uint8_t prescaler;
    
    if (s_resolutionMicros <= 4) {
        // No prescaler (1:1) - resolution limited to ~4μs
        prescaler = 1;
        compareValue = (16 * s_resolutionMicros) - 1;
        TCCR1B |= (1 << CS10);
    } else if (s_resolutionMicros <= 32) {
        // 8x prescaler
        prescaler = 8;
        compareValue = (2 * s_resolutionMicros) - 1;
        TCCR1B |= (1 << CS11);
    } else if (s_resolutionMicros <= 256) {
        // 64x prescaler
        prescaler = 64;
        compareValue = (s_resolutionMicros / 4) - 1;
        TCCR1B |= (1 << CS11) | (1 << CS10);
    } else {
        // 256x prescaler
        prescaler = 256;
        compareValue = (s_resolutionMicros / 16) - 1;
        TCCR1B |= (1 << CS12);
    }
    
    if (compareValue > 65535) {
        compareValue = 65535;  // Max value for 16-bit timer
    }
    
    OCR1A = compareValue;
    TCCR1B |= (1 << WGM12);  // CTC mode
    
    interrupts();
    
    Serial.printf("Arduino Timer1 initialized: %d μs resolution, prescaler %d\n", 
                  s_resolutionMicros, prescaler);
}

// Arduino Timer1 ISR
ISR(TIMER1_COMPA_vect) {
    if (InterruptTimer::s_callback) {
        InterruptTimer::s_callback();
    }
}

#endif

////////////////////////////////////////////////////////////////////////////////////////////////////
// Validation
////////////////////////////////////////////////////////////////////////////////////////////////////

bool InterruptTimer::isValidResolution(uint32_t resolutionMicros) {
    // Resolution must be at least 1μs and not more than 100ms
    if (resolutionMicros < 1 || resolutionMicros > 100000) {
        return false;
    }
    
    // Platform-specific validation
    #ifdef ARDUINO_ARCH_ESP32
        // ESP32 can handle 1μs to very large values
        return true;
    #elif defined(ARDUINO_AVR_UNO)
        // Arduino Uno Timer1 limitations
        return resolutionMicros <= 16384;  // ~16ms max with 256 prescaler
    #else
        return false;  // Unknown platform
    #endif
}