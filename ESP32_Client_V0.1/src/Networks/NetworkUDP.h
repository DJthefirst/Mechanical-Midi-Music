/*
 * MoppyUDP.h
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

#include "MessageHandler.h"

#define MULTICASTPORT 65534
#define UNICASTPORT 65534
#define MOPPY_MAX_PACKET_LENGTH 259

class NetworkUDP{
private:
    MessageHandler* m_ptrMessageHandler;

    uint8_t m_messagePos = 0;                         // Track current message read position
    uint8_t m_messageBuffer[MOPPY_MAX_PACKET_LENGTH]; // Max message length for Moppy messages is 259

public:
    NetworkUDP();
    void Initalize(MessageHandler* ptrMessageHandler);
    void Begin();
    void ReadMessages();

private:
    void StartOTA();
    bool StartUDP();
    void ParseMessage(uint8_t message[], int length);
    void SendData(uint8_t message[], int length);
    void SendPong();
};

#endif /* ARDUINO_ARCH_ESP8266 or ARDUINO_ARCH_ESP32 */