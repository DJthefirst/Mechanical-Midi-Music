/*
 * NetworkUDP.cpp
 * 
 * !! Work In Progress !!
 * 
 */

#ifdef MMM_NETWORK_UDP
using networkType = NetworkUDP;

#include "NetworkUDP.h"

#if !defined ARDUINO_ARCH_ESP8266 && !defined ARDUINO_ARCH_ESP32
#else

WiFiUDP MulticastUDP;
WiFiUDP UnicastUDP;


void NetworkUDP::begin() {
    Serial.begin(115200); // For debugging

    // Setup and connect to WiFi
    AsyncWebServer server(80);
    DNSServer dns;
    AsyncWiFiManager wifiManager(&server, &dns);
    wifiManager.autoConnect("FloppyDrives", "m0ppydrives");
    startOTA();
    startUDP();
}


// connect to UDP – returns true if successful or false if not
bool NetworkUDP::startUDP() {
    bool connected = false;

    Serial.println("");
    Serial.println("Connecting to UDP");

#ifdef ARDUINO_ARCH_ESP8266
    if (UnicastUDP.begin(UNICASTPORT) && 
    MulticastUDP.beginMulticast(WiFi.localIP(), IPAddress(224, 5, 6, 7), MOPPY_UDP_PORT) == 1) {
#elif ARDUINO_ARCH_ESP32

    if (UnicastUDP.begin(UNICASTPORT) && 
        MulticastUDP.beginMulticast(IPAddress(224, 5, 6, 7), MULTICASTPORT) == 1)
    {
#endif
        Serial.println("Connection successful");
        connected = true;
    } else {
        Serial.println("Connection failed");
    }

    return connected;
}

void NetworkUDP::startOTA() {
    ArduinoOTA.setPort(8377);
    ArduinoOTA.setPassword("flashdrive");

    ArduinoOTA.onStart([]() {
        String type;
        if (ArduinoOTA.getCommand() == U_FLASH) {
            type = "sketch";
        } else { // U_FS
            type = "filesystem";
        }

        // NOTE: if updating FS this would be the place to unmount FS using FS.end()
        Serial.println("Start updating " + type);
    });
    ArduinoOTA.onEnd([]() {
        Serial.println("\nEnd");
    });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    });
    ArduinoOTA.onError([](ota_error_t error) {
        Serial.printf("Error[%u]: ", error);
        if (error == OTA_AUTH_ERROR) {
            Serial.println("Auth Failed");
        } else if (error == OTA_BEGIN_ERROR) {
            Serial.println("Begin Failed");
        } else if (error == OTA_CONNECT_ERROR) {
            Serial.println("Connect Failed");
        } else if (error == OTA_RECEIVE_ERROR) {
            Serial.println("Receive Failed");
        } else if (error == OTA_END_ERROR) {
            Serial.println("End Failed");
        }
    });
    ArduinoOTA.begin();
}


std::optional<MidiMessage> NetworkUDP::readMessage() {
    // Handle OTA
    ArduinoOTA.handle();

    // Handle UDP packets
    int packetSize = MulticastUDP.parsePacket();
    if (packetSize) {

        //Serial.println("");
        //Serial.print("Received packet of size ");
        //Serial.println(packetSize);
        //Serial.print("From ");
        IPAddress remote = MulticastUDP.remoteIP();
        //for (int i = 0; i < 4; i++) {
        //    Serial.print(remote[i], DEC);
        //    if (i < 3) {
        //        Serial.print(".");
        //    }
        //}
        //Serial.print(", port ");
        //Serial.println(MulticastUDP.remotePort());

        MidiMessage message;

        // read the packet into messageBuffer
        int messageLength = MulticastUDP.read(message.buffer, MOPPY_MAX_PACKET_LENGTH);

        if (messageLength > 1 && messageLength <= 8){
            message.length = messageLength;
            MulticastUDP.flush(); // Just incase we got a really long packet
            return std::optional<MidiMessage>(message);
        }
        else{
            #ifdef DEBUG_UDP_VERBOSE
            Serial.println("PacketSize Out of Scope");
            Serial.println(message.buffer[0]);
            Serial.println(messageLength);
            #endif
        }

        #ifdef DEBUG_UDP_VERBOSE
        // DEBUG - only compile in debug builds
        // for(int i = 0; i < messageLength; i++){
        //     Serial.printf("%02x", (messageBuffer[i]));
        //     Serial.print(" ");
        // }
        Serial.println("");
        #endif

        MulticastUDP.flush(); // Just incase we got a really long packet
    }
    return std::nullopt;
}
    //Old Code
    //String msg = "";
    //Serial.println("Message: ");
    // for(int i = 0; i < 10; i++){
    //     msg += char(message[i]);
    // }
    // if(msg == "Ping"){
    //     Serial.println(msg);
    //     sendPong();
    // }
    // else{
    //     for(int i = 0; i < length; i++){
            //Serial.print(String(message[i]) + " ");
    //        Serial.printf("%02x", (message[i]));
    //        Serial.print(" ");
    //     }
    // }
    //Serial.println();

void NetworkUDP::sendMessage(uint8_t message[], uint8_t length) {

    UnicastUDP.beginPacket(MulticastUDP.remoteIP(), 65535);
    UnicastUDP.write(message, length);
    UnicastUDP.endPacket();
}

#endif /* ARDUINO_ARCH_ESP8266 or ARDUINO_ARCH_ESP32 */
#endif