/*
 * StepperSynth.h
 *
 * This is an instrument with an array of 10 Stepper Motors
 * 
 * 
 */

#pragma once
#include <array>
#include <cstdint>

//---------- Device Configuration ----------
    #define DEVICE_NAME "ESP32 PWM"
    #define DEVICE_ID 0x0003

    //Max Number of Instrument groups is 32
    #define NUM_INSTRUMENTS_VALUE 10
    //Multiplies Instrument groups into individual instruments
    #define NUM_SUBINSTRUMENTS_VALUE 1

    //Absolute Lowest Note Min=0
    #define MIN_NOTE_VALUE 0
    //Absolute Highest Note Max=127
    #define MAX_NOTE_VALUE 127

    #define VIBRATO_ENABLED

//---------- Hardware Configuration ----------
namespace HardwareConfig {

    #define INSTRUMENT_TIMEOUT_MS_VALUE 10000

    #define COMPONENT_PWM
        #define TIMER_RESOLUTION_US_VALUE 8
        #define PWM_NOTES_DOUBLE
        constexpr std::array<uint8_t,10> PINS_INSTRUMENT_PWM = {33,32,19,21,22,23,2,4,16,13};

    #define COMPONENT_SHIFTREG_74HC595
        #define SHIFTREG_HOLDTIME_NS 25
        constexpr uint8_t PIN_SHIFTREG_Data = 25;
        constexpr uint8_t PIN_SHIFTREG_Clock = 27;
        constexpr uint8_t PIN_SHIFTREG_Load = 26;

    #define PIN_LED_Data 18
}
//---------- Uncomment Your Selected Instrument Type ----------

    #define INSTRUMENT_TYPE_VALUE "Instruments/DJthefirst/StepperSynthSw.h"

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

    //Save Configuration On PowerOff - DISABLED AS PER REQUIREMENTS
    //#define EXTRA_LOCAL_STORAGE

    //Addressable LED Lighting effects
    #define EXTRA_ADDRESSABLE_LEDS

    //FAST LED Variables
    #ifdef EXTRA_ADDRESSABLE_LEDS
        #define LED_PIN     18
        #define NUM_LEDS    50
        #define BRIGHTNESS  255
        #define LED_TYPE    WS2811
        #define COLOR_ORDER GRB
        #define UPDATES_PER_SECOND 100
    #endif