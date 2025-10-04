/*
 * NetworkSerial.cpp
 *
 * Network that supports MIDI over Serial Communication.
 * 
 */

#include "NetworkSerial.h"
#include "Device.h"

#ifdef MMM_NETWORK_SERIAL

void NetworkSerial::begin() {
    Serial.begin(115200);
    startSerial();
}


// connect to Serial – returns true if successful or false if not
bool NetworkSerial::startSerial() {
    //TODO add automatic connection
    return true;
}

/* Waits for buffer to fill with a new msg (>3 Bytes). */
std::optional<MidiMessage> NetworkSerial::readMessage() {
    uint8_t messageLength = 0;
    MidiMessage message = MidiMessage();

    //Wait until full message to begin reading the buffer
    if (Serial.available() && (Serial.peek() & MSB_BITMASK) != 0){
        message.buffer[messageLength] = Serial.read();
        messageLength = 1;
    }

    //Fills buffer with one whole Msg. Msg heads are denoted by the MSB == 1
    while (Serial.available() && ((Serial.peek() & MSB_BITMASK) == 0)){   
        message.buffer[messageLength] = Serial.read();

        messageLength++;
        if(messageLength == MAX_PACKET_LENGTH){
            Serial.println("Bad Message");
            return {};
        }
    }      
    
    if (messageLength == 0) return {};
    return message;
}

//Send Byte arrays wrapped in SysEx Messages 
void NetworkSerial::sendMessage(MidiMessage message) {
    //Write the message to Serial
    Serial.write(message.buffer.data(), message.length);
}

//Serial print Strings for Debug
void NetworkSerial::sendString(String msg) {
    Serial.println(msg);
}

#endif