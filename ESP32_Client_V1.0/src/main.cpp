/*
 *-------------------------------------Mechanical-Midi-Music------------------------------------------
 *  Version: V0.5
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
#include "Instruments/PwmDriver.h"

#include "Device.h"
#include "Extras/LocalStorage.h"

//---------- Uncomment Your Selected Device Type ----------

#define PLATFORM_ESP32
//#define PLATFORM_ESP8266
//#define PLATFORM_ARDUINO_UNO
//#define PLATFORM_ARDUINO_MEGA
//#define PLATFORM_ARDUINO_DUE
//#define PLATFORM_ARDUINO_MICRO
//#define PLATFORM_ARDUINO_NANO


//---------- Uncomment Your Selected Instrument Type ----------

//FloppyDrives  instrumentController; //Not Yet Implemented
PwmDriver     instrumentController;
//StepperL298n  instrumentController;
//StepperMotors instrumentController; //Not Yet Implemented
//ShiftRegister instrumentController;
//Dulcimer      instrumentController;


//---------- Uncomment Your Selected COM Type ----------

NetworkUSB connection;
//NetworkUDP connection; //Not Yet Implemented
//NetworkDIN connection; //Not Yet Implemented


//Create a new message handler
MessageHandler messageHandler(&instrumentController);


void setup() {
  connection = NetworkUSB();

  //Initialize MessageHandler and Begin Network Connection
  messageHandler.setNetwork(&connection);
  connection.setMessageHandler(&messageHandler);
  connection.begin();  
  delay(100);

  //Load Previous Config from memory
  uint8_t data[20];
  LocalStorage localStorage = LocalStorage();

  //Device Config
  Device::Name = localStorage.GetDeviceName(data);
  //Device::OmniMode = (localStorage.GetDeviceBoolean() & BOOL_OMNIMODE) != 0;

  //Distributor Config
  uint8_t numDistributors = localStorage.GetNumOfDistributors();
  for(uint8_t i = 0; i < numDistributors; i++){
    uint8_t data[20];
    localStorage.GetDistributorConstruct(i,data);
    messageHandler.addDistributor(data);
  }

  //Reset previous config if needed.
  //localStorage.ResetDeviceConfig();




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

}

void loop() {
  //Periodicaly Read Incoming Messages
  connection.readMessage();
}