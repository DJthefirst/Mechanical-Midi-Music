#include "Instruments/ExampleInstrument.h"
#include "InterruptTimer.h"
#include "Constants.h"
#include "Arduino.h"

//Controller Attributes
    
//[Instrument][ActiveNote] MSB of note is Active last 7 is Value 
static uint8_t _activeNotes[32][16] = {0};
static uint8_t _numActiveNotes[32];

//Instrument Attributes
static uint8_t _currentPeriod[32][16];
static uint8_t _currentTick[32][16];
static bool _currentState[32];

void ExampleInstrument::SetUp()
{
    //Settup pins
    for(uint8_t i=0; i < sizeof(pins); i++){
        pinMode(pins[i], OUTPUT);
    }

    // With all pins setup, let's do a first run reset
    ResetAll();
    delay(500); // Wait a half second for safety

    // Setup timer to handle interrupts for floppy driving
    InterruptTimer::initialize(TIMER_RESOLUTION, Tick);
}

void ExampleInstrument::Reset(uint8_t instrument)
{
    //Not Yet Implemented
}

void ExampleInstrument::ResetAll()
{
    //Not Yet Implemented
}

void ExampleInstrument::PlayNote(uint8_t instrument, uint8_t note, uint8_t velocity)
{
    for(int8_t i = 0; i < 16; i++){

        if((_activeNotes[instrument][i] & MSB_BITMASK) == 0){
            _activeNotes[instrument][i] = (0x80 | note);
            _currentPeriod[instrument][i] = notePeriods[note];

            //Debug
            Serial.print("Note Location: ");
            Serial.println(i);
            Serial.print("ActiveNotes: ");
            Serial.println(_activeNotes[instrument][i]);
            Serial.print("CurrentTick: ");
            Serial.println(_currentTick[instrument][i]);
            Serial.print("CurrentPeriod: ");
            Serial.println(_currentPeriod[instrument][i]);

            return;
        }
    }
}

void ExampleInstrument::StopNote(uint8_t instrument, uint8_t note, uint8_t velocity)
{
    Serial.println("Stopping Note");

    for(int8_t i = 0; i < 16; i++){
        if((_activeNotes[instrument][i] & MSB_BITMASK) == MSB_BITMASK)
        {

            if((_activeNotes[instrument][i] & (~MSB_BITMASK)) == note)
            {
                _activeNotes[instrument][i] = 0;
                _currentPeriod[instrument][i] = 0;

                //Debug
                Serial.print("Note Location: ");
                Serial.println(i);
                Serial.print("Note Location: ");
                Serial.println(i);
                Serial.print("ActiveNotes: ");
                Serial.println(_activeNotes[instrument][i]);
                Serial.print("CurrentTick: ");
                Serial.println(_currentTick[instrument][i]);
                Serial.print("CurrentPeriod: ");
                Serial.println(_currentPeriod[instrument][i]);
                return;
            }
        }
        else{
            return;
        }
    }
}

void ExampleInstrument::SetKeyPressure(uint8_t instrument, uint8_t note, uint8_t velocity)
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
void ICACHE_RAM_ATTR ExampleInstrument::Tick()
#else
void ExampleInstrument::tick()
#endif
{
    for (int i = 0; i < 32; i++) {
        for (int n = 0; n < 16; n++){
            if (_currentPeriod[i][n] > 0) {
                _currentTick[i][n]++;
            
                if (_currentTick[i][n] >= _currentPeriod[i][n]) {
                    togglePin(i);
                    _currentTick[i][n] = 0;

                }
            }
        }
    }
}


#ifdef ARDUINO_ARCH_ESP32
void ICACHE_RAM_ATTR ExampleInstrument::togglePin(uint8_t instrument) {
#else
void ExampleInstrument::togglePin(byte driveNum, byte pin) {
#endif

    //Pulse the control pin
    _currentState[instrument] = ~_currentState[instrument];
    digitalWrite(pins[instrument], _currentState[instrument]);
        
}
#pragma GCC pop_options

////////////////////////////////////////////////////////////////////////////////////////////////////
//Getters and Setters
////////////////////////////////////////////////////////////////////////////////////////////////////

uint8_t ExampleInstrument::getNumActiveNotes(uint8_t instrument)
{
    return _numActiveNotes[instrument];
}
 
bool ExampleInstrument::isNoteActive(uint8_t instrument, uint8_t note)
{

    //Debug
    Serial.print("Note ");
    Serial.print(note);

    for(uint8_t n=0; n<16; n++){

        if ((_activeNotes[instrument][n] & (~ MSB_BITMASK)) == note){
            Serial.print(" is Active at ");
            Serial.println(n);
            return true;
        }
        //else if((_activeNotes[instrument][n] & MSB_BITMASK) == 0){
        //    return false;
        //}
    }
    Serial.println(" is inActive");
    return false;
}