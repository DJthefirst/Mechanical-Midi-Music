/*
 * ESP32_PWM.h
 *
 * Configuration for ESP32 PWM-based instrument with 12 channels
 * Supports hardware PWM output for audio synthesis
 */

#pragma once
 
// Device Configuration
#define DEVICE_NAME "ESP32 PWM"
#define DEVICE_ID 0x0042

// Maximum number of polyphonic notes (absolute max is 16)
constexpr uint8_t MAX_POLYPHONIC_NOTES = 1;

// Maximum number of instrument groups (max is 32)
constexpr uint8_t NUM_INSTRUMENTS = 12;
// Multiplies instrument groups into individual instruments
constexpr uint8_t NUM_SUBINSTRUMENTS = 1; 

// MIDI note range (0-127)
constexpr uint8_t MIN_MIDI_NOTE = 0; 
constexpr uint8_t MAX_MIDI_NOTE = 127;

// Hardware Configuration
constexpr std::array<uint8_t, 12> INSTRUMENT_PINS = {2, 4, 18, 19, 21, 22, 23, 32, 33, 25, 26, 27}; // ESP32 PWM-capable pins
 
// Network Configuration - Uncomment desired communication types
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

// Instrument Type Configuration
#include "Instruments/Utility/PWM/PwmBase.h"
using InstrumentType = PwmBase;

// Optional Features - Uncomment desired extras
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