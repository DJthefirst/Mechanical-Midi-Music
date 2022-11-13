/*
 * MoppyUSB.h
 *
 */
#pragma once

#include "Arduino.h"
#include <stdint.h>

#include "Networks\Network.h"

class NetworkUSB : public Network{
    
public:
    NetworkUSB();
    void SetMessageHandler(MessageHandler* messageHandler) override;
    void Begin() override;
    void ReadMessage() override;
    void SendMessage(uint8_t message[], int length) override;

private:
    bool StartUSB();
};