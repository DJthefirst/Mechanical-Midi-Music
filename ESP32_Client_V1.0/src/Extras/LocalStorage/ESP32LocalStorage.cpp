/*
 * ESP32LocalStorage.cpp
 * ESP32-specific implementation of local storage using NVS (Non-Volatile Storage)
 */

#include "Config.h"

#if defined(PLATFORM_ESP32)

#include "ESP32LocalStorage.h"
#include "Device.h"
#include "Distributors/Distributor.h"
#include "Distributors/DistributorManager.h"
#include <iostream>
#include <string>

// Global NVS handle and error tracking
static esp_err_t g_err;
static nvs_handle_t g_handle;

ESP32LocalStorage::ESP32LocalStorage() : m_initialized(false) {
    // Constructor intentionally minimal - actual initialization in initialize()
}

bool ESP32LocalStorage::initialize() {
    if (m_initialized) {
        return true;
    }

    // Initialize NVS
    g_err = nvs_flash_init();
    
    // Only erase if absolutely necessary (version mismatch or no free pages)
    // This will cause data loss but is required for NVS to function
    if (g_err == ESP_ERR_NVS_NO_FREE_PAGES || g_err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        // WARNING: This will erase all stored configuration
        g_err = nvs_flash_erase();
        if (g_err == ESP_OK) {
            g_err = nvs_flash_init();
        }
    }
    
    // If initialization still fails, we can't use NVS but shouldn't crash
    // Device will run with default configuration
    m_initialized = (g_err == ESP_OK);
    return m_initialized;
}

bool ESP32LocalStorage::isAvailable() {
    if (!m_initialized) {
        return false;
    }

    // Check if NVS is properly initialized by attempting a simple operation
    nvs_handle_t testHandle;
    esp_err_t err = nvs_open("storage", NVS_READONLY, &testHandle);
    
    if (err == ESP_ERR_NVS_NOT_INITIALIZED) {
        // NVS is not initialized, try to initialize it
        err = nvs_flash_init();
        if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
            // This would require erasing, but that means losing data
            // Return false to use defaults instead
            return false;
        }
        if (err != ESP_OK) {
            return false;
        }
    } else if (err == ESP_OK) {
        // NVS is working, close the test handle
        nvs_close(testHandle);
    }
    // If err is ESP_ERR_NVS_NOT_FOUND, that's normal - the "storage" namespace doesn't exist yet
    
    return true;
}

bool ESP32LocalStorage::openNvs() {
    // Open
    g_err = nvs_open("storage", NVS_READWRITE, &g_handle);
    return (g_err == ESP_OK);
}

esp_err_t ESP32LocalStorage::readNvsBlob(const char *key, uint8_t* result, uint8_t arrayLength) {
    size_t arraySize = sizeof(uint8_t) * arrayLength;
    uint8_t tempResult[arrayLength];

    if (!openNvs()) {
        return g_err;
    }

    g_err = nvs_get_blob(g_handle, key, tempResult, &arraySize);

    switch (g_err) {
        case ESP_OK:
            std::copy(tempResult, tempResult + arraySize, result);
            break;
        case ESP_ERR_NVS_NOT_FOUND:
            // Key not found - not an error, just means value hasn't been set yet
            break;
        default:
            // Other errors
            break;
    }
    
    nvs_close(g_handle);
    return g_err;
}

uint8_t ESP32LocalStorage::readNvsU8(const char *key, uint8_t defaultValue) {
    uint8_t value;

    if (!openNvs()) {
        return defaultValue;
    }
    
    g_err = nvs_get_u8(g_handle, key, &value);
    nvs_close(g_handle);

    switch (g_err) {
        case ESP_OK:
            return value;
        case ESP_ERR_NVS_NOT_FOUND:
            // Key not found, write default and return it
            if (openNvs()) {
                writeNvsU8(key, defaultValue);
                nvs_close(g_handle);
            }
            return defaultValue;
        default:
            return defaultValue;
    }
}

esp_err_t ESP32LocalStorage::writeNvsBlob(const char *key, const uint8_t* data, uint8_t arrayLength) {
    size_t arraySize = sizeof(uint8_t) * arrayLength;
    
    if (!openNvs()) {
        return g_err;
    }
    
    g_err = nvs_set_blob(g_handle, key, data, arraySize);
    if (g_err == ESP_OK) {
        g_err = nvs_commit(g_handle);
    }
    
    nvs_close(g_handle);
    return g_err;
}

void ESP32LocalStorage::writeNvsU8(const char *key, uint8_t value) {
    if (!openNvs()) {
        return;
    }
    
    g_err = nvs_set_u8(g_handle, key, value);
    if (g_err == ESP_OK) {
        g_err = nvs_commit(g_handle);
    }
    
    nvs_close(g_handle);
}

std::string ESP32LocalStorage::uint16ToKey(uint16_t value) {
    const char hex[] = "0123456789abcdef";
    std::string key = "Distributor";
    key.push_back(hex[(value >> 0) & 0x0F]);
    key.push_back(hex[(value >> 4) & 0x0F]);
    key.push_back(hex[(value >> 8) & 0x0F]);
    key.push_back(hex[(value >> 12) & 0x0F]);
    return key;
}

