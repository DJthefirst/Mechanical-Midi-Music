/*
 * NetworkSerial.h
 *
 * Network that supports Serial Communication.
 * 
 */

#pragma once

#include "Arduino.h"
#include "INetwork.h"
#include <cstdint>

class NetworkSerial : public INetwork{
public:
    NetworkSerial() = default;
    void begin() override;
    void sendMessage(MidiMessage message) override;
    void sendString(String message) override;
    std::optional<MidiMessage> readMessage() override;

private:
    bool startSerial();
};