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
  {
    //Load Previous Config from memory
    uint8_t deviceName[DEVICE_NUM_NAME_BYTES];
  
    //Device Config
    Device::Name = LocalStorages::localStorage.GetDeviceName(deviceName);
    //Device::OmniMode = (localStorage.GetDeviceBoolean() & BOOL_OMNIMODE) != 0;

    //Distributor Config
    uint8_t numDistributors = LocalStorages::localStorage.GetNumOfDistributors();
    for(uint8_t i = 0; i < numDistributors; i++){
      uint8_t distributorData[DISTRIBUTOR_NUM_CFG_BYTES];
      LocalStorages::localStorage.GetDistributorConstruct(i,distributorData);
      messageHandler.addDistributor(distributorData);
    }
  }
  #endif

  //Send Device Ready to Connect
  connection.sendMessage(&SYSEX_DeviceReady,(uint8_t)true);
}

//Periodicaly Read Incoming Messages
void loop() {
  connection.readMessage();
}