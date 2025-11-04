#pragma once

#ifndef INSTRUMENT_TYPE
    #define INSTRUMENT_TYPE SwPWM
#endif

// Platform-specific includes
#if defined(ESP32)
    #include "Instruments/Base/SwPWM/ESP32_SwPWM.h"
    #define SwPWM ESP32_SwPWM
#elif defined(__IMXRT1062__) || defined(ARDUINO_TEENSY41)
    #include "Instruments/Base/SwPWM/Teensy41_SwPWM.h"
    #define SwPWM Teensy41_SwPWM
#elif defined(ARDUINO) && !defined(ESP8266)
    #include "Instruments/Base/SwPWM/ArduinoMega_SwPWM.h"
    #define SwPWM ArduinoMega_SwPWM
#else
    // Default fallback - could be extended for other platforms
    // Use the regular PWM implementation as the platform-specific base
    #include "Instruments/Base/PWM/ESP32_PwmBase.h"
    #define SwPWM ESP32_PwmBase
#endif