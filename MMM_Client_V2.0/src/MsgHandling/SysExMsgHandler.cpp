/*
 * SysExMsgHandler.cpp
 * 
 * Handles SysEx MIDI messages for device configuration and management.
 * Extracted from the original MessageHandler to follow single responsibility principle.
 */

#include "SysExMsgHandler.h"
#include "Distributors/DistributorManager.h"
#include "Instruments/InstrumentControllerBase.h"
#include "Utility/BitManipulation.h"
#include <Arduino.h>
#include <cstring>  // For memcpy and strncpy
#include <algorithm> // For std::min

// Constructor with dependency injection
SysExMsgHandler::SysExMsgHandler(DistributorManager& distributorManager, 
    InstrumentControllerBase& instrumentController)
    : distributorManager(&distributorManager)
    , instrumentController(&instrumentController)
    , m_sourceId(Device::GetDeviceID())
{
}

// Process a SysEx message and return optional response
std::optional<MidiMessage> SysExMsgHandler::processSysExMessage(const MidiMessage& message)
{
    // Check MIDI ID
    if (message.sysExID() != SysEx::ID) return {};
    // Check Device ID or Global ID 0x00;
    if (!isValidDestination(message)) return {};
    
    // Update source ID to current runtime Device ID to ensure responses use correct ID
    m_sourceId = Device::GetDeviceID();
    m_destinationId = message.SourceID();

    std::optional<MidiMessage> response;
    if (handleDeviceCommand(message, response)) {
        return response;
    }
    if (handleDistributorCommand(message, response)) {
        return response;
    }
    if (handleInstrumentCommand(message, response)) {
        return response;
    }
    return {};
}

bool SysExMsgHandler::handleDeviceCommand(const MidiMessage& message, std::optional<MidiMessage>& response)
{
    switch (message.sysExCommand()) {
        case (SysEx::DeviceReady):
            response = sysExDeviceReady(message);
            return true;
        case (SysEx::ResetDeviceConfig):
            sysExResetDeviceConfig(message);
            response.reset();
            return true;
        case (SysEx::DiscoverDevices):
            response = sysExDiscoverDevices(message);
            return true;
        case (SysEx::DeviceConstructWithDistributors):
            if (message.length == 7) {
                response = sysExGetDeviceConstructWithDistributors(message);
                return true;
            }
            sysExSetDeviceConstructWithDistributors(message);
            response.reset();
            return true;
        case (SysEx::DeviceConstruct):
            if (message.length == 7) {
                response = sysExGetDeviceConstruct(message);
                return true;
            }
            sysExSetDeviceConstruct(message);
            response.reset();
            return true;
        case (SysEx::DeviceName):
            if (message.length == 7) {
                response = sysExGetDeviceName(message);
                return true;
            }
            sysExSetDeviceName(message);
            response.reset();
            return true;
        case (SysEx::DeviceBoolean):
            if (message.length == 7) {
                response = sysExGetDeviceBoolean(message);
                return true;
            }
            sysExSetDeviceBoolean(message);
            response.reset();
            return true;
        case (SysEx::DeviceID):
            if (message.length == 7) {
                response = sysExGetDeviceID(message);
                return true;
            }
            sysExSetDeviceID(message);
            response.reset();
            return true;
        default:
            return false;
    }
}

