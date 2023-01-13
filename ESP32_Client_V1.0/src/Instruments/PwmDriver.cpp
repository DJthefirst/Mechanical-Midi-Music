#include "Instruments/PwmDriver.h"
#include "InterruptTimer.h"
#include "Constants.h"
#include "Arduino.h"

//[Instrument][ActiveNote] MSB is set if note is Active the 7 LSBs are the Notes Value 
static std::array<uint8_t,MAX_NUM_INSTRUMENTS> m_activeNotes;
static uint8_t m_numActiveNotes;

//Instrument Attributes
static std::array<uint16_t,MAX_NUM_INSTRUMENTS> m_notePeriod;  //Base Note
static std::array<uint16_t,MAX_NUM_INSTRUMENTS> m_activePeriod;//Note Played
static std::array<uint8_t,MAX_NUM_INSTRUMENTS> m_currentTick; //Timeing
static std::array<bool,MAX_NUM_INSTRUMENTS> m_currentState; //IO

PwmDriver::PwmDriver()
{
    //Setup pins
    for(uint8_t i=0; i < pins.size(); i++){
        pinMode(pins[i], OUTPUT);
    }

    // With all pins setup, let's do a first run reset
    resetAll();
    delay(500); // Wait a half second for safety

    // Setup timer to handle interrupts for driving the instrument
    InterruptTimer::initialize(TIMER_RESOLUTION, Tick);

    //Initalize Default values
    std::fill_n(m_pitchBend, MAX_NUM_INSTRUMENTS, MIDI_CTRL_CENTER);
}

void PwmDriver::reset(uint8_t instrument)
{
    //Not Yet Implemented
}

void PwmDriver::resetAll()
{
    stopAll();
}

void PwmDriver::playNote(uint8_t instrument, uint8_t note, uint8_t velocity)
{

    //Use MSB in note to indicate if a note is active.
    //If Note inactive (MSB == 0) Update active notes, notePeriod and activePeriod
    if((m_activeNotes[instrument] & MSB_BITMASK) == 0){
        m_activeNotes[instrument] = (MSB_BITMASK | note);
        m_notePeriod[instrument] = noteDoubleTicks[note];
        double bendDeflection = ((double)m_pitchBend[instrument] - (double)MIDI_CTRL_CENTER) / (double)MIDI_CTRL_CENTER;
        m_activePeriod[instrument] = noteDoubleTicks[note] / pow(2.0, BEND_OCTAVES * bendDeflection);
        m_numActiveNotes++;
        return;
    }
}

void PwmDriver::stopNote(uint8_t instrument, uint8_t note, uint8_t velocity)
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

void PwmDriver::stopAll(){
    m_numActiveNotes = 0;
    m_activeNotes = {};
    m_notePeriod = {};
    m_activePeriod = {};
    m_currentTick = {};
    m_currentState = {};

    for(uint8_t i = 0; i < pins.size(); i++){
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
void ICACHE_RAM_ATTR PwmDriver::Tick()
#else
void PwmDriver::tick()
#endif
{
    // Go through every Instrument
    for (int i = 0; i < MAX_NUM_INSTRUMENTS; i++) {
        if(m_numActiveNotes == 0)continue;

        //If note active increase tick until period reset and toggle pin
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
void ICACHE_RAM_ATTR PwmDriver::togglePin(uint8_t instrument)
#else
void PwmDriver::togglePin(uint8_t instrument)
#endif
{
    //Pulse the control pin
    m_currentState[instrument] = !m_currentState[instrument];
    digitalWrite(pins[instrument], m_currentState[instrument]);
        
}
#pragma GCC pop_options

////////////////////////////////////////////////////////////////////////////////////////////////////
//Getters and Setters
////////////////////////////////////////////////////////////////////////////////////////////////////

uint8_t PwmDriver::getNumActiveNotes(uint8_t instrument)
{
    return (m_activeNotes[instrument] != 0) ? 1 : 0;
}
 
bool PwmDriver::isNoteActive(uint8_t instrument, uint8_t note)
{
    return ((m_activeNotes[instrument] & (~ MSB_BITMASK)) == note);
}

void PwmDriver::setPitchBend(uint8_t instrument, uint16_t bend){
    m_pitchBend[instrument] = bend;
    
    
    if(m_notePeriod[instrument] == 0) return;
    //Calculate Pitch Bend
    double bendDeflection = ((double)bend - (double)MIDI_CTRL_CENTER) / (double)MIDI_CTRL_CENTER;
    m_activePeriod[instrument] = m_notePeriod[instrument] / pow(2.0, BEND_OCTAVES * bendDeflection);

}