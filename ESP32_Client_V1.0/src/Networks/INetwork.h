/* 
 * Network.h 
 *
 * An Interface Class For Network Connections
 * 
 */

#pragma once

#include <Arduino.h>
#include <stdint.h>
#include <optional>
#include "Constants.h"
#include "MidiMessage.h"

class MessageHandler;

class INetwork{
    
protected:
    const static uint8_t NETWORK_MIN_MSG_BYTES = 3;
    
public:
    virtual ~INetwork() = default;
    virtual void begin() = 0;
    virtual void sendMessage(MidiMessage message) = 0;
    virtual void sendString(String message) = 0;
    virtual std::optional<MidiMessage> readMessage() = 0;
};