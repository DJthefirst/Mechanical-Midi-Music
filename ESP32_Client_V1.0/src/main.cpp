/*
 *-------------------------------------Mechanical-Midi-Music------------------------------------------
 *  Version: V1.0
 *  Author: DJthefirst
 *  Description: This Program implements advanced MIDI control over a microcontroller based instrument
 *----------------------------------------------------------------------------------------------------
 */

#include <Arduino.h>
#include "Distributor.h"
#include "MessageHandler.h"
#include "Networks/Network.h"
#include "Networks/NetworkUDP.h"
#include "Networks/NetworkUSB.h"
#include "Networks/NetworkDIN.h"

#include "Instruments/InstrumentController.h"
#include "Instruments/Default/PwmDriver.h"
#include "Instruments/Default/FloppyDrive.h"
#include "Instruments/Default/ShiftRegister.h"

#include "Instruments/DJthefirst/DrumSimple.h"
#include "Instruments/DJthefirst/Dulcimer.h"

#include "Extras/LocalStorage.h"

DEVICE_TYPE instrumentController;
NETWORK_TYPE connection;

MessageHandler messageHandler(&instrumentController);

void setup() {
  connection = NetworkUSB();

  //Initialize MessageHandler and Begin Network Connection
  messageHandler.setNetwork(&connection);
  connection.setMessageHandler(&messageHandler);
  connection.begin();  
  delay(100);

  //TODO move into local Storage?
  #ifdef LOCAL_STORAGE
    //Load Previous Config from memory
    uint8_t data[DEVICE_NUM_NAME_BYTES];
  
    //Device Config
    Device::Name = LocalStorages::localStorage.GetDeviceName(data);
    //Device::OmniMode = (localStorage.GetDeviceBoolean() & BOOL_OMNIMODE) != 0;

    //Distributor Config
    uint8_t numDistributors = LocalStorages::localStorage.GetNumOfDistributors();
    for(uint8_t i = 0; i < numDistributors; i++){
      uint8_t data[DISTRIBUTOR_NUM_CFG_BYTES];
      LocalStorages::localStorage.GetDistributorConstruct(i,data);
      messageHandler.addDistributor(data);
    }
  #endif

  //----Default Distributor Setup Config----//

  // //Distributor 1
  // Distributor distributor1(&instrumentController);
  // distributor1.setChannels(0x0001); // 1
  // distributor1.setInstruments(0x000000FF); // 1-8
  // distributor1.setDistributionMethod(DistributionMethod::RoundRobinBalance);
  // messageHandler.addDistributor(distributor1);

  // //Distributor 2
  // Distributor distributor2(&instrumentController);
  // distributor2.setChannels(0x0002); // 2
  // distributor2.setInstruments(0x000000FF); // 1-8
  // distributor2.setDistributionMethod(DistributionMethod::StraightThrough);
  // messageHandler.addDistributor(distributor2);

  // //Distributor 3
  // Distributor distributor3(&instrumentController);
  // distributor3.setChannels(0x0004); // 3
  // distributor3.setInstruments(0x000000FF); // 1-8
  // distributor3.setDistributionMethod(DistributionMethod::RoundRobin);
  // messageHandler.addDistributor(distributor3);

  // //Distributor 4
  // Distributor distributor4(&instrumentController);
  // distributor4.setChannels(0x0008); // 4
  // distributor4.setInstruments(0x000000FF); // 1-8
  // distributor4.setDistributionMethod(DistributionMethod::Ascending);
  // messageHandler.addDistributor(distributor4);

  //Send Device Ready to Connect
  connection.sendMessage((uint8_t*)&SYSEX_DeviceReady,(uint8_t)1);
}

void loop() {
  //Periodicaly Read Incoming Messages
  connection.readMessage();
}

