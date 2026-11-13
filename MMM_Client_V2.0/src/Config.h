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

#ifndef CFG_NUM_INSTRUMENTS
    #define CFG_NUM_INSTRUMENTS 1
#endif

#ifndef CFG_NUM_SUBINSTRUMENTS
    #define CFG_NUM_SUBINSTRUMENTS 1
#endif

#ifndef CFG_DEVICE_NAME
    #define CFG_DEVICE_NAME "New Device"
#endif

#ifndef CFG_DEVICE_ID
    #define CFG_DEVICE_ID 0
#endif

// INTERNAL preserves ESP32 NVS / Teensy EEPROM; SD_CARD is Teensy 4.1 only.
#define LOCAL_STORAGE_TYPE_INTERNAL 0
#define LOCAL_STORAGE_TYPE_SD_CARD 1
#ifndef CFG_LOCAL_STORAGE_TYPE
    #define CFG_LOCAL_STORAGE_TYPE INTERNAL
#endif
#define MMM_STORAGE_TYPE_EXPAND(type) LOCAL_STORAGE_TYPE_##type
#define MMM_STORAGE_TYPE_VALUE(type) MMM_STORAGE_TYPE_EXPAND(type)
static_assert(MMM_STORAGE_TYPE_VALUE(CFG_LOCAL_STORAGE_TYPE) == LOCAL_STORAGE_TYPE_INTERNAL ||
              MMM_STORAGE_TYPE_VALUE(CFG_LOCAL_STORAGE_TYPE) == LOCAL_STORAGE_TYPE_SD_CARD,
              "CFG_LOCAL_STORAGE_TYPE must be INTERNAL or SD_CARD");
#if MMM_STORAGE_TYPE_VALUE(CFG_LOCAL_STORAGE_TYPE) == LOCAL_STORAGE_TYPE_SD_CARD
    #define CFG_LOCAL_STORAGE_SD
#elif MMM_STORAGE_TYPE_VALUE(CFG_LOCAL_STORAGE_TYPE) != LOCAL_STORAGE_TYPE_INTERNAL
    #error "CFG_LOCAL_STORAGE_TYPE must be INTERNAL or SD_CARD"
#endif

#ifndef CFG_TIMER_RESOLUTION_US
    #define CFG_TIMER_RESOLUTION_US 8
#endif

#ifndef CFG_NOTE_TIMEOUT_MS
    #define CFG_NOTE_TIMEOUT_MS 0
#endif

#ifndef CFG_MIN_NOTE
    #define CFG_MIN_NOTE 0
#endif

#ifndef CFG_MAX_NOTE
    #define CFG_MAX_NOTE 127
#endif

#ifndef CFG_LIMITS_POS_MIN
    #define CFG_LIMITS_POS_MIN 0
#endif

#ifndef CFG_LIMITS_POS_MAX
    #define CFG_LIMITS_POS_MAX 150
#endif

#ifndef CFG_PINS_INSTRUMENT_ShiftRegister
    #define CFG_PINS_INSTRUMENT_ShiftRegister 0,0,0,0,0
#endif

#ifndef CFG_PINS_INSTRUMENT_ShiftRegister1
    #define CFG_PINS_INSTRUMENT_ShiftRegister1 0,0,0,0,0
#endif

#ifndef CFG_PINS_INSTRUMENT_ShiftRegister2
    #define CFG_PINS_INSTRUMENT_ShiftRegister2 0,0,0,0,0
#endif

#ifndef CFG_COMPONENT_PWM
    #if defined(CFG_INSTRUMENT_SWPWM) || defined(CFG_INSTRUMENT_HWPWM) || defined(CFG_INSTRUMENT_STEPSW) || defined(CFG_INSTRUMENT_STEPSWSHIFT)
        #define CFG_COMPONENT_PWM
    #endif
#endif

#ifndef CFG_COMPONENT_SHIFTREGISTER
    #if defined(CFG_INSTRUMENT_DULCIMER) || defined(CFG_INSTRUMENT_STEPSWSHIFT) || defined(CFG_INSTRUMENT_STEPPERSYNTHHW) || defined(CFG_INSTRUMENT_STEPPERSYNTHSW)
        #define CFG_COMPONENT_SHIFTREGISTER
    #endif
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
        #if defined(CFG_ESP32_S3)
            #define HAS_USB_MIDI
        #endif
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

#if defined(CFG_LOCAL_STORAGE_SD) && !defined(PLATFORM_TEENSY41)
    #error "SD local storage requires Teensy 4.1"
#endif

namespace HardwareConfig {

    constexpr uint8_t MAX_NUM_INSTRUMENTS = CFG_NUM_INSTRUMENTS * CFG_NUM_SUBINSTRUMENTS;
    static_assert(CFG_NUM_INSTRUMENTS > 0, "CFG_NUM_INSTRUMENTS must be greater than zero");
    static_assert(CFG_NUM_SUBINSTRUMENTS > 0, "CFG_NUM_SUBINSTRUMENTS must be greater than zero");
    static_assert(CFG_NUM_INSTRUMENTS * CFG_NUM_SUBINSTRUMENTS <= NUM_Instruments,
                  "Configured instrument count exceeds the 32-instrument protocol limit");
    static_assert(CFG_TIMER_RESOLUTION_US > 0, "CFG_TIMER_RESOLUTION_US must be greater than zero");
    static_assert(CFG_MIN_NOTE >= 0 && CFG_MAX_NOTE <= 127 && CFG_MIN_NOTE <= CFG_MAX_NOTE,
                  "Configured MIDI note range must be ordered and within 0-127");

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


    #ifdef CFG_PINS_INSTRUMENT_PWM
        constexpr std::array<uint8_t, NUM_Instruments> PINS_INSTRUMENT_PWM = {CFG_PINS_INSTRUMENT_PWM};
    #else
        constexpr std::array<uint8_t, NUM_Instruments> PINS_INSTRUMENT_PWM = {};
    #endif
}

namespace DeviceConfig {
    
     // Device identity
    constexpr const char* DEVICE_NAME_STR = CFG_DEVICE_NAME;
    constexpr uint16_t DEVICE_ID_VAL = CFG_DEVICE_ID;
    constexpr uint16_t FIRMWARE_VERSION = 2;

}
    // (no global pins alias — code should reference Config::INSTRUMENT_PINS or provide its own `pins`)