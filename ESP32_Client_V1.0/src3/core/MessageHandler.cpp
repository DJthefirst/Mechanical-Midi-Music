/*
 * MessageHandler.cpp - V2.0 Message Handler Implementation
 *
 * Part of the Mechanical MIDI Music V2.0 redesign
 */

#define USE_V2_CONFIG
#include "MessageHandler.h"
#include <Arduino.h>
#include <cstring>  // For strlen

// Debugging: disable debug prints on the MIDI serial port by default.
// Define DEBUG_SERIAL to enable debug prints (use a separate debug serial if needed).
#ifndef DEBUG_SERIAL
#define DEBUG_SERIAL(...) ((void)0)
#else
#define DEBUG_SERIAL(...) Serial.printf(__VA_ARGS__)
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////
// Constructor
////////////////////////////////////////////////////////////////////////////////////////////////////

MessageHandler::MessageHandler(InstrumentController* controller)
    : m_ptrInstrumentController(controller)
    , m_srcDeviceId(Config::DEVICE_ID_VAL)
    , m_destDeviceId(0xFFFF) {
    
    if (!m_ptrInstrumentController) {
        DEBUG_SERIAL("ERROR: MessageHandler created with null InstrumentController\n");
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Core Message Processing
////////////////////////////////////////////////////////////////////////////////////////////////////

std::optional<MidiMessage> MessageHandler::processMessage(const MidiMessage& message) {
    if (!message.isValid()) {
        return std::nullopt;
    }
    
    if (message.isSysEx()) {
        return processSysEx(message);
    } else {
        distributeMessage(message);
        return std::nullopt;  // No response for regular MIDI
    }
}

void MessageHandler::distributeMessage(const MidiMessage& message) {
    // Route message through all distributors
    for (auto& distributor : m_distributors) {
        distributor->processMessage(message);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Runtime Distributor Management
////////////////////////////////////////////////////////////////////////////////////////////////////

void MessageHandler::addDistributor() {
    m_distributors.push_back(
        std::make_unique<Distributor>(m_ptrInstrumentController)
    );
}

void MessageHandler::addDistributor(std::unique_ptr<Distributor> distributor) {
    if (distributor) {
        m_distributors.push_back(std::move(distributor));
    }
}

void MessageHandler::addDistributor(const uint8_t data[DISTRIBUTOR_NUM_CFG_BYTES]) {
    auto dist = std::make_unique<Distributor>(m_ptrInstrumentController);
    dist->fromSerial(data);
    m_distributors.push_back(std::move(dist));
}

void MessageHandler::setDistributor(uint8_t id, const uint8_t data[DISTRIBUTOR_NUM_CFG_BYTES]) {
    if (id < m_distributors.size()) {
        m_distributors[id]->fromSerial(data);
    } else {
        // If ID doesn't exist, add new distributor
        addDistributor(data);
    }
}

void MessageHandler::removeDistributor(uint8_t id) {
    if (id < m_distributors.size()) {
        m_distributors.erase(m_distributors.begin() + id);
    }
}

void MessageHandler::removeAllDistributors() {
    m_distributors.clear();
}

Distributor& MessageHandler::getDistributor(uint8_t id) {
    if (id >= m_distributors.size()) {
        // This should throw, but for now return the first one or create one
        if (m_distributors.empty()) {
            addDistributor();
        }
        return *m_distributors[0];
    }
    return *m_distributors[id];
}

const Distributor& MessageHandler::getDistributor(uint8_t id) const {
    if (id >= m_distributors.size()) {
        // This is a problem - should throw
        static Distributor dummy(nullptr);
        return dummy;
    }
    return *m_distributors[id];
}

std::array<uint8_t, DISTRIBUTOR_NUM_CFG_BYTES> MessageHandler::getDistributorSerial(uint8_t id) const {
    if (id < m_distributors.size()) {
        return m_distributors[id]->toSerial();
    }
    
    // Return empty array for invalid ID
    return std::array<uint8_t, DISTRIBUTOR_NUM_CFG_BYTES>{};
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// SysEx Processing (Simplified for V2.0)
////////////////////////////////////////////////////////////////////////////////////////////////////

std::optional<MidiMessage> MessageHandler::processSysEx(const MidiMessage& message) {
    if (!isValidSysExMessage(message)) {
        return std::nullopt;
    }
    
    // Parse device ID
    uint16_t deviceId = message.DestinationID();
    // Accept messages addressed to this device, the global broadcast (0x0000),
    // or the server address (SYSEX_Server) for compatibility with V1 behaviour.
    if (deviceId != m_srcDeviceId && deviceId != SYSEX_Broadcast && deviceId != SYSEX_Server) {
        return std::nullopt;  // Not for us
    }
    
    uint8_t command = message.sysExCommand();

#ifdef SYSEX_DEBUG_PARSE
    // Emit a small debug SysEx back to the message source with parsed header info
    {
        uint16_t src = message.SourceID();
        uint16_t dst = message.DestinationID();
        uint8_t cmd = message.sysExCommand();
        uint8_t len = message.sysExPayloadLength();
        uint8_t dbgPayload[6] = {
            static_cast<uint8_t>((src >> 7) & 0x7F), static_cast<uint8_t>(src & 0x7F),
            static_cast<uint8_t>((dst >> 7) & 0x7F), static_cast<uint8_t>(dst & 0x7F),
            cmd, len
        };
        MidiMessage dbgMsg(m_srcDeviceId, message.SourceID(), 0x7E /* debug cmd */, dbgPayload, 6);
        // Send immediately via network if available (non-blocking best-effort)
        // Use the global network instance declared in main.cpp
        extern NetworkType network;
        network.sendMessage(dbgMsg);
    }
#endif
    
    switch (command) {
        // System Commands (0x00-0x0F)
        case SYSEX_DeviceReady:
            return handleDeviceReady(message);
        case SYSEX_ResetDeviceConfig:
            handleResetDeviceConfig(message);
            return std::nullopt; // V1: no response after reset
        case SYSEX_DiscoverDevices:
            return handleDiscoverDevices(message);
        case SYSEX_Message:
            return handleMessage(message);
            
        // Get Device Commands (0x10-0x1F)
        case SYSEX_GetDeviceConstructWithDistributors:
            return handleGetDeviceConstructWithDistributors(message);
        case SYSEX_GetDeviceConstruct:
            return handleGetDeviceConstruct(message);
        case SYSEX_GetDeviceName:
            return handleGetDeviceName(message);
        case SYSEX_GetDeviceBoolean:
            return handleGetDeviceBoolean(message);
            
        // Set Device Commands (0x20-0x2F) - No response per protocol
        case SYSEX_SetDeviceConstructWithDistributors:
            handleSetDeviceConstructWithDistributors(message);
            return std::nullopt;
        case SYSEX_SetDeviceConstruct:
            handleSetDeviceConstruct(message);
            return std::nullopt;
        case SYSEX_SetDeviceName:
            handleSetDeviceName(message);
            return std::nullopt;
        case SYSEX_SetDeviceBoolean:
            handleSetDeviceBoolean(message);
            return std::nullopt;
            
        // Distributor Commands (0x30-0x3F)
        case SYSEX_GetNumDistributors:
            return handleGetNumDistributors(message);
        case SYSEX_AddDistributor:
            handleAddDistributor(message); // No response per protocol
            return std::nullopt;
        case SYSEX_GetAllDistributors:
            return handleGetAllDistributors(message);
        case SYSEX_RemoveDistributor:
            handleRemoveDistributor(message); // No response per protocol
            return std::nullopt;
        case SYSEX_RemoveAllDistributors:
            handleRemoveAllDistributors(message); // No response per protocol
            return std::nullopt;
        case SYSEX_ToggleMuteDistributor:
            return handleToggleMuteDistributor(message);
            
        // Get Distributor Commands (0x40-0x4F)
        case SYSEX_GetDistributorConstruct:
            return handleGetDistributorConstruct(message);
        case SYSEX_GetDistributorChannels:
            return handleGetDistributorChannels(message);
        case SYSEX_GetDistributorInstruments:
            return handleGetDistributorInstruments(message);
        case SYSEX_GetDistributorMethod:
            return handleGetDistributorMethod(message);
        case SYSEX_GetDistributorBoolValues:
            return handleGetDistributorBoolValues(message);
        case SYSEX_GetDistributorMinMaxNotes:
            return handleGetDistributorMinMaxNotes(message);
        case SYSEX_GetDistributorNumPolyphonicNotes:
            return handleGetDistributorNumPolyphonicNotes(message);
            
        // Set Distributor Commands (0x50-0x5F)
        case SYSEX_SetDistributorConstruct:
            handleSetDistributorConstruct(message);
            return std::nullopt;
        case SYSEX_SetDistributorChannels:
            handleSetDistributorChannels(message);
            return std::nullopt;
        case SYSEX_SetDistributorInstruments:
            handleSetDistributorInstruments(message);
            return std::nullopt;
        case SYSEX_SetDistributorMethod:
            handleSetDistributorMethod(message);
            return std::nullopt;
        case SYSEX_SetDistributorBoolValues:
            handleSetDistributorBoolValues(message);
            return std::nullopt;
        case SYSEX_SetDistributorMinMaxNotes:
            handleSetDistributorMinMaxNotes(message);
            return std::nullopt;
        case SYSEX_SetDistributorNumPolyphonicNotes:
            handleSetDistributorNumPolyphonicNotes(message);
            return std::nullopt;
            
        default:
            DEBUG_SERIAL("Unknown SysEx command: 0x%02X\n", command);
            return std::nullopt;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// SysEx Command Handlers (Simplified)
////////////////////////////////////////////////////////////////////////////////////////////////////

std::optional<MidiMessage> MessageHandler::handleAddDistributor(const MidiMessage& message) {
    if (message.sysExPayloadLength() >= DISTRIBUTOR_NUM_CFG_BYTES) {
        addDistributor(message.sysExCmdPayload());
        // V1: no SysEx response for AddDistributor
        return std::nullopt;
    }
    return std::nullopt;
}

std::optional<MidiMessage> MessageHandler::handleRemoveDistributor(const MidiMessage& message) {
    if (message.sysExPayloadLength() >= 1) {
        uint8_t id = message.sysExCmdPayload()[0];
        
        if (id < getDistributorCount()) {
            removeDistributor(id);
            // V1: no SysEx response for RemoveDistributor
            return std::nullopt;
        }
    }
    return std::nullopt;
}

std::optional<MidiMessage> MessageHandler::handleSetDistributor(const MidiMessage& message) {
    if (message.sysExPayloadLength() >= DISTRIBUTOR_NUM_CFG_BYTES + 1) {
        uint8_t id = message.sysExCmdPayload()[0];
        setDistributor(id, &message.sysExCmdPayload()[1]);
        // V1: no SysEx response for SetDistributor
        return std::nullopt;
    }
    return std::nullopt;
}

std::optional<MidiMessage> MessageHandler::handleGetDistributor(const MidiMessage& message) {
    if (message.sysExPayloadLength() >= 1) {
        uint8_t id = message.sysExCmdPayload()[0];
        
        if (id < getDistributorCount()) {
            // Get distributor serialized data (24 bytes)
            std::array<uint8_t, DISTRIBUTOR_NUM_CFG_BYTES> distributorData = 
                m_distributors[id]->toSerial();
            
            // Set the Distributor ID in bytes 0-1 (14-bit format)
            distributorData[0] = (id >> 7) & 0x7F;  // ID MSB (bits 7-13)
            distributorData[1] = id & 0x7F;         // ID LSB (bits 0-6)
            
            return MidiMessage(m_srcDeviceId, message.SourceID(), 
                              message.sysExCommand(), distributorData.data(), DISTRIBUTOR_NUM_CFG_BYTES);
        }
    }
    
    // V1: no response on invalid distributor request
    return std::nullopt;
}

std::optional<MidiMessage> MessageHandler::handleGetNumDistributors(const MidiMessage& message) {
    uint8_t count = getDistributorCount();
    return MidiMessage(m_srcDeviceId, message.SourceID(), message.sysExCommand(), &count, 1);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Validation
////////////////////////////////////////////////////////////////////////////////////////////////////

bool MessageHandler::isValidDistributorId(uint8_t id) const {
    return id < m_distributors.size();
}

bool MessageHandler::isValidSysExMessage(const MidiMessage& message) const {
    if (!message.isSysEx()) return false;
    if (message.sysExLength < 6) return false;  // Minimum header size
    if (message.sysExID() != SYSEX_ID) return false;
    
    return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Missing SysEx Command Handlers (V1.0 Compatibility)
////////////////////////////////////////////////////////////////////////////////////////////////////

std::optional<MidiMessage> MessageHandler::handleDeviceReady(const MidiMessage& message) {
    // V1: respond with an acknowledgement
    uint8_t ready = 1;
    return MidiMessage(m_srcDeviceId, message.SourceID(), message.sysExCommand(), &ready, 1);
}

void MessageHandler::handleResetDeviceConfig(const MidiMessage& message) {
    // Reset all distributors to default configuration
    m_distributors.clear();
    // Reset local Device name to V1 default
    Device::Name = "New Device";
    DEBUG_SERIAL("Device configuration reset\n");
}

std::optional<MidiMessage> MessageHandler::handleDiscoverDevices(const MidiMessage& message) {
    // Return Device ID Byte 0, 1 per protocol
    std::array<uint8_t, 2> deviceInfo;
    deviceInfo[0] = (m_srcDeviceId >> 7) & 0x7F;  // MSB (7-bit)
    deviceInfo[1] = m_srcDeviceId & 0x7F;         // LSB (7-bit)
    
    return MidiMessage(m_srcDeviceId, message.SourceID(), message.sysExCommand(), deviceInfo.data(), deviceInfo.size());
}

std::optional<MidiMessage> MessageHandler::handleGetDeviceConstruct(const MidiMessage& message) {
    // Use Device::GetDeviceConstruct() from Device namespace (matches src2)
    auto construct = Device::GetDeviceConstruct();
    return MidiMessage(m_srcDeviceId, message.SourceID(), message.sysExCommand(), construct.data(), DEVICE_NUM_CFG_BYTES);
}

std::optional<MidiMessage> MessageHandler::handleGetDeviceName(const MidiMessage& message) {
    // Return the current Device::Name as a null-padded buffer
    std::array<uint8_t, DEVICE_NUM_NAME_BYTES> nameBuffer{};
    size_t nameLen = Device::Name.size();
    for (size_t i = 0; i < DEVICE_NUM_NAME_BYTES; ++i) {
        nameBuffer[i] = (i < nameLen) ? static_cast<uint8_t>(Device::Name[i]) : 0x00;
    }
    return MidiMessage(m_srcDeviceId, message.SourceID(), message.sysExCommand(), nameBuffer.data(), DEVICE_NUM_NAME_BYTES);
}

std::optional<MidiMessage> MessageHandler::handleRemoveAllDistributors(const MidiMessage& message) {
    m_distributors.clear();
    // V1: no SysEx response for RemoveAllDistributors
    return std::nullopt;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Status and Diagnostics
////////////////////////////////////////////////////////////////////////////////////////////////////

// printStatus/printDistributorStatus removed - not used in V1 protocol compatibility

////////////////////////////////////////////////////////////////////////////////////////////////////
// Missing Handler Method Implementations
////////////////////////////////////////////////////////////////////////////////////////////////////

// System Commands
std::optional<MidiMessage> MessageHandler::handleMessage(const MidiMessage& message) {
    // Handle custom message - for now just acknowledge
    return std::nullopt; // No return per protocol
}

// Get Device Commands  
std::optional<MidiMessage> MessageHandler::handleGetDeviceConstructWithDistributors(const MidiMessage& message) {
    // V1: unimplemented - no response
    return std::nullopt;
}

std::optional<MidiMessage> MessageHandler::handleGetDeviceBoolean(const MidiMessage& message) {
    uint8_t deviceBool = Device::GetDeviceBoolean();
    return MidiMessage(m_srcDeviceId, message.SourceID(), message.sysExCommand(), &deviceBool, 1);
}

// Set Device Commands (no responses per protocol)
void MessageHandler::handleSetDeviceConstructWithDistributors(const MidiMessage& message) {
    // TODO: Parse and set device construct with distributors
}

void MessageHandler::handleSetDeviceConstruct(const MidiMessage& message) {
    // TODO: Parse and set device construct
}

void MessageHandler::handleSetDeviceName(const MidiMessage& message) {
    // Parse and set the device name (up to DEVICE_NUM_NAME_BYTES)
    uint8_t payloadLen = message.sysExPayloadLength();
    if (payloadLen == 0) return;
    size_t copyLen = std::min<size_t>(payloadLen, DEVICE_NUM_NAME_BYTES);
    std::string newName(reinterpret_cast<const char*>(message.sysExCmdPayload()), copyLen);
    // Trim trailing nulls
    auto pos = newName.find_last_not_of('\0');
    if (pos != std::string::npos) newName.erase(pos + 1);
    Device::Name = newName;
}

void MessageHandler::handleSetDeviceBoolean(const MidiMessage& message) {
    // TODO: Parse and set device boolean values
}

std::optional<MidiMessage> MessageHandler::handleGetAllDistributors(const MidiMessage& message) {
    uint8_t count = getDistributorCount();
    return MidiMessage(m_srcDeviceId, message.SourceID(), message.sysExCommand(), &count, 1);
}

std::optional<MidiMessage> MessageHandler::handleToggleMuteDistributor(const MidiMessage& message) {
    if (message.sysExPayloadLength() >= 2) {
        uint8_t id = message.sysExCmdPayload()[0]; // Assuming single byte for now
        if (id < getDistributorCount()) {
            m_distributors[id]->toggleMuted();
            // V1: no SysEx response for toggle
            return std::nullopt;
        }
    }
    return std::nullopt;
}

// Get Distributor Commands (0x40-0x4F)
std::optional<MidiMessage> MessageHandler::handleGetDistributorConstruct(const MidiMessage& message) {
    if (message.sysExPayloadLength() >= 1) {
        uint8_t id = message.sysExCmdPayload()[0];
        if (id < getDistributorCount()) {
            auto data = m_distributors[id]->toSerial();
            // Set distributor ID in first two bytes
            data[0] = (id >> 7) & 0x7F;
            data[1] = id & 0x7F;
            return MidiMessage(m_srcDeviceId, message.SourceID(), 
                              message.sysExCommand(), data.data(), data.size());
        }
    }
    return std::nullopt;
}

std::optional<MidiMessage> MessageHandler::handleGetDistributorChannels(const MidiMessage& message) {
    if (message.sysExPayloadLength() >= 1) {
        uint8_t id = message.sysExCmdPayload()[0];
        if (id < getDistributorCount()) {
            uint16_t channels = m_distributors[id]->getChannels();
            uint8_t data[2] = {
                static_cast<uint8_t>((channels >> 7) & 0x7F),
                static_cast<uint8_t>(channels & 0x7F)
            };
            return MidiMessage(m_srcDeviceId, message.SourceID(), 
                              message.sysExCommand(), data, 2);
        }
    }
    return std::nullopt;
}

std::optional<MidiMessage> MessageHandler::handleGetDistributorInstruments(const MidiMessage& message) {
    if (message.sysExPayloadLength() >= 1) {
        uint8_t id = message.sysExCmdPayload()[0];
        if (id < getDistributorCount()) {
            uint32_t instruments = m_distributors[id]->getInstruments();
            uint8_t data[4] = {
                static_cast<uint8_t>((instruments >> 21) & 0x7F),
                static_cast<uint8_t>((instruments >> 14) & 0x7F),
                static_cast<uint8_t>((instruments >> 7) & 0x7F),
                static_cast<uint8_t>(instruments & 0x7F)
            };
            return MidiMessage(m_srcDeviceId, message.SourceID(), 
                              message.sysExCommand(), data, 4);
        }
    }
    return std::nullopt;
}

std::optional<MidiMessage> MessageHandler::handleGetDistributorMethod(const MidiMessage& message) {
    if (message.sysExPayloadLength() >= 1) {
        uint8_t id = message.sysExCmdPayload()[0];
        if (id < getDistributorCount()) {
            uint8_t method = static_cast<uint8_t>(m_distributors[id]->getMethod());
            return MidiMessage(m_srcDeviceId, message.SourceID(), 
                              message.sysExCommand(), &method, 1);
        }
    }
    return std::nullopt;
}

std::optional<MidiMessage> MessageHandler::handleGetDistributorBoolValues(const MidiMessage& message) {
    if (message.sysExPayloadLength() >= 1) {
        uint8_t id = message.sysExCmdPayload()[0];
        if (id < getDistributorCount()) {
            uint8_t boolByte = 0;
            if (m_distributors[id]->getMuted()) boolByte |= 0x01;
            if (m_distributors[id]->getDamperPedal()) boolByte |= 0x02;
            if (m_distributors[id]->getPolyphonic()) boolByte |= 0x04;
            // Add other boolean values as needed
            return MidiMessage(m_srcDeviceId, message.SourceID(), 
                              message.sysExCommand(), &boolByte, 1);
        }
    }
    return std::nullopt;
}

std::optional<MidiMessage> MessageHandler::handleGetDistributorMinMaxNotes(const MidiMessage& message) {
    if (message.sysExPayloadLength() >= 1) {
        uint8_t id = message.sysExCmdPayload()[0];
        if (id < getDistributorCount()) {
            uint8_t data[2] = {
                m_distributors[id]->getMinNote(),
                m_distributors[id]->getMaxNote()
            };
            return MidiMessage(m_srcDeviceId, message.SourceID(), 
                              message.sysExCommand(), data, 2);
        }
    }
    return std::nullopt;
}

std::optional<MidiMessage> MessageHandler::handleGetDistributorNumPolyphonicNotes(const MidiMessage& message) {
    if (message.sysExPayloadLength() >= 1) {
        uint8_t id = message.sysExCmdPayload()[0];
        if (id < getDistributorCount()) {
            uint8_t numNotes = m_distributors[id]->getNumPolyphonicNotes();
            return MidiMessage(m_srcDeviceId, message.SourceID(), 
                              message.sysExCommand(), &numNotes, 1);
        }
    }
    return std::nullopt;
}

// Set Distributor Commands (0x50-0x5F) - Boolean returns only
bool MessageHandler::handleSetDistributorConstruct(const MidiMessage& message) {
    // Requires 24 bytes of distributor data
    if (message.sysExPayloadLength() >= 24) {
        const uint8_t* data = message.sysExCmdPayload();
        uint8_t id = ((data[0] & 0x7F) << 7) | (data[1] & 0x7F);
        if (id < getDistributorCount()) {
            m_distributors[id]->fromSerial(data);
            return true;
        }
    }
    return false;
}

bool MessageHandler::handleSetDistributorChannels(const MidiMessage& message) {
    if (message.sysExPayloadLength() >= 3) {
        const uint8_t* data = message.sysExCmdPayload();
        uint8_t id = data[0];
        uint16_t channels = ((data[1] & 0x7F) << 7) | (data[2] & 0x7F);
        if (id < getDistributorCount()) {
            m_distributors[id]->setChannels(channels);
            return true;
        }
    }
    return false;
}

bool MessageHandler::handleSetDistributorInstruments(const MidiMessage& message) {
    if (message.sysExPayloadLength() >= 5) {
        const uint8_t* data = message.sysExCmdPayload();
        uint8_t id = data[0];
        uint32_t instruments = ((data[1] & 0x7F) << 21) | 
                              ((data[2] & 0x7F) << 14) |
                              ((data[3] & 0x7F) << 7) |
                              (data[4] & 0x7F);
        if (id < getDistributorCount()) {
            m_distributors[id]->setInstruments(instruments);
            return true;
        }
    }
    return false;
}

bool MessageHandler::handleSetDistributorMethod(const MidiMessage& message) {
    if (message.sysExPayloadLength() >= 2) {
        const uint8_t* data = message.sysExCmdPayload();
        uint8_t id = data[0];
        uint8_t method = data[1];
        if (id < getDistributorCount()) {
            m_distributors[id]->setDistributionMethod(static_cast<DistributionMethod>(method));
            return true;
        }
    }
    return false;
}

bool MessageHandler::handleSetDistributorBoolValues(const MidiMessage& message) {
    if (message.sysExPayloadLength() >= 2) {
        const uint8_t* data = message.sysExCmdPayload();
        uint8_t id = data[0];
        uint8_t boolByte = data[1];
        if (id < getDistributorCount()) {
            m_distributors[id]->setMuted(boolByte & 0x01);
            m_distributors[id]->setDamperPedal(boolByte & 0x02);
            m_distributors[id]->setPolyphonic(boolByte & 0x04);
            return true;
        }
    }
    return false;
}

bool MessageHandler::handleSetDistributorMinMaxNotes(const MidiMessage& message) {
    if (message.sysExPayloadLength() >= 3) {
        const uint8_t* data = message.sysExCmdPayload();
        uint8_t id = data[0];
        uint8_t minNote = data[1];
        uint8_t maxNote = data[2];
        if (id < getDistributorCount()) {
            m_distributors[id]->setMinMaxNote(minNote, maxNote);
            return true;
        }
    }
    return false;
}

bool MessageHandler::handleSetDistributorNumPolyphonicNotes(const MidiMessage& message) {
    if (message.sysExPayloadLength() >= 2) {
        const uint8_t* data = message.sysExCmdPayload();
        uint8_t id = data[0];
        uint8_t numNotes = data[1];
        if (id < getDistributorCount()) {
            m_distributors[id]->setNumPolyphonicNotes(numNotes);
            return true;
        }
    }
    return false;
}