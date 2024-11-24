// /*
//  * NetworkRTP.h
//  *
//  * Network that supports Bluetooth Communication.
//  * 
//  */

#pragma once

#include "Arduino.h"
#include "Network.h"
#include <cstdint>

class NetworkRTP : public Network{
public:
    NetworkRTP() = default;
    void begin() override;
    void sendMessage(MidiMessage message) override;
    void sendMessage(String message) override;
    std::optional<MidiMessage> readMessage() override;

private:
    bool startRTP();
};