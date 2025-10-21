/*
 * StepperSynth.h
 *
 * This is an instrument with an array of 10 Stepper Motors
 * 
 * 
 */

#pragma once
#include <array>

//---------- Device Configuration ----------

    #define DEVICE_NAME "ESP32 PWM"
    #define DEVICE_ID 0x0003

    //Absolute max number of Polyphonic notes is 16
    constexpr uint8_t MAX_POLYPHONIC_NOTES = 1;

    //Max Number of Instrument groups is 32
    constexpr uint8_t NUM_INSTRUMENTS = 10; 
    //Multiplies Instrument groups into individual instruments
    constexpr uint8_t NUM_SUBINSTRUMENTS = 1; 

    //Absolute Lowest Note Min=0
    constexpr uint8_t MIN_MIDI_NOTE = 0; 
    //Absolute Highest Note Max=127
    constexpr uint8_t MAX_MIDI_NOTE = 127;

//---------- Hardware Configuration ----------

    constexpr std::array<uint8_t, 8> INSTRUMENT_PINS = {2, 18, 21, 23, 33, 26, 13}; // ESP32 Enable pins
    constexpr std::array<uint8_t, 8> STEP_PINS =       {4, 19, 22, 32, 25, 27, 16}; // ESP32 Direction pins

//---------- Uncomment Your Selected Instrument Type ----------

    #define INSTRUMENT_TYPE_VALUE "Instruments/DJthefirst/StepperSynth.h"

//---------- Uncomment Your Selected COM Types ----------
 
    #define MMM_NETWORK_SERIAL
    //#define MMM_NETWORK_UDP
    //#define MMM_NETWORK_USB
    //#define MMM_NETWORK_DIN

    #ifdef MMM_NETWORK_UDP
        #define MMM_NETWORK_UDP_IP 192,168,1,100
        #define MMM_NETWORK_UDP_PORT 5004
    #endif

//---------- Uncomment Your Desired Extras ----------

    //Save Configuration On PowerOff
    #define EXTRA_LOCAL_STORAGE

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