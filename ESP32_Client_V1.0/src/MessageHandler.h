/* 
 * MessageHandler.h
 *
 * The Message Handler receives MIDI messages and coordinates with specialized handlers.
 * Refactored to use composition with DistributorManager, SysExMsgHandler, and MidiMsgHandler.
 */

#pragma once

#include "Device.h"
#include "Instruments/InstrumentController.h"
#include "MidiMessage.h"
#include "Constants.h"
#include "DistributorManager.h"
#include "SysExMsgHandler.h"
#include "MidiMsgHandler.h"

#include <cstdint>
#include <optional>
#include <memory>

/* A Class to decode incoming data into MIDI events */
class MessageHandler{
private:
    
    std::shared_ptr<DistributorManager> m_distributorManager;
    std::shared_ptr<SysExMsgHandler> m_sysExHandler;
    std::shared_ptr<MidiMsgHandler> m_midiHandler;
    
    InstrumentController* m_ptrInstrumentController;
 
    
public:
    //Message Handler 
    explicit MessageHandler(InstrumentController* ptrInstrumentController);
    std::optional<MidiMessage> processMessage(MidiMessage& message);

    //Distributors - delegated to DistributorManager
    void addDistributor(); 
    void addDistributor(Distributor distributor); 
    void addDistributor(uint8_t data[]); 
    void setDistributor(uint8_t data[]); // Set/Add Distributor
    void removeDistributor(uint8_t id);
    void removeAllDistributors();
    Distributor& getDistributor(uint8_t id);
    std::array<uint8_t,DISTRIBUTOR_NUM_CFG_BYTES> getDistributorSerial(uint8_t id);
    
    // Access to component managers
    std::shared_ptr<DistributorManager> getDistributorManager() const { return m_distributorManager; }
    std::shared_ptr<SysExMsgHandler> getSysExHandler() const { return m_sysExHandler; }
    std::shared_ptr<MidiMsgHandler> getMidiHandler() const { return m_midiHandler; }

private:
    // Initialization helper
    void initializeComponents();
 
};