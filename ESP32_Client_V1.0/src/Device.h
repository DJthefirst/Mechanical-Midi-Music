/*
 * Device.h
 * A Struct Representing this devices configuration
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
////////////////////////////////////////////////////////////////////////////////////////////////////d
  
    #ifndef DEVICE_CONFIG
        // Default device config used when no selection is provided by the application
        #define DEVICE_CONFIG "Configs/ESP32_PWM.h"
    #endif

    // Include the selected config header
    // Note: using an indirection macro to include a macro-defined string
    #define STRINGIFY(x) #x
    #define INCLUDE_FILE(x) STRINGIFY(x)
    // The macro DEVICE_CONFIG must be a quoted string literal (e.g. "Configs/StepperSynth.h")
    #include DEVICE_CONFIG

    // Cleanup helper macros
    #undef STRINGIFY
    #undef INCLUDE_FILE

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
#elif defined(ARDUINO_AVR_UNO)
    #define PLATFORM_ARDUINO_UNO  
    constexpr Platform PLATFORM_TYPE = Platform::ArduinoUno;
#elif defined(ARDUINO_AVR_MEGA)
    #define PLATFORM_ARDUINO_MEGA
    constexpr Platform PLATFORM_TYPE = Platform::ArduinoMega;
#else
    #error "Unsupported platform. Add platform detection to Device.h"
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////
// Configuration Processing 
////////////////////////////////////////////////////////////////////////////////////////////////////

namespace Config {

    // INSTRUMENT_PINS is derived from the configuration.
    constexpr auto PINS = INSTRUMENT_PINS;

    // Device identity
    constexpr const char* DEVICE_NAME_STR = DEVICE_NAME;
    constexpr uint16_t DEVICE_ID_VAL = DEVICE_ID;
    constexpr uint16_t FIRMWARE_VERSION = 2;

    // Optional settings with defaults
    #ifdef TIMER_RESOLUTION
        constexpr uint32_t TIMER_RESOLUTION = TIMER_RESOLUTION_VALUE;
    #else
        constexpr uint32_t TIMER_RESOLUTION = 40;  // Default 40 microseconds
    #endif

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

    #ifdef INSTRUMENT_TIMEOUT_MS_VALUE
        constexpr uint32_t INSTRUMENT_TIMEOUT_MS = INSTRUMENT_TIMEOUT_MS_VALUE;
    #else
        constexpr uint32_t INSTRUMENT_TIMEOUT_MS = 0;  // Default 0 = infinite timeout (no timeout)
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
    #else
        constexpr bool HAS_HARDWARE_PWM_VAL = false;
        constexpr bool HAS_WIFI_VAL = false;
        constexpr bool HAS_BLE_VAL = false;
        constexpr bool HAS_NVS_VAL = false;
    #endif
}

    // (no global pins alias — code should reference Config::INSTRUMENT_PINS or provide its own `pins`)

////////////////////////////////////////////////////////////////////////////////////////////////////
// Device Defaults
////////////////////////////////////////////////////////////////////////////////////////////////////

// Device Construct Constants
constexpr uint8_t DEVICE_NUM_NAME_BYTES = 20;
constexpr uint8_t DEVICE_NAME_OFFSET= 20;
constexpr uint8_t DEVICE_NUM_CFG_BYTES = 40;
constexpr uint8_t DEVICE_BOOL_OMNIMODE = 0x01;

namespace Device{
    // Golbal Device Default Attribues
    inline uint16_t ID = DEVICE_ID;
    inline std::string Name = "New Device";
    inline bool OmniMode = false;

    // Device information functions
    static uint8_t GetDeviceBoolean(){
        uint8_t deviceBoolByte = 0;
        if(Device::OmniMode) deviceBoolByte |= (1 << 0); //bit 0
        return deviceBoolByte;
    }

    static std::array<std::uint8_t,DEVICE_NUM_CFG_BYTES> GetDeviceConstruct(){
        std::array<std::uint8_t,DEVICE_NUM_CFG_BYTES> deviceObj;

        // Use runtime device ID so it can be changed at runtime via SysEx / LocalStorage
        uint16_t runtimeId = ID;
        deviceObj[0] = static_cast<uint8_t>((runtimeId >> 7) & 0x7F); //Device ID MSB
        deviceObj[1] = static_cast<uint8_t>((runtimeId >> 0) & 0x7F); //Device ID LSB
        deviceObj[2] = GetDeviceBoolean();
        deviceObj[3] = Config::NUM_INSTRUMENTS;;
        deviceObj[4] = Config::NUM_SUBINSTRUMENTS;
        deviceObj[5] = static_cast<uint8_t>(InstrumentType::Type);
        deviceObj[6] = static_cast<uint8_t>(PLATFORM_TYPE);
        deviceObj[7] = Config::MIN_NOTE;
        deviceObj[8] = Config::MAX_NOTE;
        deviceObj[9] = static_cast<uint8_t>((Config::FIRMWARE_VERSION >> 7) & 0x7F);
        deviceObj[10] = static_cast<uint8_t>((Config::FIRMWARE_VERSION >> 0) & 0x7F);

        for(uint8_t i = 0; i < DEVICE_NUM_NAME_BYTES; i++){
            if (Device::Name.size() >  i) deviceObj[DEVICE_NAME_OFFSET+i] = Device::Name[i];
            else deviceObj[DEVICE_NAME_OFFSET+i] = 0;
        }

        return deviceObj;
    }

    // Accessors for the device ID so it can be changed at runtime and persisted
    static uint16_t GetDeviceID(){ return ID; }
    static void SetDeviceID(uint16_t id){ ID = id; }
    
    // // Additional accessors for SysEx responses
    // static const char* GetDeviceName(){ return Name.c_str(); }
    // static void SetDeviceName(const std::string& name){ Name = name; }
    // static uint16_t GetFirmwareVersion(){ return Config::FIRMWARE_VERSION; }
    // static uint8_t GetNumInstruments(){ return Config::NUM_INSTRUMENTS; }
    // static uint8_t GetMinNote(){ return Config::MIN_NOTE; }
    // static uint8_t GetMaxNote(){ return Config::MAX_NOTE; }
};
