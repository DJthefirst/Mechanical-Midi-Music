#include "Extras/AddrLED.h"
#include "Instruments/Default/PwmDriverStacked.h"
#include "Instruments/InterruptTimer.h"
#include "Arduino.h"
#include <bitset>

constexpr uint8_t VELOCITY_STEP = (127 / NUM_SUBINSTRUMENTS);

//[Instrument][ActiveNote] MSB is set if note is Active the 7 LSBs are the Notes Value 
static std::array<uint8_t,NUM_INSTRUMENTS> m_activeNotes;
static std::array<uint8_t,NUM_INSTRUMENTS> m_activeNoteVelocities;
static uint8_t m_numActiveNotes;

//Instrument Attributes
static std::array<uint16_t,MAX_NUM_INSTRUMENTS> m_notePeriod;  //Base Note
static std::array<uint16_t,MAX_NUM_INSTRUMENTS> m_activePeriod;//Note Played
static std::array<uint16_t,MAX_NUM_INSTRUMENTS> m_currentTick; //Timeing
static std::bitset<MAX_NUM_INSTRUMENTS> m_currentState; //IO

PwmDriverStacked::PwmDriverStacked()
{
    //Setup pins
    for(uint8_t i=0; i < pins.size(); i++){
        pinMode(pins[i], OUTPUT);
    }

    //Setup FAST LED
    setupLEDs();

    delay(500); // Wait a half second for safety

    // Setup timer to handle interrupts for driving the instrument
    InterruptTimer::initialize(TIMER_RESOLUTION, Tick);

    //Initalize Default values
    std::fill_n(m_pitchBend, MAX_NUM_INSTRUMENTS, MIDI_CTRL_CENTER);
}

void PwmDriverStacked::reset(uint8_t instrument)
{
    //Not Yet Implemented
}

void PwmDriverStacked::resetAll()
{
    stopAll();
}

void PwmDriverStacked::playNote(uint8_t group, uint8_t note, uint8_t velocity,  uint8_t channel)
{

    //Use MSB in note to indicate if a note is active.
    //If Note inactive (MSB == 0) Update active notes, notePeriod and activePeriod
    
    //Remove this if statement condition for note overwrite
    //if((m_activeNotes[instrument] & MSB_BITMASK) == 0){
        m_activeNotes[group] = (MSB_BITMASK | note);
        m_activeNoteVelocities[group] = (MSB_BITMASK | velocity);

        for(int i = 0; i < NUM_SUBINSTRUMENTS; ++i){
            if(velocity < VELOCITY_STEP*i) break;

            uint8_t instrument = (group*NUM_SUBINSTRUMENTS)+i;
            if(instrument >= MAX_NUM_INSTRUMENTS) break;
            
            m_notePeriod[instrument] = NOTE_TICKS_DOUBLE[note];
            double bendDeflection = ((double)m_pitchBend[instrument] - (double)MIDI_CTRL_CENTER) / (double)MIDI_CTRL_CENTER;
            m_activePeriod[instrument] = NOTE_TICKS_DOUBLE[note] / pow(2.0, BEND_OCTAVES * bendDeflection);
            m_numActiveNotes++;
            setInstumentLedOn(instrument, channel, note, velocity);
        }

    //}
}

void PwmDriverStacked::stopNote(uint8_t group, uint8_t note, uint8_t velocity)
{
    if((m_activeNotes[group] & (~MSB_BITMASK)) == note){
        m_activeNotes[group] = 0;
        m_activeNoteVelocities[group] = 0;

        for(int i = 0; i < NUM_SUBINSTRUMENTS; ++i){

            uint8_t instrument = (group*NUM_SUBINSTRUMENTS)+i;
            if(instrument >= MAX_NUM_INSTRUMENTS)return;

            m_notePeriod[instrument] = 0;
            m_activePeriod[instrument] = 0;
            digitalWrite(pins[instrument], 0);
            m_numActiveNotes--;
            setInstumentLedOff(instrument);
        }
        return;
    }
}

void PwmDriverStacked::stopAll(){
    std::fill_n(m_pitchBend, MAX_NUM_INSTRUMENTS, MIDI_CTRL_CENTER);
    m_numActiveNotes = 0;
    m_activeNotes = {};
    m_notePeriod = {};
    m_activePeriod = {};
    m_currentTick = {};
    m_currentState.reset();

    for(uint8_t i = 0; i < pins.size(); i++){
        digitalWrite(pins[i], LOW);
    }
    resetLEDs();
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
void ICACHE_RAM_ATTR PwmDriverStacked::Tick()
#else
void PwmDriverStacked::tick()
#endif
{
    // Go through every Instrument
    for (int i = 0; i < MAX_NUM_INSTRUMENTS; i++) {
        //if(m_numActiveNotes == 0)return; TODO: Check if any notes are active

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
void ICACHE_RAM_ATTR PwmDriverStacked::togglePin(uint8_t instrument)
#else
void PwmDriverStacked::togglePin(uint8_t instrument)
#endif
{
    //Pulse the control pin
    m_currentState.flip(instrument);
    digitalWrite(pins[instrument], m_currentState[instrument]);
        
}
// #pragma GCC pop_options (Legacy)

////////////////////////////////////////////////////////////////////////////////////////////////////
//Getters and Setters
////////////////////////////////////////////////////////////////////////////////////////////////////

uint8_t PwmDriverStacked::getNumActiveNotes(uint8_t instrument)
{
    return (m_activeNotes[instrument] != 0) ? 1 : 0;
}
 
bool PwmDriverStacked::isNoteActive(uint8_t instrument, uint8_t note)
{
    //Mask lower 7bits and return true if the instument is playing the respective note.
    return ((m_activeNotes[instrument] & (~ MSB_BITMASK)) == note);
}

void PwmDriverStacked::setPitchBend(uint8_t instrument, uint16_t bend){
    m_pitchBend[instrument] = bend; 
    if(m_notePeriod[instrument] == 0) return;
    //Calculate Pitch Bend
    double bendDeflection = ((double)bend - (double)MIDI_CTRL_CENTER) / (double)MIDI_CTRL_CENTER;
    m_activePeriod[instrument] = m_notePeriod[instrument] / pow(2.0, BEND_OCTAVES * bendDeflection);

}

////////////////////////////////////////////////////////////////////////////////////////////////////
//FAST LED Helper Functions
////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef EXTRA_ADDRESSABLE_LEDS

    void PwmDriverStacked::setupLEDs(){
        AddrLED::get().setup();
    }

    //Set an Instrument Led to on
    void PwmDriverStacked::setInstumentLedOn(uint8_t instrument, uint8_t channel, uint8_t note, uint8_t velocity){
        CHSV color = AddrLED::get().getColor(instrument, channel, note, velocity);
        AddrLED::get().turnLedOn(instrument, color);
    }

    //Set an Instrument Led to off
    void PwmDriverStacked::setInstumentLedOff(uint8_t instrument){
        AddrLED::get().turnLedOff(instrument);
    }

    //Reset Leds
    void PwmDriverStacked::resetLEDs(){
        AddrLED::get().reset();
    }

#else
void PwmDriverStacked::setupLEDs(){}
void PwmDriverStacked::setInstumentLedOn(uint8_t instrument, uint8_t channel, uint8_t note, uint8_t velocity){}
void PwmDriverStacked::setInstumentLedOff(uint8_t instrument){}
void PwmDriverStacked::resetLEDs(){}
#endif