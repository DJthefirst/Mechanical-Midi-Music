/*
 * Teensy41LocalStorage.cpp
 * Teensy 4.1-specific implementation of local storage using EEPROM
 */

#include "Config.h"

#if defined(PLATFORM_TEENSY41)

#include "Teensy41LocalStorage.h"
#include "Device.h"
#include "Distributors/Distributor.h"
#include "Distributors/DistributorManager.h"
#include <string>
#include <algorithm>

Teensy41LocalStorage::Teensy41LocalStorage() : m_initialized(false) {
    // Constructor intentionally minimal - actual initialization in initialize()
}

bool Teensy41LocalStorage::initialize() {
    if (m_initialized) {
        return true;
    }

    // Check if EEPROM has been initialized (has magic number)
    if (!isEEPROMInitialized()) {
        // First time initialization - write magic number
        initializeEEPROM();
    }
    
    m_initialized = true;
    return true;
}

bool Teensy41LocalStorage::isAvailable() {
    return m_initialized;
}

bool Teensy41LocalStorage::isEEPROMInitialized() {
    uint8_t magic_msb = EEPROM.read(ADDR_MAGIC);
    uint8_t magic_lsb = EEPROM.read(ADDR_MAGIC + 1);
    uint16_t magic = (static_cast<uint16_t>(magic_msb) << 8) | static_cast<uint16_t>(magic_lsb);
    return (magic == EEPROM_MAGIC);
}

void Teensy41LocalStorage::initializeEEPROM() {
    // Write magic number
    uint8_t magic_msb = static_cast<uint8_t>((EEPROM_MAGIC >> 8) & 0xFF);
    uint8_t magic_lsb = static_cast<uint8_t>(EEPROM_MAGIC & 0xFF);
    EEPROM.update(ADDR_MAGIC, magic_msb);
    EEPROM.update(ADDR_MAGIC + 1, magic_lsb);
    
    // Initialize with default values
    // Device name - write spaces
    for (uint16_t i = 0; i < DEVICE_NUM_NAME_BYTES; i++) {
        EEPROM.update(ADDR_DEVICE_NAME + i, 0x20);
    }
    
    // Device ID - write default from Device::GetDeviceID()
    uint16_t defaultID = Device::GetDeviceID();
    EEPROM.update(ADDR_DEVICE_ID, static_cast<uint8_t>((defaultID >> 7) & 0x7F));
    EEPROM.update(ADDR_DEVICE_ID + 1, static_cast<uint8_t>(defaultID & 0x7F));
    
    // Device boolean - write 0
    EEPROM.update(ADDR_DEVICE_BOOL, 0);
    EEPROM.update(ADDR_DEVICE_BOOL + 1, 0);
    
    // Number of distributors - write 0
    EEPROM.update(ADDR_NUM_DISTRIBUTORS, 0);
}

uint16_t Teensy41LocalStorage::getDistributorAddress(uint16_t distributorNum) {
    return ADDR_DISTRIBUTORS_START + (distributorNum * DISTRIBUTOR_NUM_CFG_BYTES);
}

void Teensy41LocalStorage::resetDeviceConfig() {
    if (!m_initialized) {
        return;
    }
    
    // Clear magic number to mark as uninitialized
    EEPROM.update(ADDR_MAGIC, 0);
    EEPROM.update(ADDR_MAGIC + 1, 0);
    
    // Reinitialize with defaults
    initializeEEPROM();
}

std::string Teensy41LocalStorage::getDeviceName() {
    if (!m_initialized) {
        return Device::Name;
    }
    
    // Read fixed-size, space-padded name from EEPROM
    uint8_t tmp[DEVICE_NUM_NAME_BYTES];
    for (uint16_t i = 0; i < DEVICE_NUM_NAME_BYTES; i++) {
        tmp[i] = EEPROM.read(ADDR_DEVICE_NAME + i);
    }
    
    // Convert to std::string and trim trailing spaces and null bytes
    std::string name(reinterpret_cast<char*>(tmp), DEVICE_NUM_NAME_BYTES);
    size_t end = name.find_last_not_of(" \0");
    if (end != std::string::npos) {
        name.erase(end + 1);
    } else {
        // If name is all spaces/nulls, return default
        return Device::Name;
    }
    
    return name;
}

void Teensy41LocalStorage::setDeviceName(const std::string& name) {
    if (!m_initialized) {
        return;
    }
    
    // Write fixed-size space-padded name to EEPROM
    uint8_t tmp[DEVICE_NUM_NAME_BYTES];
    memset(tmp, 0x20, DEVICE_NUM_NAME_BYTES); // Initialize with spaces
    
    size_t copyLen = std::min(name.length(), static_cast<size_t>(DEVICE_NUM_NAME_BYTES));
    memcpy(tmp, name.c_str(), copyLen);
    
    for (uint16_t i = 0; i < DEVICE_NUM_NAME_BYTES; i++) {
        EEPROM.update(ADDR_DEVICE_NAME + i, tmp[i]);
    }
}

