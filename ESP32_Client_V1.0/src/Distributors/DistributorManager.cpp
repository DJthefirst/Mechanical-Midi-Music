/*
 * DistributorManager.cpp
 *
 * Manages the collection of distributors and their operations.
 * Extracted from the original MessageHandler to follow single responsibility principle.
 */

#include "DistributorManager.h"
#include <Arduino.h>
#include <algorithm>

// Constructor
DistributorManager::DistributorManager(InstrumentController* ptrInstrumentController)
    : m_ptrInstrumentController(ptrInstrumentController) {}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Distributor Management
////////////////////////////////////////////////////////////////////////////////////////////////////

// Create a default Distributor and add it to the Distribution Pool
void DistributorManager::addDistributor()
{
    m_distributors.emplace_back(m_ptrInstrumentController);
    localStorageAddDistributor();
    broadcastDistributorChanged();
}

// Add an existing Distributor to the Distribution Pool
void DistributorManager::addDistributor(Distributor&& distributor)
{
    m_distributors.push_back(std::move(distributor));
    localStorageAddDistributor();
    broadcastDistributorChanged();
}

// Create a Distributor from a Construct and add it to the Distribution Pool
void DistributorManager::addDistributor(uint8_t data[])
{
    m_distributors.emplace_back(m_ptrInstrumentController);
    m_distributors.back().setDistributor(data);
    localStorageAddDistributor();
    broadcastDistributorChanged();
}

