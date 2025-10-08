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
    Distributor newDistributor = Distributor(m_ptrInstrumentController);
    m_distributors.push_back(newDistributor);
    localStorageAddDistributor();
    broadcastDistributorChanged();
}

// Add an existing Distributor to the Distribution Pool
void DistributorManager::addDistributor(Distributor distributor)
{
    m_distributors.push_back(distributor);
    localStorageAddDistributor();
    broadcastDistributorChanged();
}

// Create a Distributor from a Construct and add it to the Distribution Pool
void DistributorManager::addDistributor(uint8_t data[])
{
    Distributor distributor(m_ptrInstrumentController);
    distributor.setDistributor(data);
    m_distributors.push_back(distributor);
    localStorageAddDistributor();
    broadcastDistributorChanged();
}

// Updates the designated Distributor in the Distribution Pool
// from a construct or adds a new Distributor
void DistributorManager::setDistributor(uint8_t data[])
{
    // Clear active Notes
    m_ptrInstrumentController->stopAll();

    // If Distributor exists update it.
    uint16_t distributorID = data[0] << 7 | (data[1]);
    if (distributorID < m_distributors.size()) {
        localStorageUpdateDistributor(distributorID, data);
        m_distributors[distributorID].setDistributor(data);
        broadcastDistributorChanged();
        return;
    }

    // Else create Distributor and add it to the Distribution Pool (this will call broadcastDistributorChanged)
    addDistributor(data);
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

////////////////////////////////////////////////////////////////////////////////////////////////////
// Distributor Access
////////////////////////////////////////////////////////////////////////////////////////////////////

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

            case(MIDI_CC_ModulationWheel):
                m_ptrInstrumentController->setModulationWheel(message.CC_Value());
                break;
            case(MIDI_CC_FootPedal):
                m_ptrInstrumentController->setFootPedal(message.CC_Value());
                break;
            case(MIDI_CC_Volume):
                m_ptrInstrumentController->setVolume(message.CC_Value());
                break;
            case(MIDI_CC_Expression):
                m_ptrInstrumentController->setExpression(message.CC_Value());
                break;
            case(MIDI_CC_EffectCrtl_1):
                m_ptrInstrumentController->setEffectCrtl_1(message.CC_Value());
                break;
            case(MIDI_CC_EffectCrtl_2):
                m_ptrInstrumentController->setEffectCrtl_2(message.CC_Value());
                break;
            case(MIDI_CC_DamperPedal):
                m_distributors[i].setDamperPedal(message.CC_Value() > 64); // Above 64 is ON else OFF
                break;
            case(MIDI_CC_Mute):
                m_ptrInstrumentController->stopAll();
                break;
            case(MIDI_CC_Reset):
                m_ptrInstrumentController->resetAll();
                break;
            case(MIDI_CC_AllNotesOff):
                m_ptrInstrumentController->stopAll();
                break;
            case(MIDI_CC_OmniModeOff):
                m_ptrInstrumentController->stopAll();
                //m_OmniMode = false;
                break;
            case(MIDI_CC_OmniModeOn):
                m_ptrInstrumentController->stopAll();
                //m_OmniMode = true;
                break;
            case(MIDI_CC_Monophonic):
                m_distributors[i].setPolyphonic(false);
                m_ptrInstrumentController->stopAll();
                break;
            case(MIDI_CC_Polyphonic):
                m_ptrInstrumentController->stopAll();
                m_distributors[i].setPolyphonic(true);
                break;
            }
            return;
        }
    }
}

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
// Local Storage
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

#endif