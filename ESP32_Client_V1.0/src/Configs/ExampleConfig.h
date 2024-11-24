/*
 * ExampleConfig.h
 *
 * This is a Template Configuration file where you can
 * quickly configure the defualt settings for your Device
 * 
 */

#pragma once
#include <array>

//---------- Device Configuration ----------

    //Instrument type
    constexpr Instrument INSTRUMENT_TYPE = Instrument::PWM;
    //Platform type
    constexpr Platform PLATFORM_TYPE = Platform::_ESP32;
    //Absolute max number of Polyphonic notes is 16
    constexpr uint8_t MAX_POLYPHONIC_NOTES = 1;
    //Absolute max number of Instruments is 32
    constexpr uint8_t MAX_NUM_INSTRUMENTS = 8; 
    //Absolute Lowest Note Min=0
    constexpr uint8_t MIN_MIDI_NOTE = 0; 
    //Absolute Highest Note Max=127
    constexpr uint8_t MAX_MIDI_NOTE = 127;
    //A 14 bit number Representing this Devices ID
    constexpr uint16_t SYSEX_DEV_ID = 0x001;
    //Firmware Version 14bit
    constexpr uint16_t FIRMWARE_VERSION = 04;
    //Pinout
    constexpr std::array<uint8_t,8> pins = {2,4,18,19,21,22,23,32};

    //Interupt frequency. A smaller resolution produces more accurate notes but leads to instability.
    constexpr int TIMER_RESOLUTION = 8; //40 Default

    #include "Networks/NetworkSerial.h"
    #include "Networks/NetworkUSB.h"
    #include "Networks/NetworkUDP.h"
    #include "Networks/NetworkDIN.h"

    #include "Instruments/Default/PwmDriver.h"
    #include "Instruments/Default/FloppyDrive.h"
    #include "Instruments/Default/InstrAD9833.h"
    #include "Instruments/Default/Pwm8A04.h"
    #include "Instruments/Default/ShiftRegister.h"
    //#include "Instruments/Default/StepperA4988.h"
    //#include "Instruments/Default/StepperL298n.h"

//---------- Uncomment Your Selected COM Type ----------

    using networkType = NetworkSerial;
    // using networkType = NetworkUSB;
    // using networkType = NetworkUDP;
    // using networkType = NetworkDIN;

    #define MMM_NETWORK_SERIAL

//---------- Uncomment Your Selected Instrument Type ----------

    using instrumentType = PwmDriver;
    // using instrumentType = DrumSimple;
    // using instrumentType = FloppyDrive;
    // using instrumentType = StepperL298n;
    // using instrumentType = ShiftRegister;
    // using instrumentType = Dulcimer;

//---------- Uncomment Your Desired Extras ----------

    //Save Configuration On PowerOff
    #define EXTRA_LOCAL_STORAGE

    //Addressable LED Lighting effects
    #define EXTRA_ADDRESSABLE_LEDS

    //FAST LED Variables
    #ifdef EXTRA_ADDRESSABLE_LEDS
        #define LED_PIN     33
        #define NUM_LEDS    8
        #define BRIGHTNESS  255
        #define LED_TYPE    WS2811
        #define COLOR_ORDER GRB
        #define UPDATES_PER_SECOND 100
    #endif