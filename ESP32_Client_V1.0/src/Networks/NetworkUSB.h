/*
 * NetworkUSB.h
 *
 * Network that supports Serial Communication.
 * 
 */

#pragma once

#include "Arduino.h"
#include "Network.h"
#include <cstdint>

class NetworkUSB : public Network{
public:
    NetworkUSB() = default;
    void begin() override;
    void sendMessage(MidiMessage message) override;
    void sendMessage(String message) override;
    MidiMessage readMessage() override;

private:
    bool startUSB();
};