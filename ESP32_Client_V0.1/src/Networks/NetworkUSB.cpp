#include "NetworkUSB.h"
#include "MessageHandler.h"


NetworkUSB::NetworkUSB(){}

void NetworkUSB::initalize(MessageHandler* ptrMessageHandler)
{
    _ptrMessageHandler = ptrMessageHandler;
}

void NetworkUSB::begin() {
    Serial.begin(115200);
    startUSB();
}


// connect to USB – returns true if successful or false if not
bool NetworkUSB::startUSB() {
    bool connected = true;
    return connected;
}

/*
    Waits for buffer to fill with a new msg (>3 Bytes). The Msg is sent to the msg handler
*/
void NetworkUSB::readMessages() {
    int messageLength = Serial.available();

    if (messageLength >= 3){
        _messageBuffer[0] = Serial.read();
        messageLength = 1;

        //Fills buffer with one whole Msg. Msg heads are denoted by the MSB == 1
        while (Serial.available() && ((Serial.peek() & MSB_BITMASK) == 0)){
            _messageBuffer[messageLength] = Serial.read();
            messageLength++;
        }

        //Filter out incomplete or corrupt msg
        if (messageLength > 1 && messageLength <= 8){
            (*_ptrMessageHandler).processMessage(_messageBuffer);
        }
        else{
            Serial.println("PacketSize Out of Scope");
            Serial.println(_messageBuffer[0]);
            Serial.println(messageLength);
        }
    }
}

void NetworkUSB::sendData(uint8_t message[], int length) {
    //Not Yet Implemented
}

void NetworkUSB::sendPong() {
    //Not Yet Implemented
}