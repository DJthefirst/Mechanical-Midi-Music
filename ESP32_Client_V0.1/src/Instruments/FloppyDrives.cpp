#include "Instruments/FloppyDrives.h"
#include "InterruptTimer.h"
#include "Constants.h"
#include "Arduino.h"

//[Instrument][ActiveNote] MSB of note is Active last 7 is Value 
static uint8_t _activeNotes[MAX_NUM_INSTRUMENTS];
static uint8_t _numActiveNotes[MAX_NUM_INSTRUMENTS];

//Instrument Attributes

void FloppyDrives::SetUp()
{
    //Setup pins

    // With all pins setup, let's do a first run reset
    ResetAll();
    delay(500); // Wait a half second for safety

    // Setup timer to handle interrupts
    InterruptTimer::initialize(TIMER_RESOLUTION, Tick);

    //Initalize Default values
}

void FloppyDrives::Reset(uint8_t instrument)
{
    //Not Yet Implemented
}

void FloppyDrives::ResetAll()
{
    //Not Yet Implemented
}

void FloppyDrives::PlayNote(uint8_t instrument, uint8_t note, uint8_t velocity)
{
    //Not Yet Implemented
}

void FloppyDrives::StopNote(uint8_t instrument, uint8_t note, uint8_t velocity)
{
    //Not Yet Implemented
}

void FloppyDrives::StopAll()
{
    //Not Yet Implemented
}

void FloppyDrives::SetKeyPressure(uint8_t instrument, uint8_t note, uint8_t velocity)
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
void ICACHE_RAM_ATTR FloppyDrives::Tick()
#else
void FloppyDrives::tick()
#endif
{
    
}

#pragma GCC pop_options

////////////////////////////////////////////////////////////////////////////////////////////////////
//Getters and Setters
////////////////////////////////////////////////////////////////////////////////////////////////////

uint8_t FloppyDrives::getNumActiveNotes(uint8_t instrument)
{
    //Not Yet Implemented
}
 
bool FloppyDrives::isNoteActive(uint8_t instrument, uint8_t note)
{
    //Not Yet Implemented
}

void FloppyDrives::SetPitchBend(uint8_t instrument, uint16_t bend){
    //Not Yet Implemented
}
void FloppyDrives::SetProgramChange(uint8_t value){
    //Not Yet Implemented    
}
void FloppyDrives::SetChannelPressure(uint8_t value){
    //Not Yet Implemented    
}
void FloppyDrives::SetModulationWheel(uint8_t value){
    //Not Yet Implemented
}
void FloppyDrives::SetFootPedal(uint8_t value){
    //Not Yet Implemented
}
void FloppyDrives::SetVolume(uint8_t value){
    //Not Yet Implemented
}
void FloppyDrives::SetExpression(uint8_t value){
    //Not Yet Implemented
}
void FloppyDrives::SetEffectCrtl_1(uint8_t value){
    //Not Yet Implemented
}
void FloppyDrives::SetEffectCrtl_2(uint8_t value){
    //Not Yet Implemented
}