#include "MessageHandler.h"
#include "Instruments/InstrumentController.h"
#include "Constants.h"

InstrumentController* _ptrInstrumentController;

MessageHandler::MessageHandler() {}

void MessageHandler::initalize(InstrumentController* ptrInstrumentController){
    _ptrInstrumentController = ptrInstrumentController; //This Line Causes Crash
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//Process Messages
////////////////////////////////////////////////////////////////////////////////////////////////////

void MessageHandler::processMessage(uint8_t message[])
{   
    _msgType = message[0] & 0b11110000;
    _msgChannel = message[0] & 0b00001111;

    if(_msgType == SysCommon)
    {
        //Messgae type for SysCommon
        switch(_msgChannel){

        case(SysEXE):
            ProcessSysEXE(message);
            break;

        case(SysStop):
            //Not Yet Implemented
            break;

        case(SysReset):
            //Not Yet Implemented
            break;
        }
    }

    else{
        DistributeMessage(message);

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
//Process SysEXE Messages
////////////////////////////////////////////////////////////////////////////////////////////////////

void MessageHandler::ProcessSysEXE(uint8_t _message[])
{
    //Not Yet Implemented
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//Manage Distributors
////////////////////////////////////////////////////////////////////////////////////////////////////

void MessageHandler::addDistributor()
{
    Distributor newDistributor = Distributor(_ptrInstrumentController);
    _distributors.push_back(newDistributor);
}

Distributor* MessageHandler::getDistributor(uint8_t index){
    return &(_distributors[index]);
}