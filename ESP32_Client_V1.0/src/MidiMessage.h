/*
 * MidiMessage.h
 * 
 * A simple Array wrapper representing a MidiMessage with helper 
 * functions to parse Midi Messages
 */

#pragma once

#include <cstdint>
#include <array>
using std::int8_t;
using std::int16_t;

constexpr uint16_t MAX_PACKET_LENGTH = 128; //256
constexpr uint8_t SYSEX_HeaderSize = 7; //Header size of SysEx message

struct MidiMessage
{
    uint8_t length = 0;
    std::array<uint8_t, MAX_PACKET_LENGTH> buffer;
    bool isValid() {return length != 0;}

    //Generate Empty Midi Message
    MidiMessage() {};

    //Generate SysEx Midi Message from MMM Message
    MidiMessage(uint16_t src, uint16_t dest, uint8_t msgId, const uint8_t payload[], uint8_t payloadLength) {
        
        //Midi Message Header SysExStart, MidiID, DeviceID_1, DeviceID_0.
        const std::array<uint8_t, 2> header = {0xF0, SYSEX_ID};
        std::copy(header.data(), header.data()+header.size(), buffer.begin());

        //SYSEX Src and Dest addresses
        buffer[2] = (src >> 7) & 0x7F;
        buffer[3] = (src & 0x7F);
        buffer[4] = (dest >> 7) & 0x7F;
        buffer[5] = (dest & 0x7F);
        buffer[6] = (msgId);

        //Midi SysEx Message Payload
        std::copy(payload, payload+payloadLength, buffer.begin()+SYSEX_HeaderSize);
        //std::copy(payload, payload+payloadLength, buffer+SYSEX_HeaderSize);

        //Midi Message Tail SysEx End.
        const uint8_t tail = 0xF7;
        buffer[SYSEX_HeaderSize+payloadLength] = tail;
        
        length = SYSEX_HeaderSize + payloadLength + 1;    
    }

    //For Protocol https://docs.google.com/spreadsheets/d/1AgS2-iZVLSL0w_MafbeReRx4u_9m_e4OTCsIhKC-QMg/edit?usp=sharing

    uint8_t type() {return buffer[0] & 0b11110000;}
    uint8_t sysCommonType(){return buffer[0] & 0b00001111;}
    uint8_t channel(){return buffer[0] & 0b00001111;}

    uint8_t CC_Control(){return buffer[1];}
    uint8_t CC_Value(){return buffer[2];}

    //SYSEX MSG Helpers
    uint8_t sysExID(){return buffer[1];}
    uint16_t SourceID(){return (buffer[2] << 7) | buffer[3];} //Combine message SysEx ID LSB and MSB
    uint16_t DestinationID(){return (buffer[4] << 7) | buffer[5];} //Combine message SysEx ID LSB and MSB
    uint8_t sysExCommand(){return buffer[6];} 
    uint8_t* sysExCmdPayload(){return buffer.begin() + 7;} //Returns the start of data from a SysEx message
    //uint8_t* sysExCmdPayload(){return buffer + 7;} //Returns the start of data from a SysEx message

    //Return Distributor ID
    uint16_t sysExDistributorID(){return (buffer[7] << 7) | buffer[8];}
};