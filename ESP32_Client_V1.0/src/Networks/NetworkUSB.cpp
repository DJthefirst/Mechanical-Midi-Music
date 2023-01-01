/*
 * NetworkUSB.cpp
 *
 * Network that supports Serial Communication.
 * 
 */
#include "NetworkUSB.h"
#include "MessageHandler.h"

NetworkUSB::NetworkUSB(){}

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
    if (messageLength >= 3){
        message.buffer[0] = Serial.read();
        messageLength = 1;

        //Fills buffer with one whole Msg. Msg heads are denoted by the MSB == 1
        while (Serial.available() && ((Serial.peek() & MSB_BITMASK) == 0)){
            message.buffer[messageLength] = Serial.read();
            messageLength++;
        }

        //Filter out incomplete or corrupt msg
        if (messageLength > 1 && messageLength <= 64){
            message.length = messageLength;
            (*m_ptrMessageHandler).processMessage(message);
        }
        else{}
    }
}

void NetworkUSB::sendMessage(uint8_t message[], uint8_t length) {
    Serial.write(message,length);
}

 void NetworkUSB::setMessageHandler(MessageHandler* ptrMessageHandler)
    {
        m_ptrMessageHandler = ptrMessageHandler;
    }