/* 
 * MessgaeHandler.cpp
 *
 * The Message Handler Recives Midi messages and routes 
 * them to the appropiate distributor by MIDI channel.
 * 
 */

#include "MessageHandler.h"
#include "Constants.h"
#include <Arduino.h>

MessageHandler::MessageHandler(InstrumentController* ptrInstrumentController){
    m_ptrInstrumentController = ptrInstrumentController;
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
            //Not Yet Implemented
            break;

        case(MIDI_SysReset):
            //Not Yet Implemented
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
    //Check Device ID
    if(message.deviceID() != SYSEX_DEV_ID) return;

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
        case (SYSEX_AddDistributor):
            addDistributor(message.buffer + 5);
            break;
        case (SYSEX_RemoveDistributor):
            removeDistributor(message.buffer[6]);
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
    auto distributorBytes = getDistributor(message.buffer[6]).toSerial();
    (*m_ptrNetwork).sendMessage(distributorBytes.data(),distributorBytes.size());
}

void MessageHandler::sysExGetDeviceName(MidiMessage message){
    (*m_ptrNetwork).sendMessage(Device::Name,20);
}

void MessageHandler::sysExGetDeviceBoolean(MidiMessage message){
    static uint8_t deviceBoolean = 0;
    deviceBoolean += Device::OmniMode ? 0x01 : 0x00;

    (*m_ptrNetwork).sendMessage(&deviceBoolean,1);
}

void MessageHandler::sysExSetDeviceConstruct(MidiMessage message){
    
}

void MessageHandler::sysExSetDeviceName(MidiMessage message){
    for(uint8_t i=0; i<20; i++) Device::Name[i] = message.buffer[i+5];
}

void MessageHandler::sysExSetDeviceBoolean(MidiMessage message){
    Device::OmniMode = ((message.buffer[5] & 0x01) != 0);
}

void MessageHandler::sysExGetNumOfDistributors(MidiMessage message){
    uint8_t sizeByte = m_distributors.size();
    (*m_ptrNetwork).sendMessage(&sizeByte,1);
}

void MessageHandler::sysExGetDistributorConstruct(MidiMessage message){
    auto distributorBytes = getDistributor(message.buffer[6]).toSerial();
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
    getDistributor(message.buffer[6]).setChannels(channels);
}

void MessageHandler::sysExSetDistributorInstruments(MidiMessage message){
    uint32_t instruments = ((message.buffer[7] << 28) 
                          & (message.buffer[8] << 21) 
                          & (message.buffer[9] << 14) 
                          & (message.buffer[10] << 7) 
                          & (message.buffer[11] << 0));
    getDistributor(message.buffer[6]).setInstruments(instruments);
}

void MessageHandler::sysExSetDistributorMethod(MidiMessage message){
    getDistributor(message.buffer[6]).setDistributionMethod(DistributionMethod(message.buffer[7]));
}

void MessageHandler::sysExSetDistributorBoolValues(MidiMessage message){
    getDistributor(message.buffer[6]).setDamperPedal((message.buffer[7] & 0x01) != 0);
    getDistributor(message.buffer[6]).setPolyphonic((message.buffer[7] & 0x02) != 0);
    getDistributor(message.buffer[6]).setNoteOverwrite((message.buffer[7] & 0x04) != 0);
}

void MessageHandler::sysExSetDistributorMinMaxNotes(MidiMessage message){
    getDistributor(message.buffer[6]).setMinMaxNote(message.buffer[7],message.buffer[8]);
}

void MessageHandler::sysExSetDistributorNumPolyphonicNotes(MidiMessage message){
    getDistributor(message.buffer[6]).setNumPolyphonicNotes(message.buffer[7]);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//Manage Distributors
////////////////////////////////////////////////////////////////////////////////////////////////////

void MessageHandler::addDistributor()
{
    Distributor newDistributor = Distributor(m_ptrInstrumentController);
    m_distributors.push_back(newDistributor);
}

void MessageHandler::addDistributor(Distributor distributor)
{
    m_distributors.push_back(distributor);
}

void MessageHandler::addDistributor(uint8_t data[])
{
    (*m_ptrNetwork).sendMessage(data,16);
    //Decode Distributor Construct
    uint16_t distributorID = data[0] | (data[1] << 7);
    uint16_t channels = data[2] 
        | (data[3] << 7) 
        | (data[4] << 14);
    uint32_t instruments = data[5]
        | (data[6] << 7) 
        | (data[7] << 14)
        | (data[8] << 21)
        | (data[9] << 28);
    DistributionMethod distribMethod = static_cast<DistributionMethod>(data[10]);

    //Create Distributor
    Distributor distributor(m_ptrInstrumentController);
    distributor.setChannels(channels); // 1
    distributor.setInstruments(instruments); // 1,2
    distributor.setDistributionMethod(distribMethod);
    distributor.setDamperPedal((data[11] & 0x01) != 0);
    distributor.setPolyphonic((data[11] & 0x02) != 0);
    distributor.setNoteOverwrite((data[11] & 0x04) != 0);
    distributor.setMinMaxNote(data[12], data[13]);
    distributor.setNumPolyphonicNotes(data[14]);

    //Add Distributor to Distribution Pool
    m_distributors.push_back(distributor);
}

void MessageHandler::removeDistributor(uint8_t id)
{
    (*m_ptrInstrumentController).stopAll(); //Safety Stops all Playing Notes
    m_distributors.erase(m_distributors.begin() + id);
}

void MessageHandler::removeAllDistributors()
{
    (*m_ptrInstrumentController).stopAll(); //Safety Stops all Playing Notes
    m_distributors.clear();
}

Distributor& MessageHandler::getDistributor(uint8_t index){
    return (m_distributors[index]);
}