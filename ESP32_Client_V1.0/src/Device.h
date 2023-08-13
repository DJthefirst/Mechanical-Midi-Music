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

//---------- Edit These Values Per Device ----------

const uint8_t NUM_DEVICE_CFG_BYTES = 26;

//Absolute max number of Polyphonic notes is 16
const uint8_t MAX_POLYPHONIC_NOTES = 16;
//Absolute max number of Instruments is 32
const uint8_t MAX_NUM_INSTRUMENTS = 8;
//Absolute Lowest Note Min=0
const uint8_t MIN_MIDI_NOTE = 0; //C1 //36-83 dulcimer
//Absolute Highest Note Max=127
const uint8_t MAX_MIDI_NOTE = 127; //B4
//A 14 bit number Representing this Devices ID
const uint16_t SYSEX_DEV_ID = 0x01;
//Firmware Instrument type
const uint8_t FIRMWARE_TYPE = 0;
//Firmware Version 14bit
const uint16_t FIRMWARE_VERSION = 03;

namespace Device
{
    //Golbal Device Attribues
    //inline static char* Name = new char[20]();
    inline static String Name = "Default Device     ";
    inline static bool OmniMode = false;


    //C++ v11
    // extern bool g_OmniMode; //in the header file
    // bool g_OmniMode = false; // in exaclty *one* cpp file in the project
}
