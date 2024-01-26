/*
 *-------------------------------------Mechanical-Midi-Music------------------------------------------
 *  Version: V1.0
 *  Author: DJthefirst
 *  Description: This Program implements advanced MIDI control over a microcontroller based instrument
 *----------------------------------------------------------------------------------------------------
 */

#include <Arduino.h>

#include "Device.h"
#include "Networks/Network.h"
#include "Instruments/InstrumentController.h"
#include "MessageHandler.h"

#include "Extras/LocalStorage.h"

networkType network = networkType();
instrumentType instrumentController = instrumentType(); 
MessageHandler messageHandler(&instrumentController);

void setup() {
  //Begin Network Commmunication
  network.begin();  
  delay(100);

  //TODO move into local Storage?
  #ifdef EXTRA_LOCAL_STORAGE
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
  network.sendMessage(MidiMessage(&SYSEX_DeviceReady,(uint8_t)true));
}

//Periodicaly Read Incoming Messages
void loop() {
  MidiMessage message = network.readMessage();
  if (message.isValid()){
    message = messageHandler.processMessage(message);
  }
  if (message.isValid()){
    network.sendMessage(message);
  }
}