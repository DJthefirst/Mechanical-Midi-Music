/*
 * MidiMessage.h
 * 
 * Array wrapper representing a MIDI message with helper functions to parse messages
 */

#pragma once

#include "Constants.h"
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
    constexpr bool isValid() const noexcept { return length != 0; }

    // Generate empty MIDI message
    MidiMessage() = default;

    // Generate SysEx MIDI message from MMM message
    MidiMessage(uint16_t src, uint16_t dest, uint8_t msgId, const uint8_t payload[], uint8_t payloadLength) {
        
        // MIDI message header: SysExStart, MidiID, DeviceID_1, DeviceID_0
        constexpr std::array<uint8_t, 2> header = {0xF0, SysEx::ID};
        std::copy(header.data(), header.data() + header.size(), buffer.begin());

        // SysEx source and destination addresses
        buffer[2] = (src >> 7) & 0x7F; // Magic! - 7-bit encoding for MIDI
        buffer[3] = (src & 0x7F);
        buffer[4] = (dest >> 7) & 0x7F;
        buffer[5] = (dest & 0x7F);
        buffer[6] = msgId;

        // MIDI SysEx message payload
        if (payload && payloadLength > 0) {
            std::copy(payload, payload + payloadLength, buffer.begin() + SYSEX_HeaderSize);
        }

        // MIDI message tail: SysEx End
        constexpr uint8_t tail = 0xF7;
        buffer[SYSEX_HeaderSize + payloadLength] = tail;
        
        length = SYSEX_HeaderSize + payloadLength + 1;    
    }

    // Protocol reference: https://docs.google.com/spreadsheets/d/1AgS2-iZVLSL0w_MafbeReRx4u_9m_e4OTCsIhKC-QMg/edit?usp=sharing

    constexpr uint8_t type() const noexcept { return buffer[0] & 0b11110000; }
    constexpr uint8_t sysCommonType() const noexcept { return buffer[0] & 0b00001111; }
    constexpr uint8_t channel() const noexcept { return buffer[0] & 0b00001111; }

    constexpr uint8_t CC_Control() const noexcept { return buffer[1]; }
    constexpr uint8_t CC_Value() const noexcept { return buffer[2]; }

    // SysEx message helpers
    constexpr uint8_t sysExID() const noexcept { return buffer[1]; }
    constexpr uint16_t SourceID() const noexcept { return (buffer[2] << 7) | buffer[3]; } // Combine SysEx ID LSB and MSB
    constexpr uint16_t DestinationID() const noexcept { return (buffer[4] << 7) | buffer[5]; }
    constexpr uint8_t sysExCommand() const noexcept { return buffer[6]; } 
    const uint8_t* sysExCmdPayload() const noexcept { return buffer.data() + 7; } // Returns the start of data from a SysEx message
    uint8_t* sysExCmdPayload() noexcept { return buffer.data() + 7; }

    // Return distributor ID
    constexpr uint16_t sysExDistributorID() const noexcept { return (buffer[7] << 7) | buffer[8]; }
};