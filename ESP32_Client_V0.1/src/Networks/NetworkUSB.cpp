/*
 * MoppyUSB.cpp
 *
 */
#include "NetworkUSB.h"
#include "MessageHandler.h"

NetworkUSB::NetworkUSB(){}

void NetworkUSB::Begin() {
    Serial.begin(115200);
    StartUSB();
}


// connect to USB – returns true if successful or false if not
bool NetworkUSB::StartUSB() {
    bool connected = true; //Temporary
    return connected;
}

/*
    Waits for buffer to fill with a new msg (>3 Bytes). The Msg is sent to the msg handler
*/
void NetworkUSB::ReadMessage() {
    int messageLength = Serial.available();

    if (messageLength >= 3){
        m_messageBuffer[0] = Serial.read();
        messageLength = 1;

        //Fills buffer with one whole Msg. Msg heads are denoted by the MSB == 1
        while (Serial.available() && ((Serial.peek() & MSB_BITMASK) == 0)){
            m_messageBuffer[messageLength] = Serial.read();
            messageLength++;
        }

        // if (Serial.available() && (Serial.peek() == MIDI_SysEXEnd)){
        //     m_messageBuffer[messageLength] = Serial.read();
        //     messageLength++;
        // }

        //Filter out incomplete or corrupt msg
        if (messageLength > 1 && messageLength <= 64){
            (*m_ptrMessageHandler).ProcessMessage(m_messageBuffer);
        }
        else{
            //Serial.println("PacketSize Out of Scope");
            //Serial.println(m_messageBuffer[0]);
            //Serial.println(messageLength);
        }
    }
}

void NetworkUSB::SendMessage(uint8_t message[], int length) {
    Serial.write(message,length);
}

 void NetworkUSB::SetMessageHandler(MessageHandler* ptrMessageHandler)
    {
        m_ptrMessageHandler = ptrMessageHandler;
    }