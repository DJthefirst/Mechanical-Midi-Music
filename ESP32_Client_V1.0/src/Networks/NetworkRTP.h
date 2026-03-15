// /*
//  * NetworkRTP.h
//  *
//  * Network that supports Bluetooth Communication.
//  * 
//  */

#pragma once

#ifdef CFG_MMM_NETWORK_RTP

#include "Arduino.h"
#include "INetwork.h"
#include <cstdint>
#include <array>

class NetworkRTP : public INetwork{
public:
    NetworkRTP() = default;
    void begin() override;
    void sendMessage(const MidiMessage& message) override;
    void sendString(const String& message) override;
    std::optional<MidiMessage> readMessage() override;

private:
    bool startRTP();
};

#endif /* CFG_MMM_NETWORK_RTP */