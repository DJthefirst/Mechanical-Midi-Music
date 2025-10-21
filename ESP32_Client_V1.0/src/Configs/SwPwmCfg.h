/*
 * ESP32_PWM.h
 *
 * Configuration for ESP32 PWM-based instrument with 12 channels
 * Supports hardware PWM output for audio synthesis
 */

#pragma once
#include <array>

//---------- Device Configuration ----------

    #define DEVICE_NAME "ESP32 SW PWM"
    #define DEVICE_ID 0x0042

    // Maximum number of polyphonic notes (absolute max is 16)
    constexpr uint8_t MAX_POLYPHONIC_NOTES = 1;

    // Maximum number of instrument groups (max is 32)
    constexpr uint8_t NUM_INSTRUMENTS = 8;
    // Multiplies instrument groups into individual instruments
    constexpr uint8_t NUM_SUBINSTRUMENTS = 1;

    // Define MAX_NUM_INSTRUMENTS as a preprocessor macro for use in array sizing
    // This must be a macro (not constexpr) to avoid circular dependency issues
    #define CONFIG_MAX_NUM_INSTRUMENTS (8 * 1)  // NUM_INSTRUMENTS * NUM_SUBINSTRUMENTS
    
    // MIDI note range (0-127)
    constexpr uint8_t MIN_MIDI_NOTE = 0; 
    constexpr uint8_t MAX_MIDI_NOTE = 127;

//---------- Hardware Configuration ----------

    constexpr std::array<uint8_t,10> INSTRUMENT_PINS = {33,32,19,21,22,23,2,4,16,13};

    #define TIMER_RESOLUTION_US_VALUE 8 // Timer resolution in milliseconds
    #define INSTRUMENT_TIMEOUT_MS_VALUE 10000

//---------- Uncomment Your Selected Instrument Type ----------

    #define INSTRUMENT_TYPE_VALUE "Instruments/Base/SwPWM/SwPWM.h"
 
//---------- Uncomment Your Selected COM Types ----------

    #define MMM_NETWORK_SERIAL
    //#define MMM_NETWORK_UDP
    //#define MMM_NETWORK_USB
    //#define MMM_NETWORK_DIN

    #ifdef MMM_NETWORK_SERIAL
        #define MMM_NETWORK_SERIAL_BAUD 115200 // Standard MIDI baud rate
    #endif

    #ifdef MMM_NETWORK_UDP
        #define MMM_NETWORK_UDP_AsyncWebServer_PORT 80 // Default AsyncWebServer port

        #define MMM_NETWORK_UDP_ADDRESS 224, 5, 6, 7 // Default local network IP
        #define MMM_NETWORK_UDP_OTA_PORT 8337 // Default UDP port for MIDI
    #endif

//---------- Uncomment Your Desired Extras ----------

    // Save configuration on power off
    #define EXTRA_LOCAL_STORAGE

    // Addressable LED lighting effects
    #define EXTRA_ADDRESSABLE_LEDS

    // FastLED Variables
    #ifdef EXTRA_ADDRESSABLE_LEDS
        #define LED_PIN     18 // ESP32 pin for WS2811 data
        #define NUM_LEDS    64 // Number of LEDs in strip
        #define BRIGHTNESS  255 // Maximum brightness (0-255)
        #define LED_TYPE    WS2811
        #define COLOR_ORDER GRB
        #define UPDATES_PER_SECOND 10 // LED refresh rate
    #endif