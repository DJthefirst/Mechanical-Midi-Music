#pragma once

#if defined(CFG_MMM_NETWORK_USB) && defined(CFG_ESP32_S3)

#include "Networks/INetwork.h"
#include "Networks/NetworkUSB/ESP32_S3_USBMidiCodec.h"

class ESP32_S3_NetworkUSB : public INetwork {
public:
    void begin() override;
    void sendMessage(const MidiMessage& message) override;
    void sendString(const String& message) override;
    std::optional<MidiMessage> readMessage() override;

private:
    ESP32_S3_USBMidiCodec m_codec;
    bool m_started = false;
};

#endif
