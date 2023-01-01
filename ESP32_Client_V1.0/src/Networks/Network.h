/* 
 * Network.h 
 *
 * An Interface Class For Network Connections
 * 
 */

#pragma once

#include <stdint.h>
#include "Constants.h"
#include "MessageHandler.h"
#include "MidiMessage.h"

#define MAX_PACKET_LENGTH 256 //For Now

class MessageHandler;

class Network{
    
protected:
    MessageHandler* m_ptrMessageHandler;
    
public:
    virtual void setMessageHandler(MessageHandler* ptrMessageHandler);
    virtual void begin();
    virtual void readMessage();
    virtual void sendMessage(uint8_t message[], uint8_t length);
};