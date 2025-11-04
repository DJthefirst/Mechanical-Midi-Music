/*
 * NetworkUSB.h
 *
 * Network implementation for MIDI over USB communication
 */

#pragma once

#include "INetwork.h"

#ifdef MMM_NETWORK_USB

class NetworkUSB : public INetwork {
public:
    NetworkUSB() = default;
    void begin() override;
    void sendMessage(const MidiMessage& message) override;
    void sendString(const String& message) override;
    std::optional<MidiMessage> readMessage() override;
};

#endif