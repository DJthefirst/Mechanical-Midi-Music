/*
 * MessageHandler.h - V2.0 Message Handler with Runtime Distributor Management
 *
 * Part of the Mechanical MIDI Music V2.0 redesign
 * Based on REDESIGN_PART5_DISTRIBUTORS.md specifications
 *
 * Key improvements over V1.0:
 * - Polymorphic distributor support for custom algorithms
 * - Enhanced SysEx protocol for runtime management
 * - Better error handling and validation
 * - Factory pattern for custom distributors
 */

#pragma once

#include "core/Distributor.h"
#include "core/MidiMessage.h"
#include "core/Device.h"
#include "instruments/InstrumentController.h"
#include <vector>
#include <memory>
#include <optional>

////////////////////////////////////////////////////////////////////////////////////////////////////
// Message Handler - Routes MIDI Messages through Distributors
////////////////////////////////////////////////////////////////////////////////////////////////////

class MessageHandler {
private:
    // Polymorphic distributor storage - supports any distributor type
    std::vector<std::unique_ptr<Distributor>> m_distributors;
    InstrumentController* m_ptrInstrumentController;
    
    uint16_t m_srcDeviceId;
    uint16_t m_destDeviceId;

public:
    explicit MessageHandler(InstrumentController* controller);

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    // Core Message Processing
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    
    // Process incoming MIDI message
    std::optional<MidiMessage> processMessage(const MidiMessage& message);

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    // Runtime Distributor Management
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    
    // Add distributor (takes ownership)
    void addDistributor(std::unique_ptr<Distributor> distributor);
    
    // Add default distributor
    void addDistributor();
    
    // Add distributor from serialized data
    void addDistributor(const uint8_t data[DISTRIBUTOR_NUM_CFG_BYTES]);
    
    // Set/update distributor configuration
    void setDistributor(uint8_t id, const uint8_t data[DISTRIBUTOR_NUM_CFG_BYTES]);
    
    // Remove distributor
    void removeDistributor(uint8_t id);
    
    // Remove all distributors
    void removeAllDistributors();
    
    // Get distributor reference
    Distributor& getDistributor(uint8_t id);
    const Distributor& getDistributor(uint8_t id) const;
    
    // Get distributor serialized data
    std::array<uint8_t, DISTRIBUTOR_NUM_CFG_BYTES> getDistributorSerial(uint8_t id) const;
    
    // Get distributor count
    size_t getDistributorCount() const { return m_distributors.size(); }

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    // Distributor Factory (for custom types)
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    
    template<typename DistributorType, typename... Args>
    void addCustomDistributor(Args&&... args) {
        m_distributors.push_back(
            std::make_unique<DistributorType>(
                m_ptrInstrumentController,
                std::forward<Args>(args)...
            )
        );
    }
    
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    // Status and Diagnostics
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    // (printStatus functions removed for V1 compatibility)

private:
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    // Internal Message Processing
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    
    void distributeMessage(const MidiMessage& message);
    void processCC(const MidiMessage& message);
    std::optional<MidiMessage> processSysEx(const MidiMessage& message);
    
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    // SysEx Command Handlers
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    
    // System Commands (0x00-0x0F)
    std::optional<MidiMessage> handleDeviceReady(const MidiMessage& message);
    void handleResetDeviceConfig(const MidiMessage& message);
    std::optional<MidiMessage> handleDiscoverDevices(const MidiMessage& message);
    std::optional<MidiMessage> handleMessage(const MidiMessage& message);
    
    // Get Device Commands (0x10-0x1F)
    std::optional<MidiMessage> handleGetDeviceConstructWithDistributors(const MidiMessage& message);
    std::optional<MidiMessage> handleGetDeviceConstruct(const MidiMessage& message);
    std::optional<MidiMessage> handleGetDeviceName(const MidiMessage& message);
    std::optional<MidiMessage> handleGetDeviceBoolean(const MidiMessage& message);
    
    // Set Device Commands (0x20-0x2F)
    void handleSetDeviceConstructWithDistributors(const MidiMessage& message);
    void handleSetDeviceConstruct(const MidiMessage& message);
    void handleSetDeviceName(const MidiMessage& message);
    void handleSetDeviceBoolean(const MidiMessage& message);
    
    // Distributor Commands (0x30-0x3F)
    std::optional<MidiMessage> handleGetNumDistributors(const MidiMessage& message);
    std::optional<MidiMessage> handleAddDistributor(const MidiMessage& message);
    std::optional<MidiMessage> handleGetAllDistributors(const MidiMessage& message);
    std::optional<MidiMessage> handleGetDistributor(const MidiMessage& message);
    std::optional<MidiMessage> handleSetDistributor(const MidiMessage& message);
    std::optional<MidiMessage> handleRemoveDistributor(const MidiMessage& message);
    std::optional<MidiMessage> handleRemoveAllDistributors(const MidiMessage& message);
    std::optional<MidiMessage> handleToggleMuteDistributor(const MidiMessage& message);
    
    // Get Distributor Commands (0x40-0x4F)
    std::optional<MidiMessage> handleGetDistributorConstruct(const MidiMessage& message);
    std::optional<MidiMessage> handleGetDistributorChannels(const MidiMessage& message);
    std::optional<MidiMessage> handleGetDistributorInstruments(const MidiMessage& message);
    std::optional<MidiMessage> handleGetDistributorMethod(const MidiMessage& message);
    std::optional<MidiMessage> handleGetDistributorBoolValues(const MidiMessage& message);
    std::optional<MidiMessage> handleGetDistributorMinMaxNotes(const MidiMessage& message);
    std::optional<MidiMessage> handleGetDistributorNumPolyphonicNotes(const MidiMessage& message);
    
    // Set Distributor Commands (0x50-0x5F) - V1.0 compatibility: return bool (no response messages)
    bool handleSetDistributorConstruct(const MidiMessage& message);
    bool handleSetDistributorChannels(const MidiMessage& message);
    bool handleSetDistributorInstruments(const MidiMessage& message);
    bool handleSetDistributorMethod(const MidiMessage& message);
    bool handleSetDistributorBoolValues(const MidiMessage& message);
    bool handleSetDistributorMinMaxNotes(const MidiMessage& message);
    bool handleSetDistributorNumPolyphonicNotes(const MidiMessage& message);
    
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    // Response Generation
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    
    MidiMessage createResponse(uint8_t command, const uint8_t* payload, uint8_t payloadLength) const;
    void sendDistributorData(uint8_t id, const std::array<uint8_t, DISTRIBUTOR_NUM_CFG_BYTES>& data) const;
    void sendDistributorCount() const;
    
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    // Validation
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    
    bool isValidDistributorId(uint8_t id) const;
    bool isValidSysExMessage(const MidiMessage& message) const;
};