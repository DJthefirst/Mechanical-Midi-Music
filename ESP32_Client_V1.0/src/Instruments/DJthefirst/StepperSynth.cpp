#include "Extras/AddrLED.h"
#include "Instruments/DJthefirst/StepperSynth.h"
#include "Instruments/InterruptTimer.h"
#include "Arduino.h"

enum PIN_Connnections{
    PIN_SHIFTREG_Data = 25,
    PIN_SHIFTREG_Clock = 27,
    PIN_SHIFTREG_Load = 26,
    PIN_LED_Data = 18
};

const uint8_t OFFSET_PINS = 4;

//[Instrument][ActiveNote] MSB is set if note is Active the 7 LSBs are the Notes Value 
static std::array<uint8_t,MAX_NUM_INSTRUMENTS> m_activeNotes;
static uint8_t m_numActiveNotes;

//Instrument Attributes
static std::array<uint16_t,MAX_NUM_INSTRUMENTS> m_notePeriod;  //Base Note
static std::array<uint16_t,MAX_NUM_INSTRUMENTS> m_activePeriod;//Note Played
static std::array<uint16_t,MAX_NUM_INSTRUMENTS> m_currentTick; //Timeing
static std::array<bool,MAX_NUM_INSTRUMENTS> m_currentState; //IO
static std::array<bool,MAX_NUM_INSTRUMENTS> m_outputenabled; //Output Enabled

StepperSynth::StepperSynth()
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
    std::fill_n(m_pitchBend, NUM_MIDI_CH, MIDI_CTRL_CENTER);
}

void StepperSynth::reset(uint8_t instrument)
{
    //Not Yet Implemented
}

void StepperSynth::resetAll()
{
    stopAll();
}

void StepperSynth::playNote(uint8_t instrument, uint8_t note, uint8_t velocity,  uint8_t channel)
{

    //Use MSB in note to indicate if a note is active.
    //If Note inactive (MSB == 0) Update active notes, notePeriod and activePeriod
    
    //Remove this if statement condition for note overwrite
    //if((m_activeNotes[instrument] & MSB_BITMASK) == 0){
        m_outputenabled[instrument] = true;
        updateShiftRegister();
        m_activeNotes[instrument] = (MSB_BITMASK | note);
        m_notePeriod[instrument] = NOTE_TICKS_DOUBLE[note];
        double bendDeflection = ((double)m_pitchBend[channel] - (double)MIDI_CTRL_CENTER) / (double)MIDI_CTRL_CENTER;
        m_activePeriod[instrument] = NOTE_TICKS_DOUBLE[note] / pow(2.0, BEND_OCTAVES * bendDeflection);
        m_numActiveNotes++;
        setInstumentLedOn(instrument, channel, note, velocity);
        return;
    //}
}

void StepperSynth::stopNote(uint8_t instrument, uint8_t note, uint8_t velocity)
{
    if((m_activeNotes[instrument] & (~MSB_BITMASK)) == note){
        m_outputenabled[instrument] = false;
        updateShiftRegister();
        m_activeNotes[instrument] = 0;
        m_notePeriod[instrument] = 0;
        m_activePeriod[instrument] = 0;
        digitalWrite(pins[instrument + OFFSET_PINS], 0);
        m_numActiveNotes--;
        setInstumentLedOff(instrument);
        return;
    }
}

