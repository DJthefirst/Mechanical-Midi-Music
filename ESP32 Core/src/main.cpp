#include <Arduino.h>

#include <WiFi.h>

const char* ssid     = "Datanet";  //Network Name
const char* password = "******"; //Network Password
const char* host = "192.168.1.28"; //Device IP
const int port = 8088;
byte dataByte = 0;

boolean alreadyConnected = false; // whether or not the client was connected previously
String inData;

WiFiClient client;

void setup()
{
  Serial.begin(115200);
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected with IP address: ");
  Serial.println(WiFi.localIP());

    delay(1000);
  Serial.print("connecting to ");
  Serial.println(host);
    delay(500);
    if (!client.connect(host, port)) {
    Serial.println("connection failed");
    return;
  }
}

// void loop(){
//   dataByte = client.read();
//   Serial.println(dataByte);
//   client.print(dataByte);
//   delay(2000);
// }

void loop() {
  // when the client sends the first byte, say hello:
  if (client) {
    if (!alreadyConnected) {
      // clead out the input buffer:
      client.flush();    
      Serial.println("We have a new client");
      client.println("Hello, client!"); 
      alreadyConnected = true;
    } 

    if (client.available() > 0) { 
      // read the bytes incoming from the client:
      char thisChar = client.read();
      inData += thisChar; 
      // echo the bytes back to the client:
      client.write(thisChar);
      // echo the bytes to the server as well:
      Serial.write(thisChar);
    
      // echo to the server what's been received to confirm we have the string
      if (thisChar == '\n'){
        Serial.print("\nreceived:");
        Serial.print(inData);
        inData = "";          
      }
    }
  }
}
