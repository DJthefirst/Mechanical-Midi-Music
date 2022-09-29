/*
 *-------------------------------------Mechanical-Midi-Music------------------------------------------
 *  Version: V0.2
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


//Create a new message handler
MessageHandler messageHandler(&instrumentController);


//---------- Uncomment Your Selected COM Type ----------

NetworkUSB connection(&messageHandler);
//NetworkUDP connection(&messageHandler);
//NetworkDIN connection(&messageHandler);


void setup() {
  connection.Begin();
  delay(100);

  ////Testing Demo Setup Config////

  //Distributor 1
  Distributor distributor1(&instrumentController);
  distributor1.SetChannels(0x0001); // 1
  distributor1.SetInstruments(0x00000013); // 1,2,5
  distributor1.SetDistributionMethod(RoundRobinBalance);
  messageHandler.AddDistributor(distributor1);

  //Distributor 2
  Distributor distributor2(&instrumentController);
  distributor2.SetChannels(0x0006); // 2,3
  distributor2.SetInstruments(0x0000006C); // 3,4,6,7
  distributor2.SetDistributionMethod(Ascending);
  messageHandler.AddDistributor(distributor2);
}

void loop() {
  //Periodicaly Read Incomming Messages
  connection.ReadMessages();
}