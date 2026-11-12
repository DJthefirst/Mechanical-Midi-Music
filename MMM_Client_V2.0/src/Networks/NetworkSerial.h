/*
 * NetworkSerial.h
 *
 * Network implementation that supports serial communication
 */

#pragma once

#ifdef CFG_MMM_NETWORK_SERIAL

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
    MidiMessage m_pendingMessage;
    uint8_t m_expectedLength = 0;
    uint8_t m_runningStatus = 0;
    bool m_readingSysEx = false;
    bool m_discardingSysEx = false;

    bool startSerial();
    void resetPendingMessage();
    static uint8_t expectedMessageLength(uint8_t status);
};

#endif /* CFG_MMM_NETWORK_SERIAL */