/*
 * MidiMessage.h - V2.0 Enhanced MIDI Message Structure
 *
 * Part of the Mechanical MIDI Music V2.0 redesign
 * Based on REDESIGN_PART3_NETWORK.md specifications
 *
 * Key improvements over V1.0:
 * - Cleaner interface with separate SysEx handling
 * - Better type safety and const correctness
 * - More efficient memory layout
 * - Support for all MIDI message types
 */

#pragma once

#include <Arduino.h>
#include "core/Constants.h"
#include <cstdint>
#include <array>

////////////////////////////////////////////////////////////////////////////////////////////////////
// MIDI Message Structure 
////////////////////////////////////////////////////////////////////////////////////////////////////

struct MidiMessage {
    static constexpr uint8_t NO_DATA = 0xFF;          // Indicates missing data byte
    static constexpr uint8_t MAX_SYSEX_LENGTH = 120;  // Max SysEx payload size
    
    // Core MIDI data
    uint8_t status = 0;      // Status byte (includes message type and channel)
    uint8_t data1 = NO_DATA; // First data byte (note, CC number, etc.)
    uint8_t data2 = NO_DATA; // Second data byte (velocity, CC value, etc.)
    
    // SysEx data (only used for SysEx messages)
    uint8_t sysExLength = 0;
    std::array<uint8_t, MAX_SYSEX_LENGTH> sysExData{};
    
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    
    // Default constructor
    MidiMessage() = default;
    
    // Standard MIDI message constructor
    MidiMessage(uint8_t status_byte, uint8_t data1_byte, uint8_t data2_byte = NO_DATA)
        : status(status_byte), data1(data1_byte), data2(data2_byte) {}
    
    // SysEx message constructor (V1.0 compatibility)
    MidiMessage(uint16_t src, uint16_t dest, uint8_t msgId, const uint8_t payload[], uint8_t payloadLength)
        : status(0xF0), data1(NO_DATA), data2(NO_DATA) {
        
        if (payloadLength > MAX_SYSEX_LENGTH - 6) {
            payloadLength = MAX_SYSEX_LENGTH - 6; // Truncate if too long
        }
        
        // SysEx header: ID, Src MSB, Src LSB, Dest MSB, Dest LSB, Command
        sysExData[0] = SYSEX_ID;
        sysExData[1] = (src >> 7) & 0x7F;
        sysExData[2] = (src & 0x7F);
        sysExData[3] = (dest >> 7) & 0x7F;
        sysExData[4] = (dest & 0x7F);
        sysExData[5] = msgId;
        
        // Copy payload
        for (uint8_t i = 0; i < payloadLength; i++) {
            sysExData[6 + i] = payload[i];
        }
        
        sysExLength = 6 + payloadLength;
    }
    
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    // Message Type Queries
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    
    // Get MIDI message type (upper 4 bits of status)
    uint8_t type() const { 
        return status & 0xF0; 
    }
    
    // Get MIDI channel (lower 4 bits of status, 0-15)
    uint8_t channel() const { 
        return status & 0x0F; 
    }
    
    // Check if this is a SysEx message
    bool isSysEx() const { 
        return status == 0xF0; 
    }
    
    // Check if this is a valid message
    bool isValid() const { 
        if (isSysEx()) {
            return sysExLength > 0;
        } else {
            return status != 0 && data1 != NO_DATA;
        }
    }
    
    // Get SysCommon type (for system messages)
    uint8_t sysCommonType() const { 
        return status & 0x0F; 
    }
    
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    // MIDI Message Helpers
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    
    // Note On/Off helpers
    uint8_t note() const { return data1; }
    uint8_t velocity() const { return data2; }
    
    // Control Change helpers
    uint8_t CC_Control() const { return data1; }
    uint8_t CC_Value() const { return data2; }
    
    // Program Change helper
    uint8_t program() const { return data1; }
    
    // Pitch Bend helper (combines data1 and data2 into 14-bit value)
    uint16_t pitchBend() const { 
        if (data2 == NO_DATA) return data1;
        return (data2 << 7) | data1; 
    }
    
