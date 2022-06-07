#include <Arduino.h>
#include "NetworkUDP.h"
#include "MessageHandler.h"

#include "Instruments/InstrumentController.h"
#include "Instruments/FloppyDrives.h"


FloppyDrives* instrumentController;
MessageHandler messageHandler = MessageHandler();
NetworkUDP connection = NetworkUDP();

void setup() {
  messageHandler.initalize(instrumentController);
  connection.initalize(&messageHandler);
  connection.begin();
  delay(100);
}

void loop() {
  connection.readMessages();
}