bool SysExMsgHandler::handleDistributorCommand(const MidiMessage& message, std::optional<MidiMessage>& response)
{
    switch (message.sysExCommand()) {
        case (SysEx::RemoveDistributor):
            if (distributorManager) {
                distributorManager->removeDistributor(message.sysExDistributorID());
            }
            response.reset();
            return true;
        case (SysEx::RemoveAllDistributors):
            if (distributorManager) {
                distributorManager->removeAllDistributors();
            }
            response.reset();
            return true;
        case (SysEx::GetNumOfDistributors):
            response = sysExGetNumOfDistributors(message);
            return true;
        case (SysEx::GetAllDistributors):
            response = sysExGetAllDistributors(message);
            return true;
        case (SysEx::AddDistributor):
            sysExSetDistributor(message);
            response.reset();
            return true;
        case (SysEx::ToggleMuteDistributor):
            response = sysExToggleMuteDistributor(message);
            return true;
        case (SysEx::DistributorConstruct):
            if (message.length == 9) {
                response = sysExGetDistributorConstruct(message);
                return true;
            }
            sysExSetDistributor(message);
            response.reset();
            return true;
        case (SysEx::DistributorChannels):
            if (message.length == 9) {
                response = sysExGetDistributorChannels(message);
                return true;
            }
            sysExSetDistributorChannels(message);
            response.reset();
            return true;
        case (SysEx::DistributorInstruments):
            if (message.length == 9) {
                response = sysExGetDistributorInstruments(message);
                return true;
            }
            sysExSetDistributorInstruments(message);
            response.reset();
            return true;
        case (SysEx::DistributorMethod):
            if (message.length == 9) {
                response = sysExGetDistributorMethod(message);
                return true;
            }
            sysExSetDistributorMethod(message);
            response.reset();
            return true;
        case (SysEx::DistributorBoolValues):
            if (message.length == 9) {
                response = sysExGetDistributorBoolValues(message);
                return true;
            }
            sysExSetDistributorBoolValues(message);
            response.reset();
            return true;
        case (SysEx::DistributorMinMaxNotes):
            if (message.length == 9) {
                response = sysExGetDistributorMinMaxNotes(message);
                return true;
            }
            sysExSetDistributorMinMaxNotes(message);
            response.reset();
            return true;
        default:
            return false;
    }
}

bool SysExMsgHandler::handleInstrumentCommand(const MidiMessage& message, std::optional<MidiMessage>& response)
{
    switch (message.sysExCommand()) {
        case (SysEx::ResetAllInstruments):
            sysExResetAllInstruments(message);
            response.reset();
            return true;
        case (SysEx::ResetInstrument):
            sysExResetInstrument(message);
            response.reset();
            return true;
        case (SysEx::GetInstrumentNumActiveNotes):
            response = sysExGetInstrumentNumActiveNotes(message);
            return true;
        case (SysEx::SetInstrumentNoteOn):
            sysExSetInstrumentNoteOn(message);
            response.reset();
            return true;
        case (SysEx::SetInstrumentNoteOff):
            sysExSetInstrumentNoteOff(message);
            response.reset();
            return true;
        default:
            return false;
    }
}

