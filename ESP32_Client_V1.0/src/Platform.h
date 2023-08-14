/*
 * Platform.h
 * 
 * Used to Setup Predefined Devices spcifying their pinnout and platform
 */

#pragma once
#include <array>

#include <cstdint>
using std::int8_t;

#ifndef PLATFORM_ESP32
    //#define ARDUINO_ARCH_ESP32

    //Valid Pins ESP32 |2 4 12 13 16 17 18 19 21 22 23 25 26 27 32 33|
    const std::array<uint8_t,8> pins = {2,4,18,19,21,22,23,32};
    
    const char platform[] = "ESP32";

#elif PLATFORM_ESP8266
    //#define ARDUINO_ARCH_ESP32
    const uint8_t pins[] = {};

    const char platform[] = "ESP8266";

#elif PLATFORM_ARDUINO_UNO
    //#define ARDUINO_ARCH_AVR
    const uint8_t pins[] = {};

    const char platform[] = "Arduino Uno";

#elif PLATFORM_ARDUINO_MEGA
    //#define ARDUINO_ARCH_AVR
    const uint8_t pins[] = {};

    const char platform[] = "Arduino Mega";

#elif PLATFORM_ARDUINO_DUE
    //#define ARDUINO_ARCH_
    const uint8_t pins[] = {};

    const char platform[] = "Arduino Due";

#elif PLATFORM_ARDUINO_MICRO
    //#define ARDUINO_ARCH_AVR
    const uint8_t pins[] = {};

    const char platform[] = "Arduino Micro";

#elif PLATFORM_ARDUINO_NANO
    //#define ARDUINO_ARCH_AVR
    const uint8_t pins[] = {};

    const char platform[] = "Arduino Nano";

#endif
