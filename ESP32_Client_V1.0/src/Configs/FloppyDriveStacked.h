/*
 * FloppySynth.h
 *
 * This is an instrument with an array of Floppy Drives
 * Including 3.5in and 5in Drives. Also HDDs
 * 
 */

 #pragma once
 #include <array>
 
 //---------- Device Configuration ----------
 
     //Instrument type
     constexpr Instrument INSTRUMENT_TYPE = Instrument::FloppyDrive;
     //Platform type
     constexpr Platform PLATFORM_TYPE = Platform::_ESP32;
     //Absolute max number of Polyphonic notes is 16
     constexpr uint8_t MAX_POLYPHONIC_NOTES = 1;
 
     //Max Number of Instrument groups is 32
     constexpr uint8_t NUM_INSTRUMENTS = 2; 
     //Multiplies Instrument groups into individual instruments
     constexpr uint8_t NUM_SUBINSTRUMENTS = 3; 
     //Absolute max number of Instruments is 128???
     constexpr uint8_t MAX_NUM_INSTRUMENTS = NUM_INSTRUMENTS * NUM_SUBINSTRUMENTS; 
 
     //Absolute Lowest Note Min=0
     constexpr uint8_t MIN_MIDI_NOTE = 36; 
     //Absolute Highest Note Max=127
     constexpr uint8_t MAX_MIDI_NOTE = 83;
     //A 14 bit number Representing this Devices ID
     constexpr uint16_t SYSEX_DEV_ID = 0x001;
     //Firmware Version 14bit
     constexpr uint16_t FIRMWARE_VERSION = 04;
     //Pinout
     constexpr std::array<uint8_t,14> pins = {25,27,26,18,33,32,19,21,22,23,2,4,16,13};
 
     //Interupt frequency. A smaller resolution produces more accurate notes but leads to instability.
     constexpr int TIMER_RESOLUTION = 8; //40 Default
 
     #include "Networks/NetworkSerial.h"
     #include "Networks/NetworkUSB.h"
     #include "Networks/NetworkUDP.h"
     #include "Networks/NetworkDIN.h"
 
     #include "Instruments/Default/FloppyDriveStacked.h"
 
 //---------- Uncomment Your Selected COM Type ----------
 
     using networkType = NetworkSerial;
     // using networkType = NetworkUSB;
     // using networkType = NetworkUDP;
     // using networkType = NetworkDIN;
 
     #define MMM_NETWORK_SERIAL
 
 //---------- Uncomment Your Selected Instrument Type ----------
     
     using instrumentType = FloppyDriveStacked;
 
 //---------- Uncomment Your Desired Extras ----------
 
     //Save Configuration On PowerOff
     #define EXTRA_LOCAL_STORAGE
 
     //Addressable LED Lighting effects
     #define EXTRA_ADDRESSABLE_LEDS
 
     //FAST LED Variables
     #ifdef EXTRA_ADDRESSABLE_LEDS
         #define LED_PIN     18
         #define NUM_LEDS    64
         #define BRIGHTNESS  255
         #define LED_TYPE    WS2811
         #define COLOR_ORDER GRB
         #define UPDATES_PER_SECOND 10
     #endif