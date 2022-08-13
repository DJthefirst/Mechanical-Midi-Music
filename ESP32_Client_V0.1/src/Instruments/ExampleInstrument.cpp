#include "Instruments/ExampleInstrument.h"
#include "InterruptTimer.h"
#include "Constants.h"
#include "Arduino.h"

//[Instrument][ActiveNote] MSB of note is Active last 7 is Value 
static uint8_t _activeNotes[MAX_NUM_INSTRUMENTS][MAX_POLYPHONIC_NOTES];
static uint8_t _numActiveNotes[MAX_NUM_INSTRUMENTS];

//Instrument Attributes
static uint16_t _currentPeriod[MAX_NUM_INSTRUMENTS][MAX_POLYPHONIC_NOTES];  //Notes
static uint16_t _modCurrentPeriod[MAX_NUM_INSTRUMENTS][MAX_POLYPHONIC_NOTES];  //Notes
static uint8_t _currentSlot[MAX_NUM_INSTRUMENTS]; //Polyphonic
static uint8_t _currentTick[MAX_NUM_INSTRUMENTS]; //Timeing
static bool _currentState[MAX_NUM_INSTRUMENTS]; //IO

void ExampleInstrument::SetUp()
{
    //Setup pins
    for(uint8_t i=0; i < sizeof(pins); i++){
        pinMode(pins[i], OUTPUT);
    }

    // With all pins setup, let's do a first run reset
    ResetAll();
    delay(500); // Wait a half second for safety

    // Setup timer to handle interrupts for floppy driving
    InterruptTimer::initialize(TIMER_RESOLUTION, Tick);

    //Initalize Default values
    std::fill_n(_PitchBend, MAX_NUM_INSTRUMENTS, MIDI_Control_Mid);
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
    for(uint8_t i = 0; i < MAX_POLYPHONIC_NOTES; i++){

        //Use MSB in note to indicate if a note is active.
        if((_activeNotes[instrument][i] & MSB_BITMASK) == 0){
            _activeNotes[instrument][i] = (MSB_BITMASK | note);
            _currentPeriod[instrument][i] = noteDoubleTicks[note];

            double bendDeflection = ((double)_PitchBend[instrument] - (double)MIDI_Control_Mid) / (double)MIDI_Control_Mid;
            _modCurrentPeriod[instrument][i] = noteDoubleTicks[note] / pow(2.0, BEND_OCTAVES * bendDeflection);

            _numActiveNotes[instrument]++;
            return;
        }
    }
}

void ExampleInstrument::StopNote(uint8_t instrument, uint8_t note, uint8_t velocity)
{
    for(uint8_t i = 0; i < MAX_POLYPHONIC_NOTES; i++){
        if((_activeNotes[instrument][i] & (~MSB_BITMASK)) == note){
            _activeNotes[instrument][i] = 0;
            _currentPeriod[instrument][i] = 0;
            _modCurrentPeriod[instrument][i] = 0;
            _numActiveNotes[instrument]--;
            return;
        }
    }
}

void ExampleInstrument::StopAll(){
    std::fill(&_activeNotes[0][0],&_activeNotes[0][0]+sizeof(_activeNotes),0);
    std::fill(&_numActiveNotes[0],&_numActiveNotes[0]+sizeof(_numActiveNotes),0);
    std::fill(&_currentPeriod[0][0],&_currentPeriod[0][0]+sizeof(_currentPeriod),0);
    std::fill(&_modCurrentPeriod[0][0],&_modCurrentPeriod[0][0]+sizeof(_modCurrentPeriod),0);
    std::fill(&_currentSlot[0],&_currentSlot[0]+sizeof(_currentSlot),0);
    std::fill(&_currentTick[0],&_currentTick[0]+sizeof(_currentTick),0);
    std::fill(&_currentState[0],&_currentState[0]+sizeof(_currentState),0);

    uint8_t i = 0;
    for(i = 0; (i < sizeof(pins)); i++){
        digitalWrite(pins[i], LOW);
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
    for (int i = 0; i < MAX_NUM_INSTRUMENTS; i++) {
        if(_numActiveNotes[i] == 0)continue;

        if(_currentSlot[i] >= MAX_POLYPHONIC_NOTES) _currentSlot[i] = 0;

        if (_modCurrentPeriod[i][_currentSlot[i]] > 0){
            if (_currentTick[i] >= _modCurrentPeriod[i][_currentSlot[i]]) {
                togglePin(i);
                _currentSlot[i]++;
                _currentTick[i] = 0;
                continue;
            }
        _currentTick[i]++;
        }
        else{
        _currentSlot[i]++;
        }
    }
}


#ifdef ARDUINO_ARCH_ESP32
void ICACHE_RAM_ATTR ExampleInstrument::togglePin(uint8_t instrument) {
#else
void ExampleInstrument::togglePin(byte driveNum, byte pin) {
#endif

    //Pulse the control pin
    _currentState[instrument] = !_currentState[instrument];
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
    for(uint8_t n=0; n < MAX_POLYPHONIC_NOTES; n++){

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

void ExampleInstrument::SetPitchBend(uint8_t instrument, uint16_t bend){
    _PitchBend[instrument] = bend;
    
    for(uint8_t i=0; i < MAX_POLYPHONIC_NOTES; i++){
        if(_currentPeriod[instrument][i] == 0) continue;
        //Calculate Pitch Bend
        double bendDeflection = ((double)bend - (double)MIDI_Control_Mid) / (double)MIDI_Control_Mid;
        _modCurrentPeriod[instrument][i] = _currentPeriod[instrument][i] / pow(2.0, BEND_OCTAVES * bendDeflection);

    }
}
void ExampleInstrument::SetModulationWheel(uint8_t value){
    //Not Yet Implemented
}
void ExampleInstrument::SetFootPedal(uint8_t value){
    //Not Yet Implemented
}
void ExampleInstrument::SetVolume(uint8_t value){
    //Not Yet Implemented
}
void ExampleInstrument::SetExpression(uint8_t value){
    //Not Yet Implemented
}
void ExampleInstrument::SetEffectCrtl_1(uint8_t value){
    //Not Yet Implemented
}
void ExampleInstrument::SetEffectCrtl_2(uint8_t value){
    //Not Yet Implemented
}