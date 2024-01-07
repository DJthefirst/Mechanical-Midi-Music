/*
 * NetworkUSB.h
 *
 * Network that supports Serial Communication.
 * 
 */

#pragma once

#include "Arduino.h"
#include "Networks/Network.h"
#include <cstdint>

class NetworkUSB : public Network{
public:
    NetworkUSB() = default;
    void begin() override;
    void readMessage() override;
    void sendMessage(const uint8_t message[], uint8_t length) override;
    void sendMessage(String msg) override;

private:
    bool startUSB();
};