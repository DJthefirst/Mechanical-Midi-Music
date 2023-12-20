#pragma once
#include <vector>
#include "Constants.h"

//---------- Uncomment Your Selected Device Type ----------

  //#define PLATFORM_ESP32
  //#define PLATFORM_ESP8266
  //#define PLATFORM_ARDUINO_UNO
  //#define PLATFORM_ARDUINO_MEGA
  //#define PLATFORM_ARDUINO_DUE
  //#define PLATFORM_ARDUINO_MICRO
  //#define PLATFORM_ARDUINO_NANO

//---------- Uncomment Your Desired Settings ----------

  //Save Configuration On PowerOff
  #define LOCAL_STORAGE

  //Lighting effects
  //#define ADDRESSABLE_LEDS

//Interupt frequency. A smaller resolution produces more accurate notes but leads to instability.
  // constexpr int TIMER_RESOLUTION = 8; //40

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
  const static uint8_t MIN_MIDI_NOTE = 0; 
  //Absolute Highest Note Max=127
  const static uint8_t MAX_MIDI_NOTE = 127;
  //A 14 bit number Representing this Devices ID
  const static uint16_t SYSEX_DEV_ID = 0x01;
  //Firmware Version 14bit
  const static uint16_t FIRMWARE_VERSION = 01;