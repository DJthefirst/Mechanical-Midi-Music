#include "Instruments/ShiftRegister.h"
#include "InterruptTimer.h"
#include "Constants.h"
#include "Arduino.h"

//[Instrument][ActiveNote] MSB of note is Active last 7 is Value 
static uint8_t _activeNotes[MAX_NUM_INSTRUMENTS][MAX_POLYPHONIC_NOTES];
static uint8_t _numActiveNotes[MAX_NUM_INSTRUMENTS];

//Instrument Attributes
//Setup pins
const uint8_t DATA_PIN =            pins[0]; //2
const uint8_t SHIFT_CLOCK_PIN =     pins[1]; //4
const uint8_t LATCH_PIN =           pins[2]; //16
const uint8_t MASTER_CLEAR_PIN =    pins[3]; //17
const uint8_t OUTPUT_ENABLE_PIN =   pins[4]; //18

// First and last supported notes (any notes outside this range will be ignored, first note will be
// indexed as zero for shifting
#define NUM_NOTES 24 //24
const uint8_t FIRST_NOTE = 48;//79
const uint8_t LAST_NOTE = FIRST_NOTE + (NUM_NOTES-1);

#define SHIFT_TIMER_RESOLUTION 1000 //Microsecond resolution for starting and ending notes

// The velocity of the incoming notes will adjust the pulse length from MIN_PULSE_TICKS to MIN_PULSE_TICKS + PULSE_TICKS_RANGE
#define MIN_PULSE_TICKS 10 // Minimum length of "on" pulse for each bit in ticks (e.g. RESOLUTION * PULSE_TICKS microseconds)
#define PULSE_TICKS_RANGE 60 // Maximum number of ticks to add to MIN_PULSE_TICKS for maximum velocity

#define SHIFT_DATA_BYTES 3
uint8_t shiftData[SHIFT_DATA_BYTES]; // 8*3 = 24
uint8_t activeTicksLeft[NUM_NOTES];

boolean shouldShift = false; // When true, a shift will occur during the next tick


void ShiftRegisterInstrument::SetUp()
{

    // Prepare pins
    pinMode(DATA_PIN, OUTPUT);
    pinMode(SHIFT_CLOCK_PIN, OUTPUT);
    pinMode(LATCH_PIN, OUTPUT); 

    // Don't enable these unless you plan to use them!
    //pinMode(MASTER_CLEAR_PIN, OUTPUT);
    //pinMode(OUTPUT_ENABLE_PIN, OUTPUT);


    // With all pins setup, let's do a first run reset
    ResetAll();
    delay(500); // Wait a half second for safety

    // Setup timer to handle interrupts
    InterruptTimer::initialize(SHIFT_TIMER_RESOLUTION, Tick);

    //Initalize Default values
}

void ShiftRegisterInstrument::Reset(uint8_t instrument)
{
    ZeroOutputs();
}

void ShiftRegisterInstrument::ResetAll()
{
    ZeroOutputs();
}

void ShiftRegisterInstrument::PlayNote(uint8_t instrument, uint8_t note, uint8_t velocity)
{
    if (note >= FIRST_NOTE && note <= LAST_NOTE) {
        OutputOn(note - FIRST_NOTE);
        activeTicksLeft[note - FIRST_NOTE] = MIN_PULSE_TICKS + ((note * PULSE_TICKS_RANGE) / 127);
        shouldShift = true;
    }
}

void ShiftRegisterInstrument::StopNote(uint8_t instrument, uint8_t note, uint8_t velocity)
{
    //Not Yet Implemented
}

void ShiftRegisterInstrument::StopAll()
{
    ZeroOutputs();
}

void ShiftRegisterInstrument::SetKeyPressure(uint8_t instrument, uint8_t note, uint8_t velocity)
{
    //Not Yet Implemented
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
void ICACHE_RAM_ATTR ShiftRegisterInstrument::Tick()
#else
void ShiftRegisterInstrument::Tick()
#endif
{
    for (byte n = 0; n < NUM_NOTES; n++) {
        if (activeTicksLeft[n] > 0) {
            if (--activeTicksLeft[n] == 0) {
                OutputOff(n);
                shouldShift = true;
            }
        }
    }

    if (shouldShift) {
        shouldShift = false;
        ShiftAllData();
    }
}

#ifdef ARDUINO_ARCH_ESP32
void ICACHE_RAM_ATTR ShiftRegisterInstrument::ShiftAllData()
#else
void ShiftRegisterInstrument::ShiftAllData()
#endif
{
  digitalWrite(LATCH_PIN, LOW);
  for (int i=SHIFT_DATA_BYTES-1;i>=0;i--){
    shiftOut(DATA_PIN, SHIFT_CLOCK_PIN, MSBFIRST, shiftData[i]);
  }
  digitalWrite(LATCH_PIN, HIGH);
}

#ifdef ARDUINO_ARCH_ESP32
void ICACHE_RAM_ATTR ShiftRegisterInstrument::OutputOn(uint8_t outputNum)
#else
void ShiftRegisterInstrument::OutputOn(uint8_t OutputNum)
#endif
{
  bitSet(shiftData[outputNum/8],outputNum%8);
}

#ifdef ARDUINO_ARCH_ESP32
void ICACHE_RAM_ATTR ShiftRegisterInstrument::OutputOff(uint8_t outputNum)
#else
void ShiftRegisterInstrument::OutputOff(uint8_t outputNum)
#endif
{
  bitClear(shiftData[outputNum/8],outputNum%8);
}

#pragma GCC pop_options


void ShiftRegisterInstrument::ZeroOutputs()
{
  for (byte i=0;i<SHIFT_DATA_BYTES;i++){
    shiftData[i] = 0;
  }
  ShiftAllData();
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//Getters and Setters
////////////////////////////////////////////////////////////////////////////////////////////////////

uint8_t ShiftRegisterInstrument::getNumActiveNotes(uint8_t instrument)
{
    //Not Yet Implemented
}
 
bool ShiftRegisterInstrument::isNoteActive(uint8_t instrument, uint8_t note)
{
    //Not Yet Implemented
}

void ShiftRegisterInstrument::SetPitchBend(uint8_t instrument, uint16_t bend){
    //Not Yet Implemented
}
void ShiftRegisterInstrument::SetProgramChange(uint8_t value){
    //Not Yet Implemented    
}
void ShiftRegisterInstrument::SetChannelPressure(uint8_t value){
    //Not Yet Implemented    
}
void ShiftRegisterInstrument::SetModulationWheel(uint8_t value){
    //Not Yet Implemented
}
void ShiftRegisterInstrument::SetFootPedal(uint8_t value){
    //Not Yet Implemented
}
void ShiftRegisterInstrument::SetVolume(uint8_t value){
    //Not Yet Implemented
}
void ShiftRegisterInstrument::SetExpression(uint8_t value){
    //Not Yet Implemented
}
void ShiftRegisterInstrument::SetEffectCrtl_1(uint8_t value){
    //Not Yet Implemented
}
void ShiftRegisterInstrument::SetEffectCrtl_2(uint8_t value){
    //Not Yet Implemented
}