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

constexpr uint16_t MAX_PACKET_LENGTH = 64; //256

struct MidiMessage
{
    uint8_t length = 0;
    uint8_t buffer[MAX_PACKET_LENGTH];

    bool isValid() {return length != 0;}

    //Generate Empty Midi Message
    MidiMessage() {};

    //Generate SysEx Midi Message from MMM Message
    MidiMessage(const uint8_t payload[], uint8_t payloadLength) {
        //Midi Message Header SysExStart, MidiID, DeviceID_1, DeviceID_0.
        const std::array<uint8_t, 4> header = {0xF0, SYSEX_ID, 0x7F, 0x7F};
        std::copy(header.data(), header.data()+header.size(), buffer);
        //Midi SysEx Message Payload
        std::copy(payload, payload+payloadLength, buffer+header.size());
        //Midi Message Tail SysEx End.
        const uint8_t tail = 0xF7;
        buffer[header.size()+payloadLength] = tail;
        
        length = header.size() + payloadLength + 1;    
    }

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

    //Return Distributor ID
    uint16_t sysExDistributorID(){return (buffer[5] << 7) | buffer[6];}
};