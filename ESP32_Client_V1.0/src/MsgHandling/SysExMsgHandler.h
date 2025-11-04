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
    #include "Extras/LocalStorage/LocalStorageFactory.h"
#endif

#include <array>
#include <cstdint>
#include <optional>
#include <memory>
#include <functional>

// Forward declaration
class DistributorManager;

class SysExMsgHandler {
private:
    std::shared_ptr<DistributorManager> distributorManager;

    uint16_t m_sourceId = Device::GetDeviceID();
    uint16_t m_destinationId = 0;
    
    // Callback for when device configuration changes
    std::function<void()> m_deviceChangedCallback;

public:

    explicit SysExMsgHandler(std::shared_ptr<DistributorManager> distributorManager);

    std::optional<MidiMessage> processSysExMessage(MidiMessage& message);
    
    void setDeviceChangedCallback(const std::function<void()>& callback);

private:
    // Device configuration commands
    MidiMessage sysExDeviceReady(const MidiMessage& message);
    void sysExResetDeviceConfig(const MidiMessage& message);
    MidiMessage sysExDiscoverDevices(const MidiMessage& message);
    
    MidiMessage sysExGetDeviceConstructWithDistributors(const MidiMessage& message);
    MidiMessage sysExGetDeviceConstruct(const MidiMessage& message);
    MidiMessage sysExGetDeviceID(const MidiMessage& message);
    MidiMessage sysExGetDeviceName(const MidiMessage& message);
    MidiMessage sysExGetDeviceBoolean(const MidiMessage& message);
    
    void sysExSetDeviceConstructWithDistributors(const MidiMessage& message);
    void sysExSetDeviceConstruct(const MidiMessage& message);
    void sysExSetDeviceID(const MidiMessage& message);
    void sysExSetDeviceName(const MidiMessage& message);
    void sysExSetDeviceBoolean(const MidiMessage& message);
    
    // Distributor management commands (delegates to DistributorManager)
    MidiMessage sysExGetNumOfDistributors(const MidiMessage& message);
    MidiMessage sysExGetAllDistributors(const MidiMessage& message);
    MidiMessage sysExToggleMuteDistributor(const MidiMessage& message);
    
    MidiMessage sysExGetDistributorConstruct(const MidiMessage& message);
    MidiMessage sysExGetDistributorChannels(const MidiMessage& message);
    MidiMessage sysExGetDistributorInstruments(const MidiMessage& message);
    MidiMessage sysExGetDistributorMethod(const MidiMessage& message);
    MidiMessage sysExGetDistributorBoolValues(const MidiMessage& message);
    MidiMessage sysExGetDistributorMinMaxNotes(const MidiMessage& message);
    MidiMessage sysExGetDistributorNumPolyphonicNotes(const MidiMessage& message);
    
    void sysExSetDistributor(const MidiMessage& message);
    void sysExSetDistributorChannels(const MidiMessage& message);
    void sysExSetDistributorInstruments(const MidiMessage& message);
    void sysExSetDistributorMethod(const MidiMessage& message);
    void sysExSetDistributorBoolValues(const MidiMessage& message);
    void sysExSetDistributorMinMaxNotes(const MidiMessage& message);
    void sysExSetDistributorNumPolyphonicNotes(const MidiMessage& message);
    
    // Helper methods
    void broadcastDeviceChanged();
    bool isValidDestination(const MidiMessage& message) const;
    
    // Local Storage helpers
    #ifdef EXTRA_LOCAL_STORAGE
        void localStorageSetDeviceName(char* name);
        void localStorageAddDistributor();
        void localStorageRemoveDistributor(uint8_t id);
        void localStorageUpdateDistributor(uint16_t distributorID, const uint8_t* data);
        void localStorageClearDistributors();
        void localStorageReset();
    #else
        void localStorageSetDeviceName(char* name) {}
        void localStorageAddDistributor() {}
        void localStorageRemoveDistributor(uint8_t id) {}
        void localStorageUpdateDistributor(uint16_t distributorID, const uint8_t* data) {}
        void localStorageClearDistributors() {}
        void localStorageReset() {}
    #endif
};