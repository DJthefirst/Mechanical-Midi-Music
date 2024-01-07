#include "Instruments/Default/StepperL298n.h"
#include "Instruments/InterruptTimer.h"
#include "Constants.h"
#include "Arduino.h"

//[Instrument][ActiveNote] MSB of note is Active last 7 is Value 
static uint8_t m_activeNotes[MAX_NUM_INSTRUMENTS];
static uint8_t m_numActiveNotes;

//Instrument Attributes
static uint16_t m_notePeriod[MAX_NUM_INSTRUMENTS];  //Base Note
static uint16_t m_activePeriod[MAX_NUM_INSTRUMENTS];//Note Played
static uint16_t m_currentTick[MAX_NUM_INSTRUMENTS]; //Timeing

static uint8_t m_currentStep[MAX_NUM_INSTRUMENTS]; //Timeing

 const SteppingMode steppingMode = WaveDrive;

//Stepper Motor Steps By Mode
const uint8_t stepWave[4][4] = {
    {1,0,0,0},
    {0,0,1,0},
    {0,1,0,0},
    {0,0,0,1}};

const uint8_t stepFull[4][4] = {
    {1,0,1,0},
    {0,1,1,0},
    {0,1,0,1},
    {1,0,0,1}};

const uint8_t stepHalf[8][4] = {
    {1,0,0,0},
    {1,0,1,0},
    {0,0,1,0},
    {0,1,1,0},
    {0,1,0,0},
    {0,1,0,1},
    {0,0,0,1},
    {1,0,0,1}};

StepperL298n::StepperL298n()
{
    //Setup pins

    //TODO Fix This

    // for(uint8_t i=0; i < MAX_NUM_INSTRUMENTS; i++){
    //     pinMode(pins[4*i], OUTPUT);
    //     pinMode(pins[4*i+1], OUTPUT);
    //     pinMode(pins[4*i+2], OUTPUT);
    //     pinMode(pins[4*i+3], OUTPUT);
    // }

    // With all pins setup, let's do a first run reset
    resetAll();
    delay(500); // Wait a half second for safety

    // Setup timer to handle interrupts for driving the instrument
    InterruptTimer::initialize(TIMER_RESOLUTION, Tick);

    //Initalize Default values
    std::fill_n(m_pitchBend, MAX_NUM_INSTRUMENTS, MIDI_CTRL_CENTER);
}

void StepperL298n::reset(uint8_t instrument)
{
    m_activeNotes[instrument] = 0;
    m_notePeriod[instrument] = 0;
    m_activePeriod[instrument] = 0;
    digitalWrite(pins[4*instrument], LOW);
    digitalWrite(pins[4*instrument+1], LOW);
    digitalWrite(pins[4*instrument+2], LOW);
    digitalWrite(pins[4*instrument+3], LOW);
}

void StepperL298n::resetAll()
{
    for(uint8_t i = 0; (i < sizeof(pins)/4); i++){
        reset(i);
    }
}

void StepperL298n::playNote(uint8_t instrument, uint8_t note, uint8_t velocity, uint8_t channel)
{

    //Use MSB in note to indicate if a note is active.
    if((m_activeNotes[instrument] & MSB_BITMASK) == 0){
        m_activeNotes[instrument] = (MSB_BITMASK | note);
        m_notePeriod[instrument] = NOTE_TICKS_DOUBLE[note];
        double bendDeflection = ((double)m_pitchBend[instrument] - (double)MIDI_CTRL_CENTER) / (double)MIDI_CTRL_CENTER;
        m_activePeriod[instrument] = NOTE_TICKS_DOUBLE[note] / pow(2.0, BEND_OCTAVES * bendDeflection);
        m_numActiveNotes++;
        return;
    }
}

void StepperL298n::stopNote(uint8_t instrument, uint8_t note, uint8_t velocity)
{
    if((m_activeNotes[instrument] & (~MSB_BITMASK)) == note){
        reset(instrument);
        m_numActiveNotes--;
        return;
    }
}

