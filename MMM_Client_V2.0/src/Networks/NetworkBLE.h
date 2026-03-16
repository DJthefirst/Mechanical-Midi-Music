/*
 * NetworkBLE.h
 *
 * Network that supports Bluetooth Communication.
 * 
 */

#pragma once

#ifdef CFG_MMM_NETWORK_BLE

#include "Arduino.h"
#include "INetwork.h"
#include <cstdint>

class NetworkBLE : public INetwork{
public:
    NetworkBLE() = default;
    void begin() override;
    void sendMessage(MidiMessage message) override;
    void sendString(String message) override;
    std::optional<MidiMessage> readMessage() override;

private:
    bool startBLE();
};

#endif /* CFG_MMM_NETWORK_BLE */