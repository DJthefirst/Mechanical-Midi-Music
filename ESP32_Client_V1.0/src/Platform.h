#pragma once

#include <stdint.h>

#ifndef DEVICE_TYPE_ESP32
    //#define ARDUINO_ARCH_ESP32

    //Valid Pins ESP32 |2 4 12 13 16 17 18 19 21 22 23 25 26 27 32 33|
    const uint8_t pins[] = {2,4,18,19,21,22,23,13,12,25,26,27,32,33};
    
    const char deviceType[] = "ESP32";

#elif DEVICE_TYPE_ESP8266
    //#define ARDUINO_ARCH_ESP32
    const uint8_t pins[] = {};

    const char deviceType[] = "ESP8266";

#elif DEVICE_TYPE_ARDUINO_UNO
    //#define ARDUINO_ARCH_AVR
    const uint8_t pins[] = {};

    const char deviceType[] = "Arduino Uno";

#elif DEVICE_TYPE_ARDUINO_MEGA
    //#define ARDUINO_ARCH_AVR
    const uint8_t pins[] = {};

    const char deviceType[] = "Arduino Mega";

#elif DEVICE_TYPE_ARDUINO_DUE
    //#define ARDUINO_ARCH_
    const uint8_t pins[] = {};

    const char deviceType[] = "Arduino Due";

#elif DEVICE_TYPE_ARDUINO_MICRO
    //#define ARDUINO_ARCH_AVR
    const uint8_t pins[] = {};

    const char deviceType[] = "Arduino Micro";

#elif DEVICE_TYPE_ARDUINO_NANO
    //#define ARDUINO_ARCH_AVR
    const uint8_t pins[] = {};

    const char deviceType[] = "Arduino Nano";

#endif
