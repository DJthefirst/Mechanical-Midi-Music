// /*
//  * NetworkBLE.cpp
//  *
//  * Network that supports Serial Communication.
//  * 
//  */

#ifdef MMM_NETWORK_BLE
using networkType = NetworkBLE;

// #include "NetworkBLE.h"
// #include <Arduino.h>
// #include <BLEMidi.h>
// #include <vector>
// #include <array>

// void NetworkBLE::begin() {
//     Serial.begin(115200);
//     startBLE();
// }

// std::vector<MidiMessage> buffer;

// void onNoteOn(uint8_t channel, uint8_t note, uint8_t velocity, uint16_t timestamp)
// {
//     Serial.printf("Received note on : channel %d, note %d, velocity %d (timestamp %dms)\n", channel, note, velocity, timestamp);
//     MidiMessage midiMessage;
//     std::array<uint8_t,3> msg = {MIDI_NoteOn || channel, note, velocity}; 
//     std::copy(msg.data(), msg.data() + msg.size(), midiMessage.buffer);
//     midiMessage.length = msg.size();
//     buffer.push_back(midiMessage);
// }

// void onNoteOff(uint8_t channel, uint8_t note, uint8_t velocity, uint16_t timestamp)
// {
//     Serial.printf("Received note off : channel %d, note %d, velocity %d (timestamp %dms)\n", channel, note, velocity, timestamp);
//     MidiMessage midiMessage;
//     std::array<uint8_t,3> msg = {MIDI_NoteOff || channel, note, velocity}; 
//     std::copy(msg.data(), msg.data() + msg.size(), midiMessage.buffer);
//     midiMessage.length = msg.size();
//     buffer.push_back(midiMessage);
// }

// void onPitchBend(uint8_t channel, uint8_t lsb, uint8_t msb, uint16_t timestamp)
// {
//     //Serial.printf("Received control change : channel %d, controller %d, value %d (timestamp %dms)\n", channel, controller, value, timestamp);
//     MidiMessage midiMessage;
//     std::array<uint8_t,3> msg = {MIDI_PitchBend || channel, lsb, msb}; 
//     std::copy(msg.data(), msg.data() + msg.size(), midiMessage.buffer);
//     midiMessage.length = msg.size();
//     buffer.push_back(midiMessage);
// }

// void connected()
// {
//   Serial.println("Connected");
// }

// // connect to USB – returns true if successful or false if not
// bool NetworkBLE::startBLE() {
//     BLEMidiServer.begin("MMM MIDI device");
//     BLEMidiServer.setOnConnectCallback(connected);
//     BLEMidiServer.setOnDisconnectCallback([](){     // To show how to make a callback with a lambda function
//         Serial.println("Disconnected");
//     });
//     BLEMidiServer.setNoteOnCallback(onNoteOn);
//     BLEMidiServer.setNoteOffCallback(onNoteOff);
//     BLEMidiServer.setPitchBendCallback(onPitchBend);

//     bool connected = true; //TODO add automatic connection
//     return connected;
// }

// MidiMessage NetworkBLE::readMessage() {
//     MidiMessage message = MidiMessage();
//     if(buffer.size() > 0){
//         message = buffer[0];
//         buffer.erase(buffer.begin());
//     }
//     return(message);
// }

// //Send Byte arrays wrapped in SysEx Messages 
// void NetworkBLE::sendMessage(MidiMessage message) {
//     //Write the message to Serial
//     Serial.write(message.buffer, message.length);
// }

// //Serial print Strings for Debug
// void NetworkBLE::sendMessage(String msg) {
//     Serial.println(msg);
// }

#include "NetworkBLE.h"
#include <BLEMIDI_Transport.h>
#include <hardware/BLEMIDI_ESP32_NimBLE.h>
#include <Arduino.h>
#include <vector>
#include <array>

BLEMIDI_CREATE_INSTANCE("MechnicalMidiMusic", MIDI2);
std::vector<MidiMessage> buffer2;

void NoteOnMessage2(byte channel, byte note, byte velocity) {
    MidiMessage midiMessage;
    std::array<uint8_t,3> msg = {MIDI_NoteOn || (channel-1), note, velocity}; 
    std::copy(msg.data(), msg.data() + msg.size(), midiMessage.buffer.data());
    midiMessage.length = msg.size();
    buffer2.push_back(midiMessage);
}

void NoteOffMessage2(byte channel, byte note, byte velocity) {
    MidiMessage midiMessage;
    std::array<uint8_t,3> msg = {MIDI_NoteOff || (channel-1), note, velocity}; 
    std::copy(msg.data(), msg.data() + msg.size(), midiMessage.buffer.data());
    midiMessage.length = msg.size();
    buffer2.push_back(midiMessage);
}

void NetworkBLE::begin() {
    Serial.begin(115200);
    while (!Serial);
    // BLEMIDI.setHandleConnected(OnConnected);
    // BLEMIDI.setHandleDisconnected(OnDisconnected);

    MIDI2.setHandleNoteOff(NoteOffMessage2);
    MIDI2.setHandleNoteOn(NoteOnMessage2);
  
    MIDI2.begin();
}


std::optional<MidiMessage> NetworkBLE::readMessage() {
    MIDI2.read();
    MidiMessage message;
    if (buffer2.size() > 0){
        message = buffer2[0];
        buffer2.erase(buffer2.begin());
    }

    return message;
} 

//Send Byte arrays wrapped in SysEx Messages 
void NetworkBLE::sendMessage(MidiMessage message) {

}

//Serial print Strings for Debug
void NetworkBLE::sendMessage(String msg) {

}

#endif