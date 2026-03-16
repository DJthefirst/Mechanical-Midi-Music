/*
 * Teensy41LocalStorage.h
 * Teensy 4.1-specific implementation of local storage using EEPROM
 */

#pragma once

#include "ILocalStorage.h"

#if defined(PLATFORM_TEENSY41)

#include <EEPROM.h>
#include <cstdint>
#include <string>

/**
 * @brief Teensy 4.1 EEPROM-based local storage implementation
 * 
 * This implementation uses Teensy 4.1's EEPROM library (actually emulated in flash)
 * to persist device configuration across power cycles.
 * 
 * Teensy 4.1 has 4284 bytes of emulated EEPROM available.
 * 
 * Memory Layout:
 * - 0x0000-0x0001: Magic number (0xA5B6) to detect if EEPROM is initialized
 * - 0x0002-0x0015: Device name (20 bytes, space-padded)
 * - 0x0016-0x0017: Device ID (2 bytes)
 * - 0x0018-0x0019: Device boolean flags (2 bytes)
 * - 0x001A: Number of distributors (1 byte)
 * - 0x001B: Reserved (1 byte)
 * - 0x001C-0x107B: Distributor constructs (256 distributors * 16 bytes each = 4096 bytes)
 */
class Teensy41LocalStorage : public ILocalStorage {
private:
    static constexpr uint16_t EEPROM_MAGIC = 0xA5B6;
    static constexpr uint16_t ADDR_MAGIC = 0x0000;
    static constexpr uint16_t ADDR_DEVICE_NAME = 0x0002;
    static constexpr uint16_t ADDR_DEVICE_ID = 0x0016;
    static constexpr uint16_t ADDR_DEVICE_BOOL = 0x0018;
    static constexpr uint16_t ADDR_NUM_DISTRIBUTORS = 0x001A;
    static constexpr uint16_t ADDR_DISTRIBUTORS_START = 0x001C;
    
    bool m_initialized;
    
    /**
     * @brief Check if EEPROM has been initialized (magic number present)
     * @return true if initialized
     */
    bool isEEPROMInitialized();
    
    /**
     * @brief Write magic number to EEPROM to mark it as initialized
     */
    void initializeEEPROM();
    
    /**
     * @brief Get EEPROM address for distributor construct
     * @param distributorNum Distributor index
     * @return EEPROM address
     */
    uint16_t getDistributorAddress(uint16_t distributorNum);

public:
    /**
     * @brief Construct Teensy41LocalStorage instance
     */
    Teensy41LocalStorage();

    /**
     * @brief Destructor
     */
    virtual ~Teensy41LocalStorage() = default;

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

#endif // PLATFORM_TEENSY41
