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
        switch(message.value()){

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
//Process SysEX Messages
////////////////////////////////////////////////////////////////////////////////////////////////////

void MessageHandler::processSysEX(MidiMessage message)
{
    //Check MIDI ID
    if(message.buffer[1] != SYSEX_ID) return;
    //Check Device ID
    if(message.buffer[2] != SYSEX_DEV_ID0 || message.buffer[3] != SYSEX_DEV_ID1) return;

    switch(message.buffer[4]){
        case SYSEX_DistributorAdd:
            break;
        
        case SYSEX_DistributorRequest:
            sysExDistributorRequest(0);
            break;

        case SYSEX_DistributorRequestAll:
            sysExDistributorRequestAll();
            break;

        case SYSEX_DistributorSetMode:
            sysExDistributorSetMode(message);
            break;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//Process CC MIDI Messages
////////////////////////////////////////////////////////////////////////////////////////////////////

void MessageHandler::processCC(MidiMessage message)
{
    for(uint8_t i=0; i < m_distributors.size(); i++)
    {   
        if((m_distributors[i].getChannels() & (1 << message.value())) != (0)){
            switch(message.buffer[1]){

            case(MIDI_CC_ModulationWheel):
                (*m_ptrInstrumentController).setModulationWheel(message.buffer[2]);
                break;
            case(MIDI_CC_FootPedal):
                (*m_ptrInstrumentController).setFootPedal(message.buffer[2]);
                break;
            case(MIDI_CC_Volume):
                (*m_ptrInstrumentController).setVolume(message.buffer[2]);
                break;
            case(MIDI_CC_Expression):
                (*m_ptrInstrumentController).setExpression(message.buffer[2]);
                break;
            case(MIDI_CC_EffectCrtl_1):
                (*m_ptrInstrumentController).setEffectCrtl_1(message.buffer[2]);
                break;
            case(MIDI_CC_EffectCrtl_2):
                (*m_ptrInstrumentController).setEffectCrtl_2(message.buffer[2]);
                break;
            case(MIDI_CC_DamperPedal):
                m_distributors[i].setDamperPedal(message.buffer[2] < 64);
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
                m_OmniMode = false;
                break;
            case(MIDI_CC_OmniModeOn):
                (*m_ptrInstrumentController).stopAll();
                m_OmniMode = true;
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
        if((m_distributors[i].getChannels() & (1 << message.value())) != (0))
            m_distributors[i].processMessage(message);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//Handle SysEx MIDI Messages
////////////////////////////////////////////////////////////////////////////////////////////////////

void MessageHandler::sysExDistributorAdd(MidiMessage message){

}

void MessageHandler::sysExDistributorRequest(uint8_t distributor){
    (*m_ptrNetwork).sendMessage(getDistributor(0)->toSerial(),10);
}

void MessageHandler::sysExDistributorRequestAll(){

}

void MessageHandler::sysExDistributorSetMode(MidiMessage message){
    getDistributor(0)->setDistributionMethod(DistributionMethod(message.buffer[5]));
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//Manage Distributors
////////////////////////////////////////////////////////////////////////////////////////////////////

void MessageHandler::addDistributor()
{
    Distributor newDistributor = Distributor(m_ptrInstrumentController);
    m_distributors.push_back(newDistributor);
}

void MessageHandler::addDistributor(uint8_t data[])
{
    //Not Yet Implemented
}

void MessageHandler::addDistributor(Distributor distributor)
{
    m_distributors.push_back(distributor);
}

void MessageHandler::removeDistributor(uint8_t id)
{
    m_distributors.erase(m_distributors.begin() + id);
}

void MessageHandler::removeAllDistributors()
{
    m_distributors.clear();
}

Distributor* MessageHandler::getDistributor(uint8_t index){
    return &(m_distributors[index]);
}