void StepperSynth::stopAll(){
    std::fill_n(m_pitchBend, NUM_MIDI_CH, MIDI_CTRL_CENTER);
    m_numActiveNotes = 0;
    m_activeNotes = {};
    m_notePeriod = {};
    m_activePeriod = {};
    m_currentTick = {};
    m_currentState = {};
    m_outputenabled = {};

    for(uint8_t i = 0; i < MAX_NUM_INSTRUMENTS; i++){
        digitalWrite(pins[i + OFFSET_PINS], LOW);
    }
    updateShiftRegister();
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
#pragma GCC push_options
#pragma GCC optimize("Ofast") // Required to unroll this loop, but useful to try to keep this speedy
#ifdef ARDUINO_ARCH_ESP32
void ICACHE_RAM_ATTR StepperSynth::Tick()
#else
void StepperSynth::tick()
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
void ICACHE_RAM_ATTR StepperSynth::togglePin(uint8_t instrument)
#else
void StepperSynth::togglePin(uint8_t instrument)
#endif
{
    //Pulse the control pin
    m_currentState[instrument] = !m_currentState[instrument];
    digitalWrite(pins[instrument + OFFSET_PINS], m_currentState[instrument]);
        
}

#ifdef ARDUINO_ARCH_ESP32
void ICACHE_RAM_ATTR StepperSynth::updateShiftRegister() {
#else
void StepperSynth::updateShiftRegister(uint8_t instrument) {
#endif

    // Write and Shift Data
    for(uint8_t i=1; i <= MAX_NUM_INSTRUMENTS; i++ ){
        digitalWrite(PIN_SHIFTREG_Data, !m_outputenabled[MAX_NUM_INSTRUMENTS - i]); //Serial Data (active high)
        digitalWrite(PIN_SHIFTREG_Clock, HIGH); //Serial Clock
        delayMicroseconds(1); //Stabilize 
        digitalWrite(PIN_SHIFTREG_Clock,  LOW); //Serial Clock
    }
    // Toggle Load
    digitalWrite(PIN_SHIFTREG_Load, HIGH); //Register Load
    delayMicroseconds(1); //Stabilize 
    digitalWrite(PIN_SHIFTREG_Load, LOW); //Register Load
    digitalWrite(PIN_SHIFTREG_Data, HIGH); //Serial Data
        
}
#pragma GCC pop_options

////////////////////////////////////////////////////////////////////////////////////////////////////
//Getters and Setters
////////////////////////////////////////////////////////////////////////////////////////////////////

uint8_t StepperSynth::getNumActiveNotes(uint8_t instrument)
{
    return (m_activeNotes[instrument] != 0) ? 1 : 0;
}
 
bool StepperSynth::isNoteActive(uint8_t instrument, uint8_t note)
{
    return ((m_activeNotes[instrument] & (~ MSB_BITMASK)) == note);
}

void StepperSynth::setPitchBend(uint8_t instrument, uint16_t bend, uint8_t channel){
    m_pitchBend[channel] = bend; 
    if(m_notePeriod[instrument] == 0) return;
    //Calculate Pitch Bend
    double bendDeflection = ((double)bend - (double)MIDI_CTRL_CENTER) / (double)MIDI_CTRL_CENTER;
    m_activePeriod[instrument] = m_notePeriod[instrument] / pow(2.0, BEND_OCTAVES * bendDeflection);

}

////////////////////////////////////////////////////////////////////////////////////////////////////
//FAST LED Helper Functions
////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef EXTRA_ADDRESSABLE_LEDS

void StepperSynth::setupLEDs(){
    AddrLED::get().setup();
}

//Set an Instrument Led to on
void StepperSynth::setInstumentLedOn(uint8_t instrument, uint8_t channel, uint8_t note, uint8_t velocity){
    CHSV color = AddrLED::get().getColor(instrument, channel, note, velocity);
    AddrLED::get().turnLedsOn(instrument*5, instrument*5+4, color);
}

//Set an Instrument Led to off
void StepperSynth::setInstumentLedOff(uint8_t instrument){
    AddrLED::get().turnLedsOn(instrument*5, instrument*5+4, CHSV(0,0,0));
}

//Reset Leds
void StepperSynth::resetLEDs(){
    AddrLED::get().reset();
}

#else
void StepperSynth::setupLEDs(){}
void StepperSynth::setInstumentLedOn(uint8_t instrument, uint8_t channel, uint8_t note, uint8_t velocity){}
void StepperSynth::setInstumentLedOff(uint8_t instrument){}
void StepperSynth::resetLEDs(){}
#endif