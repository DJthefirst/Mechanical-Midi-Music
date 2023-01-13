/*
 * MidiMessage.h
 * 
 * A simple Array wrapper representing a MidiMessage with helper 
 * to help parse Midi Messages
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

    uint8_t type() {return buffer[0] & 0b11110000;}
    uint8_t value(){return buffer[0] & 0b00001111;}
    uint8_t sysExID(){return buffer[1];}
    uint16_t deviceID(){return (buffer[2] << 7) | buffer[3];} //Combine message SysEx ID LSB and MSB
    uint8_t sysExCommand(){return buffer[4];}
};