/*
 *-------------------------------------Mechanical-Midi-Music------------------------------------------
 *  Version: V2.0
 *  Author: DJthefirst
 *  Description: This program implements advanced MIDI control over a microcontroller-based instrument
 *----------------------------------------------------------------------------------------------------
 */

// See Device.h and the Configs folder for device setup.
// Configuration is selected in platformio.ini build_flags section

#include <Arduino.h>
#include <memory>
#include "Networks/NetworkManager.h"
#include "Instruments/InstrumentController.h"
#include "MsgHandling/MessageRouter.h"
#include "Distributors/DistributorManager.h"
#include "MsgHandling/SysExMsgHandler.h"
#include "MsgHandling/MidiMsgHandler.h"

//Include the selected Instrument Type
#define STRINGIFY(x) #x
#define INCLUDE_FILE(x) STRINGIFY(x)
  #include CFG_INSTRUMENT_TYPE_VALUE
#undef STRINGIFY
#undef INCLUDE_FILE

// Optional features
#ifdef CFG_EXTRA_LOCAL_STORAGE
  #include "Extras/LocalStorage/LocalStorageFactory.h"
#endif

// Global components
std::shared_ptr<InstrumentControllerBase> instrumentController;
std::shared_ptr<DistributorManager> distributorManager;
std::unique_ptr<NetworkManager> network;
std::unique_ptr<SysExMsgHandler> sysExHandler;
std::unique_ptr<MidiMsgHandler> midiMsgHandler;
std::unique_ptr<MessageRouter> messageRouter;


void setup() {

  instrumentController = InstrumentController<INSTRUMENT_TYPE>::getInstance();
  Device::InstrumentType = instrumentController->getInstrumentType();

  distributorManager = DistributorManager::getInstance(instrumentController);

  sysExHandler = std::make_unique<SysExMsgHandler>(*distributorManager, *instrumentController);
  midiMsgHandler = std::make_unique<MidiMsgHandler>(*distributorManager, *sysExHandler, *instrumentController);

  network = CreateNetwork();
  messageRouter = std::make_unique<MessageRouter>(*network, *midiMsgHandler, *sysExHandler, *instrumentController);

  sysExHandler->setDeviceChangedCallback([]() {
    if (messageRouter) messageRouter->broadcastDeviceChanged();
  });
  distributorManager->setDeviceChangedCallback([]() {
    if (messageRouter) messageRouter->broadcastDeviceChanged();
  });

  // Begin network communication
  if (network) {
    network->begin();
  }
  delay(100);

  // Initialize device configuration from local storage
  #ifdef CFG_EXTRA_LOCAL_STORAGE
    if (distributorManager) {
      LocalStorageFactory::getInstance().initializeDeviceConfiguration(*distributorManager);
    }
  #endif

//=== Hardcode Distributor Configuration For Startup ===

  // //Distributor 1
  // Distributor distributor1(instrumentController);
  // distributor1.setChannels(0x0001); // 1
  // distributor1.setInstruments(0x000000FF); // 1-8
  // distributor1.setDistributionMethod(DistributionMethod::RoundRobinBalance);
  // distributorManager->addDistributor(std::move(distributor1));

  // //Distributor 2
  // Distributor distributor2(instrumentController);
  // distributor2.setChannels(0x0002); // 2
  // distributor2.setInstruments(0x000000FF); // 1-8
  // distributor2.setDistributionMethod(DistributionMethod::StraightThrough);
  // distributorManager->addDistributor(std::move(distributor2));

  //===========================================

  //Reset All pins to default
  if (instrumentController) {
    instrumentController->resetAll();
  }

  //Send Device Ready to Connect
  if (network) {
    MidiMessage ready(Device::GetDeviceID(), SysEx::Server, SysEx::DeviceReady, nullptr, 0);
    network->sendMessage(ready);
  }
}

//Periodically Read Incoming Messages
void loop() {
  if (messageRouter) {
    messageRouter->processMessages();
  }
  
  // Check for instrument timeouts
  if (instrumentController) {
    instrumentController->periodic();
  }
}