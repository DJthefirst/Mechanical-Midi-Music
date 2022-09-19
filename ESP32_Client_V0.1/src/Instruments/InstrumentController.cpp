#include "InstrumentController.h"
#include "InterruptTimer.h"
#include "Constants.h"
#include "Arduino.h"

//[Instrument][ActiveNote] MSB of note is Active last 7 is Value 
static uint8_t _activeNotes[MAX_NUM_INSTRUMENTS];
static uint8_t _numActiveNotes[MAX_NUM_INSTRUMENTS];

//Instrument Attributes

InstrumentController::InstrumentController()
{
    // With all pins setup, let's do a first run reset
    //ResetAll();
    //delay(500); // Wait a half second for safetyl.

    // Setup timer to handle interrupts
    //InterruptTimer::initialize(TIMER_RESOLUTION, Tick);

    //Initalize Default values
}


void InstrumentController::Reset(uint8_t instrument)
{
    //Not Yet Implemented
}

void InstrumentController::ResetAll()
{
    //Not Yet Implemented
}

void InstrumentController::PlayNote(uint8_t instrument, uint8_t note, uint8_t velocity)
{
    //Not Yet Implemented
}

void InstrumentController::StopNote(uint8_t instrument, uint8_t note, uint8_t velocity)
{
    //Not Yet Implemented
}

void InstrumentController::StopAll()
{
    //Not Yet Implemented
}

void InstrumentController::SetKeyPressure(uint8_t instrument, uint8_t note, uint8_t velocity)
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
void ICACHE_RAM_ATTR InstrumentController::Tick()
#else
void InstrumentController::tick()
#endif
{
    
}

#pragma GCC pop_options

////////////////////////////////////////////////////////////////////////////////////////////////////
//Getters and Setters
////////////////////////////////////////////////////////////////////////////////////////////////////

uint8_t InstrumentController::getNumActiveNotes(uint8_t instrument)
{
    //Not Yet Implemented
}
 
bool InstrumentController::isNoteActive(uint8_t instrument, uint8_t note)
{
    //Not Yet Implemented
}

void InstrumentController::SetPitchBend(uint8_t instrument, uint16_t bend){
    //Not Yet Implemented
}
void InstrumentController::SetProgramChange(uint8_t value){
    //Not Yet Implemented    
}
void InstrumentController::SetChannelPressure(uint8_t value){
    //Not Yet Implemented    
}
void InstrumentController::SetModulationWheel(uint8_t value){
    //Not Yet Implemented
}
void InstrumentController::SetFootPedal(uint8_t value){
    //Not Yet Implemented
}
void InstrumentController::SetVolume(uint8_t value){
    //Not Yet Implemented
}
void InstrumentController::SetExpression(uint8_t value){
    //Not Yet Implemented
}
void InstrumentController::SetEffectCrtl_1(uint8_t value){
    //Not Yet Implemented
}
void InstrumentController::SetEffectCrtl_2(uint8_t value){
    //Not Yet Implemented
}