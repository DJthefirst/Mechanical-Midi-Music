/*
 * ILocalStorage.h
 * Abstract interface for local storage operations
 * Allows platform-specific implementations (ESP32 NVS, Teensy EEPROM, etc.)
 */

#pragma once

#include <cstdint>
#include <string>

// Forward declaration
class DistributorManager;

/**
 * @brief Interface for local storage operations
 * 
 * This interface defines the contract for all platform-specific storage implementations.
 * Implementations should handle persistent storage of device configuration and distributor settings.
 */
class ILocalStorage {
public:
    virtual ~ILocalStorage() = default;

    /**
     * @brief Initialize the storage system
     * @return true if initialization successful, false otherwise
     */
    virtual bool initialize() = 0;

    /**
     * @brief Check if storage is properly initialized and ready for use
     * @return true if storage is available, false otherwise
     */
    virtual bool isAvailable() = 0;

    /**
     * @brief Reset all device configuration to defaults
     */
    virtual void resetDeviceConfig() = 0;

    /**
     * @brief Initialize device configuration from storage
     * @param distributorManager Reference to distributor manager to populate with stored distributors
     * @return true if configuration loaded successfully, false if using defaults
     */
    virtual bool initializeDeviceConfiguration(DistributorManager& distributorManager) = 0;

    // Device Name
    /**
     * @brief Get stored device name
     * @return Device name string, or default if not stored
     */
    virtual std::string getDeviceName() = 0;

    /**
     * @brief Store device name
     * @param name Device name to store
     */
    virtual void setDeviceName(const std::string& name) = 0;

    // Device ID
    /**
     * @brief Get stored device ID
     * @return Device ID, or default if not stored
     */
    virtual uint16_t getDeviceID() = 0;

    /**
     * @brief Store device ID
     * @param id Device ID to store
     */
    virtual void setDeviceID(uint16_t id) = 0;

    // Device Boolean Flags (Muted, OmniMode, etc.)
    /**
     * @brief Get device boolean flags
     * @return Boolean flags as bitfield
     */
    virtual uint16_t getDeviceBoolean() = 0;

    /**
     * @brief Store device boolean flags
     * @param deviceBoolean Boolean flags as bitfield
     */
    virtual void setDeviceBoolean(uint16_t deviceBoolean) = 0;

    // Distributor Configuration
    /**
     * @brief Get number of stored distributors
     * @return Number of distributors
     */
    virtual uint8_t getNumOfDistributors() = 0;

    /**
     * @brief Store number of distributors
     * @param numOfDistributors Number of distributors
     */
    virtual void setNumOfDistributors(uint8_t numOfDistributors) = 0;

    /**
     * @brief Get distributor configuration
     * @param distributorNum Distributor index
     * @param construct Buffer to receive configuration data
     */
    virtual void getDistributorConstruct(uint16_t distributorNum, uint8_t* construct) = 0;

    /**
     * @brief Store distributor configuration
     * @param distributorNum Distributor index
     * @param construct Configuration data to store
     */
    virtual void setDistributorConstruct(uint16_t distributorNum, const uint8_t* construct) = 0;
};
