#pragma once

#ifndef INSTRUMENT_TYPE
    #define INSTRUMENT_TYPE HwPWM
#endif

// Platform-specific includes
#if defined(ESP32)
    #include "Instruments/Base/HwPWM/ESP32_HwPWM.h"
    #define HwPWM ESP32_HwPWM
#elif defined(__IMXRT1062__) || defined(ARDUINO_TEENSY41)
    #include "Instruments/Base/HwPWM/Teensy41_HwPWM.h"
    #define HwPWM Teensy41_HwPWM
#elif defined(ARDUINO) && !defined(ESP8266)
    #include "Instruments/Base/HwPWM/ArduinoMega_HwPWM.h"
    #define HwPWM ArduinoMega_HwPWM
#else
    // Default fallback - could be extended for other platforms
    #include "Instruments/Base/HwPWM/ESP32_HwPWM.h"
    #define HwPWM ESP32_HwPWM
#endif