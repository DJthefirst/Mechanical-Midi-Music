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

    SysExMsgHandler(std::shared_ptr<DistributorManager> distributorManager);

    std::optional<MidiMessage> processSysExMessage(MidiMessage& message);
    
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
    #else
        void localStorageSetDeviceName(char* name) {}
        void localStorageAddDistributor() {}
        void localStorageRemoveDistributor(uint8_t id) {}
        void localStorageUpdateDistributor(uint16_t distributorID, const uint8_t* data) {}
        void localStorageClearDistributors() {}
        void localStorageReset() {}
    #endif
};