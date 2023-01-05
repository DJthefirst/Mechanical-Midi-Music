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
    void sysExResetDeviceConfig(MidiMessage message);
    void sysExGetDeviceConstruct(MidiMessage message);
    void sysExGetDeviceName(MidiMessage message);
    void sysExGetDeviceBoolean(MidiMessage message);
    void sysExSetDeviceConstruct(MidiMessage message);
    void sysExSetDeviceName(MidiMessage message);
    void sysExSetDeviceBoolean(MidiMessage message);
    void sysExGetNumOfDistributors(MidiMessage message);
    void sysExGetDistributorConstruct(MidiMessage message);
    void sysExSetDistributorChannels(MidiMessage message);
    void sysExSetDistributorInstruments(MidiMessage message);
    void sysExSetDistributorMethod(MidiMessage message);
    void sysExSetDistributorBoolValues(MidiMessage message);
    void sysExSetDistributorMinMaxNotes(MidiMessage message);
    void sysExSetDistributorNumPolyphonicNotes(MidiMessage message);
 
};