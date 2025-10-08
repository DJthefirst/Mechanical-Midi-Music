/*
 * NetworkSerial.cpp
 *
 * Network implementation that supports MIDI over serial communication
 */

#include "NetworkSerial.h"
#include "Device.h"

#ifdef MMM_NETWORK_SERIAL

void NetworkSerial::begin() {
    Serial.begin(MMM_NETWORK_SERIAL_BAUD); // Standard MIDI baud rate
    startSerial();
}

// Connect to Serial – returns true if successful or false if not
bool NetworkSerial::startSerial() {
    // TODO: add automatic connection
    return true;
}

// Waits for buffer to fill with a new message (minimum 3 bytes)
std::optional<MidiMessage> NetworkSerial::readMessage() {
    uint8_t messageLength = 0;
    MidiMessage message;

    // Wait until full message to begin reading the buffer
    if (Serial.available() && (Serial.peek() & MSB_BITMASK) != 0) {
        message.buffer[messageLength] = Serial.read();
        messageLength = 1;
    }

    // Fill buffer with one whole message. Message headers are denoted by MSB == 1
    while (Serial.available() && ((Serial.peek() & MSB_BITMASK) == 0)) {   
        message.buffer[messageLength] = Serial.read();
        messageLength++;
        
        if (messageLength == MAX_PACKET_LENGTH) {
            Serial.println("Bad Message");
            return std::nullopt;
        }
    }      
    
    if (messageLength == 0) return std::nullopt;
    message.length = messageLength;
    return message;
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