/* 
 * MessgaeHandler.h
 *
 * The Message Handler Recives Midi messages and routes them to the appropiate distributor by MIDI 
 * channel.
 * 
 */

#pragma once

#include <stdint.h>
#include <vector>
#include "Instruments/InstrumentController.h"
#include "Networks/Network.h"
#include "Distributor.h"

#define MOPPY_MAX_PACKET_LENGTH 259

/* 
A Class to convert incoming data into MIDI events
*/
class Network;

class MessageHandler{
private:
    //Handler Config
    bool m_OmniMode = false;

    //Msg Data
    uint8_t m_msgType;
    uint8_t m_msgChannel;
    
    std::vector<Distributor> m_distributors;

    InstrumentController* m_ptrInstrumentController;
    Network* m_ptrNetwork;

public:
    //Msg Handler 
    MessageHandler(InstrumentController* ptrInstrumentController);
    void ProcessMessage(uint8_t message[]);

    //Distributors
    void AddDistributor();
    void AddDistributor(uint8_t data[]);
    void AddDistributor(Distributor distributor);

    void SetNetwork(Network* ptrNetwork);
    void RemoveDistributor(uint8_t id);
    void RemoveAllDistributors();
    Distributor* GetDistributor(uint8_t id);
    void SysExDistributorRequest(uint8_t message[]);

private:
    //Main Midi Functions
    void DistributeMessage(uint8_t message[]);
    void ProcessCC(uint8_t message[]);
    void ProcessSysEX(uint8_t message[]);

    //SysEx functions
    void SysExDistributorAdd(uint8_t message[]);
    //void SysExDistributorRequest(uint8_t message[]);
    void SysExDistributorRequestAll(uint8_t message[]);
    void SysExDistributorSetMode(uint8_t message[]);
 
};