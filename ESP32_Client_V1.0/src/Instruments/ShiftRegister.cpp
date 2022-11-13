#include "Instruments/ShiftRegister.h"
#include "InterruptTimer.h"
#include "Constants.h"
#include "Arduino.h"

const uint8_t startNote = 32; //Inclusive
const uint8_t endNote = 80; //Exclusive

const uint8_t MAX_NUM_NOTES = endNote - startNote;

static uint8_t m_numActiveNotes;

//Instrument Attributes
static uint16_t m_activeDuration[MAX_NUM_NOTES];//Note Played
static uint8_t m_currentTick[MAX_NUM_NOTES]; //Timeing
static bool m_currentState[MAX_NUM_NOTES]; //IO

ShiftRegister::ShiftRegister()
{
    //Setup pins
    for(uint8_t i=0; i < sizeof(pins); i++){
        pinMode(pins[0], OUTPUT); //Serial Data
        pinMode(pins[1], OUTPUT); //Serial Clock
        pinMode(pins[2], OUTPUT); //Register Clock
    }

    // With all pins setup, let's do a first run reset
    ResetAll();
    delay(500); // Wait a half second for safety

    // Setup timer to handle interrupts for driving the instrument
    InterruptTimer::initialize(TIMER_RESOLUTION, Tick);

}

void ShiftRegister::Reset(uint8_t notePos)
{
    m_activeDuration[notePos] = 0;
    m_currentTick[notePos] = 0;
    m_currentState[notePos] = HIGH;
    updateShiftRegister();
}

void ShiftRegister::ResetAll()
{
    m_numActiveNotes = 0;
    std::fill(&m_activeDuration[0],&m_activeDuration[0]+sizeof(m_activeDuration),0);
    std::fill(&m_currentTick[0],&m_currentTick[0]+sizeof(m_currentTick),0);
    std::fill(&m_currentState[0],&m_currentState[0]+sizeof(m_currentState),LOW);
    updateShiftRegister();
}

void ShiftRegister::PlayNote(uint8_t instrument, uint8_t note, uint8_t velocity)
{
    if((note < startNote) || (note >= endNote)) return;
    uint8_t notePos = note - startNote;

    //Use MSB in note to indicate if a note is active.
    double bendDeflection = ((double)m_pitchBend - (double)MIDI_CTRL_CENTER) / (double)MIDI_CTRL_CENTER;
    m_activeDuration[notePos] = NOTE_ONTIME + (100 * bendDeflection);
    m_currentTick[notePos] = 0;
    m_currentState[notePos] = HIGH;
    updateShiftRegister();
    m_numActiveNotes++;
    return;
    
}

void ShiftRegister::StopNote(uint8_t instrument, uint8_t note, uint8_t velocity)
{
    if((note < startNote) || (note >= endNote)) return;
    uint8_t notePos = note - startNote;
    if (!m_currentState[notePos]) return;

    Reset(notePos);
    m_numActiveNotes--;
    return;
}

void ShiftRegister::StopAll(){
    ResetAll();
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
void ICACHE_RAM_ATTR ShiftRegister::Tick()
#else
void ShiftRegister::tick()
#endif
{
    //Turn of not if its Duration has expired
    for (int i = 0; i < MAX_NUM_NOTES; i++) {
        if(m_numActiveNotes == 0)continue;

        if (m_activeDuration[i] > 0){
            if (m_currentTick[i] >= m_activeDuration[i]) {

                m_activeDuration[i] = 0;
                m_currentTick[i] = 0;
                m_currentState[i] = LOW;
                updateShiftRegister();
                continue;
            }
            m_currentTick[i]++;
        }
    }
}


#ifdef ARDUINO_ARCH_ESP32
void ICACHE_RAM_ATTR ShiftRegister::updateShiftRegister() {
#else
void ShiftRegister::updateShiftRegister(uint8_t instrument) {
#endif

    //Pulse the control pin
    //m_currentState[instrument] = !m_currentState[instrument];
    //digitalWrite(pins[instrument], m_currentState[instrument]);

    for(uint8_t i=0; i < MAX_NUM_NOTES; i++ ){
        digitalWrite(pins[0], m_currentState[i]); //Serial Data
        digitalWrite(pins[1], HIGH); //Serial Clock
        digitalWrite(pins[1],  LOW); //Serial Clock
    }
    digitalWrite(pins[2], HIGH); //Register Clock
    digitalWrite(pins[2], LOW ); //Register Clock
        
}
#pragma GCC pop_options

////////////////////////////////////////////////////////////////////////////////////////////////////
//Getters and Setters
////////////////////////////////////////////////////////////////////////////////////////////////////

uint8_t ShiftRegister::getNumActiveNotes(uint8_t instrument)
{
    return 0;
}
 
bool ShiftRegister::isNoteActive(uint8_t instrument, uint8_t note)
{
    if((note < startNote) || (note >= endNote)) return(false);
    uint8_t notePos = note - startNote;

    return ((m_activeDuration[ note-startNote ] != 0) ? true : false);
}

void ShiftRegister::SetPitchBend(uint8_t instrument, uint16_t bend){
    m_pitchBend = bend;
    
    
    //Calculate Pitch Bend
    double bendDeflection = ((double)bend - (double)MIDI_CTRL_CENTER) / (double)MIDI_CTRL_CENTER;
    for(uint8_t i = 0; i < MAX_NUM_NOTES; i++){
        if (!m_currentState[i]) continue;
        m_activeDuration[i] = NOTE_ONTIME + (bendDeflection * 100);
    }

}