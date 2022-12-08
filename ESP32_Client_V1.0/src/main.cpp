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
#include "Instruments/PwmDriver.h"
//#include "Instruments/FloppyDrives.h"
#include "Instruments/StepperL298n.h"
#include "Instruments/StepperMotor.h"
#include "Instruments/ShiftRegister.h"


//---------- Uncomment Your Selected Instrument Type ----------

//FloppyDrives  instrumentController; //Not Yet Implemented
//PwmDriver     instrumentController;
//StepperL298n  instrumentController;
//StepperMotors instrumentController; //Not Yet Implemented
ShiftRegister instrumentController;


//---------- Uncomment Your Selected COM Type ----------

NetworkUSB connection;
//NetworkUDP connection; //Not Yet Implemented
//NetworkDIN connection; //Not Yet Implemented


//Create a new message handler
MessageHandler messageHandler(&instrumentController);


void setup() {
  messageHandler.SetNetwork(&connection);
  connection.SetMessageHandler(&messageHandler);
  connection.Begin();
  delay(100);

  /*Example Cmd Dumps

  Note On
   0x90 0x39 0x39
   903939
  Note Off
   0x80 0x39 0x39
   803939
  Dump Data: SysEx Start, MIDI ID, DevAdr0, DevAdr2, CMD, Value, SysEx End.
   0xF0 0x7D 0x00 0x00 0x01 0x00 0xF7     
   F07D00000100F7
  (Temp) Set Distribution: SysEx Start, MIDI ID, DevAdr0, DevAdr2, CMD, Value, SysEx End.
   0xF0 0x7D 0x00 0x00 0x03 0x02 0xF7
  (Temp) Set Distribution: SysEx Start, MIDI ID, DevAdr0, DevAdr2, CMD, Value, SysEx End.
   0xF0 0x7D 0x00 0x00 0x03 0x03 0xF7
  */

  ////Testing Demo Setup Config////

  //Distributor 1
  Distributor distributor1(&instrumentController);
  distributor1.SetChannels(0x0001); // 1
  distributor1.SetInstruments(0x00000003); // 1,2
  distributor1.SetDistributionMethod(Ascending);
  messageHandler.AddDistributor(distributor1);

  //Distributor 2
  //Distributor distributor2(&instrumentController);
  //distributor2.SetChannels(0x0001); // 2
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