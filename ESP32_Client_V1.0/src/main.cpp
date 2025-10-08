/*
 *-------------------------------------Mechanical-Midi-Music------------------------------------------
 *  Version: V2.0
 *  Author: DJthefirst
 *  Description: This Program implements advanced MIDI control over a microcontroller based instrument
 *----------------------------------------------------------------------------------------------------
 */

//See Device.h and the Configs folder for Device Setup.

#include <Arduino.h>

#define DEVICE_CONFIG "Configs/ESP32_PWM.h"
// #define DEVICE_CONFIG "Configs/StepperSynth.h"

#include "Device.h"
#include "Networks/NetworkManager.h"
#include "Instruments/InstrumentController.h"
#include "MsgHandling/MessageRouter.h"

// Optional features
#ifdef EXTRA_LOCAL_STORAGE
    #include "Extras/LocalStorage.h"
#endif

// Global components
std::shared_ptr<NetworkManager> network;
std::shared_ptr<MessageRouter> messageRouter;
InstrumentType instrumentController;

void setup() {

  //Device::validateConfiguration();

  // Initialize network
  network = CreateNetwork();

  // Initialize the specialized handlers
  auto distributorManager = std::make_shared<DistributorManager>(&instrumentController);
  auto sysExHandler = std::make_shared<SysExMsgHandler>(distributorManager);
  auto midiHandler = std::make_shared<MidiMsgHandler>(&instrumentController, distributorManager, sysExHandler);
  
  // Create message router
  messageRouter = std::make_shared<MessageRouter>(network, distributorManager, sysExHandler, midiHandler);
  
  // Set device changed callbacks to use the router's broadcast function
  sysExHandler->setDeviceChangedCallback([]() {
    if (messageRouter) messageRouter->broadcastDeviceChanged();
  });
  distributorManager->setDeviceChangedCallback([]() {
    if (messageRouter) messageRouter->broadcastDeviceChanged();
  });

  //Begin Network Commmunication
  network->begin();  
  delay(100);

  //Initialize Device Configuration from Local Storage
  #ifdef EXTRA_LOCAL_STORAGE
    LocalStorage::get().InitializeDeviceConfiguration(*distributorManager);
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

  //Reset All pins to default
  instrumentController.resetAll();

  //Send Device Ready to Connect
  MidiMessage ready(Device::GetDeviceID(), SYSEX_Server, SYSEX_DeviceReady, nullptr, 0);
  network->sendMessage(ready);
}

//Periodically Read Incoming Messages
void loop() {
  if (messageRouter) {
    messageRouter->processMessages();
  }
}