/*
 * DistributorManager.cpp
 *
 * Manages the collection of distributors and their operations.
 * Extracted from the original MessageHandler to follow single responsibility principle.
 */

#include "DistributorManager.h"
#include "../Instruments/InstrumentController.h"
#include <Arduino.h>
#include <algorithm>

// Private constructor
DistributorManager::DistributorManager(std::shared_ptr<InstrumentControllerBase> instrumentController)
    : m_ptrInstrumentController(instrumentController) {
}

// Singleton instance getter
std::shared_ptr<DistributorManager> DistributorManager::getInstance(std::shared_ptr<InstrumentControllerBase> instrumentController) {
    static std::shared_ptr<DistributorManager> instance = nullptr;
    if (!instance) {
        if (!instrumentController) {
            // Error: First call must provide instrumentController
            return nullptr;
        }
        instance = std::shared_ptr<DistributorManager>(new DistributorManager(instrumentController));
    }
    return instance;
}

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
// Message Processing
////////////////////////////////////////////////////////////////////////////////////////////////////

// Send message to all distributors which accept the designated message's channel.
void DistributorManager::distributeMessage(MidiMessage& message)
{
    for (uint8_t i = 0; i < m_distributors.size(); i++) {
        if (m_distributors[i].getChannels().test(message.channel())) {
            m_distributors[i].processMessage(message);
        }
    }
}

// Process CC MIDI Messages by type
void DistributorManager::processCC(MidiMessage& message)
{
    for (uint8_t i = 0; i < m_distributors.size(); i++) {
        if (m_distributors[i].getChannels().test(message.channel())) {
            switch (message.CC_Control()) {
                case(MidiCC::ModulationWheel):
                    m_ptrInstrumentController->setModulationWheel(message.channel(), message.CC_Value());
                    break;
                case(MidiCC::DamperPedal):
                    m_distributors[i].setDamperPedal(message.CC_Value() > 64);
                    break;
                case(MidiCC::Mute):
                    m_distributors[i].stopActiveNotes();
                    break;
                case(MidiCC::AllNotesOff):
                    m_ptrInstrumentController->stopAll();
                    break;
                case(MidiCC::Monophonic):
                    m_distributors[i].setPolyphonic(false);
                    break;
                case(MidiCC::Polyphonic):
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

std::bitset<NUM_Channels> DistributorManager::getDistributorChannels(uint8_t distributorId)
{
    if (distributorId < m_distributors.size()) {
        return m_distributors[distributorId].getChannels();
    }
    return 0;
}

std::bitset<NUM_Instruments> DistributorManager::getDistributorInstruments(uint8_t distributorId)
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

void DistributorManager::setDistributorChannels(uint8_t distributorId, std::bitset<NUM_Channels> channels)
{
    if (distributorId < m_distributors.size()) {
        m_distributors[distributorId].setChannels(channels);
        broadcastDistributorChanged();
    }
}

void DistributorManager::setDistributorInstruments(uint8_t distributorId, std::bitset<NUM_Instruments> instruments)
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

void DistributorManager::setDistributorBoolValues(uint8_t distributorId, uint16_t boolValues)
{
    if (distributorId < m_distributors.size()) {
        m_distributors[distributorId].setDistributorBoolValues(boolValues);
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

uint16_t DistributorManager::getDistributorBoolValues(uint8_t distributorId)
{
    if (distributorId < m_distributors.size()) {
        return m_distributors[distributorId].getDistributorBoolValues();
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
