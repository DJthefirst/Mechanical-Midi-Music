/*
 * Device.h
 * 
 * A Struct Representing this devices configuration
 */

#pragma once

#include <stdint.h>
#include <cstdint>
using std::int8_t;

//---------- Edit These Values Per Device ----------

//Absolute max number of Polyphonic notes is 16
const uint8_t MAX_POLYPHONIC_NOTES = 16;
//Absolute max number of Instruments is 32
const uint8_t MAX_NUM_INSTRUMENTS = 8; //32
//Absolute Lowest Note Min=0
const uint8_t MIN_MIDI_NOTE = 36; //C1
//Absolute Highest Note Max=127
const uint8_t MAX_MIDI_NOTE = 83; //B4
//A 14 bit number Representing this Devices ID
const uint16_t SYSEX_DEV_ID = 0x01;

namespace Device
{
    //Golbal Device Attribues
    inline uint8_t Name[20] = {0};
    inline bool OmniMode = false;

    //C++ v11
    // extern bool g_OmniMode; //in the header file
    // bool g_OmniMode = false; // in exaclty *one* cpp file in the project
}
