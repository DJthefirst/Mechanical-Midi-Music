/*
 * NetworkSerial.cpp
 *
 * Network implementation that supports MIDI over serial communication
 */

#include "NetworkSerial.h"
#include "Config.h"

#ifdef CFG_MMM_NETWORK_SERIAL

void NetworkSerial::begin() {
    Serial.begin(CFG_MMM_NETWORK_SERIAL_BAUD); // Standard MIDI baud rate
    startSerial();
}

// Connect to Serial – returns true if successful or false if not
bool NetworkSerial::startSerial() {
    // TODO: add automatic connection
    return true;
}

uint8_t NetworkSerial::expectedMessageLength(uint8_t status) {
    if (status < 0x80) return 0;
    if (status < 0xF0) {
        const uint8_t type = status & 0xF0;
        return (type == Midi::ProgramChange || type == Midi::ChannelPressure) ? 2 : 3;
    }

    switch (status) {
        case 0xF1:
        case 0xF3:
            return 2;
        case 0xF2:
            return 3;
        case 0xF6:
        case 0xF8:
        case 0xF9:
        case 0xFA:
        case 0xFB:
        case 0xFC:
        case 0xFD:
        case 0xFE:
        case 0xFF:
            return 1;
        default:
            return 0;
    }
}

void NetworkSerial::resetPendingMessage() {
    m_pendingMessage.length = 0;
    m_expectedLength = 0;
}

// Parse one complete MIDI message while retaining partial input between calls.
std::optional<MidiMessage> NetworkSerial::readMessage() {
    while (Serial.available()) {
        const uint8_t value = static_cast<uint8_t>(Serial.read());

        // Real-time messages may occur between bytes of another message.
        if (value >= 0xF8) {
            MidiMessage realtime;
            realtime.buffer[0] = value;
            realtime.length = 1;
            return realtime;
        }

        if (m_discardingSysEx) {
            if (value == 0xF7) m_discardingSysEx = false;
            continue;
        }

        if (m_readingSysEx) {
            if (m_pendingMessage.length >= MAX_PACKET_LENGTH) {
                resetPendingMessage();
                m_readingSysEx = false;
                m_discardingSysEx = true;
                continue;
            }

            m_pendingMessage.buffer[m_pendingMessage.length++] = value;
            if (value == 0xF7) {
                MidiMessage complete = m_pendingMessage;
                resetPendingMessage();
                m_readingSysEx = false;
                return complete;
            }
            continue;
        }

        if ((value & MSB_BITMASK) != 0) {
            resetPendingMessage();
            if (value == 0xF0) {
                m_runningStatus = 0;
                m_readingSysEx = true;
                m_pendingMessage.buffer[0] = value;
                m_pendingMessage.length = 1;
                continue;
            }

            m_expectedLength = expectedMessageLength(value);
            if (m_expectedLength == 0) continue;
            m_pendingMessage.buffer[0] = value;
            m_pendingMessage.length = 1;
            m_runningStatus = value < 0xF0 ? value : 0;
        } else {
            if (m_pendingMessage.length == 0) {
                if (m_runningStatus == 0) continue;
                m_pendingMessage.buffer[0] = m_runningStatus;
                m_pendingMessage.length = 1;
                m_expectedLength = expectedMessageLength(m_runningStatus);
            }

            if (m_pendingMessage.length < MAX_PACKET_LENGTH) {
                m_pendingMessage.buffer[m_pendingMessage.length++] = value;
            }
        }

        if (m_expectedLength != 0 && m_pendingMessage.length == m_expectedLength) {
            MidiMessage complete = m_pendingMessage;
            resetPendingMessage();
            return complete;
        }
    }

    return std::nullopt;
}

// Send byte arrays wrapped in SysEx messages 
void NetworkSerial::sendMessage(const MidiMessage& message) {
    // Write the message to Serial
    Serial.write(message.buffer.data(), message.length);
}

// Serial print strings for debugging
void NetworkSerial::sendString(const String& msg) {
    Serial.println(msg);
}

#endif