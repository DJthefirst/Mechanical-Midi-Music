/*
The Message Handler Recives Midi messages and routes them to the appropiate distributor by MIDI channel.
*/
#include "MessageHandler.h"
#include "Instruments/InstrumentController.h"
#include "Constants.h"

InstrumentController* _ptrInstrumentController;

MessageHandler::MessageHandler() {}

void MessageHandler::initalize(InstrumentController* ptrInstrumentController){
    _ptrInstrumentController = ptrInstrumentController;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//Process Messages
////////////////////////////////////////////////////////////////////////////////////////////////////

void MessageHandler::processMessage(uint8_t message[])
{   
    _msgType = message[0] & 0b11110000;     // 4 MSB Represent MIDI MsgType
    _msgChannel = message[0] & 0b00001111;  // 4 LSB Represent MIDI Channel

    //Handle System Common Msg or Send to Distributors
    if(_msgType == MIDI_SysCommon)
    {
        //_msgChannel repeesents the message type for SysCommon
        switch(_msgChannel){

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
    else if(_msgType == MIDI_ControlChange){
        ProcessCC(message);
    }
    else{
        DistributeMessage(message);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//Process SysEXE Messages
////////////////////////////////////////////////////////////////////////////////////////////////////

void MessageHandler::ProcessSysEXE(uint8_t _message[])
{
    //Not Yet Implemented
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//Process CC MIDI Messages
////////////////////////////////////////////////////////////////////////////////////////////////////

void MessageHandler::ProcessCC(uint8_t message[])
{
    for(uint8_t i=0; i < _distributors.size(); i++)
    {   
        if((_distributors[i].getChannels() & (1 << _msgChannel)) != (0)){
            switch(message[1]){

            case(MIDI_CC_ModulationWheel):
                (*_ptrInstrumentController).SetModulationWheel(message[2]);
                break;
            case(MIDI_CC_FootPedal):
                (*_ptrInstrumentController).SetFootPedal(message[2]);
                break;
            case(MIDI_CC_Volume):
                (*_ptrInstrumentController).SetVolume(message[2]);
                break;
            case(MIDI_CC_Expression):
                (*_ptrInstrumentController).SetExpression(message[2]);
                break;
            case(MIDI_CC_EffectCrtl_1):
                (*_ptrInstrumentController).SetEffectCrtl_1(message[2]);
                break;
            case(MIDI_CC_EffectCrtl_2):
                (*_ptrInstrumentController).SetEffectCrtl_2(message[2]);
                break;
            case(MIDI_CC_DamperPedal):
                if(message[2] < 64)_distributors[i].setDamperPedal(false);
                else _distributors[i].setDamperPedal(true);
                break;
            case(MIDI_CC_Mute):
                (*_ptrInstrumentController).StopAll();
                break;
            case(MIDI_CC_Reset):
                (*_ptrInstrumentController).ResetAll();
                break;
            case(MIDI_CC_AllNotesOff):
                (*_ptrInstrumentController).StopAll();
                break;
            case(MIDI_CC_OmniModeOff):
                (*_ptrInstrumentController).StopAll();
                _OmniMode = false;
                break;
            case(MIDI_CC_OmniModeOn):
                (*_ptrInstrumentController).StopAll();
                _OmniMode = true;
                break;
            case(MIDI_CC_Monophonic):
                _distributors[i].setPolyphonic(false);
                (*_ptrInstrumentController).StopAll();
                break;
            case(MIDI_CC_Polyphonic):
                (*_ptrInstrumentController).StopAll();
                _distributors[i].setPolyphonic(true);
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
    for(uint8_t i=0; i < _distributors.size(); i++)
    {   
        if((_distributors[i].getChannels() & (1 << _msgChannel)) != (0))
            _distributors[i].processMessage(message);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//Manage Distributors
////////////////////////////////////////////////////////////////////////////////////////////////////

void MessageHandler::addDistributor()
{
    Distributor newDistributor = Distributor(_ptrInstrumentController);
    _distributors.push_back(newDistributor);
}

void addDistributor(uint8_t data[])
{
    //Not Yet Implemented
}

void addDistributor(Distributor distributor)
{
    //Not Yet Implemented
}

void removeDistributor(uint8_t id)
{
    //Not Yet Implemented
}

Distributor* MessageHandler::getDistributor(uint8_t index){
    return &(_distributors[index]);
}