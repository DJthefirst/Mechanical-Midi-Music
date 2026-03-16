/*
 * NullLocalStorage.h
 * No-op implementation of local storage for platforms that don't support persistent storage
 */

#pragma once

#include "ILocalStorage.h"
#include <cstdint>
#include <string>

/**
 * @brief No-op local storage implementation
 * 
 * This implementation does nothing and returns default values for all operations.
 * Used for platforms that don't support persistent storage (e.g., Teensy without EEPROM setup).
 */
class NullLocalStorage : public ILocalStorage {
public:
    NullLocalStorage() = default;
    virtual ~NullLocalStorage() = default;

    // ILocalStorage interface implementation - all no-ops
    bool initialize() override { return true; }
    bool isAvailable() override { return false; }
    void resetDeviceConfig() override {}
    bool initializeDeviceConfiguration(DistributorManager& distributorManager) override { return false; }
    
    std::string getDeviceName() override;
    void setDeviceName(const std::string& name) override {}
    
    uint16_t getDeviceID() override;
    void setDeviceID(uint16_t id) override {}
    
    uint16_t getDeviceBoolean() override { return 0; }
    void setDeviceBoolean(uint16_t deviceBoolean) override {}
    
    uint8_t getNumOfDistributors() override { return 0; }
    void setNumOfDistributors(uint8_t numOfDistributors) override {}
    
    void getDistributorConstruct(uint16_t distributorNum, uint8_t* construct) override {}
    void setDistributorConstruct(uint16_t distributorNum, const uint8_t* construct) override {}
};
