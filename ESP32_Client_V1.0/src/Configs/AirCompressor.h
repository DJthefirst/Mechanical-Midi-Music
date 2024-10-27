/*
 * AirCompressor.h
 *
 * This is an instrument uses PWM signals to switch
 * solenoids up to 1.2khz chopping air into sound.
 * 
 */

#pragma once
#include <array>

//---------- Device Configuration ----------

    //Instrument type
    constexpr Instrument INSTRUMENT_TYPE = Instrument::ShiftRegister;
    //Platform type
    constexpr Platform PLATFORM_TYPE = Platform::_ESP32;
    //Absolute max number of Polyphonic notes is 16
    constexpr uint8_t MAX_POLYPHONIC_NOTES = 1;
    //Absolute max number of Instruments is 32
    constexpr uint8_t MAX_NUM_INSTRUMENTS = 8; 
    //Absolute Lowest Note Min=0
    constexpr uint8_t MIN_MIDI_NOTE = 36; 
    //Absolute Highest Note Max=127
    constexpr uint8_t MAX_MIDI_NOTE = 83;
    //A 14 bit number Representing this Devices ID
    constexpr uint16_t SYSEX_DEV_ID = 0x001;
    //Firmware Version 14bit
    constexpr uint16_t FIRMWARE_VERSION = 04;
    //Pinout
    constexpr std::array<uint8_t,8> pins = {2,4,18,19,21,22,23,32};

    //Interupt frequency. A smaller resolution produces more accurate notes but leads to instability.
    constexpr int TIMER_RESOLUTION = 8; //40 Default

    #include "Networks/NetworkUDP.h"
    #include "Networks/NetworkUSB.h"
    #include "Networks/NetworkDIN.h"

    #include "Instruments/Default/PwmDriver.h"


//---------- Uncomment Your Selected COM Type ----------

    using networkType = NetworkUSB;
    // using networkType = NetworkUDP;
    // using networkType = NetworkDIN;

//---------- Uncomment Your Selected Instrument Type ----------
    
    using instrumentType = PwmDriver;

//---------- Uncomment Your Desired Extras ----------

    //Save Configuration On PowerOff
    #define EXTRA_LOCAL_STORAGE

    //Addressable LED Lighting effects
    #define EXTRA_ADDRESSABLE_LEDS

    //FAST LED Variables
    #ifdef EXTRA_ADDRESSABLE_LEDS
      #define LED_PIN     18
      #define NUM_LEDS    8
      #define BRIGHTNESS  255
      #define LED_TYPE    WS2811
      #define COLOR_ORDER GRB
      #define UPDATES_PER_SECOND 100
    #endif