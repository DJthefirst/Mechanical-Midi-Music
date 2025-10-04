/*
 * InterruptTimer.h - V2.0 Platform-Abstracted Timer System
 *
 * Part of the Mechanical MIDI Music V2.0 redesign
 * Based on REDESIGN_PART4_INSTRUMENTS.md and REDESIGN_PART6_IMPLEMENTATION.md
 *
 * Provides a unified interface for hardware timer interrupts across platforms
 */

#pragma once

#include "core/platform/PlatformInterface.h"
#include <functional>
#include <cstdint>

#ifdef ARDUINO_ARCH_ESP32
    // Use Arduino ESP32 timer functions (compatible with framework 3.20017)
    #include <esp32-hal-timer.h>
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////
// Interrupt Timer Interface
////////////////////////////////////////////////////////////////////////////////////////////////////

class InterruptTimer {
private:
    static std::function<void()> s_callback;
    static uint32_t s_resolutionMicros;
    static bool s_initialized;
    static bool s_running;
    
    #ifdef ARDUINO_ARCH_ESP32
        static hw_timer_t* s_timer;
    #elif defined(ARDUINO_AVR_UNO)
        // Arduino timer will be implemented if needed
    #endif

public:
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    // Timer Management
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    
    // Initialize timer with callback and resolution
    static bool initialize(uint32_t resolutionMicros, std::function<void()> callback);
    
    // Start the timer
    static void start();
    
    // Stop the timer
    static void stop();
    
    // Check if timer is running
    static bool isRunning() { return s_running; }
    
    // Check if timer is initialized
    static bool isInitialized() { return s_initialized; }
    
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    // Configuration
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    
    // Change timer resolution (stops timer if running)
    static bool setResolution(uint32_t resolutionMicros);
    
    // Get current resolution in microseconds
    static uint32_t getResolution() { return s_resolutionMicros; }
    
    // Change callback function (keeps timer running if it was running)
    static void setCallback(std::function<void()> callback);
    
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    // Diagnostics
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    
    // Print timer status
    // static void printStatus();
    
    // Get timer frequency in Hz
    static float getFrequency() { 
        return s_resolutionMicros > 0 ? 1000000.0f / s_resolutionMicros : 0.0f; 
    }

private:
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    // Platform-Specific Implementation
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    
    #ifdef ARDUINO_ARCH_ESP32
        static void IRAM_ATTR timerISR();
        static bool initializeESP32();
    #elif defined(ARDUINO_AVR_UNO)
        static void setupTimer1();
        // ISR will be defined in .cpp file
    #endif
    
    // Validate resolution
    static bool isValidResolution(uint32_t resolutionMicros);
};