// Set callback for device configuration changes
void SysExMsgHandler::setDeviceChangedCallback(const std::function<void()>& callback)
{
    m_deviceChangedCallback = callback;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Device Configuration Commands
////////////////////////////////////////////////////////////////////////////////////////////////////

// Respond with device ready
MidiMessage SysExMsgHandler::sysExDeviceReady(const MidiMessage& message)
{
    return MidiMessage(m_sourceId, m_destinationId, SysEx::DeviceReady, nullptr, 0);
}

// Reset Distributors, LocalStorage and Set Device name to "New Device"
void SysExMsgHandler::sysExResetDeviceConfig(const MidiMessage& message)
{
    if (distributorManager) {
        distributorManager->removeAllDistributors();
    }
    localStorageReset();
    char name[DEVICE_NUM_NAME_BYTES] = "New Device";
    localStorageSetDeviceName(name);
    Device::Name = "New Device";
    broadcastDeviceChanged();
}

// Respond with device ready
MidiMessage SysExMsgHandler::sysExDiscoverDevices(const MidiMessage& message)
{
    std::array<std::uint8_t, 2> deviceID;
    uint16_t runtimeId = Device::GetDeviceID();
    deviceID[0] = static_cast<uint8_t>((runtimeId >> 7) & 0x7F); // Device ID MSB
    deviceID[1] = static_cast<uint8_t>((runtimeId >> 0) & 0x7F); // Device ID LSB
    return MidiMessage(m_sourceId, m_destinationId, message.sysExCommand(), deviceID.data(), 2);
}

// Respond with Device Construct With Distributors
MidiMessage SysExMsgHandler::sysExGetDeviceConstructWithDistributors(const MidiMessage& message)
{
    // TODO: Implementation needed
    return MidiMessage(m_sourceId, m_destinationId, message.sysExCommand(), nullptr, 0);
}

// Respond with Device Construct
MidiMessage SysExMsgHandler::sysExGetDeviceConstruct(const MidiMessage& message)
{
    return MidiMessage(m_sourceId, m_destinationId, message.sysExCommand(), Device::GetDeviceConstruct().data(), DEVICE_NUM_CFG_BYTES);
}

// Respond with Device Name
MidiMessage SysExMsgHandler::sysExGetDeviceName(const MidiMessage& message)
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
MidiMessage SysExMsgHandler::sysExGetDeviceBoolean(const MidiMessage& message)
{
    std::array<std::uint8_t, 2> deviceBooleanBytes;
    uint16_t deviceBoolean = Device::GetDeviceBoolean();
    deviceBooleanBytes[0] = static_cast<uint8_t>((deviceBoolean >> 7) & 0x7F);
    deviceBooleanBytes[1] = static_cast<uint8_t>((deviceBoolean >> 0) & 0x7F);
    return MidiMessage(m_sourceId, m_destinationId, message.sysExCommand(), deviceBooleanBytes.data(), 2);
}

// Respond with Device ID (14-bit split into two 7-bit bytes)
MidiMessage SysExMsgHandler::sysExGetDeviceID(const MidiMessage& message)
{
    std::array<std::uint8_t, 2> deviceID;
    uint16_t runtimeId = Device::GetDeviceID();
    deviceID[0] = static_cast<uint8_t>((runtimeId >> 7) & 0x7F);
    deviceID[1] = static_cast<uint8_t>((runtimeId >> 0) & 0x7F);
    return MidiMessage(m_sourceId, m_destinationId, message.sysExCommand(), deviceID.data(), 2);
}

// Set Device Construct With Distributors
void SysExMsgHandler::sysExSetDeviceConstructWithDistributors(const MidiMessage& message)
{
    sysExSetDeviceConstruct(message);
    // TODO: Handle distributors from payload - needs different implementation approach to handle multiple messages and larger payload
}

// Configure Device Construct
void SysExMsgHandler::sysExSetDeviceConstruct(const MidiMessage& message)
{
    if (message.length < (SYSEX_HeaderSize + DEVICE_NUM_CFG_BYTES + 1)) return;

    if (!Device::SetDeviceConstruct(message.sysExCmdPayload(), DEVICE_NUM_CFG_BYTES)) return;

    #ifdef CFG_EXTRA_LOCAL_STORAGE
        LocalStorageFactory::getInstance().setDeviceID(Device::GetDeviceID());
        LocalStorageFactory::getInstance().setDeviceName(Device::Name);
        LocalStorageFactory::getInstance().setDeviceBoolean(Device::GetDeviceBoolean());
    #endif

    broadcastDeviceChanged();
}

// Set Device ID from incoming payload (expecting at least 2 bytes MSB,LSB)
void SysExMsgHandler::sysExSetDeviceID(const MidiMessage& message)
{
    if (message.length < SYSEX_HeaderSize + 2) return; // not enough data
    uint8_t msb = message.sysExCmdPayload()[0];
    uint8_t lsb = message.sysExCmdPayload()[1];
    uint16_t newID = (static_cast<uint16_t>(msb) << 7) | static_cast<uint16_t>(lsb);
    Device::SetDeviceID(newID);
    // persist
    #ifdef CFG_EXTRA_LOCAL_STORAGE
    LocalStorageFactory::getInstance().setDeviceID(newID);
    #endif
    broadcastDeviceChanged();
}

// Configure Device Name
void SysExMsgHandler::sysExSetDeviceName(const MidiMessage& message)
{
    // Require at least header + tail before subtracting to avoid underflow
    if (message.length < (SYSEX_HeaderSize + 1)) return;

    // Calculate payload length (total message length - SysEx header - end byte)
    size_t payloadLength = static_cast<size_t>(message.length - SYSEX_HeaderSize - 1);
    payloadLength = std::min(payloadLength, static_cast<size_t>(DEVICE_NUM_NAME_BYTES - 1));
    
    // Safely create string from payload with length validation
    std::string newName(reinterpret_cast<const char*>(message.sysExCmdPayload()), 
                       payloadLength);
    
    // Remove any null bytes from the end (clean up the string)
    const auto lastNonNull = newName.find_last_not_of('\0');
    if (lastNonNull == std::string::npos) {
        newName.clear();
    } else {
        newName.erase(lastNonNull + 1);
    }

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
void SysExMsgHandler::sysExSetDeviceBoolean(const MidiMessage& message)
{
    // Reconstruct 16-bit boolean value from 2 MIDI 7-bit bytes
    uint16_t deviceBoolValue = (static_cast<uint16_t>(message.sysExCmdPayload()[0]) << 7) | 
                                static_cast<uint16_t>(message.sysExCmdPayload()[1]);
    Device::Muted = ((deviceBoolValue & DEVICE_BOOL_MASK::MUTED) != 0);        // Bit 0
    Device::OmniMode = ((deviceBoolValue & DEVICE_BOOL_MASK::OMNIMODE) != 0);  // Bit 1
    Device::DamperPedal = ((deviceBoolValue & DEVICE_BOOL_MASK::DAMPER_PEDAL) != 0); // Bit 2
    Device::Vibrato = ((deviceBoolValue & DEVICE_BOOL_MASK::VIBRATO) != 0); // Bit 3
    
    #ifdef CFG_EXTRA_LOCAL_STORAGE
        LocalStorageFactory::getInstance().setDeviceBoolean(deviceBoolValue);
    #endif
    
    broadcastDeviceChanged();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Distributor Management Commands (delegates to DistributorManager)
////////////////////////////////////////////////////////////////////////////////////////////////////

// Respond with the Number of Distributors
MidiMessage SysExMsgHandler::sysExGetNumOfDistributors(const MidiMessage& message)
{
    uint8_t sizeByte = distributorManager ? distributorManager->getDistributorCount() : 0;
    return MidiMessage(m_sourceId, m_destinationId, message.sysExCommand(), &sizeByte, 1);
}

// Respond with All Distributors
MidiMessage SysExMsgHandler::sysExGetAllDistributors(const MidiMessage& message)
{
    // TODO: Return Multiple Messages - needs different implementation approach
    return sysExGetNumOfDistributors(message);
}

// Toggle Distributor Mute
MidiMessage SysExMsgHandler::sysExToggleMuteDistributor(const MidiMessage& message)
{
    if (distributorManager) {
        distributorManager->toggleDistributorMute(message.sysExDistributorID());
    }
    return MidiMessage(m_sourceId, m_destinationId, message.sysExCommand(), nullptr, 0);
}

// Respond with the requested Distributor Construct
MidiMessage SysExMsgHandler::sysExGetDistributorConstruct(const MidiMessage& message)
{
    if (!distributorManager) {
        return MidiMessage(m_sourceId, m_destinationId, message.sysExCommand(), nullptr, 0);
    }
    
    // Get distributor in 7-bit MIDI format
    auto distributorBytes = distributorManager->getDistributorSerial(message.sysExDistributorID());
    
    // Set Return Distributor ID from message
    distributorBytes[0] = message.sysExCmdPayload()[0];
    distributorBytes[1] = message.sysExCmdPayload()[1];
    
    return MidiMessage(m_sourceId, m_destinationId, message.sysExCommand(), distributorBytes.data(), distributorBytes.size());
}

// Respond with the requested Distributor Channel Config
MidiMessage SysExMsgHandler::sysExGetDistributorChannels(const MidiMessage& message)
{
    if (!distributorManager) {
        const uint8_t bytesToSend[3] = {0, 0, 0};
        return MidiMessage(m_sourceId, m_destinationId, message.sysExCommand(), bytesToSend, 3);
    }
    
    // Encode 16-bit channels to 3 MIDI 7-bit bytes
    auto channelsMidi = Utility::encodeTo7Bit(distributorManager->getDistributorChannels(message.sysExDistributorID()));
    return MidiMessage(m_sourceId, m_destinationId, message.sysExCommand(), channelsMidi.data(), channelsMidi.size());
}

// Respond with the requested Distributor Instrument Config
MidiMessage SysExMsgHandler::sysExGetDistributorInstruments(const MidiMessage& message)
{
    if (!distributorManager) {
        const uint8_t bytesToSend[5] = {0, 0, 0, 0, 0};
        return MidiMessage(m_sourceId, m_destinationId, message.sysExCommand(), bytesToSend, 5);
    }
    
    // Encode 32-bit instruments to 5 MIDI 7-bit bytes
    auto instrumentsMidi = Utility::encodeTo7Bit(distributorManager->getDistributorInstruments(message.sysExDistributorID()));
    return MidiMessage(m_sourceId, m_destinationId, message.sysExCommand(), instrumentsMidi.data(), instrumentsMidi.size());
}

// Respond with the requested Distributor Method Config
MidiMessage SysExMsgHandler::sysExGetDistributorMethod(const MidiMessage& message)
{

    uint8_t method = static_cast<uint8_t>(distributorManager->getDistributorMethod(message.sysExDistributorID()));
    return MidiMessage(m_sourceId, m_destinationId, message.sysExCommand(), &method, 1);
}

// Respond with the requested Distributor Bool Values Config
MidiMessage SysExMsgHandler::sysExGetDistributorBoolValues(const MidiMessage& message)
{
    uint16_t boolValues = distributorManager->getDistributorBoolValues(message.sysExDistributorID());
    // Split into 2 MIDI 7-bit bytes
    uint8_t bytesToSend[2];
    bytesToSend[0] = (boolValues >> 7) & 0x7F;
    bytesToSend[1] = (boolValues >> 0) & 0x7F;

    return MidiMessage(m_sourceId, m_destinationId, message.sysExCommand(), bytesToSend, 2);
}

// Respond with the requested Distributor Min/Max Notes Config
MidiMessage SysExMsgHandler::sysExGetDistributorMinMaxNotes(const MidiMessage& message)
{
    uint8_t minNote = 0;
    uint8_t maxNote = 127;
    if (distributorManager) {
        minNote = distributorManager->getDistributorMinNote(message.sysExDistributorID());
        maxNote = distributorManager->getDistributorMaxNote(message.sysExDistributorID());
    }
    const uint8_t bytesToSend[2] = { minNote, maxNote };
    return MidiMessage(m_sourceId, m_destinationId, message.sysExCommand(), bytesToSend, 2);
}

// Respond with the requested Distributor Num Polyphonic Notes Config
MidiMessage SysExMsgHandler::sysExGetDistributorNumPolyphonicNotes(const MidiMessage& message)
{
    // TODO: Implementation needed - need to add getDistributorNumPolyphonicNotes to DistributorManager
    uint8_t numNotes = 1;
    return MidiMessage(m_sourceId, m_destinationId, message.sysExCommand(), &numNotes, 1);
}

// Set Distributor
void SysExMsgHandler::sysExSetDistributor(const MidiMessage& message)
{
    if (distributorManager) {
        // Data is already in 7-bit MIDI format, use directly
        distributorManager->setDistributor(const_cast<uint8_t*>(message.sysExCmdPayload()));
    }
}

// Configure the designated Distributor's Channels
void SysExMsgHandler::sysExSetDistributorChannels(const MidiMessage& message)
{
    if (message.length < SYSEX_HeaderSize + 5) return; // or log error
    
    // Decode from 3 MIDI 7-bit bytes
    auto channels = Utility::decodeFrom7Bit<NUM_Channels>(&message.sysExCmdPayload()[2]);
    distributorManager->setDistributorChannels(message.sysExDistributorID(), channels.to_ulong());
}

// Configure the designated Distributor's Instruments
void SysExMsgHandler::sysExSetDistributorInstruments(const MidiMessage& message)
{   
    if (message.length < SYSEX_HeaderSize + 7) return; // or log error
    
    // Decode from 5 MIDI 7-bit bytes
    auto instruments = Utility::decodeFrom7Bit<NUM_Instruments>(&message.sysExCmdPayload()[2]);
    distributorManager->setDistributorInstruments(message.sysExDistributorID(), instruments.to_ulong());
}

// Configure the designated Distributor's Distribution Method
void SysExMsgHandler::sysExSetDistributorMethod(const MidiMessage& message)
{
    distributorManager->setDistributorMethod(message.sysExDistributorID(), 
                                             DistributionMethod(message.sysExCmdPayload()[2]));
}

// Configure the designated Distributor's Boolean Values
void SysExMsgHandler::sysExSetDistributorBoolValues(const MidiMessage& message)
{
    // Reconstruct 16-bit boolean value from 2 MIDI 7-bit bytes
    uint16_t distributorBoolValue = (static_cast<uint16_t>(message.sysExCmdPayload()[2]) << 7) | 
                                     static_cast<uint16_t>(message.sysExCmdPayload()[3]);
    distributorManager->setDistributorBoolValues(message.sysExDistributorID(), distributorBoolValue);
}

// Configure the designated Distributor's Minimum and Maximum Notes
void SysExMsgHandler::sysExSetDistributorMinMaxNotes(const MidiMessage& message)
{
    distributorManager->setDistributorMinMaxNotes(message.sysExDistributorID(),
                                                   message.sysExCmdPayload()[2],
                                                   message.sysExCmdPayload()[3]);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Instrument Direct Methods
////////////////////////////////////////////////////////////////////////////////////////////////////

void SysExMsgHandler::sysExResetAllInstruments(const MidiMessage& message)
{
    if (instrumentController) {
        instrumentController->resetAll();
    }
}

void SysExMsgHandler::sysExResetInstrument(const MidiMessage& message)
{
    if (!instrumentController || message.length < SYSEX_HeaderSize + 1) return;

    const uint8_t instrumentId = message.sysExCmdPayload()[0];
    if (instrumentId >= NUM_Instruments) return;

    instrumentController->reset(instrumentId);
}

MidiMessage SysExMsgHandler::sysExGetInstrumentNumActiveNotes(const MidiMessage& message)
{
    uint8_t numActiveNotes = 0;
    if (instrumentController && message.length >= SYSEX_HeaderSize + 1) {
        const uint8_t instrumentId = message.sysExCmdPayload()[0];
        if (instrumentId < NUM_Instruments) {
            numActiveNotes = instrumentController->getNumActiveNotes(instrumentId);
        }
    }
    return MidiMessage(m_sourceId, m_destinationId, message.sysExCommand(), &numActiveNotes, 1);
}

void SysExMsgHandler::sysExSetInstrumentNoteOn(const MidiMessage& message)
{
    if (!instrumentController || message.length < SYSEX_HeaderSize + 4) return;

    const uint8_t instrumentId = message.sysExCmdPayload()[0];
    if (instrumentId >= NUM_Instruments) return;

    instrumentController->playNote(instrumentId,// Instrument ID
        message.sysExCmdPayload()[2], // Note
        message.sysExCmdPayload()[3], // Velocity
        message.sysExCmdPayload()[1]);// Channel
}

void SysExMsgHandler::sysExSetInstrumentNoteOff(const MidiMessage& message)
{
    if (!instrumentController || message.length < SYSEX_HeaderSize + 4) return;

    const uint8_t instrumentId = message.sysExCmdPayload()[0];
    if (instrumentId >= NUM_Instruments) return;

    instrumentController->stopNote(instrumentId,// Instrument ID
        message.sysExCmdPayload()[2], // Note
        message.sysExCmdPayload()[3], // Velocity
        message.sysExCmdPayload()[1]);// Channel
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
bool SysExMsgHandler::isValidDestination(const MidiMessage& message) const
{
    return (message.DestinationID() == Device::GetDeviceID() || message.DestinationID() == 0x00);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Local Storage Helpers
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef CFG_EXTRA_LOCAL_STORAGE

void SysExMsgHandler::localStorageSetDeviceName(char* name)
{
    LocalStorageFactory::getInstance().setDeviceName(name);
    Device::Name = LocalStorageFactory::getInstance().getDeviceName();
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
    LocalStorageFactory::getInstance().setDistributorConstruct(distributorID, data);
}

void SysExMsgHandler::localStorageClearDistributors()
{
    // Handled by DistributorManager
}

void SysExMsgHandler::localStorageReset()
{
    LocalStorageFactory::getInstance().resetDeviceConfig();
}

#endif