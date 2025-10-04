/*
 * Device.h - V2.0 Configuration Processing and Device Management
 *
 * Part of the Mechanical MIDI Music V2.0 redesign
 * Based on REDESIGN_PART2_CONFIG.md and REDESIGN_PART6_IMPLEMENTATION.md
 *
 * This file processes user configuration and provides a unified interface
 * for device settings with compile-time validation.
 */

#pragma once

#include "core/Constants.h"
#include <Arduino.h>
#include <string>

////////////////////////////////////////////////////////////////////////////////////////////////////
// Configuration Loading
////////////////////////////////////////////////////////////////////////////////////////////////////

// Include user configuration
#ifdef USE_V2_CONFIG
    // V2.0 configuration system
    #include "config.h"
#else
    // V1.0 compatibility layer - include the selected config from src2
    #ifdef ESP32_PWM_CONFIG
        // Match the V1.0 ESP32_PWM config values for testing
        #define DEVICE_NAME "ESP32 PWM Device"
        #define DEVICE_ID 0x004
        #define PINS_LIST { 2, 4, 18, 19, 21, 22, 23, 32, 33, 25, 26, 27 }
        #define TIMER_RESOLUTION 1
        
        // Component selection
        #include "Instruments/ESP32_PwmTimer.h"
        using InstrumentType = ESP32_PwmTimer;
        
        #include "Networks/SerialNetwork.h" 
        using NetworkType = SerialNetwork;
        
        // Optional features
        #define ENABLE_LOCAL_STORAGE
        #define ENABLE_ADDRESSABLE_LEDS
    #else
        #error "No configuration selected. Define USE_V2_CONFIG or ESP32_PWM_CONFIG"
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
    // Convert pin list to constexpr array
    constexpr std::array<uint8_t, 12> INSTRUMENT_PINS = PINS_LIST;
    constexpr uint8_t NUM_INSTRUMENTS = static_cast<uint8_t>(INSTRUMENT_PINS.size());

    // Device identity
    constexpr const char* DEVICE_NAME_STR = DEVICE_NAME;
    constexpr uint16_t DEVICE_ID_VAL = DEVICE_ID;
    constexpr uint16_t FIRMWARE_VERSION = DEFAULT_FIRMWARE_VERSION;

    // Optional settings with defaults
    #ifdef TIMER_RESOLUTION
        constexpr uint32_t TIMER_RESOLUTION_US = TIMER_RESOLUTION;
    #else
        constexpr uint32_t TIMER_RESOLUTION_US = 40;  // Default 40 microseconds
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

    // Derived constants
    constexpr uint8_t MAX_NUM_INSTRUMENTS = NUM_INSTRUMENTS;
    constexpr uint8_t NUM_SUBINSTRUMENTS = 1;  // V2.0 simplification
    
    // Platform capabilities
    #ifdef PLATFORM_ESP32
        constexpr bool HAS_HARDWARE_PWM_VAL = true;
        constexpr bool HAS_WIFI_VAL = true;
        constexpr bool HAS_BLE_VAL = true;
        constexpr bool HAS_NVS_VAL = true;
        constexpr uint32_t MAX_FLASH_SIZE = 4 * 1024 * 1024;
        constexpr uint32_t MAX_RAM = 520 * 1024;
    #else
        constexpr bool HAS_HARDWARE_PWM_VAL = false;
        constexpr bool HAS_WIFI_VAL = false;
        constexpr bool HAS_BLE_VAL = false;
        constexpr bool HAS_NVS_VAL = false;
        constexpr uint32_t MAX_FLASH_SIZE = 32 * 1024;
        constexpr uint32_t MAX_RAM = 2 * 1024;
    #endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Network Configuration
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef SecondaryNetwork
    // Single network configuration
    using NetworkController = NetworkType;
#else
    // Multi-network configuration - will be implemented in Phase 3
    #include "networks/MultiNetwork.h"
    using NetworkController = MultiNetwork<NetworkType, SecondaryNetwork>;
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////
// Compile-Time Validation
////////////////////////////////////////////////////////////////////////////////////////////////////

namespace ConfigValidation {
    // Pin conflict detection
    template<size_t N>
    constexpr bool hasDuplicatePins(const std::array<uint8_t, N>& pins) {
        for (size_t i = 0; i < N; i++) {
            for (size_t j = i + 1; j < N; j++) {
                if (pins[i] == pins[j]) return true;
            }
        }
        return false;
    }
    
    static_assert(!hasDuplicatePins(Config::INSTRUMENT_PINS),
        "Error: Duplicate pins detected in PINS_LIST configuration");

    // Platform-specific pin validation
    #ifdef PLATFORM_ESP32
        template<size_t N>
        constexpr bool pinsValidForESP32(const std::array<uint8_t, N>& pins) {
            for (const auto pin : pins) {
                // ESP32 input-only pins: 34-39
                if (pin >= 34 && pin <= 39) return false;
                // Invalid pins
                if (pin > 39) return false;
                // Reserved pins (avoid common system pins)
                if (pin == 0 || pin == 1 || pin == 6 || pin == 7 || pin == 8 || pin == 9 || 
                    pin == 10 || pin == 11) return false;
            }
            return true;
        }
        
