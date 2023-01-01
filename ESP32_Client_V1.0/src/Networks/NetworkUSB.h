/*
 * NetworkUSB.h
 *
 * Network that supports Serial Communication.
 * 
 */

#pragma once

#include "Arduino.h"
#include <stdint.h>

#include "Networks\Network.h"

class NetworkUSB : public Network{
public:
    NetworkUSB();
    void setMessageHandler(MessageHandler* messageHandler) override;
    void begin() override;
    void readMessage() override;
    void sendMessage(uint8_t message[], uint8_t length) override;

private:
    bool startUSB();
};