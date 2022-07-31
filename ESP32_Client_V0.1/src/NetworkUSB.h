/*
 * MoppyUSB.h
 *
 */
#pragma once

#include "Arduino.h"
#include <stdint.h>

#include "MessageHandler.h"

#define MULTICASTPORT 65534
#define UNICASTPORT 65534
#define MOPPY_MAX_PACKET_LENGTH 259

class NetworkUSB{
private:
    MessageHandler* _ptrMessageHandler;

    uint8_t _messagePos = 0;                         // Track current message read position
    uint8_t _messageBuffer[MOPPY_MAX_PACKET_LENGTH]; // Max message length for Moppy messages is 259

public:
    NetworkUSB();
    void initalize(MessageHandler* ptrMessageHandler);
    void begin();
    void readMessages();

private:
    bool startUSB();
    void parseMessage(uint8_t message[], int length);
    void sendData(uint8_t message[], int length);
    void sendPong();
};