//
// Messgae Handler Converts Midi Packets into Commands
//
#pragma once

#include "Arduino.h"
#include <stdint.h>
#include <vector>
#include "InstrumentController.h"
#include "Distributor.h"

#define MOPPY_MAX_PACKET_LENGTH 259

class MessageHandler{
public:
    MessageHandler();
    void initalize(InstrumentController* instrumentController);
    void processMessage(uint8_t _message[]);

    //Distributors
    void AddDistributor();
    void RemoveDistributor();
    void ConfigDistributor();

private:
    uint8_t msg_type;
    uint8_t msg_channel;

    std::vector<Distributor> distributors;

    void DistributeMessage(uint8_t message[]);
    void ProcessSysEXE(uint8_t _message[]);
};