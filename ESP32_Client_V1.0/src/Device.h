/*
 * Device.h
 * Device functionality and runtime state management
 * Configuration definitions moved to Config.h
 */

#pragma once

#include "Config.h"
#include <array>
#include <string>
#include <cstdint>
#include <bitset>

using std::int8_t;

////////////////////////////////////////////////////////////////////////////////////////////////////
// Device Defaults
////////////////////////////////////////////////////////////////////////////////////////////////////

// Device Construct constants
constexpr uint8_t DEVICE_NUM_NAME_BYTES = 20;
constexpr uint8_t DEVICE_NAME_OFFSET= 20;
constexpr uint8_t DEVICE_NUM_CFG_BYTES = 40;

// Device Boolean bit masks (matching protocol specification)
namespace DEVICE_BOOL_MASK{
    const uint16_t MUTED = 1 << 0;
    const uint16_t OMNIMODE = 1 << 1;
};

namespace Device{
    // Global device default attributes
    inline uint16_t ID = DeviceConfig::DEVICE_ID_VAL;
    inline std::string Name = "New Device";
    inline bool Muted = false;
    inline bool OmniMode = false;
    inline Instrument InstrumentType = Instrument::None;  // Runtime instrument type

    // Device information functions
    static uint16_t GetDeviceBoolean(){
        uint16_t deviceBoolByte = 0;
        if(Device::Muted) deviceBoolByte |= DEVICE_BOOL_MASK::MUTED;     // Set bit 0 for muted
        if(Device::OmniMode) deviceBoolByte |= DEVICE_BOOL_MASK::OMNIMODE;  // Set bit 1 for omni mode
        return deviceBoolByte;
    }

    static std::array<std::uint8_t,DEVICE_NUM_CFG_BYTES> GetDeviceConstruct(){
        std::array<std::uint8_t,DEVICE_NUM_CFG_BYTES> deviceObj = {};  // Zero-initialize all bytes

        // Use runtime device ID so it can be changed at runtime via SysEx / LocalStorage
        uint16_t runtimeId = ID;
        deviceObj[0] = static_cast<uint8_t>((runtimeId >> 7) & 0x7F); // Device ID MSB
        deviceObj[1] = static_cast<uint8_t>((runtimeId >> 0) & 0x7F); // Device ID LSB
        deviceObj[2] = HardwareConfig::NUM_INSTRUMENTS;
        deviceObj[3] = HardwareConfig::NUM_SUBINSTRUMENTS;
        deviceObj[4] = static_cast<uint8_t>(InstrumentType);
        deviceObj[5] = static_cast<uint8_t>(PLATFORM_TYPE);
        deviceObj[6] = DeviceConfig::MIN_NOTE;
        deviceObj[7] = DeviceConfig::MAX_NOTE;
        deviceObj[8] = static_cast<uint8_t>((DeviceConfig::FIRMWARE_VERSION >> 7) & 0x7F);
        deviceObj[9] = static_cast<uint8_t>((DeviceConfig::FIRMWARE_VERSION >> 0) & 0x7F);
        deviceObj[10] = (GetDeviceBoolean() >> 7) & 0x7F;
        deviceObj[11] = (GetDeviceBoolean() >> 0) & 0x7F;
        
        // Bytes 12-19 are reserved and remain 0x00 (initialized above)

        for(uint8_t i = 0; i < DEVICE_NUM_NAME_BYTES; i++){
            if (Device::Name.size() >  i) deviceObj[DEVICE_NAME_OFFSET+i] = Device::Name[i];
            else deviceObj[DEVICE_NAME_OFFSET+i] = 0;
        }

        return deviceObj;
    }

    // Accessors for the device ID so it can be changed at runtime and persisted
    static uint16_t GetDeviceID() noexcept { return ID; }
    static void SetDeviceID(uint16_t id) noexcept { ID = id; }
    
    // // Additional accessors for SysEx responses
    // static const char* GetDeviceName(){ return Name.c_str(); }
    // static void SetDeviceName(const std::string& name){ Name = name; }
    // static uint16_t GetFirmwareVersion(){ return Config::FIRMWARE_VERSION; }
    // static uint8_t GetNumInstruments(){ return Config::NUM_INSTRUMENTS; }
    // static uint8_t GetMinNote(){ return Config::MIN_NOTE; }
    // static uint8_t GetMaxNote(){ return Config::MAX_NOTE; }
};
