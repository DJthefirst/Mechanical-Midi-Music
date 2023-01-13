/*
 * NetworkUDP.h
 * 
 * !! Work In Progress !!
 * 
 */

#pragma once

#if !defined ARDUINO_ARCH_ESP8266 && !defined ARDUINO_ARCH_ESP32
// For now, this will only work with ESP8266 or ESP32
#else

#include "Arduino.h"
#include <ArduinoOTA.h>
#ifdef ARDUINO_ARCH_ESP8266
#include <ESP8266WiFi.h>
#elif ARDUINO_ARCH_ESP32
#include <WiFi.h>
#endif
#include <ESPAsyncWebServer.h>   //Local WebServer used to serve the configuration portal
#include <ESPAsyncWiFiManager.h> // https://github.com/alanswx/ESPAsyncWiFiManager WiFi Configuration Magic
#include <WiFiUdp.h>
#include <stdint.h>

#include "Networks/Network.h"

#define MULTICASTPORT 65534
#define UNICASTPORT 65534
#define MOPPY_MAX_PACKET_LENGTH 259

class NetworkUDP : public Network{

public:
    NetworkUDP() = default;
    void begin() override;
    void readMessage() override;
    void sendMessage(uint8_t message[], uint8_t length) override;

private:
    void startOTA();
    bool startUDP();
};

#endif /* ARDUINO_ARCH_ESP8266 or ARDUINO_ARCH_ESP32 */