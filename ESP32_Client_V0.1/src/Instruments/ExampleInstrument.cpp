#include "Instruments/ExampleInstrument.h"
#include "InterruptTimer.h"
#include "Constants.h"
#include "Arduino.h"

//[Instrument][ActiveNote] MSB of note is Active last 7 is Value 
static uint8_t m_activeNotes[MAX_NUM_INSTRUMENTS][MAX_POLYPHONIC_NOTES];
static uint8_t m_numActiveNotes[MAX_NUM_INSTRUMENTS];

//Instrument Attributes
static uint16_t m_notePeriod[MAX_NUM_INSTRUMENTS][MAX_POLYPHONIC_NOTES];  //Base Note
static uint16_t m_activePeriod[MAX_NUM_INSTRUMENTS][MAX_POLYPHONIC_NOTES];//Note Played
static uint8_t m_currentSlot[MAX_NUM_INSTRUMENTS]; //Polyphonic
static uint8_t m_currentTick[MAX_NUM_INSTRUMENTS]; //Timeing
static bool m_currentState[MAX_NUM_INSTRUMENTS]; //IO

void ExampleInstrument::SetUp()
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
    for(uint8_t i = 0; i < MAX_POLYPHONIC_NOTES; i++){

        //Use MSB in note to indicate if a note is active.
        if((m_activeNotes[instrument][i] & MSB_BITMASK) == 0){
            m_activeNotes[instrument][i] = (MSB_BITMASK | note);
            m_notePeriod[instrument][i] = noteDoubleTicks[note];

            double bendDeflection = ((double)m_pitchBend[instrument] - (double)MIDI_CTRL_CENTER) / (double)MIDI_CTRL_CENTER;
            m_activePeriod[instrument][i] = noteDoubleTicks[note] / pow(2.0, BEND_OCTAVES * bendDeflection);

            m_numActiveNotes[instrument]++;
            return;
        }
    }
}

void ExampleInstrument::StopNote(uint8_t instrument, uint8_t note, uint8_t velocity)
{
    for(uint8_t i = 0; i < MAX_POLYPHONIC_NOTES; i++){
        if((m_activeNotes[instrument][i] & (~MSB_BITMASK)) == note){
            m_activeNotes[instrument][i] = 0;
            m_notePeriod[instrument][i] = 0;
            m_activePeriod[instrument][i] = 0;
            m_numActiveNotes[instrument]--;
            return;
        }
    }
}

void ExampleInstrument::StopAll(){
    std::fill(&m_activeNotes[0][0],&m_activeNotes[0][0]+sizeof(m_activeNotes),0);
    std::fill(&m_numActiveNotes[0],&m_numActiveNotes[0]+sizeof(m_numActiveNotes),0);
    std::fill(&m_notePeriod[0][0],&m_notePeriod[0][0]+sizeof(m_notePeriod),0);
    std::fill(&m_activePeriod[0][0],&m_activePeriod[0][0]+sizeof(m_activePeriod),0);
    std::fill(&m_currentSlot[0],&m_currentSlot[0]+sizeof(m_currentSlot),0);
    std::fill(&m_currentTick[0],&m_currentTick[0]+sizeof(m_currentTick),0);
    std::fill(&m_currentState[0],&m_currentState[0]+sizeof(m_currentState),0);

    uint8_t i = 0;
    for(i = 0; (i < sizeof(pins)); i++){
        digitalWrite(pins[i], LOW);
    }
}

void ExampleInstrument::SetKeyPressure(uint8_t instrument, uint8_t note, uint8_t velocity)
{
    //Not Yet Implemented
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
        if(m_numActiveNotes[i] == 0)continue;

        if(m_currentSlot[i] >= MAX_POLYPHONIC_NOTES) m_currentSlot[i] = 0;

        if (m_activePeriod[i][m_currentSlot[i]] > 0){
            if (m_currentTick[i] >= m_activePeriod[i][m_currentSlot[i]]) {
                togglePin(i);
                m_currentSlot[i]++;
                m_currentTick[i] = 0;
                continue;
            }
        m_currentTick[i]++;
        }
        else{
        m_currentSlot[i]++;
        }
    }
}


#ifdef ARDUINO_ARCH_ESP32
void ICACHE_RAM_ATTR ExampleInstrument::togglePin(uint8_t instrument) {
#else
void ExampleInstrument::togglePin(byte driveNum, byte pin) {
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
    return m_numActiveNotes[instrument];
}
 
bool ExampleInstrument::isNoteActive(uint8_t instrument, uint8_t note)
{
    for(uint8_t n=0; n < MAX_POLYPHONIC_NOTES; n++){

        if ((m_activeNotes[instrument][n] & (~ MSB_BITMASK)) == note){
            Serial.print(" is Active at ");
            Serial.println(n);
            return true;
        }
    }
    Serial.println(" is inActive");
    return false;
}

void ExampleInstrument::SetPitchBend(uint8_t instrument, uint16_t bend){
    m_pitchBend[instrument] = bend;
    
    for(uint8_t i=0; i < MAX_POLYPHONIC_NOTES; i++){
        if(m_notePeriod[instrument][i] == 0) continue;
        //Calculate Pitch Bend
        double bendDeflection = ((double)bend - (double)MIDI_CTRL_CENTER) / (double)MIDI_CTRL_CENTER;
        m_activePeriod[instrument][i] = m_notePeriod[instrument][i] / pow(2.0, BEND_OCTAVES * bendDeflection);

    }
}
void ExampleInstrument::SetProgramChange(uint8_t value){
    //Not Yet Implemented    
}
void ExampleInstrument::SetChannelPressure(uint8_t value){
    //Not Yet Implemented    
}
void ExampleInstrument::SetModulationWheel(uint8_t value){
    //Not Yet Implemented
}
void ExampleInstrument::SetFootPedal(uint8_t value){
    //Not Yet Implemented
}
void ExampleInstrument::SetVolume(uint8_t value){
    //Not Yet Implemented
}
void ExampleInstrument::SetExpression(uint8_t value){
    //Not Yet Implemented
}
void ExampleInstrument::SetEffectCrtl_1(uint8_t value){
    //Not Yet Implemented
}
void ExampleInstrument::SetEffectCrtl_2(uint8_t value){
    //Not Yet Implemented
}