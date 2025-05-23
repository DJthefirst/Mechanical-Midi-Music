/*
 * TestInstrument.h
 *
 * This Configuration reprents an ESP32 connected
 * with 8 buzzers used for tesing & validation.
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

    //Max Number of Instrument groups is 32
    constexpr uint8_t NUM_INSTRUMENTS = 8; 
    //Multiplies Instrument groups into individual instruments
    constexpr uint8_t NUM_SUBINSTRUMENTS = 1; 
    //Absolute max number of Instruments is 128???
    constexpr uint8_t MAX_NUM_INSTRUMENTS = NUM_INSTRUMENTS * NUM_SUBINSTRUMENTS; 

    //Absolute Lowest Note Min=0
    constexpr uint8_t MIN_MIDI_NOTE = 0; 
    //Absolute Highest Note Max=127
    constexpr uint8_t MAX_MIDI_NOTE = 127;
    //A 14 bit number Representing this Devices ID
    constexpr uint16_t SYSEX_DEV_ID = 0x001;
    //Firmware Version 14bit
    constexpr uint16_t FIRMWARE_VERSION = 04;
    //Pinout
    constexpr std::array<uint8_t,8> pins = {4,5,6,7,15,16,17,18};

        //Interupt frequency. A smaller resolution produces more accurate notes but leads to instability.
    constexpr int TIMER_RESOLUTION = 8; //40 Default

    #include "Networks/NetworkUDP.h"
    #include "Networks/NetworkUSB.h"
    #include "Networks/NetworkDIN.h"
    #include "Networks/NetworkRTP.h"
    #include "Networks/NetworkBLE.h"

    #include "Instruments/Default/PwmDriver.h"
    #include "Instruments/Default/FloppyDrive.h"
    #include "Instruments/Default/ShiftRegister.h"
    #include "Instruments/DJthefirst/DrumSimple.h"
    #include "Instruments/DJthefirst/Dulcimer.h"


//---------- Uncomment Your Selected COM Type ----------

    // using networkType = NetworkSerial;
    // using networkType = NetworkUSB;
    // using networkType = NetworkRTP;
    using networkType = NetworkBLE;
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