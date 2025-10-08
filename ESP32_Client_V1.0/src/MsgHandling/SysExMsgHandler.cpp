/*
 * SysExMsgHandler.cpp
 * 
 * Handles SysEx MIDI messages for device configuration and management.
 * Extracted from the original MessageHandler to follow single responsibility principle.
 */

#include "SysExMsgHandler.h"
#include "Distributors/DistributorManager.h"
#include <Arduino.h>
#include <cstring>  // For memcpy and strncpy
#include <algorithm> // For std::min

// Constructor with dependency injection
SysExMsgHandler::SysExMsgHandler(std::shared_ptr<DistributorManager> distributorManager)
    : m_distributorManager(distributorManager)
{
    // Ensure source ID matches current runtime device ID
    m_sourceId = Device::GetDeviceID();
}

// Process a SysEx message and return optional response
std::optional<MidiMessage> SysExMsgHandler::processSysExMessage(MidiMessage& message)
{
    // Check MIDI ID
    if (message.sysExID() != SysEx::ID) return {};
    // Check Device ID or Global ID 0x00;
    if (!isValidDestination(message)) return {};
    
    m_destinationId = message.SourceID();

    switch (message.sysExCommand()) {
        
        case (SysEx::DeviceReady):
            return sysExDeviceReady(message);

        case (SysEx::ResetDeviceConfig):
            sysExResetDeviceConfig(message); 
            return {};
            
        case (SysEx::DiscoverDevices):
            return {sysExDiscoverDevices(message)};

        case (SysEx::GetDeviceConstructWithDistributors):
            return sysExGetDeviceConstructWithDistributors(message);

        case (SysEx::GetDeviceConstruct):
            return sysExGetDeviceConstruct(message);

        case (SysEx::GetDeviceName):
            return sysExGetDeviceName(message);

        case (SysEx::GetDeviceBoolean):
            return sysExGetDeviceBoolean(message);

        case (SysEx::GetDeviceID):
            return sysExGetDeviceID(message);

        case (SysEx::SetDeviceConstructWithDistributors):
            sysExSetDeviceConstructWithDistributors(message);
            return {};

        case (SysEx::SetDeviceConstruct):
            sysExSetDeviceConstruct(message); 
            return {};

        case (SysEx::SetDeviceID):
            sysExSetDeviceID(message);
            return {};

        case (SysEx::SetDeviceName):
            sysExSetDeviceName(message); 
            return {};
            
        case (SysEx::SetDeviceBoolean):
            sysExSetDeviceBoolean(message);
            return {};

        case (SysEx::RemoveDistributor):
            if (m_distributorManager) {
                m_distributorManager->removeDistributor(message.sysExDistributorID());
            }
            return {};
            
        case (SysEx::RemoveAllDistributors):
            if (m_distributorManager) {
                m_distributorManager->removeAllDistributors();
            }
            return {};
            
        case (SysEx::GetNumOfDistributors):
            return sysExGetNumOfDistributors(message);

        case (SysEx::GetAllDistributors):
            return sysExGetAllDistributors(message);

        case (SysEx::AddDistributor):
            sysExSetDistributor(message);
            return {};
            
        case (SysEx::ToggleMuteDistributor):
            return sysExToggleMuteDistributor(message);

        case (SysEx::GetDistributorConstruct):
            return sysExGetDistributorConstruct(message);

        case (SysEx::GetDistributorChannels):
            return sysExGetDistributorChannels(message);

        case (SysEx::GetDistributorInstruments):
            return sysExGetDistributorInstruments(message);

        case (SysEx::GetDistributorMethod):
            return sysExGetDistributorMethod(message);

        case (SysEx::GetDistributorBoolValues):
            return sysExGetDistributorBoolValues(message);

        case (SysEx::GetDistributorMinMaxNotes):
            return sysExGetDistributorMinMaxNotes(message);

        case (SysEx::GetDistributorNumPolyphonicNotes):
            return sysExGetDistributorNumPolyphonicNotes(message);

        case (SysEx::SetDistributor):
            sysExSetDistributor(message); 
            return {};
            
        case (SysEx::SetDistributorChannels):
            sysExSetDistributorChannels(message); 
            return {};
            
        case (SysEx::SetDistributorInstruments):
            sysExSetDistributorInstruments(message); 
            return {};
            
        case (SysEx::SetDistributorMethod):
            sysExSetDistributorMethod(message); 
            return {};
            
        case (SysEx::SetDistributorBoolValues):
            sysExSetDistributorBoolValues(message);
            return {};
            
        case (SysEx::SetDistributorMinMaxNotes):
            sysExSetDistributorMinMaxNotes(message); 
            return {};
            
        case (SysEx::SetDistributorNumPolyphonicNotes):
            sysExSetDistributorNumPolyphonicNotes(message); 
            return {};
            
        default:
            return {};
    }
}