uint16_t Teensy41LocalStorage::getDeviceID() {
    if (!m_initialized) {
        return Device::GetDeviceID();
    }
    
    uint8_t msb = EEPROM.read(ADDR_DEVICE_ID);
    uint8_t lsb = EEPROM.read(ADDR_DEVICE_ID + 1);
    uint16_t id = (static_cast<uint16_t>(msb) << 7) | static_cast<uint16_t>(lsb);
    return id;
}

void Teensy41LocalStorage::setDeviceID(uint16_t id) {
    if (!m_initialized) {
        return;
    }
    
    uint8_t msb = static_cast<uint8_t>((id >> 7) & 0x7F);
    uint8_t lsb = static_cast<uint8_t>(id & 0x7F);
    EEPROM.update(ADDR_DEVICE_ID, msb);
    EEPROM.update(ADDR_DEVICE_ID + 1, lsb);
}

uint16_t Teensy41LocalStorage::getDeviceBoolean() {
    if (!m_initialized) {
        return 0;
    }
    
    uint8_t msb = EEPROM.read(ADDR_DEVICE_BOOL);
    uint8_t lsb = EEPROM.read(ADDR_DEVICE_BOOL + 1);
    uint16_t boolValue = (static_cast<uint16_t>(msb) << 7) | static_cast<uint16_t>(lsb);
    return boolValue;
}

void Teensy41LocalStorage::setDeviceBoolean(uint16_t deviceBoolean) {
    if (!m_initialized) {
        return;
    }
    
    uint8_t msb = static_cast<uint8_t>((deviceBoolean >> 7) & 0x7F);
    uint8_t lsb = static_cast<uint8_t>(deviceBoolean & 0x7F);
    EEPROM.update(ADDR_DEVICE_BOOL, msb);
    EEPROM.update(ADDR_DEVICE_BOOL + 1, lsb);
}

uint8_t Teensy41LocalStorage::getNumOfDistributors() {
    if (!m_initialized) {
        return 0;
    }
    
    return EEPROM.read(ADDR_NUM_DISTRIBUTORS);
}

void Teensy41LocalStorage::setNumOfDistributors(uint8_t numOfDistributors) {
    if (!m_initialized) {
        return;
    }
    
    EEPROM.update(ADDR_NUM_DISTRIBUTORS, numOfDistributors);
}

void Teensy41LocalStorage::getDistributorConstruct(uint16_t distributorNum, uint8_t* construct) {
    if (!m_initialized) {
        return;
    }
    
    uint16_t addr = getDistributorAddress(distributorNum);
    for (uint8_t i = 0; i < DISTRIBUTOR_NUM_CFG_BYTES; i++) {
        construct[i] = EEPROM.read(addr + i);
    }
}

void Teensy41LocalStorage::setDistributorConstruct(uint16_t distributorNum, const uint8_t* construct) {
    if (!m_initialized) {
        return;
    }
    
    uint16_t addr = getDistributorAddress(distributorNum);
    for (uint8_t i = 0; i < DISTRIBUTOR_NUM_CFG_BYTES; i++) {
        EEPROM.update(addr + i, construct[i]);
    }
}

bool Teensy41LocalStorage::initializeDeviceConfiguration(DistributorManager& distributorManager) {
    // Ensure storage is properly initialized
    if (!isAvailable()) {
        return false; // Storage initialization failed, use defaults
    }

    // Device Config - only load if storage is working
    Device::Name = getDeviceName();
    // Load runtime device ID (if stored) and update Device runtime value
    Device::SetDeviceID(getDeviceID());
    // Load device boolean flags (Muted, OmniMode)
    uint16_t deviceBoolValue = getDeviceBoolean();
    Device::Muted = ((deviceBoolValue & DEVICE_BOOL_MASK::MUTED) != 0);
    Device::OmniMode = ((deviceBoolValue & DEVICE_BOOL_MASK::OMNIMODE) != 0);
    
    // Distributor Config
    uint8_t numDistributors = getNumOfDistributors();
    for (uint8_t i = 0; i < numDistributors; i++) {
        uint8_t distributorData[DISTRIBUTOR_NUM_CFG_BYTES];
        getDistributorConstruct(i, distributorData);
        distributorManager.addDistributor(distributorData);
    }
    
    return true; // Successfully loaded configuration
}

#endif // PLATFORM_TEENSY41
