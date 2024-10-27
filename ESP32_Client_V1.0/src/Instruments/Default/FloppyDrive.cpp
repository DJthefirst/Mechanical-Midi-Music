#include "Instruments/Default/FloppyDrive.h"
#include "Instruments/InterruptTimer.h"
#include "Constants.h"
#include "Arduino.h"

//[Instrument][ActiveNote] MSB is set if note is Active the 7 LSBs are the Notes Value 
static std::array<uint8_t,MAX_NUM_INSTRUMENTS> m_activeNotes;
static uint8_t m_numActiveNotes;

//Instrument Attributes
static std::array<uint16_t,MAX_NUM_INSTRUMENTS> m_notePeriod;  //Base Note
static std::array<uint16_t,MAX_NUM_INSTRUMENTS> m_activePeriod;//Note Played
static std::array<uint16_t,MAX_NUM_INSTRUMENTS> m_currentTick; //Timeing
static std::array<bool,MAX_NUM_INSTRUMENTS> m_pinStateStep; //IO For Step Pins (EVEN)
static std::array<bool,MAX_NUM_INSTRUMENTS> m_pinStateDir; //IO For Direction Pins (ODD)

static std::array<uint16_t,MAX_NUM_INSTRUMENTS> m_headPosition; //Position of Drive Head
static const uint16_t m_minHeadPos = 0; 
static const uint16_t m_maxHeadPos = 150; // 79 Tracks*2 for 3.5in or 49*2 for 5.25in

FloppyDrive::FloppyDrive()
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

    //Initalize Default values
    std::fill_n(m_pitchBend, MAX_NUM_INSTRUMENTS, MIDI_CTRL_CENTER);
}

void FloppyDrive::reset(uint8_t instrument)
{
    //Not Yet Implemented
}

void FloppyDrive::resetAll()
{
    stopAll();

    //Home all Stepper Motors
    bool pinState = LOW;
    for(uint16_t i = 0; i < m_maxHeadPos; i++){
        //Toggle all Step pins (ODD)
        for(uint8_t j = 0; j < pins.size()/2; j++){
            digitalWrite(pins[(j*2)+1], pinState);
        }
        delayMicroseconds(500);
    }

    for(uint8_t i = 0; i < pins.size()/2; i++){
        digitalWrite(pins[i*2], LOW);
    }
}

void FloppyDrive::playNote(uint8_t instrument, uint8_t note, uint8_t velocity, uint8_t channel)
{

    //Use MSB in note to indicate if a note is active.
    //If Note inactive (MSB == 0) Update active notes, notePeriod and activePeriod
    
    //Remove this if statement condition for note overwrite
    //if((m_activeNotes[instrument] & MSB_BITMASK) == 0){
        m_activeNotes[instrument] = (MSB_BITMASK | note);
        m_notePeriod[instrument] = NOTE_TICKS_DOUBLE[note];
        double bendDeflection = ((double)m_pitchBend[instrument] - (double)MIDI_CTRL_CENTER) / (double)MIDI_CTRL_CENTER;
        m_activePeriod[instrument] = NOTE_TICKS_DOUBLE[note] / pow(2.0, BEND_OCTAVES * bendDeflection);
        m_numActiveNotes++;
        return;
    //}
}

void FloppyDrive::stopNote(uint8_t instrument, uint8_t note, uint8_t velocity)
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

void FloppyDrive::stopAll(){
    m_numActiveNotes = 0;
    std::fill_n(m_pitchBend, MAX_NUM_INSTRUMENTS, MIDI_CTRL_CENTER);
    m_activeNotes = {};
    m_notePeriod = {};
    m_activePeriod = {};
    m_currentTick = {};
    m_pinStateStep = {};
    m_pinStateDir = {};
    m_headPosition = {};

    //Set all Direction pins in the same direction (Even)
    for(uint8_t i = 0; i < pins.size()/2; i++){
        digitalWrite(pins[i*2], LOW);
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
void ICACHE_RAM_ATTR FloppyDrive::Tick()
#else
void FloppyDrive::tick()
#endif
{
    // Go through every Instrument
    for (int i = 0; i < MAX_NUM_INSTRUMENTS; i++) {
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
void ICACHE_RAM_ATTR FloppyDrive::togglePin(uint8_t instrument)
#else
void FloppyDrive::togglePin(uint8_t instrument)
#endif
{
    //Increment/Decrement Head position
    m_pinStateDir[instrument] ? m_headPosition[instrument]-- : m_headPosition[instrument]++;

    //Hybrid Drive Setup
    // //Toggle Direction if the Drive Head is at a limit. #3 being a 5in Floppy
    // if ((m_headPosition[instrument] == m_maxHeadPos) || (m_headPosition[instrument] == m_minHeadPos) || ( instrument == 2 && m_headPosition[instrument] == 80)){
    //     m_pinStateDir[instrument] = !m_pinStateDir[instrument];
    //     digitalWrite(pins[instrument*2+1], m_pinStateDir[instrument]);
    // }

    //Toggle Direction if the Drive Head is at a limit.
    if ((m_headPosition[instrument] == m_maxHeadPos) || (m_headPosition[instrument] == m_minHeadPos)){
        m_pinStateDir[instrument] = !m_pinStateDir[instrument];
        digitalWrite(pins[instrument*2+1], m_pinStateDir[instrument]);
    }

    //Pulse the step pin.
    m_pinStateStep[instrument] = !m_pinStateStep[instrument];
    digitalWrite(pins[instrument*2], m_pinStateStep[instrument]);
        
}
//#pragma GCC pop_options (Legacy)

////////////////////////////////////////////////////////////////////////////////////////////////////
//Getters and Setters
////////////////////////////////////////////////////////////////////////////////////////////////////

uint8_t FloppyDrive::getNumActiveNotes(uint8_t instrument)
{
    return (m_activeNotes[instrument] != 0) ? 1 : 0;
}
 
bool FloppyDrive::isNoteActive(uint8_t instrument, uint8_t note)
{
    return ((m_activeNotes[instrument] & (~ MSB_BITMASK)) == note);
}

void FloppyDrive::setPitchBend(uint8_t instrument, uint16_t bend){
    m_pitchBend[instrument] = bend;
    if(m_notePeriod[instrument] == 0) return;
    //Calculate Pitch Bend
    double bendDeflection = ((double)bend - (double)MIDI_CTRL_CENTER) / (double)MIDI_CTRL_CENTER;
    m_activePeriod[instrument] = m_notePeriod[instrument] / pow(2.0, BEND_OCTAVES * bendDeflection);

}