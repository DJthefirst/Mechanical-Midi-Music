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
//#include "Instruments/FloppyDrives.h"
#include "Instruments/StepperL298n.h"
#include "Instruments/StepperMotor.h"
#include "Instruments/ShiftRegister.h"
#include "Instruments/Dulcimer.h"


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

  //Initialize MessageHandler and Begin Network Connection
  messageHandler.setNetwork(&connection);
  connection.setMessageHandler(&messageHandler);
  connection.begin();
  delay(100);

  //----Testing Demo Setup Config----//

  //Distributor 1
  Distributor distributor1(&instrumentController);
  distributor1.setChannels(0x0001); // 1
  distributor1.setInstruments(0x0000007F); // 1,2
  distributor1.setDistributionMethod(Ascending);
  messageHandler.addDistributor(distributor1);
}

void loop() {
  //Periodicaly Read Incoming Messages
  connection.readMessage();
}