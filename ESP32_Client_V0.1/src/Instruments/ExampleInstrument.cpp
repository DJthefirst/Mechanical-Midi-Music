#include "Instruments/ExampleInstrument.h"
#include "InterruptTimer.h"
#include "Constants.h"
#include "Arduino.h"

//[Instrument][ActiveNote] MSB of note is Active last 7 is Value 
static uint8_t m_activeNotes[MAX_NUM_INSTRUMENTS];
static uint8_t m_numActiveNotes;

//Instrument Attributes
static uint16_t m_notePeriod[MAX_NUM_INSTRUMENTS];  //Base Note
static uint16_t m_activePeriod[MAX_NUM_INSTRUMENTS];//Note Played
static uint8_t m_currentTick[MAX_NUM_INSTRUMENTS]; //Timeing
static bool m_currentState[MAX_NUM_INSTRUMENTS]; //IO

ExampleInstrument::ExampleInstrument()
{
    //Setup pins
    for(uint8_t i=0; i < sizeof(pins); i++){
        pinMode(pins[i], OUTPUT);
    }

    // With all pins setup, let's do a first run reset
    ResetAll();
    delay(500); // Wait a half second for safety

    // Setup timer to handle interrupts for floppy driving
    InterruptTimer::initialize(TIMER_RESOLUTION, Tick);

    //Initalize Default values
    std::fill_n(m_pitchBend, MAX_NUM_INSTRUMENTS, MIDI_CTRL_CENTER);
}

void ExampleInstrument::Reset(uint8_t instrument)
{
    //Not Yet Implemented
}

void ExampleInstrument::ResetAll()
{
    //Not Yet Implemented
}

void ExampleInstrument::PlayNote(uint8_t instrument, uint8_t note, uint8_t velocity)
{

    //Use MSB in note to indicate if a note is active.
    if((m_activeNotes[instrument] & MSB_BITMASK) == 0){
        m_activeNotes[instrument] = (MSB_BITMASK | note);
        m_notePeriod[instrument] = noteDoubleTicks[note];
        double bendDeflection = ((double)m_pitchBend[instrument] - (double)MIDI_CTRL_CENTER) / (double)MIDI_CTRL_CENTER;
        m_activePeriod[instrument] = noteDoubleTicks[note] / pow(2.0, BEND_OCTAVES * bendDeflection);
        m_numActiveNotes++;
        return;
    }
}

void ExampleInstrument::StopNote(uint8_t instrument, uint8_t note, uint8_t velocity)
{
    if((m_activeNotes[instrument] & (~MSB_BITMASK)) == note){
        m_activeNotes[instrument] = 0;
        m_notePeriod[instrument] = 0;
        m_activePeriod[instrument] = 0;
        digitalWrite(pins[instrument], 0);
        m_numActiveNotes--;
        return;
    }
}

void ExampleInstrument::StopAll(){
    m_numActiveNotes = 0;
    std::fill(&m_activeNotes[0],&m_activeNotes[0]+sizeof(m_activeNotes),0);
    std::fill(&m_notePeriod[0],&m_notePeriod[0]+sizeof(m_notePeriod),0);
    std::fill(&m_activePeriod[0],&m_activePeriod[0]+sizeof(m_activePeriod),0);
    std::fill(&m_currentTick[0],&m_currentTick[0]+sizeof(m_currentTick),0);
    std::fill(&m_currentState[0],&m_currentState[0]+sizeof(m_currentState),0);

    uint8_t i = 0;
    for(i = 0; (i < sizeof(pins)); i++){
        digitalWrite(pins[i], LOW);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//Tick
////////////////////////////////////////////////////////////////////////////////////////////////////

/*
Called by the timer interrupt at the specified resolution.  Because this is called extremely often,
it's crucial that any computations here be kept to a minimum!

Additionally, the ICACHE_RAM_ATTR helps avoid crashes with WiFi libraries, but may increase speed generally anyway
 */
#pragma GCC push_options
#pragma GCC optimize("Ofast") // Required to unroll this loop, but useful to try to keep this speedy
#ifdef ARDUINO_ARCH_ESP32
void ICACHE_RAM_ATTR ExampleInstrument::Tick()
#else
void ExampleInstrument::tick()
#endif
{
    for (int i = 0; i < MAX_NUM_INSTRUMENTS; i++) {
        if(m_numActiveNotes == 0)continue;

        if (m_activePeriod[i] > 0){
            if (m_currentTick[i] >= m_activePeriod[i]) {
                togglePin(i);

                m_currentTick[i] = 0;
                continue;
            }
            m_currentTick[i]++;
        }
    }
}


#ifdef ARDUINO_ARCH_ESP32
void ICACHE_RAM_ATTR ExampleInstrument::togglePin(uint8_t instrument) {
#else
void ExampleInstrument::togglePin(uint8_t instrument) {
#endif

    //Pulse the control pin
    m_currentState[instrument] = !m_currentState[instrument];
    digitalWrite(pins[instrument], m_currentState[instrument]);
        
}
#pragma GCC pop_options

////////////////////////////////////////////////////////////////////////////////////////////////////
//Getters and Setters
////////////////////////////////////////////////////////////////////////////////////////////////////

uint8_t ExampleInstrument::getNumActiveNotes(uint8_t instrument)
{
    return (m_activeNotes[instrument] != 0) ? 1 : 0;
}
 
bool ExampleInstrument::isNoteActive(uint8_t instrument, uint8_t note)
{
    return ((m_activeNotes[instrument] & (~ MSB_BITMASK)) == note);
}

void ExampleInstrument::SetPitchBend(uint8_t instrument, uint16_t bend){
    m_pitchBend[instrument] = bend;
    
    
    if(m_notePeriod[instrument] == 0) return;
    //Calculate Pitch Bend
    double bendDeflection = ((double)bend - (double)MIDI_CTRL_CENTER) / (double)MIDI_CTRL_CENTER;
    m_activePeriod[instrument] = m_notePeriod[instrument] / pow(2.0, BEND_OCTAVES * bendDeflection);

}