void StepperL298n::stopAll(){
    m_numActiveNotes = 0;
    resetAll();
    std::fill(&m_currentTick[0],&m_currentTick[0]+sizeof(m_currentTick),0);
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
void ICACHE_RAM_ATTR StepperL298n::Tick()
#else
void StepperL298n::tick()
#endif
{
    for (int i = 0; i < MAX_NUM_INSTRUMENTS; i++) {
        if(m_numActiveNotes == 0)continue;

        //Step StepperMotor Position at Note Frequency.
        if (m_activePeriod[i] > 0){
            if (m_currentTick[i] >= m_activePeriod[i]) {
                stepMotor(i);

                m_currentTick[i] = 0;
                continue;
            }
            m_currentTick[i]++;
        }
    }
}


#ifdef ARDUINO_ARCH_ESP32
void ICACHE_RAM_ATTR StepperL298n::stepMotor(uint8_t instrument)
#else
void StepperL298n::stepMotor(uint8_t instrument)
#endif
{
    switch (steppingMode)
    {
    case WaveDrive:
        digitalWrite(pins[4*instrument+0], stepWave[m_currentStep[instrument]][0]);
        digitalWrite(pins[4*instrument+1], stepWave[m_currentStep[instrument]][1]);
        digitalWrite(pins[4*instrument+2], stepWave[m_currentStep[instrument]][2]);
        digitalWrite(pins[4*instrument+3], stepWave[m_currentStep[instrument]][3]);

        //Increment Step 0-3
        m_currentStep[instrument] = m_currentStep[instrument] >= 3 ? 0 : m_currentStep[instrument]+1;
        break;

    case FullStep:
        digitalWrite(pins[4*instrument+0], stepFull[m_currentStep[instrument]][0]);
        digitalWrite(pins[4*instrument+1], stepFull[m_currentStep[instrument]][1]);
        digitalWrite(pins[4*instrument+2], stepFull[m_currentStep[instrument]][2]);
        digitalWrite(pins[4*instrument+3], stepFull[m_currentStep[instrument]][3]);

        //Increment Step 0-3
        m_currentStep[instrument] = m_currentStep[instrument] >= 3 ? 0 : m_currentStep[instrument]+1;
        break;

    case HalfStep:
        digitalWrite(pins[4*instrument+0], stepHalf[m_currentStep[instrument]][0]);
        digitalWrite(pins[4*instrument+1], stepHalf[m_currentStep[instrument]][1]);
        digitalWrite(pins[4*instrument+2], stepHalf[m_currentStep[instrument]][2]);
        digitalWrite(pins[4*instrument+3], stepHalf[m_currentStep[instrument]][3]);

        //Increment Step 0-7
        m_currentStep[instrument] = m_currentStep[instrument] >= 7 ? 0 : m_currentStep[instrument]+1;
        break;

    }   
}
#pragma GCC pop_options

////////////////////////////////////////////////////////////////////////////////////////////////////
//Getters and Setters
////////////////////////////////////////////////////////////////////////////////////////////////////

uint8_t StepperL298n::getNumActiveNotes(uint8_t instrument)
{
    return (m_activeNotes[instrument] != 0) ? 1 : 0;
}
 
bool StepperL298n::isNoteActive(uint8_t instrument, uint8_t note)
{
    return ((m_activeNotes[instrument] & (~ MSB_BITMASK)) == note);
}

void StepperL298n::setPitchBend(uint8_t instrument, uint16_t bend){
    m_pitchBend[instrument] = bend;
    if(m_notePeriod[instrument] == 0) return;
    //Calculate Pitch Bend
    double bendDeflection = ((double)bend - (double)MIDI_CTRL_CENTER) / (double)MIDI_CTRL_CENTER;
    m_activePeriod[instrument] = m_notePeriod[instrument] / pow(2.0, BEND_OCTAVES * bendDeflection);

}