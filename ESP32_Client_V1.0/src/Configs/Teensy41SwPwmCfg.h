
/*
 * Teensy41_SwPWM.h
 *
 * Configuration 0.f
 * or Teensy 4.1 Software PWM-based instrument with 16 channels
 * Supports software PWM output for audio synthesis using IntervalTimer
 */

#pragma once
#include <array>

//---------- Device Configuration ----------

    #define DEVICE_NAME "Teensy 4.1 SW PWM"
    #define DEVICE_ID 0x0043

    #define NUM_.INSTRUMENTS_VALUE 16
    #define NUM_SUBINSTRUMENTS_VALUE 1
    
    #define MIN_NOTE_VALUE 0
    #define MAX_NOTE_VALUE 127

//---------- Hardware Configuration ----------

    // Teensy 4.1 has many digital I/O pins available for software PWM
    // Using 16 pins for good coverage
    constexpr std::array<uint8_t, 16> INSTRUMENT_PINS = {
        2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 14, 15, 16, 17, 18
    };

    #define TIMER_RESOLUTION_US_VALUE 1 // Timer resolution in microseconds
    #define INSTRUMENT_TIMEOUT_MS_VALUE 10000

//---------- Uncomment Your Selected Instrument Type ----------

    #define INSTRUMENT_TYPE_VALUE "Instruments/Base/SwPWM/SwPWM.h"
 
//---------- Uncomment Your Selected COM Types ----------

    #define MMM_NETWORK_SERIAL
    //#define MMM_NETWORK_USB
    //#define MMM_NETWORK_DIN

    #ifdef MMM_NETWORK_SERIAL
        #define MMM_NETWORK_SERIAL_BAUD 115200 // Standard baud rate
    #endif

//---------- Uncomment Your Desired Extras ----------

    // Note: Local storage removed as per requirements
    // Addressable LED lighting effects
    //#define EXTRA_ADDRESSABLE_LEDS

    // FastLED Variables
    #ifdef EXTRA_ADDRESSABLE_LEDS
        #define LED_PIN     20 // Teensy pin for WS2811 data
        #define NUM_LEDS    64 // Number of LEDs in strip
        #define BRIGHTNESS  255 // Maximum brightness (0-255)
        #define LED_TYPE    WS2811
        #define COLOR_ORDER GRB
        #define UPDATES_PER_SECOND 10 // LED refresh rate
    #endif