void ESP32LocalStorage::resetDeviceConfig() {
    if (!openNvs()) {
        return;
    }
    
    // Erase device-specific keys
    nvs_erase_key(g_handle, "Device_name");
    nvs_erase_key(g_handle, "Device_id");
    nvs_erase_key(g_handle, "Device_bool");
    nvs_erase_key(g_handle, "Distributor_num");
    
    // Erase all distributor constructs (support up to 256 distributors)
    for (uint16_t i = 0; i < 256; i++) {
        std::string key = uint16ToKey(i);
        nvs_erase_key(g_handle, key.c_str());
    }
    
    // Commit changes
    nvs_commit(g_handle);
    nvs_close(g_handle);
}

std::string ESP32LocalStorage::getDeviceName() {
    // Read a fixed-size, space-padded name blob from NVS and convert to std::string
    uint8_t tmp[DEVICE_NUM_NAME_BYTES];
    // Initialize with null bytes to ensure trailing bytes are padded if stored blob is shorter
    memset(tmp, 0x00, DEVICE_NUM_NAME_BYTES);
    esp_err_t err = readNvsBlob("Device_name", tmp, DEVICE_NUM_NAME_BYTES);

    // If not found or any error occurred, return default
    if (err != ESP_OK) {
        return Device::Name;
    }

    // Convert to std::string and trim trailing spaces and null bytes
    std::string name(reinterpret_cast<char*>(tmp), DEVICE_NUM_NAME_BYTES);
    size_t end = name.find_last_not_of(" \0");
    if (end != std::string::npos) {
        name.erase(end + 1);
    } else {
        // If name is all spaces/nulls, return default instead of empty string
        return Device::Name;
    }

    return name;
}

void ESP32LocalStorage::setDeviceName(const std::string& name) {
    // Write a fixed-size space-padded blob to NVS
    uint8_t tmp[DEVICE_NUM_NAME_BYTES];
    memset(tmp, 0x20, DEVICE_NUM_NAME_BYTES); // Initialize with spaces

    size_t copyLen = std::min(name.length(), static_cast<size_t>(DEVICE_NUM_NAME_BYTES));
    memcpy(tmp, name.c_str(), copyLen);
    writeNvsBlob("Device_name", tmp, DEVICE_NUM_NAME_BYTES);
}

uint16_t ESP32LocalStorage::getDeviceID() {
    uint8_t tmp[2];
    // Default to compile-time SYSEX_DEV_ID if not found
    tmp[0] = static_cast<uint8_t>((Device::GetDeviceID() >> 7) & 0x7F);
    tmp[1] = static_cast<uint8_t>((Device::GetDeviceID() >> 0) & 0x7F);
    esp_err_t r = readNvsBlob("Device_id", tmp, 2);
    if (r == ESP_ERR_NVS_NOT_FOUND) {
        return Device::GetDeviceID();
    }
    uint16_t id = (static_cast<uint16_t>(tmp[0]) << 7) | static_cast<uint16_t>(tmp[1]);
    return id;
}

void ESP32LocalStorage::setDeviceID(uint16_t id) {
    uint8_t tmp[2];
    tmp[0] = static_cast<uint8_t>((id >> 7) & 0x7F);
    tmp[1] = static_cast<uint8_t>((id >> 0) & 0x7F);
    writeNvsBlob("Device_id", tmp, 2);
}

uint16_t ESP32LocalStorage::getDeviceBoolean() {
    uint8_t tmp[2];
    // Default to 0 (no flags set) if not found
    tmp[0] = 0;
    tmp[1] = 0;
    esp_err_t r = readNvsBlob("Device_bool", tmp, 2);
    if (r == ESP_ERR_NVS_NOT_FOUND) {
        return 0;
    }
    uint16_t boolValue = (static_cast<uint16_t>(tmp[0]) << 7) | static_cast<uint16_t>(tmp[1]);
    return boolValue;
}

void ESP32LocalStorage::setDeviceBoolean(uint16_t deviceBoolean) {
    uint8_t tmp[2];
    tmp[0] = static_cast<uint8_t>((deviceBoolean >> 7) & 0x7F);
    tmp[1] = static_cast<uint8_t>((deviceBoolean >> 0) & 0x7F);
    writeNvsBlob("Device_bool", tmp, 2);
}

uint8_t ESP32LocalStorage::getNumOfDistributors() {
    return readNvsU8("Distributor_num", 0);
}

void ESP32LocalStorage::setNumOfDistributors(uint8_t numOfDistributors) {
    writeNvsU8("Distributor_num", numOfDistributors);
}

void ESP32LocalStorage::getDistributorConstruct(uint16_t distributorNum, uint8_t* construct) {
    std::string key = uint16ToKey(distributorNum);
    const char *ptr_key = key.c_str();
    readNvsBlob(ptr_key, construct, DISTRIBUTOR_NUM_CFG_BYTES);
}

void ESP32LocalStorage::setDistributorConstruct(uint16_t distributorNum, const uint8_t* construct) {
    std::string key = uint16ToKey(distributorNum);
    const char *ptr_key = key.c_str();
    writeNvsBlob(ptr_key, construct, DISTRIBUTOR_NUM_CFG_BYTES);
}

bool ESP32LocalStorage::initializeDeviceConfiguration(DistributorManager& distributorManager) {
    // Ensure NVS is properly initialized
    if (!isAvailable()) {
        return false; // NVS initialization failed, use defaults
    }

    // Device Config - only load if NVS is working
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

#endif // PLATFORM_ESP32
