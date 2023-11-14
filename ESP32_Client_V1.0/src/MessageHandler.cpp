/* 
 * MessgaeHandler.cpp
 *
 * The Message Handler Recives Midi messages and routes 
 * them to the appropiate distributor by MIDI channel.
 * 
 */

#include "MessageHandler.h"
#include <Arduino.h>

MessageHandler::MessageHandler(InstrumentController* ptrInstrumentController){
    m_ptrInstrumentController = ptrInstrumentController;
    localStorageInit();
}

void MessageHandler::setNetwork(Network* ptrNetwork){
    m_ptrNetwork = ptrNetwork;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//Process Messages
////////////////////////////////////////////////////////////////////////////////////////////////////

void MessageHandler::processMessage(MidiMessage message)
{   
    //Handle System Common Msg or Send to Distributors
    if(message.type() == MIDI_SysCommon)
    {
        //message.type() represents the message type for SysCommon msg
        switch(message.sysCommonType()){

        case(MIDI_SysEX):
            processSysEX(message);
            break;

        case(MIDI_SysStop):
            (*m_ptrInstrumentController).stopAll();
            break;

        case(MIDI_SysReset):
            (*m_ptrInstrumentController).resetAll();
            break;

        default:
            break;
        }
    }
    else if(message.type() == MIDI_ControlChange){
        processCC(message);
    }
    else{
        distributeMessage(message);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//Process SysEX Messages (Used For Device Config)
////////////////////////////////////////////////////////////////////////////////////////////////////

void MessageHandler::processSysEX(MidiMessage message)
{
    //Check MIDI ID
    if(message.sysExID() != SYSEX_ID) return;
    //Check Device ID or Global ID 0x00;
    if(message.deviceID() != SYSEX_DEV_ID && message.deviceID() != 0x00) return;

    switch(message.sysExCommand()){
        
        case (SYSEX_ResetDeviceConfig):
            sysExResetDeviceConfig(message);
            break;
        case (SYSEX_GetDeviceConstruct):
            sysExGetDeviceConstruct(message);
            break;
        case (SYSEX_GetDeviceName):
            sysExGetDeviceName(message);
            break;
        case (SYSEX_GetDeviceBoolean):
            sysExGetDeviceBoolean(message);
            break;
        case (SYSEX_SetDeviceConstruct):
            sysExSetDeviceConstruct(message);
            break;
        case (SYSEX_SetDeviceName):
            sysExSetDeviceName(message);
            break;
        case (SYSEX_SetDeviceBoolean):
            sysExSetDeviceBoolean(message);
            break;
        case (SYSEX_SetDistributor):
            setDistributor(message.sysExCmdOffset());
            break;
        case (SYSEX_RemoveDistributor):
            removeDistributor(message.sysExDistributorID());
            break;
        case (SYSEX_RemoveAllDistributors):
            removeAllDistributors();
            break;
        case (SYSEX_GetNumOfDistributors):
            sysExGetNumOfDistributors(message);
            break;
        case (SYSEX_GetDistributorConstruct):
            sysExGetDistributorConstruct(message);
            break;
        case (SYSEX_SetDistributorChannels):
            sysExSetDistributorChannels(message);
            break;
        case (SYSEX_SetDistributorInstruments):
            sysExSetDistributorInstruments(message);
            break;
        case (SYSEX_SetDistributorMethod):
            sysExSetDistributorMethod(message);
            break;
        case (SYSEX_SetDistributorBoolValues):
            sysExSetDistributorBoolValues(message);
            break;
        case (SYSEX_SetDistributorMinMaxNotes):
            sysExSetDistributorMinMaxNotes(message);
            break;
        case (SYSEX_SetDistributorNumPolyphonicNotes):
            sysExSetDistributorNumPolyphonicNotes(message);
            break;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//Process CC MIDI Messages (Used For the InstrumentController)
////////////////////////////////////////////////////////////////////////////////////////////////////

void MessageHandler::processCC(MidiMessage message)
{
    for(uint8_t i=0; i < m_distributors.size(); i++)
    {   
        if((m_distributors[i].getChannels() & (1 << message.channel())) != (0)){
            switch(message.CC_Control()){

            case(MIDI_CC_ModulationWheel):
                (*m_ptrInstrumentController).setModulationWheel(message.CC_Value());
                break;
            case(MIDI_CC_FootPedal):
                (*m_ptrInstrumentController).setFootPedal(message.CC_Value());
                break;
            case(MIDI_CC_Volume):
                (*m_ptrInstrumentController).setVolume(message.CC_Value());
                break;
            case(MIDI_CC_Expression):
                (*m_ptrInstrumentController).setExpression(message.CC_Value());
                break;
            case(MIDI_CC_EffectCrtl_1):
                (*m_ptrInstrumentController).setEffectCrtl_1(message.CC_Value());
                break;
            case(MIDI_CC_EffectCrtl_2):
                (*m_ptrInstrumentController).setEffectCrtl_2(message.CC_Value());
                break;
            case(MIDI_CC_DamperPedal):
                m_distributors[i].setDamperPedal(message.CC_Value()< 64);
                break;
            case(MIDI_CC_Mute):
                (*m_ptrInstrumentController).stopAll();
                break;
            case(MIDI_CC_Reset):
                (*m_ptrInstrumentController).resetAll();
                break;
            case(MIDI_CC_AllNotesOff):
                (*m_ptrInstrumentController).stopAll();
                break;
            case(MIDI_CC_OmniModeOff):
                (*m_ptrInstrumentController).stopAll();
                //m_OmniMode = false;
                break;
            case(MIDI_CC_OmniModeOn):
                (*m_ptrInstrumentController).stopAll();
                //m_OmniMode = true;
                break;
            case(MIDI_CC_Monophonic):
                m_distributors[i].setPolyphonic(false);
                (*m_ptrInstrumentController).stopAll();
                break;
            case(MIDI_CC_Polyphonic):
                (*m_ptrInstrumentController).stopAll();
                m_distributors[i].setPolyphonic(true);
                break;
            }
            return;
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//Distribute MIDI Messages 
////////////////////////////////////////////////////////////////////////////////////////////////////

void MessageHandler::distributeMessage(MidiMessage message)
{  
    for(uint8_t i=0; i < m_distributors.size(); i++)
    {   
        if((m_distributors[i].getChannels() & (1 << message.channel())) != (0))
            m_distributors[i].processMessage(message);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//Handle SysEx MIDI Messages (Used For Device Config)
////////////////////////////////////////////////////////////////////////////////////////////////////

void MessageHandler::sysExResetDeviceConfig(MidiMessage message){
    m_distributors.clear();
}

void MessageHandler::sysExGetDeviceConstruct(MidiMessage message){
    std::array<std::uint8_t,NUM_DEVICE_CFG_BYTES> deviceObj;

    uint8_t deviceBoolByte = 0;
    if(Device::OmniMode) deviceBoolByte |= (1 << 0);

    deviceObj[0] = static_cast<uint8_t>((SYSEX_DEV_ID >> 7) & 0x7F); //Device ID MSB
    deviceObj[1] = static_cast<uint8_t>((SYSEX_DEV_ID >> 0) & 0x7F); //Device ID LSB
    deviceObj[2] = deviceBoolByte;
    deviceObj[3] = MAX_NUM_INSTRUMENTS;
    deviceObj[4] = INSTRUMENT_TYPE;
    deviceObj[5] = PLATFORM_TYPE;
    deviceObj[6] = MIN_MIDI_NOTE;
    deviceObj[7] = MAX_MIDI_NOTE;
    deviceObj[8] = static_cast<uint8_t>((FIRMWARE_VERSION >> 7) & 0x7F);
    deviceObj[9] = static_cast<uint8_t>((FIRMWARE_VERSION >> 0) & 0x7F);
    
    for(uint8_t i = 0; i < 20; i++){
        if (Device::Name.size() >  i) deviceObj[10+i] = Device::Name[i];
        else deviceObj[10+i] = 0;
    }

    (*m_ptrNetwork).sendMessage(deviceObj.data(),deviceObj.size());
}

void MessageHandler::sysExGetDeviceName(MidiMessage message){
    //(*m_ptrNetwork).sendMessage(Device::Name,20);
}

void MessageHandler::sysExGetDeviceBoolean(MidiMessage message){
    static uint8_t deviceBoolean = 0;
    deviceBoolean += Device::OmniMode ? 0x01 : 0x00;

    (*m_ptrNetwork).sendMessage(&deviceBoolean,1);
}

void MessageHandler::sysExSetDeviceConstruct(MidiMessage message){
    
}

void MessageHandler::sysExSetDeviceName(MidiMessage message){
    char name[20];
    for(uint8_t i=0; i<20; i++) name[i] = message.buffer[i+5];
    localStorageSetDeviceName(name);
}

void MessageHandler::sysExSetDeviceBoolean(MidiMessage message){
    Device::OmniMode = ((message.buffer[5] & 0x01) != 0);
}

void MessageHandler::sysExGetNumOfDistributors(MidiMessage message){
    uint8_t sizeByte = m_distributors.size();
    (*m_ptrNetwork).sendMessage(&sizeByte,1);
}

void MessageHandler::sysExGetDistributorConstruct(MidiMessage message){
    auto distributorBytes = getDistributor(message.sysExDistributorID()).toSerial();
    distributorBytes[0] = message.buffer[5];
    distributorBytes[1] = message.buffer[6];
    (*m_ptrNetwork).sendMessage(distributorBytes.data(),distributorBytes.size());
}

// void MessageHandler::sysExGetDistributorChannels(MidiMessage message){
//     uint16_t channels = getDistributor(message.buffer[6])->getChannels();
//     uint8_t bytesToSend[2] = {static_cast<uint8_t>( channels >> 0), 
//                               static_cast<uint8_t>( channels >> 8) };
//     (*m_ptrNetwork).sendMessage(bytesToSend,2);
// }

// void MessageHandler::sysExGetDistributorInstruments(MidiMessage message){
//     uint32_t instruments = getDistributor(message.buffer[6])->getInstruments();
//     uint8_t bytesToSend[4] = {static_cast<uint8_t>( channels >> 0), 
//                               static_cast<uint8_t>( channels >> 8),
//                               static_cast<uint8_t>( channels >> 16),
//                               static_cast<uint8_t>( channels >> 24)};
//     (*m_ptrNetwork).sendMessage(bytesToSend,4);
//}

void MessageHandler::sysExSetDistributorChannels(MidiMessage message){
    uint16_t channels = ((message.buffer[7] << 14) 
                       & (message.buffer[8] << 7) 
                       & (message.buffer[9] << 0));
    getDistributor(message.sysExDistributorID()).setChannels(channels);
}

void MessageHandler::sysExSetDistributorInstruments(MidiMessage message){
    uint32_t instruments = ((message.buffer[7] << 28) 
                          & (message.buffer[8] << 21) 
                          & (message.buffer[9] << 14) 
                          & (message.buffer[10] << 7) 
                          & (message.buffer[11] << 0));
    getDistributor(message.sysExDistributorID()).setInstruments(instruments);
}

void MessageHandler::sysExSetDistributorMethod(MidiMessage message){
    getDistributor(message.sysExDistributorID()).setDistributionMethod(DistributionMethod(message.buffer[7]));
}

void MessageHandler::sysExSetDistributorBoolValues(MidiMessage message){
    getDistributor(message.sysExDistributorID()).setDamperPedal((message.buffer[7] & 0x01) != 0);
    getDistributor(message.sysExDistributorID()).setPolyphonic((message.buffer[7] & 0x02) != 0);
    getDistributor(message.sysExDistributorID()).setNoteOverwrite((message.buffer[7] & 0x04) != 0);
}

void MessageHandler::sysExSetDistributorMinMaxNotes(MidiMessage message){
    getDistributor(message.sysExDistributorID()).setMinMaxNote(message.buffer[7],message.buffer[8]);
}

void MessageHandler::sysExSetDistributorNumPolyphonicNotes(MidiMessage message){
    getDistributor(message.sysExDistributorID()).setNumPolyphonicNotes(message.buffer[7]);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//Manage Distributors
////////////////////////////////////////////////////////////////////////////////////////////////////

void MessageHandler::addDistributor()
{
    Distributor newDistributor = Distributor(m_ptrInstrumentController);
    m_distributors.push_back(newDistributor);
    localStorageAddDistributor();
}

void MessageHandler::addDistributor(Distributor distributor)
{
    m_distributors.push_back(distributor);
    localStorageAddDistributor();
}

void MessageHandler::addDistributor(uint8_t data[])
{
    Distributor distributor(m_ptrInstrumentController);
    distributor.setDistributor(data);
    m_distributors.push_back(distributor);
    localStorageAddDistributor();
}

void MessageHandler::setDistributor(uint8_t data[])
{
    // If Distributor exists update it.
    uint16_t distributorID = data[0] << 7| (data[1]);
    if (distributorID < m_distributors.size()){
        localStorageUpdateDistributor(distributorID,data);
        m_distributors[distributorID].setDistributor(data);
        return;
    }

    // Else create Distributor and add it to vector
    addDistributor(data);
}

void MessageHandler::removeDistributor(uint8_t id)
{
    (*m_ptrInstrumentController).stopAll(); //Safety Stops all Playing Notes
    if(id >= m_distributors.size()) id = m_distributors.size();
    m_distributors.erase(m_distributors.begin() + id);
    localStorageRemoveDistributor(id);
}

void MessageHandler::removeAllDistributors()
{
    (*m_ptrInstrumentController).stopAll(); //Safety Stops all Playing Notes
    m_distributors.clear();
    localStorageClaerDistributors();
}

Distributor& MessageHandler::getDistributor(uint8_t index){
    return (m_distributors[index]);
}

std::array<uint8_t, NUM_DISTRIBUTOR_CFG_BYTES> MessageHandler::getDistributorSerial(uint8_t index){

    auto distributorObj = m_distributors[index].toSerial();
    distributorObj[0] = static_cast<uint8_t>((index >> 7) & 0x7F);
    distributorObj[1] = static_cast<uint8_t>((index >> 0) & 0x7F);
    return distributorObj;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//Local Storage
////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef LOCAL_STORAGE

//Device Local Storage Solution
LocalStorage localStorage; 

void MessageHandler::localStorageInit(){
    localStorage = LocalStorage();
}

void MessageHandler::localStorageSetDeviceName(char* name){
    localStorage.SetDeviceName(name);
    Device::Name = localStorage.GetDeviceName((uint8_t*)name);
}

void MessageHandler::localStorageAddDistributor(){
    uint8_t index = m_distributors.size()-1;
    localStorage.SetDistributorConstruct(index,getDistributorSerial(index).data());
    localStorage.SetNumOfDistributors(m_distributors.size());
}

void MessageHandler::localStorageRemoveDistributor(uint8_t id){
    localStorage.SetNumOfDistributors(m_distributors.size());

    for(int i = id; i < m_distributors.size(); i++){
        localStorage.SetDistributorConstruct(i,getDistributorSerial(i).data());
    }
}

void MessageHandler::localStorageUpdateDistributor(uint16_t distributorID, uint8_t* data){
    localStorage.SetDistributorConstruct(distributorID,data);
}

void MessageHandler::localStorageClaerDistributors(){
    localStorage.SetNumOfDistributors(m_distributors.size());
}
#else
void MessageHandler::localStorageInit(){}
void MessageHandler::localStorageSetDeviceName(char* name){}
void MessageHandler::localStorageAddDistributor(){}
void MessageHandler::localStorageRemoveDistributor(uint8_t id){}
void MessageHandler::localStorageUpdateDistributor(uint16_t distributorID, uint8_t* data){}
void MessageHandler::localStorageClaerDistributors(){}
#endif