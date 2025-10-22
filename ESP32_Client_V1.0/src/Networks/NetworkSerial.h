/*
 * NetworkSerial.h
 *
 * Network implementation that supports serial communication
 */

#pragma once

#include "Arduino.h"
#include "INetwork.h"
#include <cstdint>

class NetworkSerial : public INetwork{
public:
    NetworkSerial() = default;
    void begin() override;
    void sendMessage(const MidiMessage& message) override;
    void sendString(const String& message) override;
    std::optional<MidiMessage> readMessage() override;

private:
    bool startSerial();
};