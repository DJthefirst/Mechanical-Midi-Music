/* 
 * MessgaeHandler.cpp
 *
 * The Message Handler Recives Midi messages and routes 
 * them to the appropiate distributor by MIDI channel.
 * 
 */

#include "MessageHandler.h"
#include <Arduino.h>

//Message Handler constructor used to pass in a ptr to the instrument controller.
MessageHandler::MessageHandler(InstrumentController* ptrInstrumentController){
    m_ptrInstrumentController = ptrInstrumentController;
    localStorageInit();
}

//Set the ptr to the Network.
void MessageHandler::setNetwork(Network* ptrNetwork){
    m_ptrNetwork = ptrNetwork;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//Process Messages
////////////////////////////////////////////////////////////////////////////////////////////////////

//Consumes a MidiMessage and Handles the message based on its type.
void MessageHandler::processMessage(MidiMessage& message)
{   
    //Handle System Common Msg or Send to Distributors
    if(message.type() == MIDI_SysCommon){

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

//Process SysEX Messages by type
void MessageHandler::processSysEX(MidiMessage& message)
{
    //Check MIDI ID
    if(message.sysExID() != SYSEX_ID) return;
    //Check Device ID or Global ID 0x00;
    if(message.deviceID() != SYSEX_DEV_ID && message.deviceID() != 0x00) return;

    switch(message.sysExCommand()){
        
        case (SYSEX_DeviceReady):
            sysExDeviceReady(message);
            break;
        case (SYSEX_ResetDeviceConfig):
            sysExResetDeviceConfig(message);
            break;
        case (SYSEX_GetDeviceConstructWithDistributors):
            //TODO
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
        case (SYSEX_SetDeviceConstructWithDistributors):
            //TODO
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
        case (SYSEX_RemoveDistributor):
            removeDistributor(message.sysExDistributorID());
            break;
        case (SYSEX_RemoveAllDistributors):
            removeAllDistributors();
            break;
        case (SYSEX_GetNumOfDistributors):
            sysExGetNumOfDistributors(message);
            break;
        case (SYSEX_GetAllDistributors):
            sysExGetNumOfDistributors(message);
            break;
        case (SYSEX_AddDistributor):
            setDistributor(message.sysExCmdPayload());
            break;
        case (SYSEX_ToggleMuteDistributor):
            //TODO
            break;
        case (SYSEX_GetDistributorConstruct):
            sysExGetDistributorConstruct(message);
            break;
        case (SYSEX_GetDistributorChannels):
            sysExGetDistributorChannels(message);
            break;
        case (SYSEX_GetDistributorInstruments):
            //TODO
            break;
        case (SYSEX_GetDistributorMethod):
            //TODO
            break;
        case (SYSEX_GetDistributorBoolValues):
            //TODO
            break;
        case (SYSEX_GetDistributorMinMaxNotes):
            //TODO
            break;
        case (SYSEX_GetDistributorNumPolyphonicNotes):
            //TODO
            break;
        case (SYSEX_SetDistributor):
            setDistributor(message.sysExCmdPayload());
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

//Process CC MIDI Messages by type
void MessageHandler::processCC(MidiMessage& message)
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
                m_distributors[i].setDamperPedal(message.CC_Value()> 64);
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

//Send messgae to all distributors which accept the designated message's channel.
void MessageHandler::distributeMessage(MidiMessage& message)
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

//Respond with device ready
void MessageHandler::sysExDeviceReady(MidiMessage& message){
    (*m_ptrNetwork).sendMessage(&SYSEX_DeviceReady,(uint8_t)true);
}

//Reset Distributors, LocalStorage and Set Device name to "New Device"
void MessageHandler::sysExResetDeviceConfig(MidiMessage& message){
    removeAllDistributors();
    localStorageReset();
    char name[DEVICE_NUM_NAME_BYTES] = "New Device";
    localStorageSetDeviceName(name);
}

//Respond with Device Construct
void MessageHandler::sysExGetDeviceConstruct(MidiMessage& message){
    (*m_ptrNetwork).sendMessage(Device::GetDeviceConstruct().data(),DEVICE_NUM_CFG_BYTES);
}

//Respond with Device Name
void MessageHandler::sysExGetDeviceName(MidiMessage& message){
    const uint8_t* name = reinterpret_cast<uint8_t*>(Device::Name.data());
    (*m_ptrNetwork).sendMessage(name,DEVICE_NUM_NAME_BYTES);
}

//Respond with Device Boolean
void MessageHandler::sysExGetDeviceBoolean(MidiMessage& message){
    static uint8_t deviceBoolean = Device::GetDeviceBoolean();
    (*m_ptrNetwork).sendMessage(&deviceBoolean,1);
}

//Configure Device Construct
void MessageHandler::sysExSetDeviceConstruct(MidiMessage& message){
   //TODO 
}

//Configure Device Name
void MessageHandler::sysExSetDeviceName(MidiMessage& message){
    char* deviceName = reinterpret_cast<char*>(message.sysExCmdPayload());
    Device::Name = deviceName;
    localStorageSetDeviceName(deviceName);
}

//Configure Device Boolean
void MessageHandler::sysExSetDeviceBoolean(MidiMessage& message){
    Device::OmniMode = ((message.sysExCmdPayload()[0] & DEVICE_BOOL_OMNIMODE) != 0);
}

//Respond with the Number of Distributors
void MessageHandler::sysExGetNumOfDistributors(MidiMessage& message){
    uint8_t sizeByte = m_distributors.size();
    (*m_ptrNetwork).sendMessage(&sizeByte,1);
}

//Respond with the requested Distributor Construct
void MessageHandler::sysExGetDistributorConstruct(MidiMessage& message){
    auto distributorBytes = getDistributor(message.sysExDistributorID()).toSerial();
    //Set Return Distributor ID from message
    distributorBytes[0] = message.sysExCmdPayload()[0];
    distributorBytes[1] = message.sysExCmdPayload()[1];
    (*m_ptrNetwork).sendMessage(distributorBytes.data(),distributorBytes.size());
}

//Respond with the requested Distributor Channel Config
void MessageHandler::sysExGetDistributorChannels(MidiMessage& message){
    uint16_t channels = getDistributor(message.sysExDistributorID()).getChannels();
    const uint8_t bytesToSend[2] = {static_cast<uint8_t>( channels >> 7) & 0x7F, 
                              static_cast<uint8_t>( channels >> 0) & 0x7F};
    (*m_ptrNetwork).sendMessage(bytesToSend,2);
}

//Respond with the requested Distributor Instrument Config
void MessageHandler::sysExGetDistributorInstruments(MidiMessage& message){
    uint32_t instruments = getDistributor(message.sysExDistributorID()).getInstruments();
    const uint8_t bytesToSend[4] = {static_cast<uint8_t>( instruments >> 21) & 0x7F, 
                              static_cast<uint8_t>( instruments >> 14) & 0x7F,
                              static_cast<uint8_t>( instruments >> 7) & 0x7F,
                              static_cast<uint8_t>( instruments >> 0) & 0x7F};
    (*m_ptrNetwork).sendMessage(bytesToSend,4);
}

//Configure the designated Distributor's Channels
void MessageHandler::sysExSetDistributorChannels(MidiMessage& message){
    uint16_t channels = ((message.sysExCmdPayload()[2] << 14) 
                       & (message.sysExCmdPayload()[3] << 7) 
                       & (message.sysExCmdPayload()[4] << 0));
    getDistributor(message.sysExDistributorID()).setChannels(channels);
}

//Configure the designated Distributor's Instruments
void MessageHandler::sysExSetDistributorInstruments(MidiMessage& message){
    uint32_t instruments = ((message.sysExCmdPayload()[2] << 28) 
                          & (message.sysExCmdPayload()[3] << 21) 
                          & (message.sysExCmdPayload()[4] << 14) 
                          & (message.sysExCmdPayload()[5] << 7) 
                          & (message.sysExCmdPayload()[6] << 0));
    getDistributor(message.sysExDistributorID()).setInstruments(instruments);
}

//Configure the designated Distributor's Distribution Method
void MessageHandler::sysExSetDistributorMethod(MidiMessage& message){
    getDistributor(message.sysExDistributorID()).setDistributionMethod(DistributionMethod(message.sysExCmdPayload()[2]));
}

//Configure the designated Distributor's Boolean Values
void MessageHandler::sysExSetDistributorBoolValues(MidiMessage& message){
    // Distributor distributor = getDistributor(message.sysExDistributorID());
    uint8_t distributorBoolByte = message.sysExCmdPayload()[2];
    
    getDistributor(message.sysExDistributorID()).setMuted((distributorBoolByte & DISTRIBUTOR_BOOL_MUTED) != 0);
    getDistributor(message.sysExDistributorID()).setDamperPedal((distributorBoolByte & DISTRIBUTOR_BOOL_DAMPERPEDAL) != 0);
    getDistributor(message.sysExDistributorID()).setPolyphonic((distributorBoolByte & DISTRIBUTOR_BOOL_DAMPERPEDAL) != 0);
    getDistributor(message.sysExDistributorID()).setNoteOverwrite((distributorBoolByte & DISTRIBUTOR_BOOL_NOTEOVERWRITE) != 0);
}

//Configure the designated Distributor's Minimum and Maximum Notes
void MessageHandler::sysExSetDistributorMinMaxNotes(MidiMessage& message){
    getDistributor(message.sysExDistributorID()).setMinMaxNote(message.sysExCmdPayload()[2],message.sysExCmdPayload()[3]);
}

//Configure the designated Distributor's Max number of Polyphonic Notes
void MessageHandler::sysExSetDistributorNumPolyphonicNotes(MidiMessage& message){
    getDistributor(message.sysExDistributorID()).setNumPolyphonicNotes(message.sysExCmdPayload()[2]);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//Manage Distributors
////////////////////////////////////////////////////////////////////////////////////////////////////

//Create a default Distributor and add it to the Distribution Pool
void MessageHandler::addDistributor()
{
    Distributor newDistributor = Distributor(m_ptrInstrumentController);
    m_distributors.push_back(newDistributor);
    localStorageAddDistributor();
}

//Add an existing Distributor to the Distribution Pool
void MessageHandler::addDistributor(Distributor distributor)
{
    m_distributors.push_back(distributor);
    localStorageAddDistributor();
}

//Create a Distributor from a Construct and add it to the Distribution Pool
void MessageHandler::addDistributor(uint8_t data[])
{
    Distributor distributor(m_ptrInstrumentController);
    distributor.setDistributor(data);
    m_distributors.push_back(distributor);
    localStorageAddDistributor();
}

//Updates the designated Distributor in the Distribution Pool
// from a construct or adds a new Distributor
void MessageHandler::setDistributor(uint8_t data[])
{
    //Clear active Notes
    (*m_ptrInstrumentController).stopAll();

    // If Distributor exists update it.
    uint16_t distributorID = data[0] << 7| (data[1]);
    if (distributorID < m_distributors.size()){
        localStorageUpdateDistributor(distributorID,data);
        m_distributors[distributorID].setDistributor(data);
        return;
    }

    // Else create Distributor and add it to the Distribution Pool
    addDistributor(data);
}

//Removes the designated Distributor from the Distribution Pool
void MessageHandler::removeDistributor(uint8_t id)
{
    (*m_ptrInstrumentController).stopAll(); //Safety Stops all Playing Notes
    if(id >= m_distributors.size()) id = m_distributors.size();
    m_distributors.erase(m_distributors.begin() + id);
    localStorageRemoveDistributor(id);
}

//Clear Distribution Pool
void MessageHandler::removeAllDistributors()
{
    (*m_ptrInstrumentController).stopAll(); //Safety Stops all Playing Notes
    m_distributors.clear();
    localStorageClearDistributors();
}

//Returns the indexed Distributor from the Distribution Pool
Distributor& MessageHandler::getDistributor(uint8_t index){
    return (m_distributors[index]);
}

//Returns indexed Distributor Construct
std::array<uint8_t, DISTRIBUTOR_NUM_CFG_BYTES> MessageHandler::getDistributorSerial(uint8_t index){

    //Append Distributor ID to the Construct
    auto distributorObj = m_distributors[index].toSerial();
    distributorObj[0] = static_cast<uint8_t>((index >> 7) & 0x7F);
    distributorObj[1] = static_cast<uint8_t>((index >> 0) & 0x7F);
    return distributorObj;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//Local Storage
////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef LOCAL_STORAGE

void MessageHandler::localStorageInit(){
    LocalStorages::localStorage = LocalStorage();
}

void MessageHandler::localStorageSetDeviceName(char* name){
    LocalStorages::localStorage.SetDeviceName(name);
    Device::Name = LocalStorages::localStorage.GetDeviceName(reinterpret_cast<uint8_t*>(name));
}

void MessageHandler::localStorageAddDistributor(){
    uint8_t index = m_distributors.size()-1;
    LocalStorages::localStorage.SetDistributorConstruct(index,getDistributorSerial(index).data());
    LocalStorages::localStorage.SetNumOfDistributors(m_distributors.size());
}

void MessageHandler::localStorageRemoveDistributor(uint8_t id){
    LocalStorages::localStorage.SetNumOfDistributors(m_distributors.size());

    for(int i = id; i < m_distributors.size(); i++){
        LocalStorages::localStorage.SetDistributorConstruct(i,getDistributorSerial(i).data());
    }
}

void MessageHandler::localStorageUpdateDistributor(uint16_t distributorID, const uint8_t* data){
    LocalStorages::localStorage.SetDistributorConstruct(distributorID,data);
}

void MessageHandler::localStorageClearDistributors(){
    LocalStorages::localStorage.SetNumOfDistributors(m_distributors.size());
}

void MessageHandler::localStorageReset(){
    LocalStorages::localStorage.ResetDeviceConfig();
    LocalStorages::localStorage = LocalStorage();
}

#else
void MessageHandler::localStorageInit(){}
void MessageHandler::localStorageSetDeviceName(char* name){}
void MessageHandler::localStorageAddDistributor(){}
void MessageHandler::localStorageRemoveDistributor(uint8_t id){}
void MessageHandler::localStorageUpdateDistributor(uint16_t distributorID, uint8_t* data){}
void MessageHandler::localStorageClearDistributors(){}
void MessageHandler::localStorageReset(){}
#endif