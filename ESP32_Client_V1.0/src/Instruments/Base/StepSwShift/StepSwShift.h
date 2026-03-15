#pragma once

#ifndef INSTRUMENT_TYPE
    #define INSTRUMENT_TYPE StepSwShift
#endif

// Platform-specific includes
#if defined(PLATFORM_ESP32) || defined(ARDUINO_ARCH_ESP32)
    #include "Instruments/Base/StepSwShift/ESP32_StepSwShift.h"
    #define StepSwShift ESP32_StepSw
#elif defined(__IMXRT1062__) || defined(ARDUINO_TEENSY41)
    #include "Instruments/Base/StepSwShift/Teensy41_StepSwShift.h"
    #define StepSwShift Teensy41_StepSwShift
#elif defined(ARDUINO) && !defined(ESP8266)
    // Potential Arduino Mega or similar - add specific implementation if available
    //#include "Instruments/Base/StepSw/ArduinoMega_StepSw.h"
    //using PlatformStepSw = ArduinoMega_StepSw;
#else
    // Default fallback - use a generic PWM base implementation
    #include "Instruments/Base/StepSwShift/ESP32_StepSwShift.h"
    #define StepSwShift ESP32_StepSw
#endif