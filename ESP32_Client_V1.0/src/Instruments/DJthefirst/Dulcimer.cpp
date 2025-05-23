#include "Instruments/DJthefirst/Dulcimer.h"

#include "Arduino.h"
#include "Instruments/InterruptTimer.h"
#include "Extras/AddrLED.h"
#include "Constants.h"

const uint8_t startNote = 48; //Inclusive 50
const uint8_t endNote = 84; //Exclusive 98

const uint8_t MAX_NUM_NOTES = endNote - startNote;
const uint8_t MAX_NUM_REGISTERS = 48;

static uint8_t m_numActiveNotes;
// static bool powerEnabled = false;

enum PinName{
    PIN_SerialData,
    PIN_SerialClock,
    PIN_LoadRegister,
    PIN_ResetSerial,
    PIN_ResetRegister,
    //PIN_24vSense
};

//Map Registers to Notes                                      
// const uint8_t RegisterMap[MAX_NUM_REGISTERS] = { 0,  1,  2,  3,  4,  5,  6,  7,
//                                                  8,  9,  10, 11, 12, 13, 14, 15,
//                                                  16, 17, 18, 19, 20, 21, 22, 23,
//                                                  24, 25, 26, 27, 28, 29, 30, 31,
//                                                  32, 33, 34, 35, 36, 37, 38, 39,
//                                                  40, 41, 42, 43, 44, 45, 46, 47};

// Note -> Register                             C   C#  D   D#  E   F   F#  G   G#  A   A#  B
const uint8_t RegisterMap[MAX_NUM_REGISTERS] = {255,255, 14,255, 13,255, 12, 11, 34, 36, 10,  9, //C3 - B3
                                                  8, 37,  0, 35, 15,  2, 39,  3, 38,  4,  5, 29, //C4 - B4
                                                 27,  6, 25,  7, 16, 18, 28, 26, 20, 24, 22, 17, //C5 - B5
                                                 19,255, 21,255, 23,255,255,255,255,255,255,255  //C6 - B6
                                                };


//Instrument Attributes
static std::array<uint16_t,MAX_NUM_REGISTERS> m_activeDuration; //Note Played
static std::array<uint16_t,MAX_NUM_REGISTERS> m_currentTick; //Timeing
static std::array<bool,MAX_NUM_REGISTERS> m_currentState; //IO

Dulcimer::Dulcimer()
{
    //Setup pins
    pinMode(pins[PIN_SerialData], OUTPUT); //Serial Data
    pinMode(pins[PIN_SerialClock], OUTPUT); //Serial Clock
    pinMode(pins[PIN_LoadRegister], OUTPUT); //Load Registers
    pinMode(pins[PIN_ResetSerial], OUTPUT); //Reset Serial
    pinMode(pins[PIN_ResetRegister], OUTPUT); //Reset Registers
    //pinMode(pins[PIN_24vSense], INPUT);

    //Setup FAST LED
    setupLEDs();

    delay(500); // Wait a half second for safety

    // Setup timer to handle interrupts for driving the instrument
    InterruptTimer::initialize(TIMER_RESOLUTION, Tick);

}

void Dulcimer::reset(uint8_t note)
{
    uint8_t notePos = RegisterMap[note - startNote];
    m_activeDuration[notePos] = 0;
    m_currentTick[notePos] = 0;
    m_currentState[notePos] = LOW;
    updateShiftRegister();
}

void Dulcimer::resetAll()
{
    m_numActiveNotes = 0;
    m_activeDuration = {};
    m_currentTick = {};
    m_currentState = {};
    resetLEDs();

    // updateShiftRegister();
    //Reset Shift Registers
    //if(powerEnabled){
        digitalWrite(pins[PIN_ResetRegister], LOW); //Serial Clock
        digitalWrite(pins[PIN_ResetSerial], LOW); //Serial Clock
        delayMicroseconds(1); //Stabilize 
        digitalWrite(pins[PIN_ResetRegister], HIGH); //Serial Clock
        digitalWrite(pins[PIN_ResetSerial], HIGH); //Serial Clock
    //}
}

