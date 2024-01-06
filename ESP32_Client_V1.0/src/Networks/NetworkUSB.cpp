/*
 * NetworkUSB.cpp
 *
 * Network that supports Serial Communication.
 * 
 */
#include "NetworkUSB.h"
#include "MessageHandler.h"

void NetworkUSB::begin() {
    Serial.begin(115200);
    startUSB();
}


// connect to USB – returns true if successful or false if not
bool NetworkUSB::startUSB() {
    bool connected = true; //Temporary
    return connected;
}

/* Waits for buffer to fill with a new msg (>3 Bytes). The Msg is sent to the msg handler */
void NetworkUSB::readMessage() {
    int messageLength = Serial.available();

    MidiMessage message;

    //Wait until full message to begin reading the buffer
    if (messageLength >= NETWORK_MIN_MSG_BYTES){
        message.buffer[0] = Serial.read();
        messageLength = 0;

        //Fills buffer with one whole Msg. Msg heads are denoted by the MSB == 1
        while (Serial.available() && ((Serial.peek() & MSB_BITMASK) == 0)){
            messageLength++;
            if(messageLength == MAX_PACKET_LENGTH) return; //ToDo: Error     
            message.buffer[messageLength] = Serial.read();
        }      
        message.length = messageLength;
        (*m_ptrMessageHandler).processMessage(message);
    }
}

//Send Byte arrays wrapped in SysEx Messages 
void NetworkUSB::sendMessage(uint8_t message[], uint8_t length) {
    //Midi Message Header SysExStart, MidiID, DeviceID_1, DeviceID_0.
    uint8_t header[] = {0xF0, 0x7D, 0x7F, 0x7F};
    //Midi Message Tail SysEx End.
    uint8_t tail[] = {0xF7};

    //Write the message to Serial
    Serial.write(header,4);
    Serial.write(message,length);
    Serial.write(tail,1);
}

//Serial print Strings for Debug
void NetworkUSB::sendMessage(String msg) {
    Serial.println(msg);
}