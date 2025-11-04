/*
 * Teensy41_HwPWM.h
 *
 * Configuration for Teensy 4.1 PWM-based instrument with 12 channels
 * Supports hardware PWM output for audio synthesis
 */

#pragma once
#include <array>
 
//---------- Device Configuration ----------

    #define DEVICE_NAME "Teensy 4.1 HW PWM"
    #define DEVICE_ID 0x0041

    #define NUM_INSTRUMENTS_VALUE 12
    #define NUM_SUBINSTRUMENTS_VALUE 1

    #define MIN_NOTE_VALUE 0
    #define MAX_NOTE_VALUE 127

//---------- Hardware Configuration ----------

    // Teensy 4.1 FlexPWM Timer Mapping (12 independent timers, no conflicts):
    // FlexPWM1_SM0: Pin 6  | FlexPWM1_SM1: Pin 8  | FlexPWM1_SM2: Pin 10
    // FlexPWM2_SM0: Pin 4  | FlexPWM2_SM1: Pin 5  | FlexPWM2_SM2: Pin 9
    // FlexPWM3_SM0: Pin 14 | FlexPWM3_SM1: Pin 18 | FlexPWM3_SM2: Pin 22
    // FlexPWM4_SM0: Pin 2  | FlexPWM4_SM1: Pin 3  | FlexPWM4_SM2: Pin 28
    // Each pin uses a unique timer/submodule combination for independent frequency control
    constexpr std::array<uint8_t, 12> INSTRUMENT_PINS = {2, 3, 4, 5, 6, 8, 9, 10, 14, 18, 22, 28};

    #define INSTRUMENT_TIMEOUT_MS_VALUE 10000

//---------- Uncomment Your Selected Instrument Type ----------

    #define INSTRUMENT_TYPE_VALUE "Instruments/Base/HwPWM/HwPWM.h"
 
//---------- Uncomment Your Selected COM Types ----------

    #define MMM_NETWORK_SERIAL
    //#define MMM_NETWORK_USB
    //#define MMM_NETWORK_DIN

    #ifdef MMM_NETWORK_SERIAL
        #define MMM_NETWORK_SERIAL_BAUD 115200 // Standard baud rate
    #endif

//---------- Uncomment Your Desired Extras ----------

    // Local storage for device configuration persistence
    #define EXTRA_LOCAL_STORAGE
    
    // Addressable LED lighting effects
    //#define EXTRA_ADDRESSABLE_LEDS

    // FastLED Variables
    #ifdef EXTRA_ADDRESSABLE_LEDS
        #define LED_PIN     14 // Teensy pin for WS2811 data
        #define NUM_LEDS    64 // Number of LEDs in strip
        #define BRIGHTNESS  255 // Maximum brightness (0-255)
        #define LED_TYPE    WS2811
        #define COLOR_ORDER GRB
        #define UPDATES_PER_SECOND 10 // LED refresh rate
    #endif
