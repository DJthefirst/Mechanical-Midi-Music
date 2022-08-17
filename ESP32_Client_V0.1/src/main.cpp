/*
 *-------------------------------------Mechanical-Midi-Music------------------------------------------
 *  Version: V0.1
 *  Author: DJthefirst
 *  Description: This Program implements advanced MIDI control over a microcontroller based instrument
 *----------------------------------------------------------------------------------------------------
 */

#include <Arduino.h>
#include "MessageHandler.h"
#include "Distributor.h"
#include "Networks/NetworkUDP.h"
#include "Networks/NetworkUSB.h"
#include "Networks/NetworkDIN.h"

#include "Instruments/InstrumentController.h"
#include "Instruments/ExampleInstrument.h"
#include "Instruments/FloppyDrives.h"
//#include "Instruments/PWMDriver.h"
//#include "Instruments/StepperMotors.h"
#include "Instruments/ShiftRegister.h"

//Create a new message handler
MessageHandler* messageHandler = new(MessageHandler);


//---------- Uncomment Your Selected Instrument Type ----------

ExampleInstrument* instrumentController = 
  new(ExampleInstrument);
//new(FloppyDrives);
//new(PWMDriver);
//new(StepperMotors);
//new(ShiftRegister);


//---------- Uncomment Your Selected COM Type ----------

NetworkUSB connection;
//NetworkUDP connection;
//NetworkDIN connection;


void setup() {
  messageHandler -> Initalize(instrumentController);
  connection.Initalize(messageHandler);
  connection.Begin();
  delay(100);

  instrumentController->SetUp();

  //Testing Demo Setup Config
  messageHandler -> AddDistributor();
  Distributor* distributor = messageHandler -> GetDistributor(0);
  distributor -> SetChannels(0xFFFF); // 1-16
  distributor -> SetInstruments(0x0000000F); // 1-4
  distributor -> SetDistributionMethod(StrightThrough);
}

void loop() {
  //Periodicaly Read Incomming Messages
  connection.ReadMessages();
}