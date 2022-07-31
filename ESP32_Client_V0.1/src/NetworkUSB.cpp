#include "NetworkUSB.h"
#include "MessageHandler.h"

#if !defined ARDUINO_ARCH_ESP8266 && !defined ARDUINO_ARCH_ESP32
#else


/*
 * Serial communications implementation for Arduino.  Instrument
 * has its handler functions called for device and system messages
 */
NetworkUSB::NetworkUSB(){}

void NetworkUSB::initalize(MessageHandler* ptrMessageHandler)
{
    _ptrMessageHandler = ptrMessageHandler;
}

void NetworkUSB::begin() {
    Serial.begin(115200); // For debugging

    startUSB();
}


// connect to USB – returns true if successful or false if not
bool NetworkUSB::startUSB() {
    bool connected = true;
    return connected;
}

void NetworkUSB::readMessages() {
    int messageLength = Serial.available();

    if (messageLength >= 3){
        _messageBuffer[0] = Serial.read();
        messageLength = 1;
        while (Serial.available() && ((Serial.peek() & MSB_BITMASK) == 0)){
            _messageBuffer[messageLength] = Serial.read();
            messageLength++;
        }

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

}

void NetworkUSB::sendPong() {

}

#endif /* ARDUINO_ARCH_ESP8266 or ARDUINO_ARCH_ESP32 */