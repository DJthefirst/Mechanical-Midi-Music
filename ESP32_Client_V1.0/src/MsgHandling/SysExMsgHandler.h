/*
 * SysExMsgHandler.h
 * 
 * Handles SysEx MIDI messages for device configuration and management.
 * Extracted from the original MessageHandler to follow single responsibility principle.
 */

#pragma once

#include "Device.h" 
#include "MidiMessage.h"
#include "Constants.h"

#ifdef EXTRA_LOCAL_STORAGE
    #include "Extras/LocalStorage.h"
#endif

#include <array>
#include <cstdint>
#include <optional>
#include <memory>
#include <functional>

// Forward declaration
class DistributorManager;

/**
 * Handles SysEx messages for device configuration and management.
 * Uses dependency injection to communicate with network and distributor layers.
 */
class SysExMsgHandler {
private:
    std::shared_ptr<DistributorManager> m_distributorManager;
    uint16_t m_sourceId = Device::GetDeviceID();
    uint16_t m_destinationId = 0;
    
    // Callback for when device configuration changes
    std::function<void()> m_deviceChangedCallback;
    
public:
    /**
     * Constructor with dependency injection
     * @param distributorManager Interface to distributor management
     */
    explicit SysExMsgHandler(
        std::shared_ptr<DistributorManager> distributorManager = nullptr
    );
    
    /**
     * Process a SysEx message and return optional response
     * @param message The SysEx message to process
     * @return Optional response message
     */
    std::optional<MidiMessage> processSysExMessage(MidiMessage& message);
    
    /**
     * Set callback for device configuration changes
     * @param callback Function to call when device configuration changes
     */
    void setDeviceChangedCallback(std::function<void()> callback);

private:
    // Device configuration commands
    MidiMessage sysExDeviceReady(MidiMessage& message);
    void sysExResetDeviceConfig(MidiMessage& message);
    MidiMessage sysExDiscoverDevices(MidiMessage& message);
    
    MidiMessage sysExGetDeviceConstructWithDistributors(MidiMessage& message);
    MidiMessage sysExGetDeviceConstruct(MidiMessage& message);
    MidiMessage sysExGetDeviceID(MidiMessage& message);
    MidiMessage sysExGetDeviceName(MidiMessage& message);
    MidiMessage sysExGetDeviceBoolean(MidiMessage& message);
    
    void sysExSetDeviceConstructWithDistributors(MidiMessage& message);
    void sysExSetDeviceConstruct(MidiMessage& message);
    void sysExSetDeviceID(MidiMessage& message);
    void sysExSetDeviceName(MidiMessage& message);
    void sysExSetDeviceBoolean(MidiMessage& message);
    
    // Distributor management commands (delegates to DistributorManager)
    MidiMessage sysExGetNumOfDistributors(MidiMessage& message);
    MidiMessage sysExGetAllDistributors(MidiMessage& message);
    MidiMessage sysExToggleMuteDistributor(MidiMessage& message);
    
    MidiMessage sysExGetDistributorConstruct(MidiMessage& message);
    MidiMessage sysExGetDistributorChannels(MidiMessage& message);
    MidiMessage sysExGetDistributorInstruments(MidiMessage& message);
    MidiMessage sysExGetDistributorMethod(MidiMessage& message);
    MidiMessage sysExGetDistributorBoolValues(MidiMessage& message);
    MidiMessage sysExGetDistributorMinMaxNotes(MidiMessage& message);
    MidiMessage sysExGetDistributorNumPolyphonicNotes(MidiMessage& message);
    
    void sysExSetDistributor(MidiMessage& message);
    void sysExSetDistributorChannels(MidiMessage& message);
    void sysExSetDistributorInstruments(MidiMessage& message);
    void sysExSetDistributorMethod(MidiMessage& message);
    void sysExSetDistributorBoolValues(MidiMessage& message);
    void sysExSetDistributorMinMaxNotes(MidiMessage& message);
    void sysExSetDistributorNumPolyphonicNotes(MidiMessage& message);
    
    // Helper methods
    void broadcastDeviceChanged();
    bool isValidDestination(MidiMessage& message) const;
    
    // Local Storage helpers
    #ifdef EXTRA_LOCAL_STORAGE
    void localStorageSetDeviceName(char* name);
    void localStorageAddDistributor();
    void localStorageRemoveDistributor(uint8_t id);
    void localStorageUpdateDistributor(uint16_t distributorID, const uint8_t* data);
    void localStorageClearDistributors();
    void localStorageReset();
    #endif
};