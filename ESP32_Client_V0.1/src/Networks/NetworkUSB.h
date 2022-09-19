/*
 * MoppyUSB.h
 *
 */
#pragma once

#include "Arduino.h"
#include <stdint.h>

#include "MessageHandler.h"

#define MAX_PACKET_LENGTH 259 //For Now

class NetworkUSB{
private:
    MessageHandler* m_ptrMessageHandler;

    uint8_t m_messagePos = 0;                         // Track current message read position
    uint8_t m_messageBuffer[MOPPY_MAX_PACKET_LENGTH]; // Max message length (ToDo add SysExe)

public:
    NetworkUSB(MessageHandler* ptrMessageHandler);
    void Begin();
    void ReadMessages();

private:
    bool StartUSB();
    void ParseMessage(uint8_t message[], int length);
    void SendData(uint8_t message[], int length);
    void SendPong();
};