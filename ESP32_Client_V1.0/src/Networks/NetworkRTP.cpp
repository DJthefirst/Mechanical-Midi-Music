#ifdef MMM_NETWORK_RTP
using networkType = NetworkRTP;

#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiUdp.h>
#include "NetworkRTP.h"
#include <Arduino.h>
#include <vector>
#include <array>

#define SerialMon Serial
#include <AppleMIDI.h>

char ssid[] = "DBLT5"; //  your network SSID (name)
char pass[] = "123456789";    // your network password (use for WPA, or use as key for WEP)

unsigned long t0 = millis();
int8_t isConnected = 0;

APPLEMIDI_CREATE_DEFAULTSESSION_INSTANCE();
std::vector<MidiMessage> buffer;

void NoteOnMessage(byte channel, byte note, byte velocity) {
    MidiMessage midiMessage;
    std::array<uint8_t,3> msg = {MIDI_NoteOn | (channel-1), note, velocity}; 
    std::copy(msg.data(), msg.data() + msg.size(), midiMessage.buffer.data());
    midiMessage.length = msg.size();
    buffer.push_back(midiMessage);
}

void NoteOffMessage(byte channel, byte note, byte velocity) {
    MidiMessage midiMessage;
    std::array<uint8_t,3> msg = {MIDI_NoteOff | (channel-1), note, velocity}; 
    std::copy(msg.data(), msg.data() + msg.size(), midiMessage.buffer.data());
    midiMessage.length = msg.size();
    buffer.push_back(midiMessage);
}

void NetworkRTP::begin() {
    AM_DBG_SETUP(115200);
    AM_DBG(F("Booting"));

    WiFi.begin(ssid, pass);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        AM_DBG(F("Establishing connection to WiFi.."));
    }
    AM_DBG(F("Connected to network"));

    AM_DBG(F("OK, now make sure you an rtpMIDI session that is EnaRTPd"));
    AM_DBG(F("Add device named Arduino with Host"), WiFi.localIP(), "Port", AppleMIDI.getPort(), "(Name", AppleMIDI.getName(), ")");
    AM_DBG(F("Select and then press the Connect button"));
    AM_DBG(F("Then open a MIDI listener and monitor incoming notes"));
    AM_DBG(F("Listen to incoming MIDI commands"));

    MIDI.begin();

    AppleMIDI.setHandleConnected([](const APPLEMIDI_NAMESPACE::ssrc_t & ssrc, const char* name) {
        isConnected++;
        AM_DBG(F("Connected to session"), ssrc, name);
    });
    AppleMIDI.setHandleDisconnected([](const APPLEMIDI_NAMESPACE::ssrc_t & ssrc) {
        isConnected--;
        AM_DBG(F("Disconnected"), ssrc);
    });
    
    MIDI.setHandleNoteOn([](byte channel, byte note, byte velocity) {
        NoteOnMessage(channel,note,velocity);
    });
    MIDI.setHandleNoteOff([](byte channel, byte note, byte velocity) {
        //AM_DBG(F("NoteOff"), note);
        NoteOffMessage(channel,note,velocity);
    });
}


std::optional<MidiMessage> NetworkRTP::readMessage() {
    MIDI.read();
    MidiMessage message;
    if (buffer.size() > 0){
        message = buffer[0];
        buffer.erase(buffer.begin());
        Serial.write(message.buffer.data(), message.length);
    }

    return message;
} 

//Send Byte arrays wrapped in SysEx Messages 
void NetworkRTP::sendMessage(MidiMessage message) {

}

//Serial print Strings for Debug
void NetworkRTP::sendMessage(String msg) {

}

#endif