#include "Instruments/Utility/SwPWM/ESP32_SwPwmBase.h"
#include "Instruments/Utility/InterruptTimer.h"
#include "Instruments/Utility/NoteTable.h"
#include "Arduino.h"
#include <bitset>

//[Instrument][ActiveNote] MSB is set if note is Active the 7 LSBs are the Notes Value 
static std::array<uint8_t,Config::MAX_NUM_INSTRUMENTS> m_activeNotes;
static uint8_t m_numActiveNotes;

//Instrument Attributes
static std::array<uint16_t,Config::MAX_NUM_INSTRUMENTS> m_notePeriod;  //Base Note
static std::array<uint16_t,Config::MAX_NUM_INSTRUMENTS> m_activePeriod;//Note Played
static std::array<uint16_t,Config::MAX_NUM_INSTRUMENTS> m_currentTick; //Timeing
static std::array<uint8_t,Config::MAX_NUM_INSTRUMENTS> m_noteCh; //Midi Channel
static std::bitset<Config::MAX_NUM_INSTRUMENTS> m_currentState; //IO

ESP32_SwPwmBase::ESP32_SwPwmBase()
{
    //Setup pins
    for(uint8_t i=0; i < Config::PINS.size(); i++){
        pinMode(Config::PINS[i], OUTPUT);
    }

    delay(500); // Wait a half second for safety

    // Setup timer to handle interrupts for driving the instrument
    InterruptTimer::initialize(Config::TIMER_RESOLUTION, Tick);

    //Initalize Default values
    std::fill_n(m_pitchBend, NUM_MIDI_CH, MIDI_CTRL_CENTER);
}

void ESP32_SwPwmBase::reset(uint8_t instrument)
{
    //Not Yet Implemented
}

void ESP32_SwPwmBase::resetAll()
{
    stopAll();
}

void ESP32_SwPwmBase::playNote(uint8_t instrument, uint8_t note, uint8_t velocity,  uint8_t channel)
{

    //Use MSB in note to indicate if a note is active.
    //If Note inactive (MSB == 0) Update active notes, notePeriod and activePeriod
    
    //Remove this if statement condition for note overwrite
    //if((m_activeNotes[instrument] & MSB_BITMASK) == 0){
        m_activeNotes[instrument] = (MSB_BITMASK | note);
        m_notePeriod[instrument] = NoteTables::NOTE_TICKS_DOUBLE[note];
        m_activePeriod[instrument] = NoteTables::applyPitchBendToNote(note, m_pitchBend[channel]);
        m_noteCh[instrument] = channel;
        m_numActiveNotes++;
        return;
    //}
}

void ESP32_SwPwmBase::stopNote(uint8_t instrument, uint8_t note, uint8_t velocity)
{
    if((m_activeNotes[instrument] & (~MSB_BITMASK)) == note){
        m_activeNotes[instrument] = 0;
        m_notePeriod[instrument] = 0;
        m_activePeriod[instrument] = 0;
        digitalWrite(Config::PINS[instrument], 0);
        m_noteCh[instrument] = -1; // -1 indicates no channel
        m_numActiveNotes--;
        return;
    }
}

void ESP32_SwPwmBase::stopAll(){
    std::fill_n(m_pitchBend, NUM_MIDI_CH, MIDI_CTRL_CENTER);
    m_noteCh.fill(-1); // -1 indicates no channel
    m_numActiveNotes = 0;
    m_activeNotes = {};
    m_notePeriod = {};
    m_activePeriod = {};
    m_currentTick = {};
    m_currentState.reset();

    for(uint8_t i = 0; i < Config::PINS.size(); i++){
        digitalWrite(Config::PINS[i], LOW);
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
// #pragma GCC push_options (Legacy)
// #pragma GCC optimize("Ofast") // Required to unroll this loop, but useful to try to keep this speedy (Legacy)
#ifdef ARDUINO_ARCH_ESP32
void ICACHE_RAM_ATTR ESP32_SwPwmBase::Tick()
#else
void ESP32_SwPwmBase::tick()
#endif
{
    // Go through every Instrument
    for (int i = 0; i < Config::MAX_NUM_INSTRUMENTS; i++) {
        if(m_numActiveNotes == 0)return;

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
void ICACHE_RAM_ATTR ESP32_SwPwmBase::togglePin(uint8_t instrument)
#else
void ESP32_SwPwmBase::togglePin(uint8_t instrument)
#endif
{
    //Pulse the control pin
    m_currentState.flip(instrument);
    digitalWrite(Config::PINS[instrument], m_currentState[instrument]);
        
}
// #pragma GCC pop_options (Legacy)

////////////////////////////////////////////////////////////////////////////////////////////////////
//Getters and Setters
////////////////////////////////////////////////////////////////////////////////////////////////////

uint8_t ESP32_SwPwmBase::getNumActiveNotes(uint8_t instrument)
{
    return (m_activeNotes[instrument] != 0) ? 1 : 0;
}
 
bool ESP32_SwPwmBase::isNoteActive(uint8_t instrument, uint8_t note)
{
    //Mask lower 7bits and return true if the instument is playing the respective note.
    return ((m_activeNotes[instrument] & (~ MSB_BITMASK)) == note);
}

void ESP32_SwPwmBase::setPitchBend(uint8_t instrument, uint16_t bend, uint8_t channel){
    m_pitchBend[channel] = bend; 
    if(m_notePeriod[instrument] == 0) return;
    if(m_noteCh[instrument] != channel) return;
    m_activePeriod[instrument] = NoteTables::applyPitchBendToNote(m_activeNotes[instrument], bend);

}