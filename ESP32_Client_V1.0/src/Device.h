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
constexpr uint8_t DEVICE_NAME_OFFSET= 36;
constexpr uint8_t DEVICE_NUM_CFG_BYTES = 56;

// Device Boolean bit masks (matching protocol specification)
namespace DEVICE_BOOL_MASK{
    constexpr uint16_t MUTED = 1 << 0;
    constexpr uint16_t OMNIMODE = 1 << 1;
    constexpr uint16_t DAMPER_PEDAL = 1 << 2;
    constexpr uint16_t VIBRATO = 1 << 3;
};

namespace Device{
    // Global device default attributes
    inline uint16_t ID = DeviceConfig::DEVICE_ID_VAL;
    inline std::string Name = "New Device";
    inline bool Muted = false;
    inline bool OmniMode = false;
    inline bool DamperPedal = false;
    inline bool Vibrato = false;
    inline uint8_t NumPolyphonicNotes = 0; //0 for fully polyphonic, 1 for monophonic, >1 for limited polyphony
    inline Instrument InstrumentType = Instrument::None;  // Runtime instrument type

    // Device information functions
    static uint16_t GetDeviceBoolean(){
        uint16_t deviceBoolByte = 0;
        if(Device::Muted) deviceBoolByte |= DEVICE_BOOL_MASK::MUTED;     // Set bit 0 for muted
        if(Device::OmniMode) deviceBoolByte |= DEVICE_BOOL_MASK::OMNIMODE;  // Set bit 1 for omni mode
        if(Device::DamperPedal) deviceBoolByte |= DEVICE_BOOL_MASK::DAMPER_PEDAL;  // Set bit 2 for damper pedal
        if(Device::Vibrato) deviceBoolByte |= DEVICE_BOOL_MASK::VIBRATO;  // Set bit 3 for vibrato
        return deviceBoolByte;
    }

    static std::array<std::uint8_t,DEVICE_NUM_CFG_BYTES> GetDeviceConstruct(){
        std::array<std::uint8_t,DEVICE_NUM_CFG_BYTES> deviceObj = {};  // Zero-initialize all bytes

        // Use runtime device ID so it can be changed at runtime via SysEx / LocalStorage
        uint16_t runtimeId = ID;
        deviceObj[0] = static_cast<uint8_t>((runtimeId >> 7) & 0x7F); // Device ID MSB
        deviceObj[1] = static_cast<uint8_t>((runtimeId >> 0) & 0x7F); // Device ID LSB
        deviceObj[2] = CFG_NUM_INSTRUMENTS;
        deviceObj[3] = CFG_NUM_SUBINSTRUMENTS;
        deviceObj[4] = 0; // Reserved byte, set to 0
        deviceObj[5] = static_cast<uint8_t>(InstrumentType);
        deviceObj[6] = 0; // Reserved byte, set to 0
        deviceObj[7] = static_cast<uint8_t>(PLATFORM_TYPE);
        deviceObj[8] = 0; // Reserved byte, set to 0
        deviceObj[9] = static_cast<uint8_t>((DeviceConfig::FIRMWARE_VERSION >> 7) & 0x7F);
        deviceObj[10] = static_cast<uint8_t>((DeviceConfig::FIRMWARE_VERSION >> 0) & 0x7F);
        deviceObj[11] = 0; // Reserved byte, set to 0
        deviceObj[12] = DeviceConfig::MIN_NOTE;
        deviceObj[13] = DeviceConfig::MAX_NOTE;
        deviceObj[14] = (GetDeviceBoolean() >> 7) & 0x7F;
        deviceObj[15] = (GetDeviceBoolean() >> 0) & 0x7F;
        deviceObj[16] = 0; // Reserved byte, set to 0
        deviceObj[17] = 0; // Reserved byte, set to 0
        deviceObj[18] = Device::NumPolyphonicNotes; // Number of active notes per instrument
        // Bytes 12-19 are reserved and remain 0x00 (initialized above)

        for(uint8_t i = 0; i < DEVICE_NUM_NAME_BYTES; i++){
            if (Device::Name.size() >  i) deviceObj[DEVICE_NAME_OFFSET+i] = Device::Name[i];
            else deviceObj[DEVICE_NAME_OFFSET+i] = 0;
        }

        return deviceObj;
    }

    static bool SetDeviceConstruct(const uint8_t* data, size_t length = DEVICE_NUM_CFG_BYTES){
        if (data == nullptr || length < DEVICE_NUM_CFG_BYTES) {
            return false;
        }

        // Runtime mutable fields
        ID = (static_cast<uint16_t>(data[0] & 0x7F) << 7) |
            static_cast<uint16_t>(data[1] & 0x7F);

        InstrumentType = static_cast<Instrument>(data[5] & 0x7F);

        uint16_t deviceBoolValue = (static_cast<uint16_t>(data[14] & 0x7F) << 7) |
                                    static_cast<uint16_t>(data[15] & 0x7F);
        Muted = (deviceBoolValue & DEVICE_BOOL_MASK::MUTED) != 0;
        OmniMode = (deviceBoolValue & DEVICE_BOOL_MASK::OMNIMODE) != 0;
        DamperPedal = (deviceBoolValue & DEVICE_BOOL_MASK::DAMPER_PEDAL) != 0;
        Vibrato = (deviceBoolValue & DEVICE_BOOL_MASK::VIBRATO) != 0;

        NumPolyphonicNotes = data[18] & 0x7F;

        // Name payload is fixed-width and zero padded
        size_t nameLength = 0;
        while (nameLength < DEVICE_NUM_NAME_BYTES && data[DEVICE_NAME_OFFSET + nameLength] != 0x00) {
            ++nameLength;
        }
        //Name.assign(reinterpret_cast<const char*>(&data[DEVICE_NAME_OFFSET]), nameLength);
        return true;
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
