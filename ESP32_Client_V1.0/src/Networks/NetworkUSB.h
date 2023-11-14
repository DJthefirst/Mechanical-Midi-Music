/*
 * NetworkUSB.h
 *
 * Network that supports Serial Communication.
 * 
 */

#pragma once

#include "Arduino.h"
#include <cstdint>

#include "Networks/Network.h"

class NetworkUSB : public Network{
public:
    NetworkUSB() = default;
    void begin() override;
    void readMessage() override;
    void sendMessage(uint8_t message[], uint8_t length) override;
    void sendMessage(String msg);

private:
    bool startUSB();
};