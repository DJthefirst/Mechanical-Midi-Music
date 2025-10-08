/*
 * DistributorManager.h
 *
 * Manages the collection of distributors and their operations.
 * Extracted from the original MessageHandler to follow single responsibility principle.
 */

#pragma once

#include "Distributor.h"
#include "MidiMessage.h"
#include "Constants.h"
#include "Instruments/InstrumentController.h"

#ifdef EXTRA_LOCAL_STORAGE
    #include "Extras/LocalStorage.h"
#endif

#include <vector>
#include <array>
#include <cstdint>
#include <functional>

/**
 * Manages a collection of distributors and provides operations for
 * adding, removing, configuring, and accessing distributors.
 */
class DistributorManager {
private:
    std::vector<Distributor> m_distributors;
    InstrumentController* m_ptrInstrumentController;
    std::function<void()> m_deviceChangedCallback;

public:
    /**
     * Constructor
     * @param ptrInstrumentController Pointer to the instrument controller
     */
    explicit DistributorManager(InstrumentController* ptrInstrumentController);
    
    // Distributor management
    void addDistributor(); 
    void addDistributor(Distributor distributor); 
    void addDistributor(uint8_t data[]); 
    void setDistributor(uint8_t data[]); 
    void removeDistributor(uint8_t id);
    void removeAllDistributors();
    
    // Distributor access
    Distributor& getDistributor(uint8_t id);
    std::array<uint8_t, DISTRIBUTOR_NUM_CFG_BYTES> getDistributorSerial(uint8_t id);
    size_t getDistributorCount() const { return m_distributors.size(); }

    /**
     * Set callback for distributor changes
     * @param callback Function to call when distributors are modified
     */
    void setDeviceChangedCallback(std::function<void()> callback) { m_deviceChangedCallback = callback; }
    
    // Message processing
    void distributeMessage(MidiMessage& message);
    void processCC(MidiMessage& message);
    
    // Distributor configuration helpers
    void setDistributorChannels(uint8_t distributorId, uint16_t channels);
    void setDistributorInstruments(uint8_t distributorId, uint32_t instruments);
    void setDistributorMethod(uint8_t distributorId, DistributionMethod method);
    void setDistributorBoolValues(uint8_t distributorId, uint8_t boolValues);
    void setDistributorMinMaxNotes(uint8_t distributorId, uint8_t minNote, uint8_t maxNote);
    void setDistributorNumPolyphonicNotes(uint8_t distributorId, uint8_t numNotes);
    void toggleDistributorMute(uint8_t distributorId);
    
    // Distributor query helpers
    uint16_t getDistributorChannels(uint8_t distributorId);
    uint32_t getDistributorInstruments(uint8_t distributorId);
    DistributionMethod getDistributorMethod(uint8_t distributorId);
    uint8_t getDistributorBoolValues(uint8_t distributorId);
    uint8_t getDistributorMinNote(uint8_t distributorId);
    uint8_t getDistributorMaxNote(uint8_t distributorId);
    uint8_t getDistributorNumPolyphonicNotes(uint8_t distributorId);

private:
    // Helper to broadcast distributor changes
    void broadcastDistributorChanged() {
        if (m_deviceChangedCallback) {
            m_deviceChangedCallback();
        }
    }

    // Local Storage helpers
    #ifdef EXTRA_LOCAL_STORAGE
    void localStorageAddDistributor();
    void localStorageRemoveDistributor(uint8_t id);
    void localStorageUpdateDistributor(uint16_t distributorID, const uint8_t* data);
    void localStorageClearDistributors();
    #else
    void localStorageAddDistributor() {}
    void localStorageRemoveDistributor(uint8_t id) {}
    void localStorageUpdateDistributor(uint16_t distributorID, const uint8_t* data) {}
    void localStorageClearDistributors() {}
    #endif
};