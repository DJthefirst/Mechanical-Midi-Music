/* 
 * Network.h 
 *
 * An Interface Class For Network Connections
 * 
 */

#pragma once

#include <Arduino.h>
#include <stdint.h>
#include "Constants.h"
#include "MessageHandler.h"
#include "MidiMessage.h"

class MessageHandler;

class Network{
    
protected:
    const static uint8_t NETWORK_MIN_MSG_BYTES = 3;
    MessageHandler* m_ptrMessageHandler;
    
public:
    void setMessageHandler(MessageHandler* ptrMessageHandler) { m_ptrMessageHandler = ptrMessageHandler; };
    
    virtual void begin() = 0;
    virtual void readMessage() = 0;
    virtual void sendMessage(const uint8_t* message, uint8_t length) = 0;
    virtual void sendMessage(String msg) = 0;
};