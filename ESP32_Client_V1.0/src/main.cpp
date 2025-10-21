/*
 *-------------------------------------Mechanical-Midi-Music------------------------------------------
 *  Version: V3.0
 *  Author: DJthefirst
 *  Description: This program implements advanced MIDI control over a microcontroller-based instrument
 *----------------------------------------------------------------------------------------------------
 */

// See Device.h and the Configs folder for device setup.
// Configuration is selected in platformio.ini build_flags section


#include <Arduino.h>
#include "Config.h"
#include "Networks/NetworkManager.h"
#include "Instruments/InstrumentController.h"
#include "MsgHandling/MessageRouter.h"
#include "Distributors/DistributorManager.h"
#include "MsgHandling/SysExMsgHandler.h"
#include "MsgHandling/MidiMsgHandler.h"

//Include the selected Instrument Type
#define STRINGIFY(x) #x
#define INCLUDE_FILE(x) STRINGIFY(x)
  #include INSTRUMENT_TYPE_VALUE
#undef STRINGIFY
#undef INCLUDE_FILE

// Optional features
#ifdef EXTRA_LOCAL_STORAGE
    #include "Extras/LocalStorage.h"
#endif

// Global components
std::shared_ptr<NetworkManager> network;
std::shared_ptr<MessageRouter> messageRouter;
std::shared_ptr<InstrumentControllerBase> instrumentController;
std::shared_ptr<DistributorManager> distributorManager;
std::shared_ptr<SysExMsgHandler> sysExHandler;
std::shared_ptr<MidiMsgHandler> midiMsgHandler;


void setup() {
  // Device::validateConfiguration();

  // Initialize core components with proper dependency injection
  instrumentController = InstrumentController<INSTRUMENT_TYPE>::getInstance();
  Device::InstrumentType = instrumentController->getInstrumentType();

  distributorManager = DistributorManager::getInstance(instrumentController); 

  sysExHandler = std::make_shared<SysExMsgHandler>(distributorManager);
  midiMsgHandler = std::make_shared<MidiMsgHandler>(distributorManager, sysExHandler, instrumentController);

  // Initialize network and message router
  network = CreateNetwork();
  messageRouter = std::make_shared<MessageRouter>(network, midiMsgHandler, sysExHandler, instrumentController);
  
  // Set device changed callbacks to use the router's broadcast function
  sysExHandler->setDeviceChangedCallback([]() {
    if (messageRouter) messageRouter->broadcastDeviceChanged();
  });
  distributorManager->setDeviceChangedCallback([]() {
    if (messageRouter) messageRouter->broadcastDeviceChanged();
  });

  // Begin network communication
  network->begin();  
  delay(100);

  // Initialize device configuration from local storage
  #ifdef EXTRA_LOCAL_STORAGE
    LocalStorage::get().InitializeDeviceConfiguration(*distributorManager);
  #endif

//=== Distributor Configuration For Startup ===

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

  //===========================================

  //Reset All pins to default
  instrumentController->resetAll();

  //Send Device Ready to Connect
  MidiMessage ready(Device::GetDeviceID(), SysEx::Server, SysEx::DeviceReady, nullptr, 0);
  network->sendMessage(ready);
}

//Periodically Read Incoming Messages
void loop() {
  if (messageRouter) {
    messageRouter->processMessages();
  }
}