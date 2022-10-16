//
// An Interface Class For Network Connections
//
#pragma once

#include "Constants.h"
#include "MessageHandler.h"
#include <stdint.h>

#define MAX_PACKET_LENGTH 259 //For Now

class MessageHandler;

class Network{
    
protected:
    MessageHandler* m_ptrMessageHandler;

    uint8_t m_messagePos = 0;                         // Track current message read position
    uint8_t m_messageBuffer[MAX_PACKET_LENGTH]; // Max message length (ToDo add SysExe)

public:
    virtual void SetMessageHandler(MessageHandler* ptrMessageHandler);
    virtual void Begin();
    virtual void ReadMessage();
    virtual void SendMessage(uint8_t message[], int length);
};