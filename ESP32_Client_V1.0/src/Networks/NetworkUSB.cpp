/*
 * NetworkUSB.cpp
 *
 * Network that supports Serial Communication.
 * 
 */
#include "NetworkUSB.h"

void NetworkUSB::begin() {
    Serial.begin(115200);
    startUSB();
}


// connect to USB – returns true if successful or false if not
bool NetworkUSB::startUSB() {
    bool connected = true; //TODO add automatic connection
    return connected;
}

/* Waits for buffer to fill with a new msg (>3 Bytes). */
MidiMessage NetworkUSB::readMessage() {
    int messageLength = Serial.available();

    MidiMessage message;

    //Wait until full message to begin reading the buffer
    if (messageLength >= NETWORK_MIN_MSG_BYTES){
        message.buffer[0] = Serial.read();
        messageLength = 0;

        //Fills buffer with one whole Msg. Msg heads are denoted by the MSB == 1
        while (Serial.available() && ((Serial.peek() & MSB_BITMASK) == 0)){
            messageLength++;
            if(messageLength == MAX_PACKET_LENGTH) return MidiMessage(); //TODO: Error     
            message.buffer[messageLength] = Serial.read();
        }      
        message.length = messageLength;
    }
    return message;
}

//Send Byte arrays wrapped in SysEx Messages 
void NetworkUSB::sendMessage(MidiMessage message) {
    //Write the message to Serial
    Serial.write(message.buffer, message.length);
}

//Serial print Strings for Debug
void NetworkUSB::sendMessage(String msg) {
    Serial.println(msg);
}