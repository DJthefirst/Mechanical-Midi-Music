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
//NetworkUDP connection(messageHandler);
//NetworkDIN connection(messageHandler);


void setup() {
  connection.Begin();
  delay(100);

  //Testing Demo Setup Config
  Distributor distributor(&instrumentController);
  distributor.SetChannels(0xFFFF); // 1-16
  distributor.SetInstruments(0x0000000F); // 1-4
  distributor.SetDistributionMethod(StraightThrough);
  messageHandler.AddDistributor(distributor);
}

void loop() {
  //Periodicaly Read Incomming Messages
  connection.ReadMessages();
}