/*
 * MidiMessage.h
 * 
 * A simple Array wrapper representing a MidiMessage with helper 
 * to help parse Midi Messages
 */

#pragma once

#include <stdint.h>

#define MAX_PACKET_LENGTH 255

struct MidiMessage
{
    uint8_t length;
    uint8_t buffer[MAX_PACKET_LENGTH];
    uint8_t type() {return buffer[0] & 0b11110000;}
    uint8_t value(){return buffer[0] & 0b00001111;}
};