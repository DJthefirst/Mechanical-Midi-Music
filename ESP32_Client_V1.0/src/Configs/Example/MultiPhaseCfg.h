/*
 * ESP32_PWM.h
 *
 * Configuration for ESP32 PWM-based instrument with 12 channels
 * Supports hardware PWM output for audio synthesis
 */

#pragma once
#include <array>
#include <cstdint>

//---------- Device Configuration ----------

    #define DEVICE_NAME "ESP32 MultiPhase"
    #define DEVICE_ID 0x0047

    //Max Number of Instrument groups is 32
    #define NUM_INSTRUMENTS_VALUE 2
    //Multiplies Instrument groups into individual instruments
    #define NUM_SUBINSTRUMENTS_VALUE 1

    //Absolute Lowest Note Min=0
    #define MIN_NOTE_VALUE 0
    //Absolute Highest Note Max=127
    #define MAX_NOTE_VALUE 127

//---------- Hardware Configuration ----------
namespace HardwareConfig {
    #define INSTRUMENT_TIMEOUT_MS_VALUE 10000

    #define COMPONENT_MultiPhase
        //Min Resolutions, ESP32: 8us, Teensy4.1: 1us
        #define TIMER_RESOLUTION_US_VALUE 8 // Timer resolution in microseconds
        #define PWM_NOTES_DOUBLE
        #define ENABLE_PIN true
        #define WAVE_TABLE_OUTPUTS 3
        #define WAVE_TABLE_STEPS 3
        #define STEP_MAX_POS 0 // 76 Max for 8in Floppy Drives. 0 for continous.
        constexpr bool WAVE_TABLE[WAVE_TABLE_STEPS][WAVE_TABLE_OUTPUTS] {
            {1,0,0},
            {0,1,0},
            {0,0,1},
        };
        constexpr std::array<uint8_t, 10> PINS_INSTRUMENT_PWM = {2, 4, 18, 19, 21, 22, 23, 33}; // Enable?, Out1, ... ,Out(WAVE_TABLE_OUTPUTS) X Instruments
}

//---------- Uncomment Your Selected Instrument Type ----------

    #define INSTRUMENT_TYPE_VALUE "Instruments/Base/MultiPhase/MultiPhase.h"
 
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
    //#define EXTRA_ADDRESSABLE_LEDS

    // FastLED Variables
    #ifdef EXTRA_ADDRESSABLE_LEDS
        #define LED_PIN     16 // ESP32 pin for WS2811 data
        #define NUM_LEDS    64 // Number of LEDs in strip
        #define BRIGHTNESS  255 // Maximum brightness (0-255)
        #define LED_TYPE    WS2811
        #define COLOR_ORDER GRB
        #define UPDATES_PER_SECOND 10 // LED refresh rate
    #endif