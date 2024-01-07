/* 
 * MessgaeHandler.h
 *
 * The Message Handler Recives Midi messages and routes 
 * them to the appropiate distributor by MIDI channel.
 */

#pragma once

#include <vector>
#include <cstdint>
using std::int8_t;

#include "Instruments/InstrumentController.h"
#include "Networks/Network.h"
#include "Distributor.h"
#include "MidiMessage.h"
#include "Device.h"

#include "Extras/LocalStorage.h"

class Network;

/* A Class to decode incoming data into MIDI events */
class MessageHandler{
private:
    
    std::vector<Distributor> m_distributors;

    InstrumentController* m_ptrInstrumentController;
    Network* m_ptrNetwork;
 
public:
    //Set Network Connection
    void setNetwork(Network* ptrNetwork);

    //Message Handler 
    explicit MessageHandler(InstrumentController* ptrInstrumentController);
    void processMessage(MidiMessage& message);

    //Distributors
    void addDistributor(); // Internal Function
    void addDistributor(Distributor distributor); // Internal Function
    void addDistributor(uint8_t data[]); // Internal Function
    void setDistributor(uint8_t data[]); // Set/Add Distributor
    void removeDistributor(uint8_t id);
    void removeAllDistributors();
    Distributor& getDistributor(uint8_t id);
    std::array<uint8_t,DISTRIBUTOR_NUM_CFG_BYTES> getDistributorSerial(uint8_t id);
    
private:
    //Main Midi Functions
    void distributeMessage(MidiMessage& message);
    void processCC(MidiMessage& message);
    void processSysEX(MidiMessage& message);

    //SysEx functions
    void sysExDeviceReady(MidiMessage& message);
    void sysExResetDeviceConfig(MidiMessage& message);

    void sysExGetDeviceConstructWithDistributors(MidiMessage& message);
    void sysExGetDeviceConstruct(MidiMessage& message);
    void sysExGetDeviceName(MidiMessage& message);
    void sysExGetDeviceBoolean(MidiMessage& message);

    void sysExSetDeviceConstructWithDistributors(MidiMessage& message);
    void sysExSetDeviceConstruct(MidiMessage& message);
    void sysExSetDeviceName(MidiMessage& message);
    void sysExSetDeviceBoolean(MidiMessage& message);

    // void sysExRemoveDistributor(MidiMessage& message); //Already a Dedicated Function
    // void sysExRemoveAllDistributors(MidiMessage& message); //Already a Dedicated Function
    void sysExGetNumOfDistributors(MidiMessage& message);
    void sysExGetAllDistributors(MidiMessage& message);
    // void sysExAddDistributor(MidiMessage& message); //SetDistributor Provides this Function
    void sysExToggleMuteDistributor(MidiMessage& message);
    
    void sysExGetDistributorConstruct(MidiMessage& message);
    void sysExGetDistributorChannels(MidiMessage& message);
    void sysExGetDistributorInstruments(MidiMessage& message);
    void sysExGetDistributorMethod(MidiMessage& message);
    void sysExGetDistributorBoolValues(MidiMessage& message);
    void sysExGetDistributorMinMaxNotes(MidiMessage& message);
    void sysExGetDistributorNumPolyphonicNotes(MidiMessage& message);

    // void sysExSetDistributor(MidiMessage& message); //Already a Dedicated Function
    void sysExSetDistributorChannels(MidiMessage& message);
    void sysExSetDistributorInstruments(MidiMessage& message);
    void sysExSetDistributorMethod(MidiMessage& message);
    void sysExSetDistributorBoolValues(MidiMessage& message);
    void sysExSetDistributorMinMaxNotes(MidiMessage& message);
    void sysExSetDistributorNumPolyphonicNotes(MidiMessage& message);

    //Local Storage
    void localStorageInit();
    void localStorageSetDeviceName(char* name);
    void localStorageAddDistributor();
    void localStorageRemoveDistributor(uint8_t id);
    void localStorageUpdateDistributor(uint16_t distributorID, const uint8_t* data);
    void localStorageClearDistributors();
    void localStorageReset();
 
};