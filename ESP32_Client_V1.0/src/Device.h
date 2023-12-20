/*
 * Device.h
 * A Struct Representing this devices configuration
 */

#pragma once

#include "Arduino.h"
#include <stdint.h>
#include <cstdint>
#include "Constants.h"
#include "Configs/AirCompressor.h"
using std::int8_t;

//Number 
const uint8_t NUM_DEVICE_CFG_BYTES = 30;

namespace Device
{
    //Golbal Device Attribues
    //inline char[20] Name = "New Device";
    inline std::string Name = "New Device";
    inline bool OmniMode = false;


    //C++ v11
    // extern bool g_OmniMode; //in the header file
    // bool g_OmniMode = false; // in exaclty *one* cpp file in the project
}

// ---------- Device Platform ----------

#ifndef PLATFORM_ESP32
    //#define ARDUINO_ARCH_ESP32

    //Valid Pins ESP32 |2 4 12 13 16 17 18 19 21 22 23 25 26 27 32 33|
    const std::array<uint8_t,8> pins = {2,4,18,19,21,22,23,32};
    // const std::array<uint8_t,16> pins = {2, 4, 12, 13, 16, 17, 18, 19, 21, 22, 23,
    //  25, 26, 27, 32, 33};
    
    const char platformName[] = "ESP32";
    const PlatformType platform = PLATFORM_ESP32;

#elif PLATFORM_ESP8266
    //#define ARDUINO_ARCH_ESP32
    const uint8_t pins[] = {};

    const char platform[] = "ESP8266";
    const PlatformType platform = PLATFORM_ESP8266;

#elif PLATFORM_ARDUINO_UNO
    //#define ARDUINO_ARCH_AVR
    const uint8_t pins[] = {};

    const char platform[] = "Arduino Uno";
    const PlatformType platform = PLATFORM_ArduinoUno;

#elif PLATFORM_ARDUINO_MEGA
    //#define ARDUINO_ARCH_AVR
    const uint8_t pins[] = {};

    const char platform[] = "Arduino Mega";
    const PlatformType platform = PLATFORM_ArduinoMega;

#elif PLATFORM_ARDUINO_DUE
    //#define ARDUINO_ARCH_
    const uint8_t pins[] = {};

    const char platform[] = "Arduino Due";
    const PlatformType platform = PLATFORM_ArduinoDue;

#elif PLATFORM_ARDUINO_MICRO
    //#define ARDUINO_ARCH_AVR
    const uint8_t pins[] = {};

    const char platform[] = "Arduino Micro";
    const PlatformType platform = PLATFORM_ArduinoMicro;

#elif PLATFORM_ARDUINO_NANO
    //#define ARDUINO_ARCH_AVR
    const uint8_t pins[] = {};

    const char platform[] = "Arduino Nano";
    const PlatformType platform = PLATFORM_ArduinoNano;

#endif