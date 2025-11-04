/*
 * Config.h
 * Hardware and device configuration structure and platform-specific definitions
 * Split from Device.h to separate configuration from device functionality
 */

#pragma once

#include "Arduino.h"
#include "Constants.h"
#include <array>
#include <stdint.h>
#include <cstdint>
using std::int8_t;

////////////////////////////////////////////////////////////////////////////////////////////////////
// Device Config Include
////////////////////////////////////////////////////////////////////////////////////////////////////
  
    // Default device config used when no selection is provided by the application
    #ifndef DEVICE_CONFIG
        // #define DEVICE_CONFIG "Configs/Example/HwPwmCfg.h"
        // #define DEVICE_CONFIG "Configs/Example/SwPwmCfg.h"
        // #define DEVICE_CONFIG "Configs/DJthefirst/Teensy41HwPwmCfg.h"
        // #define DEVICE_CONFIG "Configs/DJthefirst/Teensy41SwPwmCfg.h"
        #define DEVICE_CONFIG "Configs/DJthefirst/StepperSynthTeensyCfg.h"
        // #define DEVICE_CONFIG "Configs/DJthefirst/StepperSynthSwCfg.h"
        // #define DEVICE_CONFIG "Configs/DJthefirst/StepperSynthHwCfg.h"
    #endif

    // Include the selected config header
    #define STRINGIFY(x) #x
    #define INCLUDE_FILE(x) STRINGIFY(x)
        #include DEVICE_CONFIG
    #undef STRINGIFY
    #undef INCLUDE_FILE

    #ifndef INSTRUMENT_TYPE_VALUE
        #define INSTRUMENT_TYPE_VALUE "Instruments/Base/SwPWM/SwPWM.h"
    #endif

////////////////////////////////////////////////////////////////////////////////////////////////////
// Platform Detection
////////////////////////////////////////////////////////////////////////////////////////////////////

    #if defined(ARDUINO_ARCH_ESP32)
        #define PLATFORM_ESP32
        constexpr Platform PLATFORM_TYPE = Platform::_ESP32;
        #define HAS_HARDWARE_PWM
        #define HAS_WIFI
        #define HAS_BLE
        #define HAS_NVS
    #elif defined(ARDUINO_TEENSY41) || defined(__IMXRT1062__)
        #define PLATFORM_TEENSY41
        constexpr Platform PLATFORM_TYPE = Platform::_Teensy41;
        #define HAS_HARDWARE_PWM
        #define HAS_USB_MIDI
    #elif defined(ARDUINO_AVR_UNO)
        #define PLATFORM_ARDUINO_UNO  
        constexpr Platform PLATFORM_TYPE = Platform::ArduinoUno;
    #elif defined(ARDUINO_AVR_MEGA)
        #define PLATFORM_ARDUINO_MEGA
        constexpr Platform PLATFORM_TYPE = Platform::ArduinoMega;
    #else
        #error "Unsupported platform. Add platform detection to Config.h"
    #endif

////////////////////////////////////////////////////////////////////////////////////////////////////
// Configuration Processing 
////////////////////////////////////////////////////////////////////////////////////////////////////

namespace HardwareConfig {

    // INSTRUMENT_PINS is derived from the configuration.
    constexpr auto PINS = INSTRUMENT_PINS;

    // Optional settings with defaults
    #ifdef TIMER_RESOLUTION_US_VALUE
        constexpr uint32_t TIMER_RESOLUTION = TIMER_RESOLUTION_US_VALUE;
    #else
        constexpr uint32_t TIMER_RESOLUTION = 40;  // Default 40 microseconds
    #endif

    #ifdef INSTRUMENT_TIMEOUT_MS_VALUE
        constexpr uint32_t INSTRUMENT_TIMEOUT_MS = INSTRUMENT_TIMEOUT_MS_VALUE;
    #else
        constexpr uint32_t INSTRUMENT_TIMEOUT_MS = 15000;  // Default 0 = infinite timeout (no timeout)
    #endif

    #ifdef NUM_INSTRUMENTS_VALUE
    constexpr uint8_t NUM_INSTRUMENTS = NUM_INSTRUMENTS_VALUE;
    #else
    constexpr uint8_t NUM_INSTRUMENTS = static_cast<uint8_t>(PINS.size());
    #endif

    #ifdef NUM_SUBINSTRUMENTS_VALUE
    constexpr uint8_t NUM_SUBINSTRUMENTS = NUM_SUBINSTRUMENTS_VALUE;
    #else
    constexpr uint8_t NUM_SUBINSTRUMENTS = 1;
    #endif

    constexpr uint8_t MAX_NUM_INSTRUMENTS = NUM_INSTRUMENTS * NUM_SUBINSTRUMENTS;


    // Platform capabilities
    #ifdef PLATFORM_ESP32
        constexpr bool HAS_HARDWARE_PWM_VAL = true;
        constexpr bool HAS_WIFI_VAL = true;
        constexpr bool HAS_BLE_VAL = true;
        constexpr bool HAS_NVS_VAL = true;
    #elif defined(PLATFORM_TEENSY41)
        constexpr bool HAS_HARDWARE_PWM_VAL = true;
        constexpr bool HAS_WIFI_VAL = false;
        constexpr bool HAS_BLE_VAL = false;
        constexpr bool HAS_NVS_VAL = false;
    #else
        constexpr bool HAS_HARDWARE_PWM_VAL = false;
        constexpr bool HAS_WIFI_VAL = false;
        constexpr bool HAS_BLE_VAL = false;
        constexpr bool HAS_NVS_VAL = false;
    #endif
}

namespace DeviceConfig {
    
     // Device identity
    constexpr const char* DEVICE_NAME_STR = DEVICE_NAME;
    constexpr uint16_t DEVICE_ID_VAL = DEVICE_ID;
    constexpr uint16_t FIRMWARE_VERSION = 2;

    #ifdef MIN_NOTE_VALUE
        constexpr uint8_t MIN_NOTE = MIN_NOTE_VALUE;
    #else
        constexpr uint8_t MIN_NOTE = 0;
    #endif

    #ifdef MAX_NOTE_VALUE
        constexpr uint8_t MAX_NOTE = MAX_NOTE_VALUE;
    #else
        constexpr uint8_t MAX_NOTE = 127;
    #endif

}
    // (no global pins alias — code should reference Config::INSTRUMENT_PINS or provide its own `pins`)