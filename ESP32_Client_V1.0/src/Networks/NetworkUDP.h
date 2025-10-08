/*
 * NetworkUDP.h
 *
 * Network implementation for MIDI over UDP with WiFi manager and OTA support
 */

#pragma once

#ifdef MMM_NETWORK_UDP

#include "INetwork.h"
#include <WiFi.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <ESPAsyncWebServer.h>
#include <DNSServer.h>
#include <ESPAsyncWiFiManager.h>

class NetworkUDP : public INetwork {
public:
    NetworkUDP() = default;
    void begin() override;
    void sendMessage(const MidiMessage& message) override;
    void sendString(const String& message) override;
    std::optional<MidiMessage> readMessage() override;

private:
    bool startUDP();
    void startOTA();
    
    // Legacy method - needs refactoring
    void sendMessage(uint8_t message[], uint8_t length);
};

#endif