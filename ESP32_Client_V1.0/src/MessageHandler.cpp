/* 
 * MessgaeHandler.cpp
 *
 * The Message Handler Recives Midi messages and routes 
 * them to the appropiate distributor by MIDI channel.
 * 
 */

#include "MessageHandler.h"
#include <Arduino.h>
#include <cstring>  // For memcpy and strncpy
#include <algorithm> // For std::min

//Message Handler constructor used to pass in a ptr to the instrument controller.
MessageHandler::MessageHandler(InstrumentController* ptrInstrumentController){
    m_ptrInstrumentController = ptrInstrumentController;
    // Ensure source ID matches current runtime device ID
    m_src = Device::GetDeviceID();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//Process Messages
////////////////////////////////////////////////////////////////////////////////////////////////////

//Consumes a MidiMessage and Handles the message based on its type.
std::optional<MidiMessage> MessageHandler::processMessage(MidiMessage& message)
{   
    //Handle System Common Msg or Send to Distributors
    if(message.type() == MIDI_SysCommon){

        switch(message.sysCommonType()){

            case(MIDI_SysEX):
                return(processSysEX(message));
                break;

            case(MIDI_SysStop):
                m_ptrInstrumentController->stopAll();
                break;

            case(MIDI_SysReset):
                m_ptrInstrumentController->resetAll();
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
    return {};
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//Process SysEX Messages (Used For Device Config)
////////////////////////////////////////////////////////////////////////////////////////////////////

//Process SysEX Messages by type
std::optional<MidiMessage> MessageHandler::processSysEX(MidiMessage& message)
{

    //Check MIDI ID
    if(message.sysExID() != SYSEX_ID) return {};
    //Check Device ID or Global ID 0x00;
    if(message.DestinationID() != Device::GetDeviceID() && message.DestinationID() != 0x00) return {};
    m_dest = message.SourceID();

    switch(message.sysExCommand()){
        
        case (SYSEX_DeviceReady):
            return sysExDeviceReady(message);

        case (SYSEX_ResetDeviceConfig):
            sysExResetDeviceConfig(message); 
            return {};
        case (SYSEX_DiscoverDevices):
            return {sysExDiscoverDevices(message)};

        case (SYSEX_GetDeviceConstructWithDistributors):
            return {}; //TODO

        case (SYSEX_GetDeviceConstruct):
            return sysExGetDeviceConstruct(message);

        case (SYSEX_GetDeviceName):
            return sysExGetDeviceName(message);

        case (SYSEX_GetDeviceBoolean):
            return sysExGetDeviceBoolean(message);

        case (SYSEX_GetDeviceID):
            return sysExGetDeviceID(message);

        case (SYSEX_SetDeviceConstructWithDistributors):
            return {}; //TODO

        case (SYSEX_SetDeviceConstruct):
            sysExSetDeviceConstruct(message); 
            return {};

        case (SYSEX_SetDeviceID):
            sysExSetDeviceID(message);
            return {};

        case (SYSEX_SetDeviceName):
            sysExSetDeviceName(message); 
            return {};
            
        case (SYSEX_SetDeviceBoolean):
            sysExSetDeviceBoolean(message);
            return {};

        case (SYSEX_RemoveDistributor):
            removeDistributor(message.sysExDistributorID());
            return {};
        case (SYSEX_RemoveAllDistributors):
            removeAllDistributors();
            return {};
        case (SYSEX_GetNumOfDistributors):
            return sysExGetNumOfDistributors(message);

        case (SYSEX_GetAllDistributors):
            return sysExGetNumOfDistributors(message);

        case (SYSEX_AddDistributor):
            setDistributor(message.sysExCmdPayload());
            return {};
        case (SYSEX_ToggleMuteDistributor):
            getDistributor(message.sysExDistributorID()).toggleMuted();
            return {};

        case (SYSEX_GetDistributorConstruct):
            return sysExGetDistributorConstruct(message);

        case (SYSEX_GetDistributorChannels):
            return sysExGetDistributorChannels(message);

        case (SYSEX_GetDistributorInstruments):
            return {};//TODO

        case (SYSEX_GetDistributorMethod):
            return {};//TODO

        case (SYSEX_GetDistributorBoolValues):
            return {};//TODO

        case (SYSEX_GetDistributorMinMaxNotes):
            return {};//TODO

        case (SYSEX_GetDistributorNumPolyphonicNotes):
            return {};//TODO

        case (SYSEX_SetDistributor):
            setDistributor(message.sysExCmdPayload()); 
            return {};
        case (SYSEX_SetDistributorChannels):
            sysExSetDistributorChannels(message); 
            return {};
        case (SYSEX_SetDistributorInstruments):
            sysExSetDistributorInstruments(message); 
            return {};
        case (SYSEX_SetDistributorMethod):
            sysExSetDistributorMethod(message); 
            return {};
        case (SYSEX_SetDistributorBoolValues):
            sysExSetDistributorBoolValues(message);
            return {};
        case (SYSEX_SetDistributorMinMaxNotes):
            sysExSetDistributorMinMaxNotes(message); 
            return {};
        case (SYSEX_SetDistributorNumPolyphonicNotes):
            sysExSetDistributorNumPolyphonicNotes(message); 
            return {};
        default:
            return {};
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
                m_distributors[i].setDamperPedal(message.CC_Value()> 64); //Above 64 is ON else OFF
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
//Distribute MIDI Messages 
////////////////////////////////////////////////////////////////////////////////////////////////////

//Send messgae to all distributors which accept the designated message's channel.
void MessageHandler::distributeMessage(MidiMessage& message)
{  
    // Pre-calculate channel mask to avoid repeated bit operations
    const uint16_t channelMask = (1 << message.channel());
    
    for(uint8_t i=0; i < m_distributors.size(); i++)
    {   
        if((m_distributors[i].getChannels() & channelMask) != 0) {
            m_distributors[i].processMessage(message);
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//Handle SysEx MIDI Messages (Used For Device Config)
////////////////////////////////////////////////////////////////////////////////////////////////////

//Respond with device ready
MidiMessage MessageHandler::sysExDeviceReady(MidiMessage& message){
    return MidiMessage(m_src,m_dest,message.sysExCommand(),&SYSEX_DeviceReady,(uint8_t)true);
}

//Reset Distributors, LocalStorage and Set Device name to "New Device"
void MessageHandler::sysExResetDeviceConfig(MidiMessage& message){
    removeAllDistributors();
    localStorageReset();
    char name[DEVICE_NUM_NAME_BYTES] = "New Device";
    localStorageSetDeviceName(name);
    Device::Name = "New Device";
    return;
}

//Respond with device ready
MidiMessage MessageHandler::sysExDiscoverDevices(MidiMessage& message){
    std::array<std::uint8_t,2> deviceID;
    uint16_t runtimeId = Device::GetDeviceID();
    deviceID[0] = static_cast<uint8_t>((runtimeId >> 7) & 0x7F); //Device ID MSB
    deviceID[1] = static_cast<uint8_t>((runtimeId >> 0) & 0x7F); //Device ID LSB
    return MidiMessage(m_src,m_dest,message.sysExCommand(),deviceID.data(),2);
}

//Respond with Device Construct
MidiMessage MessageHandler::sysExGetDeviceConstruct(MidiMessage& message){
    return MidiMessage(m_src,m_dest,message.sysExCommand(),Device::GetDeviceConstruct().data(), DEVICE_NUM_CFG_BYTES);
}

//Respond with Device Name
MidiMessage MessageHandler::sysExGetDeviceName(MidiMessage& message){
    // Create a properly formatted buffer with null padding
    uint8_t nameBuffer[DEVICE_NUM_NAME_BYTES];
    
    // Initialize entire buffer with null bytes
    memset(nameBuffer, 0x00, DEVICE_NUM_NAME_BYTES);
    
    // Copy device name (up to the buffer size, ensuring we don't overflow)
    size_t copyLength = std::min(Device::Name.length(), static_cast<size_t>(DEVICE_NUM_NAME_BYTES));
    memcpy(nameBuffer, Device::Name.c_str(), copyLength);

    return MidiMessage(m_src, m_dest, message.sysExCommand(), nameBuffer, DEVICE_NUM_NAME_BYTES);
}

//Respond with Device Boolean
MidiMessage MessageHandler::sysExGetDeviceBoolean(MidiMessage& message){
    static uint8_t deviceBoolean = Device::GetDeviceBoolean();
    return MidiMessage(m_src,m_dest,message.sysExCommand(),&deviceBoolean,1);
}

// Respond with Device ID (14-bit split into two 7-bit bytes)
MidiMessage MessageHandler::sysExGetDeviceID(MidiMessage& message){
    std::array<std::uint8_t,2> deviceID;
    uint16_t runtimeId = Device::GetDeviceID();
    deviceID[0] = static_cast<uint8_t>((runtimeId >> 7) & 0x7F);
    deviceID[1] = static_cast<uint8_t>((runtimeId >> 0) & 0x7F);
    return MidiMessage(m_src,m_dest,message.sysExCommand(),deviceID.data(),2);
}

// Set Device ID from incoming payload (expecting at least 2 bytes MSB,LSB)
void MessageHandler::sysExSetDeviceID(MidiMessage& message){
    if (message.length < SYSEX_HeaderSize + 2) return; // not enough data
    uint8_t msb = message.sysExCmdPayload()[0];
    uint8_t lsb = message.sysExCmdPayload()[1];
    uint16_t newID = (static_cast<uint16_t>(msb) << 7) | static_cast<uint16_t>(lsb);
    Device::SetDeviceID(newID);
    // persist
    LocalStorage::get().SetDeviceID(newID);
    // update source id used for replies
    m_src = Device::GetDeviceID();
}

//Configure Device Construct
void MessageHandler::sysExSetDeviceConstruct(MidiMessage& message){
    //TODO
    return;
}

//Configure Device Name
void MessageHandler::sysExSetDeviceName(MidiMessage& message){
    // Calculate payload length (total message length - SysEx header - end byte)
    uint8_t payloadLength = message.length - SYSEX_HeaderSize - 1;
    
    // Safely create string from payload with length validation
    std::string newName(reinterpret_cast<const char*>(message.sysExCmdPayload()), 
                       std::min(payloadLength, static_cast<uint8_t>(DEVICE_NUM_NAME_BYTES - 1)));
    
    // Remove any null bytes from the end (clean up the string)
    newName.erase(newName.find_last_not_of('\0') + 1);

    // Assign the processed name (no padding needed for std::string)
    Device::Name = newName;
    
    // Convert to C-string for local storage (ensure null termination)
    char deviceNameBuffer[DEVICE_NUM_NAME_BYTES];
    memset(deviceNameBuffer, 0x00, DEVICE_NUM_NAME_BYTES);
    strncpy(deviceNameBuffer, newName.c_str(), DEVICE_NUM_NAME_BYTES - 1);
    localStorageSetDeviceName(deviceNameBuffer);
}

//Configure Device Boolean
void MessageHandler::sysExSetDeviceBoolean(MidiMessage& message){
    Device::OmniMode = ((message.sysExCmdPayload()[0] & DEVICE_BOOL_OMNIMODE) != 0);

}

// TODO: Return Multiple Messages
// MidiMessage MessageHandler::sysExGetAllDistributors(MidiMessage& message){
//     return MidiMessage(m_src,m_dest,message.sysExCommand(),&SYSEX_DeviceReady,(uint8_t)true);
// }


//Respond with the Number of Distributors
MidiMessage MessageHandler::sysExGetNumOfDistributors(MidiMessage& message){
    uint8_t sizeByte = m_distributors.size();
    return MidiMessage(m_src,m_dest,message.sysExCommand(),&sizeByte,1);
}

//Respond with the requested Distributor Construct
MidiMessage MessageHandler::sysExGetDistributorConstruct(MidiMessage& message){
    auto distributorBytes = getDistributor(message.sysExDistributorID()).toSerial();
    //Set Return Distributor ID from message
    distributorBytes[0] = message.sysExCmdPayload()[0];
    distributorBytes[1] = message.sysExCmdPayload()[1];
    return MidiMessage(m_src,m_dest,message.sysExCommand(),distributorBytes.data(),distributorBytes.size());
}

//Respond with the requested Distributor Channel Config
MidiMessage MessageHandler::sysExGetDistributorChannels(MidiMessage& message){
    uint16_t channels = getDistributor(message.sysExDistributorID()).getChannels();
    const uint8_t bytesToSend[2] = {static_cast<uint8_t>(( channels >> 7) & 0x7F), 
                              static_cast<uint8_t>(( channels >> 0) & 0x7F)};
    return MidiMessage(m_src,m_dest,message.sysExCommand(),bytesToSend,2);
}

//Respond with the requested Distributor Instrument Config
MidiMessage MessageHandler::sysExGetDistributorInstruments(MidiMessage& message){
    uint32_t instruments = getDistributor(message.sysExDistributorID()).getInstruments();
    const uint8_t bytesToSend[4] = {static_cast<uint8_t>(( instruments >> 21) & 0x7F), 
                              static_cast<uint8_t>(( instruments >> 14) & 0x7F),
                              static_cast<uint8_t>(( instruments >> 7) & 0x7F),
                              static_cast<uint8_t>(( instruments >> 0) & 0x7F)};
    return MidiMessage(m_src,m_dest,message.sysExCommand(),bytesToSend,4);
}

//Configure the designated Distributor's Channels
void MessageHandler::sysExSetDistributorChannels(MidiMessage& message){
    uint16_t channels = ((message.sysExCmdPayload()[2] << 14) 
                       | (message.sysExCmdPayload()[3] << 7) 
                       | (message.sysExCmdPayload()[4] << 0));
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
    getDistributor(message.sysExDistributorID()).setPolyphonic((distributorBoolByte & DISTRIBUTOR_BOOL_POLYPHONIC) != 0);
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
    m_ptrInstrumentController->stopAll();

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
    m_ptrInstrumentController->stopAll(); //Safety Stops all Playing Notes
    if(m_distributors.size() == 0) return;
    if(id >= m_distributors.size()) id = m_distributors.size();
    m_distributors.erase(m_distributors.begin() + id);
    localStorageRemoveDistributor(id);
}

//Clear Distribution Pool
void MessageHandler::removeAllDistributors()
{
    m_ptrInstrumentController->stopAll(); //Safety Stops all Playing Notes
    m_distributors.clear();
    localStorageClearDistributors();
}

//Returns the indexed Distributor from the Distribution Pool
Distributor& MessageHandler::getDistributor(uint8_t index){
    if(index >= m_distributors.size()) index = m_distributors.size()-1;
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
#ifdef EXTRA_LOCAL_STORAGE

void MessageHandler::localStorageSetDeviceName(char* name){
    LocalStorage::get().SetDeviceName(name);
    Device::Name = LocalStorage::get().GetDeviceName();
}

void MessageHandler::localStorageAddDistributor(){
    uint8_t index = m_distributors.size()-1;
    LocalStorage::get().SetDistributorConstruct(index,getDistributorSerial(index).data());
    LocalStorage::get().SetNumOfDistributors(m_distributors.size());
}

void MessageHandler::localStorageRemoveDistributor(uint8_t id){
    LocalStorage::get().SetNumOfDistributors(m_distributors.size());

    for(int i = id; i < m_distributors.size(); i++){
        LocalStorage::get().SetDistributorConstruct(i,getDistributorSerial(i).data());
    }
}

void MessageHandler::localStorageUpdateDistributor(uint16_t distributorID, const uint8_t* data){
    LocalStorage::get().SetDistributorConstruct(distributorID,data);
}

void MessageHandler::localStorageClearDistributors(){
    LocalStorage::get().SetNumOfDistributors(m_distributors.size());
}

void MessageHandler::localStorageReset(){
    LocalStorage::get().ResetDeviceConfig();
}

#else
    void MessageHandler::localStorageSetDeviceName(char* name){}
    void MessageHandler::localStorageAddDistributor(){}
    void MessageHandler::localStorageRemoveDistributor(uint8_t id){}
    void MessageHandler::localStorageUpdateDistributor(uint16_t distributorID, const uint8_t* data){}
    void MessageHandler::localStorageClearDistributors(){}
    void MessageHandler::localStorageReset(){}
#endif