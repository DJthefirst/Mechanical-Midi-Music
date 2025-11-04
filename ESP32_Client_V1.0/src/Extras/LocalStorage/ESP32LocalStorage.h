/*
 * ESP32LocalStorage.h
 * ESP32-specific implementation of local storage using NVS (Non-Volatile Storage)
 */

#pragma once

#include "ILocalStorage.h"

#if defined(PLATFORM_ESP32)

#include "nvs_flash.h"
#include "nvs.h"
#include <cstdint>
#include <string>

/**
 * @brief ESP32 NVS-based local storage implementation
 * 
 * This implementation uses ESP32's Non-Volatile Storage (NVS) library
 * to persist device configuration across power cycles.
 */
class ESP32LocalStorage : public ILocalStorage {
private:
    bool m_initialized;
    
    /**
     * @brief Open NVS storage handle
     * @return true if handle opened successfully
     */
    bool openNvs();

    /**
     * @brief Convert uint16_t to NVS key string
     * @param value Value to convert
     * @return Key string
     */
    std::string uint16ToKey(uint16_t value);

    /**
     * @brief Read blob data from NVS
     * @param key NVS key
     * @param result Buffer to store result
     * @param arrayLength Length of buffer
     * @return ESP error code
     */
    esp_err_t readNvsBlob(const char *key, uint8_t* result, uint8_t arrayLength);

    /**
     * @brief Read uint8_t value from NVS
     * @param key NVS key
     * @param defaultValue Default value if key not found
     * @return Stored value or default
     */
    uint8_t readNvsU8(const char *key, uint8_t defaultValue);

    /**
     * @brief Write blob data to NVS
     * @param key NVS key
     * @param data Data to write
     * @param arrayLength Length of data
     * @return ESP error code
     */
    esp_err_t writeNvsBlob(const char *key, const uint8_t* data, uint8_t arrayLength);

    /**
     * @brief Write uint8_t value to NVS
     * @param key NVS key
     * @param value Value to write
     */
    void writeNvsU8(const char *key, uint8_t value);

public:
    /**
     * @brief Construct ESP32LocalStorage instance
     */
    ESP32LocalStorage();

    /**
     * @brief Destructor
     */
    virtual ~ESP32LocalStorage() = default;

    // ILocalStorage interface implementation
    bool initialize() override;
    bool isAvailable() override;
    void resetDeviceConfig() override;
    bool initializeDeviceConfiguration(DistributorManager& distributorManager) override;
    
    std::string getDeviceName() override;
    void setDeviceName(const std::string& name) override;
    
    uint16_t getDeviceID() override;
    void setDeviceID(uint16_t id) override;
    
    uint16_t getDeviceBoolean() override;
    void setDeviceBoolean(uint16_t deviceBoolean) override;
    
    uint8_t getNumOfDistributors() override;
    void setNumOfDistributors(uint8_t numOfDistributors) override;
    
    void getDistributorConstruct(uint16_t distributorNum, uint8_t* construct) override;
    void setDistributorConstruct(uint16_t distributorNum, const uint8_t* construct) override;
};

#endif // PLATFORM_ESP32