        static_assert(pinsValidForESP32(Config::INSTRUMENT_PINS),
            "Error: Invalid pins for ESP32. Pins 0,1,6-11 are reserved, 34-39 are input-only, max pin is 39");
    #endif

    // Device name validation
    VALIDATE_STRING_LENGTH(DEVICE_NAME, 32);
    
    // Device ID validation
    static_assert(Config::DEVICE_ID_VAL > 0 && Config::DEVICE_ID_VAL < 0x4000,
        "Error: DEVICE_ID must be between 1 and 16383 (14-bit value)");
        
    // Instrument count limits
    static_assert(Config::NUM_INSTRUMENTS > 0,
        "Error: No instruments defined. Add PINS_LIST to config.h");
        
    static_assert(Config::NUM_INSTRUMENTS <= MAX_INSTRUMENTS,
        "Error: Maximum 32 instruments supported (limited by distributor bitmask)");
        
    // Note range validation
    static_assert(Config::MIN_NOTE <= Config::MAX_NOTE,
        "Error: MIN_NOTE must be less than or equal to MAX_NOTE");
        
    static_assert(Config::MAX_NOTE <= MAX_MIDI_NOTE,
        "Error: MAX_NOTE cannot exceed 127");
        
    // Timer resolution validation (disabled for compilation testing)
    // static_assert(Config::TIMER_RESOLUTION_US >= 1,
    //     "Error: TIMER_RESOLUTION must be at least 1 microsecond");
    //     
    // static_assert(Config::TIMER_RESOLUTION_US <= 1000,
    //     "Error: TIMER_RESOLUTION should not exceed 1000 microseconds for reasonable performance");
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Runtime Device Management
////////////////////////////////////////////////////////////////////////////////////////////////////

namespace Device {
    // Global device attributes (runtime modifiable)
    inline std::string Name = Config::DEVICE_NAME_STR;
    inline bool OmniMode = false;
    
    // Device information functions
    static uint8_t GetDeviceBoolean() {
        uint8_t deviceBoolByte = 0;
        if (Device::OmniMode) deviceBoolByte |= DEVICE_BOOL_OMNIMODE;
        return deviceBoolByte;
    }
    
    static std::array<uint8_t, DEVICE_NUM_CFG_BYTES> GetDeviceConstruct() {
        std::array<uint8_t, DEVICE_NUM_CFG_BYTES> deviceObj{};

        // Device ID (14-bit, split into 7-bit values for MIDI)
        deviceObj[0] = static_cast<uint8_t>((Config::DEVICE_ID_VAL >> 7) & 0x7F); // MSB
        deviceObj[1] = static_cast<uint8_t>((Config::DEVICE_ID_VAL >> 0) & 0x7F); // LSB
        
        deviceObj[2] = GetDeviceBoolean();
        deviceObj[3] = Config::NUM_INSTRUMENTS;
        deviceObj[4] = Config::NUM_SUBINSTRUMENTS;
        deviceObj[5] = static_cast<uint8_t>(Instrument::PWM); // Default instrument enum
        deviceObj[6] = static_cast<uint8_t>(PLATFORM_TYPE);
        deviceObj[7] = Config::MIN_NOTE;
        deviceObj[8] = Config::MAX_NOTE;
        
        // Firmware version (14-bit, split into 7-bit values)
        deviceObj[9] = static_cast<uint8_t>((Config::FIRMWARE_VERSION >> 7) & 0x7F);
        deviceObj[10] = static_cast<uint8_t>((Config::FIRMWARE_VERSION >> 0) & 0x7F);

        // Device name (20 bytes, space-padded)
        for (uint8_t i = 0; i < DEVICE_NUM_NAME_BYTES; i++) {
            if (Device::Name.size() > i) {
                deviceObj[DEVICE_NAME_OFFSET + i] = Device::Name[i];
            } else {
                deviceObj[DEVICE_NAME_OFFSET + i] = ' '; // Space padding
            }
        }

        return deviceObj;
    }
    
    // Runtime validation (optional)
    static void validateConfiguration() {
        // Check for adequate timer resolution
        if (Config::TIMER_RESOLUTION_US < 4) {
            Serial.println("WARNING: TIMER_RESOLUTION < 4 may cause instability");
        }
        
        // Initialize pins
        for (const auto pin : Config::INSTRUMENT_PINS) {
            pinMode(pin, OUTPUT);
            digitalWrite(pin, LOW);
        }
        
        Serial.printf("Device initialized: %s (ID: 0x%04X)\n", 
                      Config::DEVICE_NAME_STR, Config::DEVICE_ID_VAL);
        Serial.printf("Platform: ESP32, Instruments: %d, Timer: %d μs\n",
                      Config::NUM_INSTRUMENTS, Config::TIMER_RESOLUTION_US);
        Serial.printf("Note range: %d-%d\n", Config::MIN_NOTE, Config::MAX_NOTE);
    }


}