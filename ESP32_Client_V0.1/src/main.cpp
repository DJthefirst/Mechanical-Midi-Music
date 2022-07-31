#include <Arduino.h>
#include "NetworkUDP.h"
#include "NetworkUSB.h"
#include "MessageHandler.h"
#include "Distributor.h"

#include "Instruments/InstrumentController.h"
#include "Instruments/FloppyDrives.h"
#include "Instruments/ExampleInstrument.h"


ExampleInstrument* instrumentController = new(ExampleInstrument);
MessageHandler* messageHandler = new(MessageHandler);
NetworkUSB connection;
//NetworkUDP connection;

void setup() {
  messageHandler -> initalize(instrumentController);
  connection.initalize(messageHandler);
  connection.begin();
  delay(100);

  instrumentController->SetUp();
  //Testing
  messageHandler -> addDistributor();
  Distributor* distributor = messageHandler -> getDistributor(0);
  distributor -> setChannels(0xFFFF); // 1-16
  distributor -> setInstruments(0x0000000F); // 1-4
}

void loop() {
  connection.readMessages();
}