#include <ModbusMaster.h>
#include "Extras/AddrLED.h"
#include "Instruments/Default/Pwm8A04.h"
#include "Instruments/InterruptTimer.h"
#include <Arduino.h>
//#include <HardwareSerial.h>

/*!
  We're using a MAX485-compatible RS485 Transceiver.
  Rx/Tx is hooked up to the hardware serial port at 'Serial'.
  The Data Enable and Receiver Enable pins are hooked up as follows:
*/

#define RXD2 16
#define TXD2 17
#define BAUDRATE 9600

//Assign UART1
HardwareSerial MySerial(1);

//[Instrument][ActiveNote] MSB is set if note is Active the 7 LSBs are the Notes Value 
static std::array<uint8_t,MAX_NUM_INSTRUMENTS> m_activeNotes;
static uint8_t m_numActiveNotes;

//Instrument Attributes
static std::array<uint16_t,MAX_NUM_INSTRUMENTS> m_noteFrequency;  //Base Note
static std::array<uint16_t,MAX_NUM_INSTRUMENTS> m_activeFrequency;//Note Played
static std::array<bool,MAX_NUM_INSTRUMENTS> m_currentState; //IO

// instantiate ModbusMaster object
ModbusMaster node;

Pwm8A04::Pwm8A04()
{

    // Modbus communication runs at 115200 baud
    //MySerial.setRxBufferSize(1024);
    //MySerial.begin(9600, SERIAL_8N1, RXD2, TXD2);
    Serial1.begin(9600, SERIAL_8N1, RXD2, TXD2);

    // // Modbus slave ID 1
    node.begin(1, Serial1);

    //Setup FAST LED
    setupLEDs();

    delay(500); // Wait a half second for safety

    //Initalize Default values
    std::fill_n(m_pitchBend, NUM_MIDI_CH, MIDI_CTRL_CENTER);
}

void Pwm8A04::reset(uint8_t instrument)
{
    //Not Yet Implemented
}

void Pwm8A04::resetAll()
{
    stopAll();
}

void Pwm8A04::playNote(uint8_t instrument, uint8_t note, uint8_t velocity,  uint8_t channel)
{

    //Use MSB in note to indicate if a note is active.
    //If Note inactive (MSB == 0) Update active notes, notePeriod and activePeriod
    
    //Remove this if statement condition for note overwrite
    //if((m_activeNotes[instrument] & MSB_BITMASK) == 0){
        m_activeNotes[instrument] = (MSB_BITMASK | note);
        m_noteFrequency[instrument] = noteFrequency[note];
        //double bendDeflection = ((double)m_pitchBend[channel] - (double)MIDI_CTRL_CENTER) / (double)MIDI_CTRL_CENTER;
        //m_activeFrequency[instrument] = noteDoubleTicks[note] / pow(2.0, BEND_OCTAVES * bendDeflection);
        m_activeFrequency[instrument] = noteFrequency[note];
        m_numActiveNotes++;
        setModBusInstrument(instrument,noteFrequency[note]);
        setInstumentLedOn(instrument, channel, note, velocity);
        return;
    //}
}

void Pwm8A04::stopNote(uint8_t instrument, uint8_t note, uint8_t velocity)
{
    if((m_activeNotes[instrument] & (~MSB_BITMASK)) == note){
        m_activeNotes[instrument] = 0;
        m_noteFrequency[instrument] = 0;
        m_activeFrequency[instrument] = 0;

        m_numActiveNotes--;
        setModBusInstrument(instrument,0);
        setInstumentLedOff(instrument);
        return;
    }
}

void Pwm8A04::stopAll(){
    std::fill_n(m_pitchBend, NUM_MIDI_CH, MIDI_CTRL_CENTER);
    m_numActiveNotes = 0;
    m_activeNotes = {};
    m_noteFrequency = {};
    m_activeFrequency = {};
    m_currentState = {};

    for(uint8_t i = 0; i < MAX_NUM_INSTRUMENTS; i++){
        setModBusInstrument(i,0);
    }
    resetLEDs();
}

void Pwm8A04::setModBusInstrument(uint8_t instrument, uint16_t frequency){
    //if(frequency == 0) return;
    node.writeSingleRegister(instrument, frequency);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//Getters and Setters
////////////////////////////////////////////////////////////////////////////////////////////////////

uint8_t Pwm8A04::getNumActiveNotes(uint8_t instrument)
{
    return (m_activeNotes[instrument] != 0) ? 1 : 0;
}
 
bool Pwm8A04::isNoteActive(uint8_t instrument, uint8_t note)
{
    return ((m_activeNotes[instrument] & (~ MSB_BITMASK)) == note);
}

void Pwm8A04::setPitchBend(uint8_t instrument, uint16_t bend, uint8_t channel){
    m_pitchBend[channel] = bend; 
    if(m_noteFrequency[instrument] == 0) return;
    //Calculate Pitch Bend
    //double bendDeflection = ((double)bend - (double)MIDI_CTRL_CENTER) / (double)MIDI_CTRL_CENTER;
    //m_activePeriod[instrument] = m_notePeriod[instrument] / pow(2.0, BEND_OCTAVES * bendDeflection);

}

////////////////////////////////////////////////////////////////////////////////////////////////////
//FAST LED Helper Functions
////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef EXTRA_ADDRESSABLE_LEDS

void Pwm8A04::setupLEDs(){
    AddrLED::get().setup();
}

//Set an Instrument Led to on
void Pwm8A04::setInstumentLedOn(uint8_t instrument, uint8_t channel, uint8_t note, uint8_t velocity){
    CHSV color = AddrLED::get().getColor(instrument, channel, note, velocity);
    AddrLED::get().turnLedOn(instrument, color);
}

//Set an Instrument Led to off
void Pwm8A04::setInstumentLedOff(uint8_t instrument){
    AddrLED::get().turnLedOff(instrument);
}

//Reset Leds
void Pwm8A04::resetLEDs(){
    AddrLED::get().reset();
}

#else
void Pwm8A04::setupLEDs(){}
void Pwm8A04::setInstumentLedOn(uint8_t instrument, uint8_t channel, uint8_t note, uint8_t velocity){}
void Pwm8A04::setInstumentLedOff(uint8_t instrument){}
void Pwm8A04::resetLEDs(){}
#endif