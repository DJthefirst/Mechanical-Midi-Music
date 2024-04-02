#include "Instruments/Default/ShiftRegister.h"
#include "Instruments/InterruptTimer.h"
#include "Constants.h"
#include "Arduino.h"

const uint8_t startNote = 38; //Inclusive
const uint8_t endNote = 77; //Exclusive

const uint8_t MAX_NUM_NOTES = endNote - startNote;

static uint8_t m_numActiveNotes;

//Instrument Attributes
static std::array<uint16_t,MAX_NUM_NOTES> m_activeDuration; //Note Played
static std::array<uint16_t,MAX_NUM_NOTES> m_currentTick; //Timeing
static std::array<bool,MAX_NUM_NOTES> m_currentState; //IO

ShiftRegister::ShiftRegister()
{
    //Setup pins
    pinMode(pins[0], OUTPUT); //Serial Data
    pinMode(pins[1], OUTPUT); //Serial Clock
    pinMode(pins[2], OUTPUT); //Load Registers
    pinMode(pins[3], OUTPUT); //Reset Serial
    pinMode(pins[4], OUTPUT); //Reset Registers

    // With all pins setup, let's do a first run reset
    this->resetAll();
    delay(500); // Wait a half second for safety

    // Setup timer to handle interrupts for driving the instrument
    InterruptTimer::initialize(TIMER_RESOLUTION, Tick);

}

void ShiftRegister::reset(uint8_t notePos)
{
    m_activeDuration[notePos] = 0;
    m_currentTick[notePos] = 0;
    m_currentState[notePos] = LOW;
    updateShiftRegister();
}

void ShiftRegister::resetAll()
{
    m_numActiveNotes = 0;
    m_activeDuration = {};
    m_currentTick = {};
    m_currentState = {};
    // Reset Shift Registers
    // digitalWrite(pins[3], LOW); //Reset Serial
    // digitalWrite(pins[4], LOW); //Reset Registers
    // delayMicroseconds(1); //Stabilize 
    // digitalWrite(pins[3],  HIGH); //Reset Serial
    // digitalWrite(pins[4],  HIGH); //Reset Registers
    updateShiftRegister();
}

void ShiftRegister::playNote(uint8_t instrument, uint8_t note, uint8_t velocity, uint8_t channel)
{
    if((note < startNote) || (note >= endNote)) return;
    uint8_t notePos = (note - startNote);

    m_activeDuration[notePos] = NOTE_ONTIME;
    m_currentTick[notePos] = 0;
    m_currentState[notePos] = HIGH;
    updateShiftRegister();
    m_numActiveNotes++;
    return;
    
}

void ShiftRegister::stopNote(uint8_t instrument, uint8_t note, uint8_t velocity)
{
    if((note < startNote) || (note >= endNote)) return;
    uint8_t notePos = note - startNote;
    if (!m_currentState[notePos]) return;
    reset(notePos);
    m_numActiveNotes--;
    return;
}

void ShiftRegister::stopAll(){
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
void ICACHE_RAM_ATTR ShiftRegister::Tick()
#else
void ShiftRegister::tick()
#endif
{
    //Turn off note if its Duration has expired
    for (int i = 0; i < MAX_NUM_NOTES; i++) {
        if(m_numActiveNotes == 0)break;

        if (m_activeDuration[i] > 0){
            if (m_currentTick[i] >= m_activeDuration[i]) {

                m_currentState[i] = LOW;
                m_activeDuration[i] = 0;
                m_currentTick[i] = 0;
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

    // Write and Shift Data
    for(uint8_t i=0; i < MAX_NUM_NOTES; i++ ){
        digitalWrite(pins[0], m_currentState[i]); //Serial Data
        digitalWrite(pins[1], HIGH); //Serial Clock
        delayMicroseconds(1); //Stabilize 
        digitalWrite(pins[1],  LOW); //Serial Clock
    }
    // Toggle Load
    digitalWrite(pins[2], HIGH); //Register Load
    delayMicroseconds(1); //Stabilize 
    digitalWrite(pins[2], LOW); //Register Load
    digitalWrite(pins[0], LOW); //Serial Data
        
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

    return ((m_activeDuration[ notePos ] != 0) ? true : false);
}