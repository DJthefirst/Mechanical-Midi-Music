/* 
 * MessgaeHandler.h
 *
 * The Message Handler Recives Midi messages and routes 
 * them to the appropiate distributor by MIDI channel.
 */

#pragma once

#include <stdint.h>
#include <vector>
#include "Instruments/InstrumentController.h"
#include "Networks/Network.h"
#include "Distributor.h"
#include "MidiMessage.h"

class Network;

/* A Class to decode incoming data into MIDI events */
class MessageHandler{
private:
    //Handler Config
    bool m_OmniMode = false;
    
    std::vector<Distributor> m_distributors;

    InstrumentController* m_ptrInstrumentController;
    Network* m_ptrNetwork;

public:
    //Msg Handler 
    MessageHandler(InstrumentController* ptrInstrumentController);
    void processMessage(MidiMessage message);

    //Distributors
    void addDistributor();
    void addDistributor(uint8_t data[]);
    void addDistributor(Distributor distributor);

    void setNetwork(Network* ptrNetwork);
    void removeDistributor(uint8_t id);
    void removeAllDistributors();
    Distributor* getDistributor(uint8_t id);
    
private:
    //Main Midi Functions
    void distributeMessage(MidiMessage message);
    void processCC(MidiMessage message);
    void processSysEX(MidiMessage message);

    //SysEx functions
    void sysExDistributorAdd(MidiMessage message);
    void sysExDistributorRequest(uint8_t distributorNum);
    void sysExDistributorRequestAll();
    void sysExDistributorSetMode(MidiMessage message);
 
};