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

    constexpr uint8_t MAX_NUM_INSTRUMENTS = CFG_NUM_INSTRUMENTS * CFG_NUM_SUBINSTRUMENTS;


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
    constexpr const char* DEVICE_NAME_STR = CFG_DEVICE_NAME;
    constexpr uint16_t DEVICE_ID_VAL = CFG_DEVICE_ID;
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