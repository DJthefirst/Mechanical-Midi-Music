#include "MessageHandler.h"
#include "Instruments/InstrumentController.h"
#include "Constants.h"

MessageHandler::MessageHandler() {}

void MessageHandler::initalize(InstrumentController* ptrInstrumentController){
    ptr_instrumentController = ptrInstrumentController;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//Process Messages
////////////////////////////////////////////////////////////////////////////////////////////////////

void MessageHandler::processMessage(uint8_t message[])
{   
    msg_type = message[0] & 0b11110000;
    msg_channel = message[0] & 0b00001111;

    if(msg_type == SysCommon)
    {
        switch(msg_channel){

        case(SysEXE):
            ProcessSysEXE(message);
            break;

        case(SysStop):

            break;

        case(SysReset):

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
    for(uint8_t i=0; i < distributors.size(); i++)
    {
        if((distributors[i].getChannels() & (1 << msg_channel)) != (0))
        {
            distributors[i].processMessage(message);
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//Process SysEXE Messages
////////////////////////////////////////////////////////////////////////////////////////////////////

void MessageHandler::ProcessSysEXE(uint8_t _message[])
{

}

////////////////////////////////////////////////////////////////////////////////////////////////////
//Manage Distributors
////////////////////////////////////////////////////////////////////////////////////////////////////

void MessageHandler::addDistributor()
{
    Distributor newDistributor = Distributor(ptr_instrumentController);
    distributors.push_back(newDistributor);
}

Distributor* MessageHandler::getDistributor(uint8_t index){
    return &(distributors[index]);
}