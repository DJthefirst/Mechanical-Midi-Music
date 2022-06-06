#include "MessageHandler.h"
#include "InstrumentController.h"
#include "Constants.h"

MessageHandler::MessageHandler() {}

void MessageHandler::initalize(InstrumentController* instrumentController){

}

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

void MessageHandler::ProcessSysEXE(uint8_t _message[])
{

}