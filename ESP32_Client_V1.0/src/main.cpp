/*
 *-------------------------------------Mechanical-Midi-Music------------------------------------------
 *  Version: V2.0
 *  Author: DJthefirst
 *  Description: This Program implements advanced MIDI control over a microcontroller based instrument
 *----------------------------------------------------------------------------------------------------
 */

//See Device.h and the Configs folder for Device Setup.

#include <Arduino.h>

#include "Device.h"
#include "Networks/NetworkManager.h"
#include "Instruments/InstrumentController.h"
#include "MessageHandler.h"

#include "Extras/LocalStorage.h"

// Optional features
#ifdef ENABLE_LOCAL_STORAGE
    // #include "extras/LocalStorage.h"
#endif

std::unique_ptr<MultiNetwork> network = CreateNetwork();
InstrumentType instrumentController; 

MessageHandler messageHandler(&instrumentController);

void setup() {

  //Device::validateConfiguration();

  //Begin Network Commmunication
  network->begin();  
  delay(100);

  //TODO move into local Storage?
  #ifdef EXTRA_LOCAL_STORAGE
  {
    // Ensure NVS is properly initialized
    if (!LocalStorage::get().EnsureNVSInitialized()) {
      // Handle NVS initialization failure - could log error or use defaults
      // For now, continue with default values in Device namespace
    } else {
      //Device Config - only load if NVS is working
      Device::Name = LocalStorage::get().GetDeviceName();
      // Load runtime device ID (if stored) and update Device runtime value
      Device::SetDeviceID(LocalStorage::get().GetDeviceID());
      // Normalize the stored blob to ensure consistent 20-byte space-padded format
      LocalStorage::get().SetDeviceName(Device::Name);
      // Persist runtime device ID back (ensures formatting/consistency)
      LocalStorage::get().SetDeviceID(Device::GetDeviceID());
      //Device::OmniMode = (localStorage.GetDeviceBoolean() & BOOL_OMNIMODE) != 0;

      //Distributor Config
      uint8_t numDistributors = LocalStorage::get().GetNumOfDistributors();
      for(uint8_t i = 0; i < numDistributors; i++){
        uint8_t distributorData[DISTRIBUTOR_NUM_CFG_BYTES];
        LocalStorage::get().GetDistributorConstruct(i,distributorData);
        messageHandler.addDistributor(distributorData);
      }
    }
  }
  #endif

  //----Testing Demo Setup Config----//

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

  //Reset All pins to default
  instrumentController.resetAll();

  //Send Device Ready to Connect
  MidiMessage ready(Device::GetDeviceID(), SYSEX_Server, SYSEX_DeviceReady, nullptr, 0);
  network->sendMessage(ready);
}

//Periodicaly Read Incoming Messages
void loop() {
  std::optional<MidiMessage> message = network->readMessage();

  //If the network returns a new message process it.
  if (!message) return;
  message = messageHandler.processMessage(*message);
  
  //If there is a response send message.
  if (message) network->sendMessage(*message);
}