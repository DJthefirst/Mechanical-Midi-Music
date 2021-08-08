#include <Arduino.h>

#include <WiFi.h>

const char* ssid     = "Datanet";  //Network Name
const char* password = "********"; //Network Password
const char* host = "192.168.1.28"; //Device IP
const int port = 8088;

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

void loop()
{
  if(client.read() != -1){
  Serial.println(client.read());
  client.print(client.read());
  }
}