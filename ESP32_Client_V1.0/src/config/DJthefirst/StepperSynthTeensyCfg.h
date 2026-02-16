/*
 * Teensy41_HwPWM.h
 *
 * Configuration for Teensy 4.1 PWM-based instrument with 12 channels
 * Supports hardware PWM output for audio synthesis
 */

#pragma once
#include <array>
#include <cstdint>
 
//---------- Device Configuration ----------
    #define DEVICE_NAME "Teensy 4.1 HW PWM"
    #define DEVICE_ID 0x0041

    #define NUM_INSTRUMENTS_VALUE 10
    #define NUM_SUBINSTRUMENTS_VALUE 1

    #define MIN_NOTE_VALUE 0
    #define MAX_NOTE_VALUE 127

//---------- Hardware Configuration ----------
namespace HardwareConfig {

    #define INSTRUMENT_TIMEOUT_MS_VALUE 10000

    #define COMPONENT_PWM
        #define TIMER_RESOLUTION_US_VALUE 1
        constexpr std::array<uint8_t, 10> PINS_INSTRUMENT_PWM = {23,22,28,6,7,24,3,4,5,36};
        // constexpr std::array<uint8_t, 10> PINS_INSTRUMENT_PWM = {2, 3, 4, 5, 6, 7, 8, 9, 10, 12};

    #define COMPONENT_SHIFTREG_74HC595
        //#define SHIFTREG_USE_FLEXIO         // DISABLED: FlexIO implementation needs debugging - Comment out to use software bit-banging instead
        #define SHIFTREG_HOLDTIME_NS 25
        constexpr uint8_t PIN_SHIFTREG_Data = 17;
        constexpr uint8_t PIN_SHIFTREG_Clock = 16;
        constexpr uint8_t PIN_SHIFTREG_Load = 15;

    #define PIN_LED_Data 13
}
//---------- Uncomment Your Selected Instrument Type ----------

    #define INSTRUMENT_TYPE_VALUE "Instruments/DJthefirst/StepperSynthSw.h"
 
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
    #define EXTRA_ADDRESSABLE_LEDS

    // FastLED Variables
    #ifdef EXTRA_ADDRESSABLE_LEDS
        #define LED_PIN     13 // Teensy pin for WS2811 data
        #define NUM_LEDS    64 // Number of LEDs in strip
        #define BRIGHTNESS  255 // Maximum brightness (0-255)
        #define LED_TYPE    WS2811
        #define COLOR_ORDER GRB
        #define UPDATES_PER_SECOND 10 // LED refresh rate
    #endif
