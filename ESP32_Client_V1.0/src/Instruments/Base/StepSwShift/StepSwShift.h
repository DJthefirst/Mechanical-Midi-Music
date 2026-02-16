#pragma once

#ifndef INSTRUMENT_TYPE
    #define INSTRUMENT_TYPE StepSw
#endif

// Platform-specific includes
#if defined(PLATFORM_ESP32) || defined(ARDUINO_ARCH_ESP32)
    #include "Instruments/Base/StepSw/ESP32_StepSw.h"
    #define StepSw ESP32_StepSw
#elif defined(PLATFORM_TEENSY41) || defined(ARDUINO_ARCH_TEENSY41)
    #include "Instruments/Base/StepSw/Teensy41_StepSw.h"
    #define StepSw Teensy41_StepSw
#elif defined(ARDUINO) && !defined(ESP8266)
    // Potential Arduino Mega or similar - add specific implementation if available
    //#include "Instruments/Base/StepSw/ArduinoMega_StepSw.h"
    //using PlatformStepSw = ArduinoMega_StepSw;
#else
    // Default fallback - use a generic PWM base implementation
    #include "Instruments/Base/PWM/ESP32_PwmBase.h"
    #define StepSw ESP32_PwmBase
#endif