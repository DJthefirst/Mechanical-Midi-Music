#include "Instruments/Default/InstrAD9833.h"
#include "Instruments/InterruptTimer.h"
#include <Arduino.h>
#include <SPI.h>
#include "AD9833.h"
#include "Extras/AddrLED.h"

constexpr uint8_t NUM_AD9833 = 8;
constexpr uint8_t SPI_data = 13;
constexpr uint8_t SPI_clock = 12;

SPIClass * myspi = new SPIClass(VSPI);
bool isSetup = false;

AD9833 AD[NUM_AD9833] =
{
  AD9833(pins[0], myspi),
  AD9833(pins[1], myspi),
  AD9833(pins[2], myspi),
  AD9833(pins[3], myspi),
  AD9833(pins[4], myspi),
  AD9833(pins[5], myspi),
  AD9833(pins[6], myspi),
  AD9833(pins[7], myspi)
};  //  8 devices.

//[Instrument][ActiveNote] MSB is set if note is Active the 7 LSBs are the Notes Value 
static std::array<uint8_t,MAX_NUM_INSTRUMENTS> m_activeNotes;
static uint8_t m_numActiveNotes;

//Instrument Attributes
static std::array<uint16_t,MAX_NUM_INSTRUMENTS> m_noteFrequency;  //Base Note
static std::array<uint16_t,MAX_NUM_INSTRUMENTS> m_activeFrequency;//Note Played
static std::array<bool,MAX_NUM_INSTRUMENTS> m_currentState; //IO

// instantiate ModbusMaster object


InstrAD9833::InstrAD9833()
{

    //Setup FAST LED
    //setupLEDs();

    // With all pins setup, let's do a first run reset
    //resetAll();
    delay(500); // Wait a half second for safety

    //Initalize Default values
    std::fill_n(m_pitchBend, MAX_NUM_INSTRUMENTS, MIDI_CTRL_CENTER);
}

void InstrAD9833::reset(uint8_t instrument)
{
    //Not Yet Implemented
}

void InstrAD9833::resetAll()
{
    stopAll();
}

void InstrAD9833::playNote(uint8_t instrument, uint8_t note, uint8_t velocity,  uint8_t channel)
{

    //Use MSB in note to indicate if a note is active.
    //If Note inactive (MSB == 0) Update active notes, notePeriod and activePeriod
    
    //Remove this if statement condition for note overwrite
    //if((m_activeNotes[instrument] & MSB_BITMASK) == 0){
        m_activeNotes[instrument] = (MSB_BITMASK | note);
        m_noteFrequency[instrument] = noteFrequency[note];
        //double bendDeflection = ((double)m_pitchBend[instrument] - (double)MIDI_CTRL_CENTER) / (double)MIDI_CTRL_CENTER;
        //m_activeFrequency[instrument] = noteDoubleTicks[note] / pow(2.0, BEND_OCTAVES * bendDeflection);
        m_activeFrequency[instrument] = noteFrequency[note];
        m_numActiveNotes++;
        setInstrumentFrequency(instrument,noteFrequency[note]);
        //setInstumentLedOn(instrument, channel, note, velocity);
        return;
    //}
}

void InstrAD9833::stopNote(uint8_t instrument, uint8_t note, uint8_t velocity)
{
    if((m_activeNotes[instrument] & (~MSB_BITMASK)) == note){
        m_activeNotes[instrument] = 0;
        m_noteFrequency[instrument] = 0;
        m_activeFrequency[instrument] = 0;

        m_numActiveNotes--;
        setInstrumentFrequency(instrument,0);
        //setInstumentLedOff(instrument);
        return;
    }
}

void InstrAD9833::stopAll(){
    std::fill_n(m_pitchBend, MAX_NUM_INSTRUMENTS, MIDI_CTRL_CENTER);
    m_numActiveNotes = 0;
    m_activeNotes = {};
    m_noteFrequency = {};
    m_activeFrequency = {};

    for(uint8_t i = 0; i < MAX_NUM_INSTRUMENTS; i++){
        setInstrumentFrequency(i,0);
    }
    //resetLEDs();
}




void InstrAD9833::setInstrumentFrequency(uint8_t instrument, float frequency){
    if(!isSetup){
        myspi->begin(SPI_clock, SPI_data);

        //Begin AD9833s
        for (int i = 0; i < NUM_AD9833; i++) AD[i].begin();

        //Set Initial Frequency
        for (int i = 0; i < NUM_AD9833; i++) AD[i].setFrequency(1000, 0);

        //Set all AD9833s to SquareWave outputs.
        for (int i = 0; i < NUM_AD9833; i++) AD[i].setWave(AD9833_TRIANGLE);

        isSetup = true;
    }
    
    
    if(instrument >= NUM_AD9833) return;
    AD[instrument].setFrequency(frequency, 0);

    if(m_currentState[instrument] == 0){
        AD[instrument].setWave(AD9833_TRIANGLE);
    }
    // if(frequency == 0){
    //     AD[instrument].setPowerMode(AD9833_OFF);
    // }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//Getters and Setters
////////////////////////////////////////////////////////////////////////////////////////////////////

uint8_t InstrAD9833::getNumActiveNotes(uint8_t instrument)
{
    return (m_activeNotes[instrument] != 0) ? 1 : 0;
}
 
bool InstrAD9833::isNoteActive(uint8_t instrument, uint8_t note)
{
    return ((m_activeNotes[instrument] & (~ MSB_BITMASK)) == note);
}

void InstrAD9833::setPitchBend(uint8_t instrument, uint16_t bend){
    m_pitchBend[instrument] = bend; 
    if(m_noteFrequency[instrument] == 0) return;
    //Calculate Pitch Bend
    //double bendDeflection = ((double)bend - (double)MIDI_CTRL_CENTER) / (double)MIDI_CTRL_CENTER;
    //m_activePeriod[instrument] = m_notePeriod[instrument] / pow(2.0, BEND_OCTAVES * bendDeflection);

}

////////////////////////////////////////////////////////////////////////////////////////////////////
//FAST LED Helper Functions
////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef EXTRA_ADDRESSABLE_LEDS

void InstrAD9833::setupLEDs(){
    AddrLED::get().setup();
}

//Set an Instrument Led to on
void InstrAD9833::setInstumentLedOn(uint8_t instrument, uint8_t channel, uint8_t note, uint8_t velocity){
    CHSV color = AddrLED::get().getColor(instrument, channel, note, velocity);
    AddrLED::get().turnLedOn(instrument, color);
}

//Set an Instrument Led to off
void InstrAD9833::setInstumentLedOff(uint8_t instrument){
    AddrLED::get().turnLedOff(instrument);
}

//Reset Leds
void InstrAD9833::resetLEDs(){
    AddrLED::get().reset();
}

#else
void InstrAD9833::setupLEDs(){}
void InstrAD9833::setInstumentLedOn(uint8_t instrument, uint8_t channel, uint8_t note, uint8_t velocity){}
void InstrAD9833::setInstumentLedOff(uint8_t instrument){}
void InstrAD9833::resetLEDs(){}
#endif