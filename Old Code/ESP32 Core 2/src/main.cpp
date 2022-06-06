#include <Arduino.h>

#include <WiFi.h>

//const char* ssid     = "Datanet";  //Network Name
//const char* password = "Berry143"; //Network Password

const char * ssid = "DavidB";
const char * password = "123456789";
const char* host = "192.168.137.1"; //Device IP


const int port = 8888;
byte dataByte = 0;
boolean Connected = false;
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

void loop() {
  if (client) {
    if (!Connected) {
      client.flush();                         //Clear Buffer
      Serial.println("Client Connected to Host");
      client.println("Hello From :" + WiFi.localIP()); 
      Connected = true;
    } 

    if (client.available() > 0) { 
      
      //Read Incoming Byte Stream
      char thisChar = client.read();
      inData += thisChar; 
      client.write(thisChar); //Echo to Host
      Serial.write(thisChar); //Print to log
    
      // echo to the server what's been received to confirm we have the string
      if (thisChar == '\n'){
        Serial.print("\nreceived:");
        Serial.print(inData);
        inData = "";          
      }
    }
  }
}
