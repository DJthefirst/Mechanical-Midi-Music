/*
 * MidiMessage.h
 * 
 * A simple Array wrapper representing a MidiMessage with helper 
 * functions to parse Midi Messages
 */

#pragma once

#include <cstdint>
using std::int8_t;
using std::int16_t;

const uint16_t MAX_PACKET_LENGTH = 256;

struct MidiMessage
{
    uint8_t length;
    uint8_t buffer[MAX_PACKET_LENGTH];

    //For Protocol https://docs.google.com/spreadsheets/d/1AgS2-iZVLSL0w_MafbeReRx4u_9m_e4OTCsIhKC-QMg/edit?usp=sharing

    uint8_t type() {return buffer[0] & 0b11110000;}
    uint8_t sysCommonType(){return buffer[0] & 0b00001111;}
    uint8_t channel(){return buffer[0] & 0b00001111;}

    uint8_t CC_Control(){return buffer[1];}
    uint8_t CC_Value(){return buffer[2];}

    //SYSEX MSG Helpers
    uint8_t sysExID(){return buffer[1];}
    uint16_t deviceID(){return (buffer[2] << 7) | buffer[3];} //Combine message SysEx ID LSB and MSB
    uint8_t sysExCommand(){return buffer[4];} 
    uint8_t* sysExCmdPayload(){return buffer + 5;} //Returns the start of data from a SysEx message

    uint16_t sysExDistributorID(){return (buffer[5] << 7) | buffer[6];}
};