    // Channel Pressure helper
    uint8_t pressure() const { return data1; }
    
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    // SysEx Message Helpers (V1.0 Compatibility)
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    
    uint8_t sysExID() const { 
        return sysExLength > 0 ? sysExData[0] : 0; 
    }
    
    uint16_t SourceID() const { 
        if (sysExLength < 3) return 0;
        return (sysExData[1] << 7) | sysExData[2]; 
    }
    
    uint16_t DestinationID() const { 
        if (sysExLength < 5) return 0;
        return (sysExData[3] << 7) | sysExData[4]; 
    }
    
    uint8_t sysExCommand() const { 
        return sysExLength > 5 ? sysExData[5] : 0; 
    }
    
    const uint8_t* sysExCmdPayload() const { 
        return sysExLength > 6 ? &sysExData[6] : nullptr; 
    }
    
    uint8_t sysExPayloadLength() const {
        return sysExLength > 6 ? sysExLength - 6 : 0;
    }
    
    // Return Distributor ID from SysEx payload
    uint16_t sysExDistributorID() const {
        if (sysExLength < 8) return 0;
        return (sysExData[6] << 7) | sysExData[7];
    }


    
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    // Factory Methods for Common Messages
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    
    static MidiMessage noteOn(uint8_t channel, uint8_t note, uint8_t velocity) {
        return MidiMessage(MIDI_NoteOn | (channel & 0x0F), note, velocity);
    }
    
    static MidiMessage noteOff(uint8_t channel, uint8_t note, uint8_t velocity = 64) {
        return MidiMessage(MIDI_NoteOff | (channel & 0x0F), note, velocity);
    }
    
    static MidiMessage controlChange(uint8_t channel, uint8_t controller, uint8_t value) {
        return MidiMessage(MIDI_ControlChange | (channel & 0x0F), controller, value);
    }
    
    static MidiMessage programChange(uint8_t channel, uint8_t program) {
        return MidiMessage(MIDI_ProgramChange | (channel & 0x0F), program);
    }
    
    static MidiMessage pitchBend(uint8_t channel, uint16_t value) {
        return MidiMessage(MIDI_PitchBend | (channel & 0x0F), 
                          value & 0x7F, 
                          (value >> 7) & 0x7F);
    }
    
    static MidiMessage channelPressure(uint8_t channel, uint8_t pressure) {
        return MidiMessage(MIDI_ChannelPressure | (channel & 0x0F), pressure);
    }
    
    static MidiMessage keyPressure(uint8_t channel, uint8_t note, uint8_t pressure) {
        return MidiMessage(MIDI_KeyPressure | (channel & 0x0F), note, pressure);
    }
    
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    // Utility Methods
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    
    // Get expected message length based on status byte
    static uint8_t getMessageLength(uint8_t status) {
        uint8_t type = status & 0xF0;
        switch (type) {
            case MIDI_NoteOff:
            case MIDI_NoteOn:
            case MIDI_KeyPressure:
            case MIDI_ControlChange:
            case MIDI_PitchBend:
                return 3;  // Status + 2 data bytes
                
            case MIDI_ProgramChange:
            case MIDI_ChannelPressure:
                return 2;  // Status + 1 data byte
                
            case MIDI_SysCommon:
                if ((status & 0x0F) == MIDI_SysEX) {
                    return 0;  // Variable length, terminated by 0xF7
                } else {
                    return 1;  // Other system messages
                }
                
            default:
                return 1;
        }
    }
    
    // Debug string representation
    String toString() const {
        String result = "MIDI: ";
        
        if (isSysEx()) {
            result += "SysEx ID=" + String(sysExID(), HEX) + 
                     " Src=" + String(SourceID(), HEX) +
                     " Dest=" + String(DestinationID(), HEX) +
                     " Cmd=" + String(sysExCommand(), HEX) +
                     " Len=" + String(sysExLength);
        } else {
            result += "Type=" + String(type(), HEX) + 
                     " Ch=" + String(channel()) +
                     " D1=" + String(data1) + 
                     " D2=" + String(data2);
        }
        
        return result;
    }
};