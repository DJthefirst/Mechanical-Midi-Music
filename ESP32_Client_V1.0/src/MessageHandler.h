/* 
 * MessgaeHandler.h
 *
 * The Message Handler Recives Midi messages and routes 
 * them to the appropiate distributor by MIDI channel.
 */

#pragma once

#include "Device.h"
#include "Extras/LocalStorage.h"
#include "Instruments/InstrumentController.h"
#include "Networks/INetwork.h"
#include "Distributor.h"
#include "MidiMessage.h"
#include "Constants.h"

#include <array>
#include <vector>
#include <cstdint>
#include <optional>
using std::int8_t;
using std::int16_t;

class Network;

/* A Class to decode incoming data into MIDI events */
class MessageHandler{
private:
    
    std::vector<Distributor> m_distributors;
    uint16_t m_src = Device::GetDeviceID();
    uint16_t m_dest;

    InstrumentController* m_ptrInstrumentController;
 
    
public:
    //Message Handler 
    explicit MessageHandler(InstrumentController* ptrInstrumentController);
    std::optional<MidiMessage> processMessage(MidiMessage& message);

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
    std::optional<MidiMessage> processSysEX(MidiMessage& message);

    //SysEx functions
    MidiMessage sysExDeviceReady(MidiMessage& message);
    void sysExResetDeviceConfig(MidiMessage& message);
    MidiMessage sysExDiscoverDevices(MidiMessage& message);

    MidiMessage sysExGetDeviceConstructWithDistributors(MidiMessage& message);
    MidiMessage sysExGetDeviceConstruct(MidiMessage& message);
    MidiMessage sysExGetDeviceID(MidiMessage& message);
    MidiMessage sysExGetDeviceName(MidiMessage& message);
    MidiMessage sysExGetDeviceBoolean(MidiMessage& message);

    void sysExSetDeviceConstructWithDistributors(MidiMessage& message);
    void sysExSetDeviceConstruct(MidiMessage& message);
    void sysExSetDeviceID(MidiMessage& message);
    void sysExSetDeviceName(MidiMessage& message);
    void sysExSetDeviceBoolean(MidiMessage& message);

    // void sysExRemoveDistributor(MidiMessage& message); //Already a Dedicated Function
    // void sysExRemoveAllDistributors(MidiMessage& message); //Already a Dedicated Function
    MidiMessage sysExGetNumOfDistributors(MidiMessage& message);
    MidiMessage sysExGetAllDistributors(MidiMessage& message);
    // void sysExAddDistributor(MidiMessage& message); //SetDistributor Provides this Function
    MidiMessage sysExToggleMuteDistributor(MidiMessage& message);
    
    MidiMessage sysExGetDistributorConstruct(MidiMessage& message);
    MidiMessage sysExGetDistributorChannels(MidiMessage& message);
    MidiMessage sysExGetDistributorInstruments(MidiMessage& message);
    MidiMessage sysExGetDistributorMethod(MidiMessage& message);
    MidiMessage sysExGetDistributorBoolValues(MidiMessage& message);
    MidiMessage sysExGetDistributorMinMaxNotes(MidiMessage& message);
    MidiMessage sysExGetDistributorNumPolyphonicNotes(MidiMessage& message);

    // void sysExSetDistributor(MidiMessage& message); //Already a Dedicated Function
    void sysExSetDistributorChannels(MidiMessage& message);
    void sysExSetDistributorInstruments(MidiMessage& message);
    void sysExSetDistributorMethod(MidiMessage& message);
    void sysExSetDistributorBoolValues(MidiMessage& message);
    void sysExSetDistributorMinMaxNotes(MidiMessage& message);
    void sysExSetDistributorNumPolyphonicNotes(MidiMessage& message);

    //Local Storage
    void localStorageSetDeviceName(char* name);
    void localStorageAddDistributor();
    void localStorageRemoveDistributor(uint8_t id);
    void localStorageUpdateDistributor(uint16_t distributorID, const uint8_t* data);
    void localStorageClearDistributors();
    void localStorageReset();
 
};