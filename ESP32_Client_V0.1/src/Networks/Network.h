//
// An Interface Class For Network Connections
//
#pragma once

#include "MessageHandler.h"
#define MAX_PACKET_LENGTH 259 //For Now

#include <stdint.h>

class Network{
protected:
    MessageHandler* m_ptrMessageHandler;

    uint8_t m_messagePos = 0;                         // Track current message read position
    uint8_t m_messageBuffer[MOPPY_MAX_PACKET_LENGTH]; // Max message length (ToDo add SysExe)

public:
    virtual void Begin() = 0;
    virtual void ReadMessage() = 0;
    virtual void SendMessage(uint8_t message[], int length) = 0;
};