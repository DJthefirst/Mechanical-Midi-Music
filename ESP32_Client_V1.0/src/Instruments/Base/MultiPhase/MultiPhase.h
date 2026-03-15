#pragma once

#ifndef INSTRUMENT_TYPE
    #define INSTRUMENT_TYPE MultiPhase
#endif


// Platform-specific includes
#if defined(PLATFORM_ESP32) || defined(ARDUINO_ARCH_ESP32)
    #include "Instruments/Base/MultiPhase/ESP32_MultiPhase.h"
    #define MultiPhase ESP32_MultiPhase
#elif defined(__IMXRT1062__) || defined(ARDUINO_TEENSY41)
    #include "Instruments/Base/MultiPhase/Teensy41_MultiPhase.h"
    #define MultiPhase Teensy41_MultiPhase
#elif defined(ARDUINO) && !defined(ESP8266)
    // Potential Arduino Mega or similar - add specific implementation if available
    #include "Instruments/Base/MultiPhase/ArduinoMega_MultiPhase.h"
    using PlatformMultiPhase = ArduinoMega_MultiPhase;
#else
    // Default fallback - use a generic base implementation
    #include "Instruments/Base/MultiPhase/ESP32_MultiPhase.h"
    #define MultiPhase ESP32_MultiPhase
#endif