void Dulcimer::playNote(uint8_t instrument, uint8_t note, uint8_t velocity, uint8_t channel)
{
    if((note < startNote) || (note >= endNote)) return;
    uint8_t notePos = RegisterMap[note - startNote];
    if(notePos == 255)return; //False Notes
    m_activeDuration[notePos] = NOTE_ONTIME;
    m_currentTick[notePos] = 0;
    m_currentState[notePos] = HIGH;
    updateShiftRegister();
    setInstumentLedOn(instrument, channel, notePos, velocity);
    m_numActiveNotes++;
    return;
    
}

void Dulcimer::stopNote(uint8_t instrument, uint8_t note, uint8_t velocity)
{
    if((note < startNote) || (note >= endNote)) return;
    uint8_t notePos = RegisterMap[note - startNote];
    setInstumentLedOff(notePos);
    if (!m_currentState[notePos]) return;
    reset(notePos);
    m_numActiveNotes--;
    return;
}

void Dulcimer::stopAll(){
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
void ICACHE_RAM_ATTR Dulcimer::Tick()
#else
void Dulcimer::tick()
#endif
{
    // if((m_numActiveNotes == 0) || (!powerEnabled)){
    //     powerEnabled = digitalRead(PIN_24vSense);
    //     if(powerEnabled){     
    //         digitalWrite(pins[PIN_ResetRegister], HIGH); //Serial Clock
    //         digitalWrite(pins[PIN_ResetSerial], HIGH); //Serial Clock
    //     }
    //     else{
    //         digitalWrite(pins[PIN_ResetRegister], LOW); //Serial Clock
    //         digitalWrite(pins[PIN_ResetSerial], LOW); //Serial Clock
    //     }
    //     return;
    // }


    if(m_numActiveNotes == 0)return;
    //Turn off note if its Duration has expired
    for (int i = 0; i < MAX_NUM_REGISTERS; i++) {

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
void ICACHE_RAM_ATTR Dulcimer::updateShiftRegister() {
#else
void Dulcimer::updateShiftRegister(uint8_t instrument) {
#endif

    for(uint8_t i=0; i <= MAX_NUM_REGISTERS; i++ ){
        digitalWrite(pins[PIN_SerialData], m_currentState[MAX_NUM_REGISTERS - i]); //Serial Data
        digitalWrite(pins[PIN_SerialClock], HIGH); //Serial Clock
        delayMicroseconds(1); //Stabilize 
        digitalWrite(pins[PIN_SerialClock],  LOW); //Serial Clock
    }

    // Toggle Load
    digitalWrite(pins[PIN_LoadRegister], HIGH); //Register Load
    delayMicroseconds(1); //Stabilize 
    digitalWrite(pins[PIN_LoadRegister], LOW); //Register Load
    digitalWrite(pins[PIN_SerialData], LOW); //Serial Data
        
}
#pragma GCC pop_options

////////////////////////////////////////////////////////////////////////////////////////////////////
//Getters and Setters
////////////////////////////////////////////////////////////////////////////////////////////////////

uint8_t Dulcimer::getNumActiveNotes(uint8_t instrument)
{
    return 1;
}
 
bool Dulcimer::isNoteActive(uint8_t instrument, uint8_t note)
{
    //TODO Fix this for LEDs
    return true;

    // if((note < startNote) || (note >= endNote)) return(false);
    // uint8_t notePos = RegisterMap[note - startNote];

    // return ((m_activeDuration[ note-startNote ] != 0) ? true : false);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//FAST LED Helper Functions
////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef EXTRA_ADDRESSABLE_LEDS

void Dulcimer::setupLEDs(){
    AddrLED::get().setup();
}

//Set an Instrument Led to on
void Dulcimer::setInstumentLedOn(uint8_t instrument, uint8_t channel, uint8_t notePos, uint8_t velocity){
    CHSV color = AddrLED::get().getColor(instrument, channel, notePos, 127);
    AddrLED::get().turnLedOn(notePos*3, color);
}

//Set an Instrument Led to off
void Dulcimer::setInstumentLedOff(uint8_t notePos){
    AddrLED::get().turnLedOff(notePos*3);
}

//Reset Leds
void Dulcimer::resetLEDs(){
    AddrLED::get().reset();
}

#else
void Dulcimer::setupLEDs(){}
void Dulcimer::setInstumentLedOn(uint8_t instrument, uint8_t channel, uint8_t note, uint8_t velocity){}
void Dulcimer::setInstumentLedOff(uint8_t note){}
void Dulcimer::resetLEDs(){}
#endif