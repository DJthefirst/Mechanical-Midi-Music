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
#include "MidiMessage.h"

class MessageHandler;

class Network{
    
protected:
    const static uint8_t NETWORK_MIN_MSG_BYTES = 3;
    
public:
    virtual void begin() = 0;
    virtual void sendMessage(MidiMessage message) = 0;
    virtual void sendMessage(String message) = 0;
    virtual MidiMessage readMessage() = 0;
};