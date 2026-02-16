#pragma once
#include <array>
#include <cstdint>

//---------- Device Configuration ----------

    #define DEVICE_NAME "Step SW Shift"
    #define DEVICE_ID 0x0037

    //Max Number of Instrument groups is 32
    #define NUM_INSTRUMENTS_VALUE 8
    //Multiplies Instrument groups into individual instruments
    #define NUM_SUBINSTRUMENTS_VALUE 1

    //Absolute Lowest Note Min=0
    #define MIN_NOTE_VALUE 0
    //Absolute Highest Note Max=127
    #define MAX_NOTE_VALUE 127

//---------- Hardware Configuration ----------
namespace HardwareConfig {
    
    #define INSTRUMENT_TIMEOUT_MS_VALUE 10000

    #define COMPONENT_PWM
        //Min Resolutions, ESP32: 8us, Teensy4.1: 1us
        #define TIMER_RESOLUTION_US_VALUE 1 // Timer resolution in microseconds
        #define PWM_NOTES_DOUBLE
        constexpr std::array<uint8_t, 10> PINS_INSTRUMENT_PWM = {23,22,28,6,7,24,3,4,5,36};

    #define COMPONENT_STEP_SHIFT
        #define STEP_MIN_HEAD_POS 0
        #define STEP_MAX_HEAD_POS 150 // 79 Tracks*2 for 3.5in or 49*2 for 5.25in
    
    #define COMPONENT_SHIFTREG_74HC595
        //#define SHIFTREG_USE_FLEXIO         // DISABLED: FlexIO implementation needs debugging - Comment out to use software bit-banging instead
        #define SHIFTREG_HOLDTIME_NS 25
        constexpr uint8_t PIN_SHIFTREG_Data = 17;
        constexpr uint8_t PIN_SHIFTREG_Clock = 16;
        constexpr uint8_t PIN_SHIFTREG_Load = 15;

    #define PIN_LED_Data 13
    
}

//---------- Uncomment Your Selected Instrument Type ----------

    #define INSTRUMENT_TYPE_VALUE "Instruments/Base/StepSw/StepSw.h"
 
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
    #define CFG_EXTRA_LOCAL_STORAGE

    // Addressable LED lighting effects
    //#define CFG_EXTRA_ADDRESSABLE_LEDS

    // FastLED Variables
    #ifdef CFG_EXTRA_ADDRESSABLE_LEDS
        #define LED_PIN     13 // Teensy pin for WS2811 data
        #define NUM_LEDS    64 // Number of LEDs in strip
        #define BRIGHTNESS  255 // Maximum brightness (0-255)
        #define LED_TYPE    WS2811
        #define COLOR_ORDER GRB
        #define UPDATES_PER_SECOND 10 // LED refresh rate
    #endif