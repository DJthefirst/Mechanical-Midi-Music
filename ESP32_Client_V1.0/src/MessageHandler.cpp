/* 
 * MessageHandler.cpp
 *
 * The Message Handler receives MIDI messages and coordinates with specialized handlers.
 * Refactored to use composition with DistributorManager, SysExMsgHandler, and MidiMsgHandler.
 */

#include "MessageHandler.h"
#include <Arduino.h>

//Message Handler constructor used to pass in a ptr to the instrument controller.
MessageHandler::MessageHandler(InstrumentController* ptrInstrumentController) 
    : m_ptrInstrumentController(ptrInstrumentController)
{
    initializeComponents();
}

// Initialize the component managers
void MessageHandler::initializeComponents()
{
    // Create the distributor manager
    m_distributorManager = std::make_shared<DistributorManager>(m_ptrInstrumentController);
    
    // Create the SysEx handler with distributor manager dependency
    m_sysExHandler = std::make_shared<SysExMsgHandler>(m_distributorManager);
    
    // Create the MIDI handler with both dependencies
    m_midiHandler = std::make_shared<MidiMsgHandler>(m_ptrInstrumentController, m_distributorManager, m_sysExHandler);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//Process Messages
////////////////////////////////////////////////////////////////////////////////////////////////////

// Consumes a MidiMessage and delegates to the appropriate handler
std::optional<MidiMessage> MessageHandler::processMessage(MidiMessage& message)
{   
    // Delegate to the MIDI message handler
    if (m_midiHandler) {
        return m_midiHandler->processMessage(message);
    }
    return {};
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//Distributor Management - Delegate to DistributorManager
////////////////////////////////////////////////////////////////////////////////////////////////////

// Create a default Distributor and add it to the Distribution Pool
void MessageHandler::addDistributor()
{
    if (m_distributorManager) {
        m_distributorManager->addDistributor();
    }
}

// Add an existing Distributor to the Distribution Pool
void MessageHandler::addDistributor(Distributor distributor)
{
    if (m_distributorManager) {
        m_distributorManager->addDistributor(distributor);
    }
}

// Create a Distributor from a Construct and add it to the Distribution Pool
void MessageHandler::addDistributor(uint8_t data[])
{
    if (m_distributorManager) {
        m_distributorManager->addDistributor(data);
    }
}

// Updates the designated Distributor in the Distribution Pool
// from a construct or adds a new Distributor
void MessageHandler::setDistributor(uint8_t data[])
{
    if (m_distributorManager) {
        m_distributorManager->setDistributor(data);
    }
}

// Removes the designated Distributor from the Distribution Pool
void MessageHandler::removeDistributor(uint8_t id)
{
    if (m_distributorManager) {
        m_distributorManager->removeDistributor(id);
    }
}

// Clear Distribution Pool
void MessageHandler::removeAllDistributors()
{
    if (m_distributorManager) {
        m_distributorManager->removeAllDistributors();
    }
}

// Returns the indexed Distributor from the Distribution Pool
Distributor& MessageHandler::getDistributor(uint8_t index)
{
    if (m_distributorManager) {
        return m_distributorManager->getDistributor(index);
    }
    // This is a fallback that should never be reached in normal operation
    static Distributor dummy(m_ptrInstrumentController);
    return dummy;
}

// Returns indexed Distributor Construct
std::array<uint8_t, DISTRIBUTOR_NUM_CFG_BYTES> MessageHandler::getDistributorSerial(uint8_t index)
{
    if (m_distributorManager) {
        return m_distributorManager->getDistributorSerial(index);
    }
    // Return empty array if no manager available
    std::array<uint8_t, DISTRIBUTOR_NUM_CFG_BYTES> empty = {};
    return empty;
}