// Updates the designated Distributor in the Distribution Pool
// from a construct or adds a new Distributor
void DistributorManager::setDistributor(uint8_t data[])
{
    // Clear active Notes
    m_ptrInstrumentController->stopAll();
    
    // Decode distributor ID from the first two bytes
    uint16_t distributorID = (static_cast<uint16_t>(data[0]) << 7) | static_cast<uint16_t>(data[1]);
    
    // If distributor ID is beyond current range, add new distributors
    while (distributorID >= m_distributors.size()) {
        addDistributor();
    }
    
    // Update the specified distributor
    m_distributors[distributorID].setDistributor(data);
    localStorageUpdateDistributor(distributorID, data);
    broadcastDistributorChanged();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Timeout Management
////////////////////////////////////////////////////////////////////////////////////////////////////

void DistributorManager::checkInstrumentTimeouts()
{
    // Delegate timeout checking to the instrument controller
    if (m_ptrInstrumentController) {
        m_ptrInstrumentController->checkInstrumentTimeouts();
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Message Processing
////////////////////////////////////////////////////////////////////////////////////////////////////

// Send message to all distributors which accept the designated message's channel.
void DistributorManager::distributeMessage(MidiMessage& message)
{
    // Pre-calculate channel mask to avoid repeated bit operations
    const uint16_t channelMask = (1 << message.channel());
    
    for (uint8_t i = 0; i < m_distributors.size(); i++) {
        if ((m_distributors[i].getChannels() & channelMask) != 0) {
            m_distributors[i].processMessage(message);
        }
    }
}

// Process CC MIDI Messages by type
void DistributorManager::processCC(MidiMessage& message)
{
    for (uint8_t i = 0; i < m_distributors.size(); i++) {
        if ((m_distributors[i].getChannels() & (1 << message.channel())) != (0)) {
            switch (message.CC_Control()) {
                case(MidiCC::DamperPedal):
                    m_distributors[i].setDamperPedal(message.CC_Value() > 64);
                    break;
                case(MidiCC::Mute):
                    m_ptrInstrumentController->stopAll();
                    break;
                case(MidiCC::AllNotesOff):
                    m_ptrInstrumentController->stopAll();
                    break;
                case(MidiCC::Monophonic):
                    m_distributors[i].setPolyphonic(false);
                    m_ptrInstrumentController->stopAll();
                    break;
                case(MidiCC::Polyphonic):
                    m_ptrInstrumentController->stopAll();
                    m_distributors[i].setPolyphonic(true);
                    break;
                default:
                    break;
            }
            return;
        }
    }
}

// Removes the designated Distributor from the Distribution Pool
void DistributorManager::removeDistributor(uint8_t id)
{
    m_ptrInstrumentController->stopAll(); // Safety Stops all Playing Notes
    if (m_distributors.size() == 0) return;
    if (id >= m_distributors.size()) id = m_distributors.size();
    m_distributors.erase(m_distributors.begin() + id);
    localStorageRemoveDistributor(id);
    broadcastDistributorChanged();
}

// Clear Distribution Pool
void DistributorManager::removeAllDistributors()
{
    m_ptrInstrumentController->stopAll(); // Safety Stops all Playing Notes
    m_distributors.clear();
    localStorageClearDistributors();
    broadcastDistributorChanged();
}

// Returns the indexed Distributor from the Distribution Pool
Distributor& DistributorManager::getDistributor(uint8_t index)
{
    if (index >= m_distributors.size()) index = m_distributors.size() - 1;
    return (m_distributors[index]);
}

// Returns indexed Distributor Construct
std::array<uint8_t, DISTRIBUTOR_NUM_CFG_BYTES> DistributorManager::getDistributorSerial(uint8_t index)
{
    // Append Distributor ID to the Construct
    auto distributorObj = m_distributors[index].toSerial();
    distributorObj[0] = static_cast<uint8_t>((index >> 7) & 0x7F);
    distributorObj[1] = static_cast<uint8_t>((index >> 0) & 0x7F);
    return distributorObj;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Distributor Query Helpers
////////////////////////////////////////////////////////////////////////////////////////////////////

uint16_t DistributorManager::getDistributorChannels(uint8_t distributorId)
{
    if (distributorId < m_distributors.size()) {
        return m_distributors[distributorId].getChannels();
    }
    return 0;
}

uint32_t DistributorManager::getDistributorInstruments(uint8_t distributorId)
{
    if (distributorId < m_distributors.size()) {
        return m_distributors[distributorId].getInstruments();
    }
    return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Local Storage Implementation
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef EXTRA_LOCAL_STORAGE

void DistributorManager::localStorageAddDistributor()
{
    uint8_t index = m_distributors.size() - 1;
    LocalStorage::get().SetDistributorConstruct(index, getDistributorSerial(index).data());
    LocalStorage::get().SetNumOfDistributors(m_distributors.size());
}

void DistributorManager::localStorageRemoveDistributor(uint8_t id)
{
    LocalStorage::get().SetNumOfDistributors(m_distributors.size());

    for (int i = id; i < m_distributors.size(); i++) {
        LocalStorage::get().SetDistributorConstruct(i, getDistributorSerial(i).data());
    }
}

void DistributorManager::localStorageUpdateDistributor(uint16_t distributorID, const uint8_t* data)
{
    LocalStorage::get().SetDistributorConstruct(distributorID, data);
}

void DistributorManager::localStorageClearDistributors()
{
    LocalStorage::get().SetNumOfDistributors(m_distributors.size());
}

#else

void DistributorManager::localStorageAddDistributor()
{
    // No-op when local storage is disabled
}

void DistributorManager::localStorageRemoveDistributor(uint8_t id)
{
    // No-op when local storage is disabled  
}

void DistributorManager::localStorageUpdateDistributor(uint16_t distributorID, const uint8_t* data)
{
    // No-op when local storage is disabled
}

void DistributorManager::localStorageClearDistributors()
{
    // No-op when local storage is disabled
}

#endif

////////////////////////////////////////////////////////////////////////////////////////////////////
// Distributor Configuration Helpers
////////////////////////////////////////////////////////////////////////////////////////////////////

void DistributorManager::setDistributorChannels(uint8_t distributorId, uint16_t channels)
{
    if (distributorId < m_distributors.size()) {
        m_distributors[distributorId].setChannels(channels);
        broadcastDistributorChanged();
    }
}

void DistributorManager::setDistributorInstruments(uint8_t distributorId, uint32_t instruments)
{
    if (distributorId < m_distributors.size()) {
        m_distributors[distributorId].setInstruments(instruments);
        broadcastDistributorChanged();
    }
}

void DistributorManager::setDistributorMethod(uint8_t distributorId, DistributionMethod method)
{
    if (distributorId < m_distributors.size()) {
        m_distributors[distributorId].setDistributionMethod(method);
        broadcastDistributorChanged();
    }
}

void DistributorManager::setDistributorBoolValues(uint8_t distributorId, uint8_t boolValues)
{
    if (distributorId < m_distributors.size()) {
        m_distributors[distributorId].setMuted((boolValues & DISTRIBUTOR_BOOL_MUTED) != 0);
        m_distributors[distributorId].setDamperPedal((boolValues & DISTRIBUTOR_BOOL_DAMPERPEDAL) != 0);
        m_distributors[distributorId].setPolyphonic((boolValues & DISTRIBUTOR_BOOL_POLYPHONIC) != 0);
        m_distributors[distributorId].setNoteOverwrite((boolValues & DISTRIBUTOR_BOOL_NOTEOVERWRITE) != 0);
        broadcastDistributorChanged();
    }
}

void DistributorManager::setDistributorMinMaxNotes(uint8_t distributorId, uint8_t minNote, uint8_t maxNote)
{
    if (distributorId < m_distributors.size()) {
        m_distributors[distributorId].setMinMaxNote(minNote, maxNote);
        broadcastDistributorChanged();
    }
}

void DistributorManager::setDistributorNumPolyphonicNotes(uint8_t distributorId, uint8_t numNotes)
{
    if (distributorId < m_distributors.size()) {
        m_distributors[distributorId].setNumPolyphonicNotes(numNotes);
        broadcastDistributorChanged();
    }
}

void DistributorManager::toggleDistributorMute(uint8_t distributorId)
{
    if (distributorId < m_distributors.size()) {
        m_distributors[distributorId].toggleMuted();
        m_ptrInstrumentController->stopAll();
        broadcastDistributorChanged();
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Additional Getter Methods
////////////////////////////////////////////////////////////////////////////////////////////////////

DistributionMethod DistributorManager::getDistributorMethod(uint8_t distributorId)
{
    if (distributorId < m_distributors.size()) {
        return m_distributors[distributorId].getDistributionMethod();
    }
    return DistributionMethod::RoundRobinBalance; // Default fallback
}

uint8_t DistributorManager::getDistributorBoolValues(uint8_t distributorId)
{
    if (distributorId < m_distributors.size()) {
        // Reconstruct bool values byte from distributor state
        auto distributorSerial = m_distributors[distributorId].toSerial();
        return distributorSerial[11]; // Boolean values are stored in byte 11
    }
    return 0;
}

uint8_t DistributorManager::getDistributorMinNote(uint8_t distributorId)
{
    if (distributorId < m_distributors.size()) {
        auto distributorSerial = m_distributors[distributorId].toSerial();
        return distributorSerial[12]; // Min note is stored in byte 12
    }
    return 0;
}

uint8_t DistributorManager::getDistributorMaxNote(uint8_t distributorId)
{
    if (distributorId < m_distributors.size()) {
        auto distributorSerial = m_distributors[distributorId].toSerial();
        return distributorSerial[13]; // Max note is stored in byte 13
    }
    return 127;
}

uint8_t DistributorManager::getDistributorNumPolyphonicNotes(uint8_t distributorId)
{
    if (distributorId < m_distributors.size()) {
        auto distributorSerial = m_distributors[distributorId].toSerial();
        return distributorSerial[14]; // Num polyphonic notes is stored in byte 14
    }
    return 1;
}