// Set callback for device configuration changes
void SysExMsgHandler::setDeviceChangedCallback(std::function<void()> callback)
{
    m_deviceChangedCallback = callback;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Device Configuration Commands
////////////////////////////////////////////////////////////////////////////////////////////////////

// Respond with device ready
MidiMessage SysExMsgHandler::sysExDeviceReady(MidiMessage& message)
{
    return MidiMessage(m_sourceId, m_destinationId, message.sysExCommand(), &SysEx::DeviceReady, (uint8_t)true);
}

// Reset Distributors, LocalStorage and Set Device name to "New Device"
void SysExMsgHandler::sysExResetDeviceConfig(MidiMessage& message)
{
    if (m_distributorManager) {
        m_distributorManager->removeAllDistributors();
    }
    localStorageReset();
    char name[DEVICE_NUM_NAME_BYTES] = "New Device";
    localStorageSetDeviceName(name);
    Device::Name = "New Device";
    broadcastDeviceChanged();
}

// Respond with device ready
MidiMessage SysExMsgHandler::sysExDiscoverDevices(MidiMessage& message)
{
    std::array<std::uint8_t, 2> deviceID;
    uint16_t runtimeId = Device::GetDeviceID();
    deviceID[0] = static_cast<uint8_t>((runtimeId >> 7) & 0x7F); // Device ID MSB
    deviceID[1] = static_cast<uint8_t>((runtimeId >> 0) & 0x7F); // Device ID LSB
    return MidiMessage(m_sourceId, m_destinationId, message.sysExCommand(), deviceID.data(), 2);
}

// Respond with Device Construct With Distributors
MidiMessage SysExMsgHandler::sysExGetDeviceConstructWithDistributors(MidiMessage& message)
{
    // TODO: Implementation needed
    return MidiMessage(m_sourceId, m_destinationId, message.sysExCommand(), nullptr, 0);
}

// Respond with Device Construct
MidiMessage SysExMsgHandler::sysExGetDeviceConstruct(MidiMessage& message)
{
    return MidiMessage(m_sourceId, m_destinationId, message.sysExCommand(), Device::GetDeviceConstruct().data(), DEVICE_NUM_CFG_BYTES);
}

// Respond with Device Name
MidiMessage SysExMsgHandler::sysExGetDeviceName(MidiMessage& message)
{
    // Create a properly formatted buffer with null padding
    uint8_t nameBuffer[DEVICE_NUM_NAME_BYTES];
    
    // Initialize entire buffer with null bytes
    memset(nameBuffer, 0x00, DEVICE_NUM_NAME_BYTES);
    
    // Copy device name (up to the buffer size, ensuring we don't overflow)
    size_t copyLength = std::min(Device::Name.length(), static_cast<size_t>(DEVICE_NUM_NAME_BYTES));
    memcpy(nameBuffer, Device::Name.c_str(), copyLength);

    return MidiMessage(m_sourceId, m_destinationId, message.sysExCommand(), nameBuffer, DEVICE_NUM_NAME_BYTES);
}

// Respond with Device Boolean
MidiMessage SysExMsgHandler::sysExGetDeviceBoolean(MidiMessage& message)
{
    static uint8_t deviceBoolean = Device::GetDeviceBoolean();
    return MidiMessage(m_sourceId, m_destinationId, message.sysExCommand(), &deviceBoolean, 1);
}

// Respond with Device ID (14-bit split into two 7-bit bytes)
MidiMessage SysExMsgHandler::sysExGetDeviceID(MidiMessage& message)
{
    std::array<std::uint8_t, 2> deviceID;
    uint16_t runtimeId = Device::GetDeviceID();
    deviceID[0] = static_cast<uint8_t>((runtimeId >> 7) & 0x7F);
    deviceID[1] = static_cast<uint8_t>((runtimeId >> 0) & 0x7F);
    return MidiMessage(m_sourceId, m_destinationId, message.sysExCommand(), deviceID.data(), 2);
}

// Set Device Construct With Distributors
void SysExMsgHandler::sysExSetDeviceConstructWithDistributors(MidiMessage& message)
{
    // TODO: Implementation needed
    broadcastDeviceChanged();
}

// Configure Device Construct
void SysExMsgHandler::sysExSetDeviceConstruct(MidiMessage& message)
{
    // TODO: Implementation needed
    broadcastDeviceChanged();
}

// Set Device ID from incoming payload (expecting at least 2 bytes MSB,LSB)
void SysExMsgHandler::sysExSetDeviceID(MidiMessage& message)
{
    if (message.length < SYSEX_HeaderSize + 2) return; // not enough data
    uint8_t msb = message.sysExCmdPayload()[0];
    uint8_t lsb = message.sysExCmdPayload()[1];
    uint16_t newID = (static_cast<uint16_t>(msb) << 7) | static_cast<uint16_t>(lsb);
    Device::SetDeviceID(newID);
    // persist
    #ifdef EXTRA_LOCAL_STORAGE
    LocalStorage::get().SetDeviceID(newID);
    #endif
    // update source id used for replies
    m_sourceId = Device::GetDeviceID();
    broadcastDeviceChanged();
}

// Configure Device Name
void SysExMsgHandler::sysExSetDeviceName(MidiMessage& message)
{
    // Calculate payload length (total message length - SysEx header - end byte)
    uint8_t payloadLength = message.length - SYSEX_HeaderSize - 1;
    
    // Safely create string from payload with length validation
    std::string newName(reinterpret_cast<const char*>(message.sysExCmdPayload()), 
                       std::min(payloadLength, static_cast<uint8_t>(DEVICE_NUM_NAME_BYTES - 1)));
    
    // Remove any null bytes from the end (clean up the string)
    newName.erase(newName.find_last_not_of('\0') + 1);

    // Assign the processed name (no padding needed for std::string)
    Device::Name = newName;
    
    // Convert to C-string for local storage (ensure null termination)
    char deviceNameBuffer[DEVICE_NUM_NAME_BYTES];
    memset(deviceNameBuffer, 0x00, DEVICE_NUM_NAME_BYTES);
    strncpy(deviceNameBuffer, newName.c_str(), DEVICE_NUM_NAME_BYTES - 1);
    localStorageSetDeviceName(deviceNameBuffer);
    broadcastDeviceChanged();
}

// Configure Device Boolean
void SysExMsgHandler::sysExSetDeviceBoolean(MidiMessage& message)
{
    Device::OmniMode = ((message.sysExCmdPayload()[0] & DEVICE_BOOL_OMNIMODE) != 0);
    broadcastDeviceChanged();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Distributor Management Commands (delegates to DistributorManager)
////////////////////////////////////////////////////////////////////////////////////////////////////

// Respond with the Number of Distributors
MidiMessage SysExMsgHandler::sysExGetNumOfDistributors(MidiMessage& message)
{
    uint8_t sizeByte = m_distributorManager ? m_distributorManager->getDistributorCount() : 0;
    return MidiMessage(m_sourceId, m_destinationId, message.sysExCommand(), &sizeByte, 1);
}

// Respond with All Distributors
MidiMessage SysExMsgHandler::sysExGetAllDistributors(MidiMessage& message)
{
    // TODO: Return Multiple Messages - needs different implementation approach
    return sysExGetNumOfDistributors(message);
}

// Toggle Distributor Mute
MidiMessage SysExMsgHandler::sysExToggleMuteDistributor(MidiMessage& message)
{
    if (m_distributorManager) {
        m_distributorManager->toggleDistributorMute(message.sysExDistributorID());
    }
    return MidiMessage(m_sourceId, m_destinationId, message.sysExCommand(), nullptr, 0);
}

// Respond with the requested Distributor Construct
MidiMessage SysExMsgHandler::sysExGetDistributorConstruct(MidiMessage& message)
{
    if (!m_distributorManager) {
        return MidiMessage(m_sourceId, m_destinationId, message.sysExCommand(), nullptr, 0);
    }
    
    auto distributorBytes = m_distributorManager->getDistributorSerial(message.sysExDistributorID());
    // Set Return Distributor ID from message
    distributorBytes[0] = message.sysExCmdPayload()[0];
    distributorBytes[1] = message.sysExCmdPayload()[1];
    return MidiMessage(m_sourceId, m_destinationId, message.sysExCommand(), distributorBytes.data(), distributorBytes.size());
}

// Respond with the requested Distributor Channel Config
MidiMessage SysExMsgHandler::sysExGetDistributorChannels(MidiMessage& message)
{
    if (!m_distributorManager) {
        const uint8_t bytesToSend[2] = {0, 0};
        return MidiMessage(m_sourceId, m_destinationId, message.sysExCommand(), bytesToSend, 2);
    }
    
    uint16_t channels = m_distributorManager->getDistributorChannels(message.sysExDistributorID());
    const uint8_t bytesToSend[2] = {static_cast<uint8_t>((channels >> 7) & 0x7F), 
                                   static_cast<uint8_t>((channels >> 0) & 0x7F)};
    return MidiMessage(m_sourceId, m_destinationId, message.sysExCommand(), bytesToSend, 2);
}

// Respond with the requested Distributor Instrument Config
MidiMessage SysExMsgHandler::sysExGetDistributorInstruments(MidiMessage& message)
{
    if (!m_distributorManager) {
        const uint8_t bytesToSend[4] = {0, 0, 0, 0};
        return MidiMessage(m_sourceId, m_destinationId, message.sysExCommand(), bytesToSend, 4);
    }
    
    uint32_t instruments = m_distributorManager->getDistributorInstruments(message.sysExDistributorID());
    const uint8_t bytesToSend[4] = {static_cast<uint8_t>((instruments >> 21) & 0x7F), 
                                   static_cast<uint8_t>((instruments >> 14) & 0x7F),
                                   static_cast<uint8_t>((instruments >> 7) & 0x7F),
                                   static_cast<uint8_t>((instruments >> 0) & 0x7F)};
    return MidiMessage(m_sourceId, m_destinationId, message.sysExCommand(), bytesToSend, 4);
}

// Respond with the requested Distributor Method Config
MidiMessage SysExMsgHandler::sysExGetDistributorMethod(MidiMessage& message)
{
    // TODO: Implementation needed - need to add getDistributorMethod to DistributorManager
    uint8_t method = 0;
    return MidiMessage(m_sourceId, m_destinationId, message.sysExCommand(), &method, 1);
}

// Respond with the requested Distributor Bool Values Config
MidiMessage SysExMsgHandler::sysExGetDistributorBoolValues(MidiMessage& message)
{
    // TODO: Implementation needed - need to add getDistributorBoolValues to DistributorManager
    uint8_t boolValues = 0;
    return MidiMessage(m_sourceId, m_destinationId, message.sysExCommand(), &boolValues, 1);
}

// Respond with the requested Distributor Min/Max Notes Config
MidiMessage SysExMsgHandler::sysExGetDistributorMinMaxNotes(MidiMessage& message)
{
    // TODO: Implementation needed - need to add getDistributorMinMaxNotes to DistributorManager
    const uint8_t bytesToSend[2] = {0, 127};
    return MidiMessage(m_sourceId, m_destinationId, message.sysExCommand(), bytesToSend, 2);
}

// Respond with the requested Distributor Num Polyphonic Notes Config
MidiMessage SysExMsgHandler::sysExGetDistributorNumPolyphonicNotes(MidiMessage& message)
{
    // TODO: Implementation needed - need to add getDistributorNumPolyphonicNotes to DistributorManager
    uint8_t numNotes = 1;
    return MidiMessage(m_sourceId, m_destinationId, message.sysExCommand(), &numNotes, 1);
}

// Set Distributor
void SysExMsgHandler::sysExSetDistributor(MidiMessage& message)
{
    if (m_distributorManager) {
        m_distributorManager->setDistributor(message.sysExCmdPayload());
    }
}

// Configure the designated Distributor's Channels
void SysExMsgHandler::sysExSetDistributorChannels(MidiMessage& message)
{
    if (!m_distributorManager) return;
    
    uint16_t channels = ((message.sysExCmdPayload()[2] << 14) 
                       | (message.sysExCmdPayload()[3] << 7) 
                       | (message.sysExCmdPayload()[4] << 0));
    m_distributorManager->setDistributorChannels(message.sysExDistributorID(), channels);
}

// Configure the designated Distributor's Instruments
void SysExMsgHandler::sysExSetDistributorInstruments(MidiMessage& message)
{
    if (!m_distributorManager) return;
    
    uint32_t instruments = ((message.sysExCmdPayload()[2] << 28) 
                          & (message.sysExCmdPayload()[3] << 21) 
                          & (message.sysExCmdPayload()[4] << 14) 
                          & (message.sysExCmdPayload()[5] << 7) 
                          & (message.sysExCmdPayload()[6] << 0));
    m_distributorManager->setDistributorInstruments(message.sysExDistributorID(), instruments);
}

// Configure the designated Distributor's Distribution Method
void SysExMsgHandler::sysExSetDistributorMethod(MidiMessage& message)
{
    if (!m_distributorManager) return;
    
    m_distributorManager->setDistributorMethod(message.sysExDistributorID(), 
                                             DistributionMethod(message.sysExCmdPayload()[2]));
}

// Configure the designated Distributor's Boolean Values
void SysExMsgHandler::sysExSetDistributorBoolValues(MidiMessage& message)
{
    if (!m_distributorManager) return;
    
    uint8_t distributorBoolByte = message.sysExCmdPayload()[2];
    m_distributorManager->setDistributorBoolValues(message.sysExDistributorID(), distributorBoolByte);
}

// Configure the designated Distributor's Minimum and Maximum Notes
void SysExMsgHandler::sysExSetDistributorMinMaxNotes(MidiMessage& message)
{
    if (!m_distributorManager) return;
    
    m_distributorManager->setDistributorMinMaxNotes(message.sysExDistributorID(),
                                                   message.sysExCmdPayload()[2],
                                                   message.sysExCmdPayload()[3]);
}

// Configure the designated Distributor's Max number of Polyphonic Notes
void SysExMsgHandler::sysExSetDistributorNumPolyphonicNotes(MidiMessage& message)
{
    if (!m_distributorManager) return;
    
    m_distributorManager->setDistributorNumPolyphonicNotes(message.sysExDistributorID(),
                                                          message.sysExCmdPayload()[2]);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Helper Methods
////////////////////////////////////////////////////////////////////////////////////////////////////

// Broadcast device configuration change
void SysExMsgHandler::broadcastDeviceChanged()
{
    if (m_deviceChangedCallback) {
        m_deviceChangedCallback();
    }
}

// Check if message destination is valid
bool SysExMsgHandler::isValidDestination(MidiMessage& message) const
{
    return (message.DestinationID() == Device::GetDeviceID() || message.DestinationID() == 0x00);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Local Storage Helpers
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef EXTRA_LOCAL_STORAGE

void SysExMsgHandler::localStorageSetDeviceName(char* name)
{
    LocalStorage::get().SetDeviceName(name);
    Device::Name = LocalStorage::get().GetDeviceName();
}

void SysExMsgHandler::localStorageAddDistributor()
{
    // Handled by DistributorManager
}

void SysExMsgHandler::localStorageRemoveDistributor(uint8_t id)
{
    // Handled by DistributorManager
}

void SysExMsgHandler::localStorageUpdateDistributor(uint16_t distributorID, const uint8_t* data)
{
    LocalStorage::get().SetDistributorConstruct(distributorID, data);
}

void SysExMsgHandler::localStorageClearDistributors()
{
    // Handled by DistributorManager
}

void SysExMsgHandler::localStorageReset()
{
    LocalStorage::get().ResetDeviceConfig();
}

#endif