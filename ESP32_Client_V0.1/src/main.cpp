#include <Arduino.h>
#include "NetworkUDP.h"
#include "InstrumentController.h"
#include "MessageHandler.h"

InstrumentController instrumentController;
MessageHandler messageHandler = MessageHandler();
NetworkUDP connection = NetworkUDP();

void setup() {
  messageHandler.initalize(&instrumentController);
  connection.initalize(&messageHandler);
  connection.begin();
  delay(100);
}

void loop() {
  connection.readMessages();
}