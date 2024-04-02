/*
 * ExampleConfig.h
 *
 * This is a Template Configuration file where you can
 * quickly configure the defualt settings for your Device
 * 
 */

#pragma once
#include <vector>
#include "Constants.h"

//---------- Uncomment Your Selected Instrument Type ----------

    //#define DEVICE_TYPE DrumSimple
    //#define DEVICE_TYPE FloppyDrive
    #define DEVICE_TYPE PwmDriver  
    //#define DEVICE_TYPE StepperL298n
    //#define DEVICE_TYPE ShiftRegister
    //#define DEVICE_TYPE Dulcimer

//---------- Uncomment Your Selected COM Type ----------

    #define NETWORK_TYPE NetworkUSB
    //#define NETWORK_TYPE NetworkUDP //Not Yet Implemented
    //#define NETWORK_TYPE NetworkDIN //Not Yet Implemented

//---------- Uncomment Your Desired Settings ----------

    //Save Configuration On PowerOff
    #define LOCAL_STORAGE

    //Addressable LED Lighting effects
    //#define ADDRESSABLE_LEDS

    //FAST LED Variables
    #ifdef ADDRESSABLE_LEDS
        #define LED_PIN     33
        #define NUM_LEDS    8
        #define BRIGHTNESS  255
        #define LED_TYPE    WS2811
        #define COLOR_ORDER GRB
        #define UPDATES_PER_SECOND 100
    #endif
  

//Interupt frequency. A smaller resolution produces more accurate notes but leads to instability.
    constexpr int TIMER_RESOLUTION = 8; //40 Default

//---------- Device Configuration ----------

    //Instrument type
    const static uint8_t INSTRUMENT_TYPE = INSTRUMENT_PWM;
    //Platform type
    const static uint8_t PLATFORM_TYPE = PLATFORM_ESP32;
    //Absolute max number of Polyphonic notes is 16
    const static uint8_t MAX_POLYPHONIC_NOTES = 1;
    //Absolute max number of Instruments is 32
    const static uint8_t MAX_NUM_INSTRUMENTS = 8; 
    //Absolute Lowest Note Min=0
    const static uint8_t MIN_MIDI_NOTE = 36; 
    //Absolute Highest Note Max=127
    const static uint8_t MAX_MIDI_NOTE = 83;
    //A 14 bit number Representing this Devices ID
    const static uint16_t SYSEX_DEV_ID = 0x001;
    //Firmware Version 14bit
    const static uint16_t FIRMWARE_VERSION = 04;
    //Pinout
    const std::array<uint8_t,8> pins = {2,4,18,19,21,22,23,32};