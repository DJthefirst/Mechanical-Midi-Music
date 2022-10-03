/*
 *-------------------------------------Mechanical-Midi-Music------------------------------------------
 *  Version: V0.2
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
#include "Instruments/ExampleInstrument.h"
//#include "Instruments/FloppyDrives.h"
//#include "Instruments/PWMDriver.h"
//#include "Instruments/StepperMotors.h"
//#include "Instruments/ShiftRegister.h"


//---------- Uncomment Your Selected Instrument Type ----------

ExampleInstrument instrumentController;
//FloppyDrives  instrumentController;
//PWMDriver     instrumentController;
//StepperMotors instrumentController;
//ShiftRegister instrumentController;


//---------- Uncomment Your Selected COM Type ----------

 NetworkUSB connection;
//NetworkUDP connection;
//NetworkDIN connection;


//Create a new message handler
MessageHandler messageHandler(&instrumentController);


void setup() {
  messageHandler.SetNetwork(&connection);
  connection.SetMessageHandler(&messageHandler);
  connection.Begin();
  delay(100);

  ////Testing Demo Setup Config////

  //Distributor 1
  Distributor distributor1(&instrumentController);
  distributor1.SetChannels(0x0001); // 1
  distributor1.SetInstruments(0x0000007F); // 1,2
  distributor1.SetDistributionMethod(RoundRobinBalance);
  messageHandler.AddDistributor(distributor1);

  ////Distributor 2
  //Distributor distributor2(&instrumentController);
  //distributor2.SetChannels(0x0002); // 2
  //distributor2.SetInstruments(0x0000000C); // 3,4
  //distributor2.SetDistributionMethod(Ascending);
  //messageHandler.AddDistributor(distributor2);

  ////Distributor 3
  //Distributor distributor3(&instrumentController);
  //distributor3.SetChannels(0x0004); // 3
  //distributor3.SetInstruments(0x00000030); // 5,6
  //distributor3.SetDistributionMethod(Ascending);
  //messageHandler.AddDistributor(distributor3);

  ////Distributor 4
  //Distributor distributor4(&instrumentController);
  //distributor4.SetChannels(0x0008); // 4
  //distributor4.SetInstruments(0x00000040); // 7
  //distributor4.SetDistributionMethod(Ascending);
  //messageHandler.AddDistributor(distributor4);
}

void loop() {
  //Periodicaly Read Incomming Messages
  connection.ReadMessage();
}