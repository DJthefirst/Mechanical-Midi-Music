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
#include "DistributorManager.h"
#include "SysExMsgHandler.h"
#include "MidiMsgHandler.h"

// Optional features
#ifdef EXTRA_LOCAL_STORAGE
    #include "Extras/LocalStorage.h"
#endif

std::unique_ptr<NetworkManager> network = CreateNetwork();
InstrumentType instrumentController; 

// Create the specialized handlers
std::shared_ptr<DistributorManager> distributorManager;
std::shared_ptr<SysExMsgHandler> sysExHandler;
std::shared_ptr<MidiMsgHandler> midiHandler;

// Current network context for device changed notifications
thread_local INetwork* currentSourceNetwork = nullptr;

// Device changed callback to broadcast notifications to all other networks
void broadcastDeviceChanged() {
    MidiMessage deviceChangedMsg(Device::GetDeviceID(), SYSEX_Broadcast, SYSEX_DeviceChanged, nullptr, 0);
    if (currentSourceNetwork) {
        // Send to all networks except the one that originated the change
        network->sendMessageToOthers(deviceChangedMsg, currentSourceNetwork);
    } else {
        // Send to all networks if no source network (e.g., internal change)
        network->sendMessage(deviceChangedMsg);
    }
}

void setup() {

  //Device::validateConfiguration();

  // Initialize the specialized handlers
  distributorManager = std::make_shared<DistributorManager>(&instrumentController);
  sysExHandler = std::make_shared<SysExMsgHandler>(distributorManager);
  midiHandler = std::make_shared<MidiMsgHandler>(&instrumentController, distributorManager, sysExHandler);
  
  // Set device changed callbacks for both handlers
  sysExHandler->setDeviceChangedCallback(broadcastDeviceChanged);
  distributorManager->setDeviceChangedCallback(broadcastDeviceChanged);

  //Begin Network Commmunication
  network->begin();  
  delay(100);

  //TODO move into local Storage?
  #ifdef EXTRA_LOCAL_STORAGE

  //LocalStorage::get().InitializeDeviceConfiguration(messageHandler);
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
        distributorManager->addDistributor(distributorData);
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

  //Reset All pins to default
  instrumentController.resetAll();

  //Send Device Ready to Connect
  MidiMessage ready(Device::GetDeviceID(), SYSEX_Server, SYSEX_DeviceReady, nullptr, 0);
  network->sendMessage(ready);
}

//Periodicaly Read Incoming Messages
void loop() {
  // Loop through each network individually
  for (size_t i = 0; i < network->numberOfNetworks(); i++) {
    INetwork* currentNetwork = network->getNetwork(i);
    if (!currentNetwork) continue;

    // Read message from this specific network
    std::optional<MidiMessage> message = currentNetwork->readMessage();
    if (!message) continue;

    // Set the current source network for device changed notifications
    currentSourceNetwork = currentNetwork;

    // Process message based on type
    if (message->type() == MIDI_SysCommon && message->sysCommonType() == MIDI_SysEX) {
      // Handle SysEx messages with SysExMsgHandler
      if (sysExHandler) {
        std::optional<MidiMessage> response = sysExHandler->processSysExMessage(*message);
        // Send response back to the originating network only
        if (response) {
          currentNetwork->sendMessage(*response);
        }
      }
    } else {
      // Handle other MIDI messages with MidiMsgHandler
      if (midiHandler) {
        midiHandler->processMessage(*message);
      }
    }

    // Clear the source network context
    currentSourceNetwork = nullptr;
  }
}