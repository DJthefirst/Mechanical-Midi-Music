/* 
 * MessgaeHandler.cpp
 *
 * The Message Handler Recives Midi messages and routes them to the appropiate distributor by MIDI 
 * channel.
 * 
 */

#include "MessageHandler.h"
#include "Instruments/InstrumentController.h"
#include "Constants.h"

InstrumentController* m_ptrInstrumentController;

MessageHandler::MessageHandler(InstrumentController* ptrInstrumentController){
    m_ptrInstrumentController = ptrInstrumentController;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//Process Messages
////////////////////////////////////////////////////////////////////////////////////////////////////

void MessageHandler::ProcessMessage(uint8_t message[])
{   
    m_msgType = message[0] & 0b11110000;     // 4 MSB Represent MIDI MsgType
    m_msgChannel = message[0] & 0b00001111;  // 4 LSB Represent MIDI Channel

    //Handle System Common Msg or Send to Distributors
    if(m_msgType == MIDI_SysCommon)
    {
        //_msgChannel represents the message type for SysCommon
        switch(m_msgChannel){

        case(MIDI_SysEXE):
            ProcessSysEXE(message);
            break;

        case(MIDI_SysStop):
            //Not Yet Implemented
            break;

        case(MIDI_SysReset):
            //Not Yet Implemented
            break;
        }
    }
    else if(m_msgType == MIDI_ControlChange){
        ProcessCC(message);
    }
    else{
        DistributeMessage(message);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//Process SysEXE Messages
////////////////////////////////////////////////////////////////////////////////////////////////////

void MessageHandler::ProcessSysEXE(uint8_t message[])
{
    //Not Yet Implemented
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//Process CC MIDI Messages
////////////////////////////////////////////////////////////////////////////////////////////////////

void MessageHandler::ProcessCC(uint8_t message[])
{
    for(uint8_t i=0; i < m_distributors.size(); i++)
    {   
        if((m_distributors[i].GetChannels() & (1 << m_msgChannel)) != (0)){
            switch(message[1]){

            case(MIDI_CC_ModulationWheel):
                (*m_ptrInstrumentController).SetModulationWheel(message[2]);
                break;
            case(MIDI_CC_FootPedal):
                (*m_ptrInstrumentController).SetFootPedal(message[2]);
                break;
            case(MIDI_CC_Volume):
                (*m_ptrInstrumentController).SetVolume(message[2]);
                break;
            case(MIDI_CC_Expression):
                (*m_ptrInstrumentController).SetExpression(message[2]);
                break;
            case(MIDI_CC_EffectCrtl_1):
                (*m_ptrInstrumentController).SetEffectCrtl_1(message[2]);
                break;
            case(MIDI_CC_EffectCrtl_2):
                (*m_ptrInstrumentController).SetEffectCrtl_2(message[2]);
                break;
            case(MIDI_CC_DamperPedal):
                m_distributors[i].SetDamperPedal(message[2] < 64);
                break;
            case(MIDI_CC_Mute):
                (*m_ptrInstrumentController).StopAll();
                break;
            case(MIDI_CC_Reset):
                (*m_ptrInstrumentController).ResetAll();
                break;
            case(MIDI_CC_AllNotesOff):
                (*m_ptrInstrumentController).StopAll();
                break;
            case(MIDI_CC_OmniModeOff):
                (*m_ptrInstrumentController).StopAll();
                m_OmniMode = false;
                break;
            case(MIDI_CC_OmniModeOn):
                (*m_ptrInstrumentController).StopAll();
                m_OmniMode = true;
                break;
            case(MIDI_CC_Monophonic):
                m_distributors[i].SetPolyphonic(false);
                (*m_ptrInstrumentController).StopAll();
                break;
            case(MIDI_CC_Polyphonic):
                (*m_ptrInstrumentController).StopAll();
                m_distributors[i].SetPolyphonic(true);
                break;
            }
            return;
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//Distribute MIDI Messages
////////////////////////////////////////////////////////////////////////////////////////////////////

void MessageHandler::DistributeMessage(uint8_t message[])
{  
    for(uint8_t i=0; i < m_distributors.size(); i++)
    {   
        if((m_distributors[i].GetChannels() & (1 << m_msgChannel)) != (0))
            m_distributors[i].ProcessMessage(message);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//Manage Distributors
////////////////////////////////////////////////////////////////////////////////////////////////////

void MessageHandler::AddDistributor()
{
    Distributor newDistributor = Distributor(m_ptrInstrumentController);
    m_distributors.push_back(newDistributor);
}

void MessageHandler::AddDistributor(uint8_t data[])
{
    //Not Yet Implemented
}

void MessageHandler::AddDistributor(Distributor distributor)
{
    m_distributors.push_back(distributor);
}

void MessageHandler::RemoveDistributor(uint8_t id)
{
    //Not Yet Implemented
}

Distributor* MessageHandler::GetDistributor(uint8_t index){
    return &(m_distributors[index]);
}