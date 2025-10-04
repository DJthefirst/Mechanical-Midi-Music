/*
 * SerialNetwork.h - V2.0 Serial Network Implementation
 *
 * Part of the Mechanical MIDI Music V2.0 redesign
 * Based on REDESIGN_PART3_NETWORK.md specifications
 */

#pragma once

#include "Network.h"
#include <Arduino.h>

class SerialNetwork : public Network {
private:
    static constexpr uint32_t BAUD_RATE = 115200;
    static constexpr uint8_t BUFFER_SIZE = 128;
    
    uint8_t m_buffer[BUFFER_SIZE];
    uint8_t m_bufferPos = 0;
    bool m_inSysEx = false;
    
public:
    void begin() override {
        Serial.begin(BAUD_RATE);
        while (!Serial && millis() < 3000) {
            delay(10);  // Wait for Serial with timeout
        }
        m_bufferPos = 0;
        m_inSysEx = false;
    }
    
    void sendMessage(const MidiMessage& message) override {
        if (message.isSysEx()) {
            // For SysEx messages, send the complete message including 0xF0 start and 0xF7 end
            // The sysExData already contains the full SysEx message
            Serial.write(0xF0);  // SysEx start
            Serial.write(message.sysExData.data(), message.sysExLength);
            Serial.write(0xF7);  // SysEx end
        } else {
            // For regular MIDI messages, send status and data bytes
            Serial.write(message.status);
            
            if (message.data1 != MidiMessage::NO_DATA) {
                Serial.write(message.data1);
                
                if (message.data2 != MidiMessage::NO_DATA) {
                    Serial.write(message.data2);
                }
            }
        }
    }
    
    std::optional<MidiMessage> readMessage() override {
        while (Serial.available()) {
            uint8_t byte = Serial.read();
            
            // Status byte (0x80-0xFF)
            if (byte & 0x80) {
                if (byte == 0xF0) {
                    // Start SysEx
                    m_inSysEx = true;
                    m_buffer[0] = byte;
                    m_bufferPos = 1;
                } else if (byte == 0xF7 && m_inSysEx) {
                    // End SysEx
                    m_inSysEx = false;
                    return parseSysEx();
                } else if (!m_inSysEx) {
                    // Regular MIDI message
                    m_buffer[0] = byte;
                    m_bufferPos = 1;
                    
                    uint8_t expectedLength = MidiMessage::getMessageLength(byte);
                    if (expectedLength == 1) {
                        // Single byte message (system messages)
                        MidiMessage msg;
                        msg.status = byte;
                        m_bufferPos = 0;
                        return msg;
                    }
                }
            } else if (m_inSysEx) {
                // SysEx data byte
                if (m_bufferPos < BUFFER_SIZE) {
                    m_buffer[m_bufferPos++] = byte;
                }
            } else if (m_bufferPos > 0) {
                // Data byte for regular message
                m_buffer[m_bufferPos++] = byte;
                
                uint8_t expectedLength = MidiMessage::getMessageLength(m_buffer[0]);
                if (m_bufferPos >= expectedLength) {
                    return parseMessage();
                }
            }
        }
        
        return std::nullopt;
    }
    
    bool isConnected() const override {
        return Serial;
    }
    
    const char* getName() const override {
        return "Serial";
    }
    
    void printStatus() const override {
        Serial.printf("Serial Network: %d baud, %s\n", 
                      BAUD_RATE, 
                      isConnected() ? "Connected" : "Disconnected");
    }
    
    const char* getConnectionType() const override {
        return "UART";
    }
    
    uint32_t getDataRate() const override {
        return BAUD_RATE;
    }
    
private:
    std::optional<MidiMessage> parseMessage() {
        if (m_bufferPos < 2) return std::nullopt;
        
        MidiMessage msg;
        msg.status = m_buffer[0];
        msg.data1 = m_buffer[1];
        msg.data2 = (m_bufferPos > 2) ? m_buffer[2] : MidiMessage::NO_DATA;
        
        m_bufferPos = 0;
        return msg;
    }
    
    std::optional<MidiMessage> parseSysEx() {
        if (m_bufferPos < 2) return std::nullopt;
        
        MidiMessage msg;
        msg.status = 0xF0;
        msg.sysExLength = m_bufferPos - 1;  // Exclude 0xF0
        
        if (msg.sysExLength > MidiMessage::MAX_SYSEX_LENGTH) {
            msg.sysExLength = MidiMessage::MAX_SYSEX_LENGTH;
        }
        
        for (uint8_t i = 0; i < msg.sysExLength; i++) {
            msg.sysExData[i] = m_buffer[1 + i];
        }
        
        m_bufferPos = 0;
        return msg;
    }
};