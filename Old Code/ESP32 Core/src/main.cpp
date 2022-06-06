// #include <Arduino.h>

// #include <WiFi.h>

// const char* ssid     = "SSID";  //Network Name
// const char* password = "Password"; //Network Password
// const char* host = "IP"; //Device IP
// const int port = 8088;

// boolean Connected = false;
// String charData;
// const int BUFFER_SIZE = 10;
// byte buf[BUFFER_SIZE];

// WiFiClient client;

// void setup()
// {
//   Serial.begin(115200);
//   Serial.print("Connecting to ");
//   Serial.println(ssid);
  
//   WiFi.begin(ssid, password);
  
//   while (WiFi.status() != WL_CONNECTED) {
//     delay(500);
//     Serial.print(".");
//   }
//   Serial.println("");
//   Serial.println("WiFi connected with IP address: ");
//   Serial.println(WiFi.localIP());

//     delay(1000);
//   Serial.print("connecting to ");
//   Serial.println(host);
//     delay(500);
//     if (!client.connect(host, port)) {
//     Serial.println("connection failed");
//     return;
//   }
// }

// void loop() {
//   if (client) {
//     if (!Connected) {
//       client.flush();                         //Clear Buffer
//       Serial.println("Client Connected to Host");
//       client.print("Hello From :" + WiFi.localIP()); 
//       Connected = true;
//     } 


//     if (client.available() > 0) { 

//       //Read Incoming Byte Stream
//       char thisChar = client.read();
//     charData += thisChar; 
//       Serial.write(thisChar); //Print to log

//       // int msgLength = client.read(buf,BUFFER_SIZE);
//       // Serial.print("Length msgLength = ");
//       // Serial.println(msgLength);
      

//       //Print Bytes
//       // for(int i = 0; i < msgLength; i++){ 
//       //   Serial.print(buf[i],BIN);    
//       //   Serial.print(" | ");    
//       //   client.write(buf[i]); //Echo to Host
//       // }
//       // Serial.println();
    

//       //Print Characters
//       if (thisChar == '\n'){
//         Serial.print("\nreceived:");
//         Serial.print(charData);
//       charData = "";          
//       }
//     }
//   }
// }
