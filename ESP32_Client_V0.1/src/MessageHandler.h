//
// Messgae Handler Converts Midi Packets into Commands
//
#pragma once

#include "Arduino.h"
#include <stdint.h>
#include <vector>
#include "Instruments/InstrumentController.h"
#include "Distributor.h"

#define MOPPY_MAX_PACKET_LENGTH 259

/* 
A Class to convert incoming data into MIDI events
*/
class MessageHandler{
private:
    uint8_t _msgType;
    uint8_t _msgChannel;

    std::vector<Distributor> _distributors;

public:
    //Msg Handler 
    MessageHandler();
    void initalize(InstrumentController* instrumentController);
    void processMessage(uint8_t _message[]);

    //Distributors
    void addDistributor();
    void addDistributor(uint8_t data[]);
    void addDistributor(Distributor distributor);

    void removeDistributor(uint8_t id);
    Distributor* getDistributor(uint8_t id);

private:
    bool _OmniMode = false;

    void DistributeMessage(uint8_t message[]);
    void ProcessCC(uint8_t _message[]);
    void ProcessSysEXE(uint8_t _message[]);
 
};