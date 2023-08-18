/*
 * Device.h
 * 
 * A Struct Representing this devices configuration
 */

#pragma once

#include "Arduino.h"
#include <stdint.h>
#include <cstdint>
using std::int8_t;


enum InstrumentType
{ 
    INSTRUMENT_PWM = 1,
    INSTRUMENT_ShiftRegister,
    INSTRUMENT_StepperMotor,
    INSTRUMENT_FloppyDrive
};

enum PlatformType
{ 
    PLATFORM_ESP32 = 1,
    PLATFORM_ESP8266,
    PLATFORM_ArduinoUno,
    PLATFORM_ArduinoMega,
    PLATFORM_ArduinoDue,
    PLATFORM_ArduinoMicro,
    PLATFORM_ArduinoNano
};

//---------- Edit These Values Per Device ----------

const uint8_t NUM_DEVICE_CFG_BYTES = 30;

//Instrument type
const uint8_t INSTRUMENT_TYPE = INSTRUMENT_PWM;
//Platform type
const uint8_t PLATFORM_TYPE = PLATFORM_ESP32;

//Absolute max number of Polyphonic notes is 16
const uint8_t MAX_POLYPHONIC_NOTES = 16;
//Absolute max number of Instruments is 32
const uint8_t MAX_NUM_INSTRUMENTS = 8;
//Absolute Lowest Note Min=0
const uint8_t MIN_MIDI_NOTE = 36; //C1 //36-83 dulcimer
//Absolute Highest Note Max=127
const uint8_t MAX_MIDI_NOTE = 83; //B4
//A 14 bit number Representing this Devices ID
const uint16_t SYSEX_DEV_ID = 0x01;
//Firmware Version 14bit
const uint16_t FIRMWARE_VERSION = 03;

namespace Device
{
    //Golbal Device Attribues
    //inline static char* Name = new char[20]();
    inline static String Name = "Air Compressor";
    inline static bool OmniMode = false;


    //C++ v11
    // extern bool g_OmniMode; //in the header file
    // bool g_OmniMode = false; // in exaclty *one* cpp file in the project
}
