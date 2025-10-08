// /*
//  * NetworkRTP.h
//  *
//  * Network that supports Bluetooth Communication.
//  * 
//  */

#pragma once

#include "Arduino.h"
#include "INetwork.h"
#include <cstdint>
#include <array>

class NetworkRTP : public INetwork{
public:
    NetworkRTP() = default;
    void begin() override;
    void sendMessage(MidiMessage message) override;
    void sendString(String message) override;
    std::optional<MidiMessage> readMessage() override;

private:
    bool startRTP();
};