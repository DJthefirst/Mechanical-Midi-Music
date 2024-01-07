#include "Instruments/DJthefirst/DrumSimple.h"
#include "Instruments/InterruptTimer.h"
#include "Constants.h"
#include "Arduino.h"

const uint8_t startNote = 36; //Inclusive
const uint8_t endNote = 44; //Exclusive

const uint8_t MAX_NUM_NOTES = endNote - startNote > MAX_NUM_INSTRUMENTS? MAX_NUM_INSTRUMENTS : endNote - startNote;

static uint8_t m_numActiveNotes;

//Instrument Attributes
static std::array<uint16_t,MAX_NUM_NOTES> m_activeDuration; //Note Played
static std::array<uint16_t,MAX_NUM_NOTES> m_currentTick; //Timeing
static std::array<bool,MAX_NUM_NOTES> m_currentState; //IO

DrumSimple::DrumSimple()
{
    //Setup pins
    for(uint8_t i=0; i < pins.size(); i++){
        pinMode(pins[i], OUTPUT);
    }

    // With all pins setup, let's do a first run reset
    this->resetAll();
    delay(500); // Wait a half second for safety

    // Setup timer to handle interrupts for driving the instrument
    InterruptTimer::initialize(TIMER_RESOLUTION, Tick);

}

void DrumSimple::reset(uint8_t notePos)
{
    m_activeDuration[notePos] = 0;
    m_currentTick[notePos] = 0;
    m_currentState[notePos] = LOW;

    digitalWrite(pins[notePos], LOW);
}

void DrumSimple::resetAll()
{
    m_numActiveNotes = 0;
    m_activeDuration = {};
    m_currentTick = {};
    m_currentState = {};

    for(uint8_t i = 0; i < pins.size(); i++){
        digitalWrite(pins[i], LOW);
    }
}

void DrumSimple::playNote(uint8_t instrument, uint8_t note, uint8_t velocity, uint8_t channel)
{
    if((note < startNote) || (note >= endNote)) return;
    uint8_t notePos = (note - startNote);

    m_activeDuration[notePos] = NOTE_ONTIME;
    m_currentTick[notePos] = 0;
    // m_currentState[notePos] = HIGH;
    // digitalWrite(pins[notePos], HIGH);
    m_currentState[notePos] = !m_currentState[notePos];
    digitalWrite(pins[notePos], m_currentState[notePos]);
    m_numActiveNotes++;
    return;
    
}

void DrumSimple::stopNote(uint8_t instrument, uint8_t note, uint8_t velocity)
{
    if((note < startNote) || (note >= endNote)) return;
    uint8_t notePos = note - startNote;
    if(notePos == 0){
        if (!m_currentState[notePos]) return;
        reset(notePos);
    }
    m_numActiveNotes--;
    return;
}

void DrumSimple::stopAll(){
    resetAll();
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
void ICACHE_RAM_ATTR DrumSimple::Tick()
#else
void DrumSimple::tick()
#endif
{
    //Turn off note if its Duration has expired
    // for (int i = 0; i < MAX_NUM_NOTES; i++) {
    for (int i = 0; i < 1; i++) {
        if(m_numActiveNotes == 0)break;

        if (m_activeDuration[i] > 0){
            if (m_currentTick[i] >= m_activeDuration[i]) {

                m_currentState[i] = LOW;
                m_activeDuration[i] = 0;
                m_currentTick[i] = 0;
                digitalWrite(pins[i], LOW);
                continue;
            }
            m_currentTick[i]++;
        }
    }
}

#pragma GCC pop_options

////////////////////////////////////////////////////////////////////////////////////////////////////
//Getters and Setters
////////////////////////////////////////////////////////////////////////////////////////////////////

uint8_t DrumSimple::getNumActiveNotes(uint8_t instrument)
{
    return 0;
}
 
bool DrumSimple::isNoteActive(uint8_t instrument, uint8_t note)
{
    if((note < startNote) || (note >= endNote)) return(false);
    uint8_t notePos = note - startNote;

    return ((m_activeDuration[ notePos ] != 0) ? true : false);
}