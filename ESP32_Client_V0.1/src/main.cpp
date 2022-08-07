#include <Arduino.h>
#include "NetworkUDP.h"
#include "NetworkUSB.h"
#include "MessageHandler.h"
#include "Distributor.h"

#include "Instruments/InstrumentController.h"
#include "Instruments/FloppyDrives.h"
#include "Instruments/ExampleInstrument.h"

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
  messageHandler -> initalize(instrumentController);
  connection.initalize(messageHandler);
  connection.begin();
  delay(100);

  instrumentController->SetUp();

  //Testing Demo Setup Config
  messageHandler -> addDistributor();
  Distributor* distributor = messageHandler -> getDistributor(0);
  distributor -> setChannels(0xFFFF); // 1-16
  distributor -> setInstruments(0x0000000F); // 1-4
}

void loop() {
  //Periodicaly Read Incomming Messages
  connection.readMessages();
}