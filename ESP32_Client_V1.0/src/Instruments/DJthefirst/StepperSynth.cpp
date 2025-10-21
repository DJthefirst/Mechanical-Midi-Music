#include "Extras/AddrLED.h"
#include "Instruments/DJthefirst/StepperSynth.h"
#include "Arduino.h"

#include "Device.h"

enum PIN_Connnections{
    PIN_SHIFTREG_Data = 25,
    PIN_SHIFTREG_Clock = 27,
    PIN_SHIFTREG_Load = 26,
    PIN_LED_Data = 18
};


static std::array<bool,HardwareConfig::NUM_INSTRUMENTS> m_outputenabled; //Output Enabled

StepperSynth::StepperSynth() : SwPWM()
{
    //Setup pins
    pinMode(PIN_SHIFTREG_Data, OUTPUT);
    pinMode(PIN_SHIFTREG_Clock, OUTPUT);
    pinMode(PIN_SHIFTREG_Load, OUTPUT); 
    pinMode(PIN_LED_Data, OUTPUT);

    //Setup FAST LED
    setupLEDs();

    delay(500); // Wait a half second for safety
}

void StepperSynth::playNote(uint8_t instrument, uint8_t note, uint8_t velocity,  uint8_t channel)
{
    m_outputenabled[instrument] = true;
    updateShiftRegister();
    SwPWM::playNote(instrument, note, velocity, channel);
    setInstrumentLedOn(instrument, channel, note, velocity);
    return;
}

void StepperSynth::stopNote(uint8_t instrument, uint8_t velocity)
{
    // Check if instrument has any active note (getNumActiveNotes returns 1 if active, 0 if not)
    if(getNumActiveNotes(instrument) > 0){
        SwPWM::stopNote(instrument, velocity);
        
        m_outputenabled[instrument] = false;
        updateShiftRegister();
        setInstrumentLedOff(instrument);
    }
}

void StepperSynth::reset(uint8_t instrument){
    SwPWM::reset(instrument);
    m_outputenabled[instrument] = false;
    updateShiftRegister();
    setInstrumentLedOff(instrument);
}

void StepperSynth::resetAll(){
    stopAll();
}

void StepperSynth::stopAll(){
    SwPWM::stopAll();
    m_outputenabled = {};
    updateShiftRegister();
    resetLEDs();
}

void StepperSynth::updateShiftRegister() {

    // Write and Shift Data
    for(uint8_t i=1; i <= HardwareConfig::NUM_INSTRUMENTS; i++ ){
        digitalWrite(PIN_SHIFTREG_Data, !m_outputenabled[HardwareConfig::NUM_INSTRUMENTS - i]); //Serial Data (active high)
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

////////////////////////////////////////////////////////////////////////////////////////////////////
//FAST LED Helper Functions
////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef EXTRA_ADDRESSABLE_LEDS

void StepperSynth::setupLEDs(){
    AddrLED::get().setup();
}

//Set an Instrument Led to on
void StepperSynth::setInstrumentLedOn(uint8_t instrument, uint8_t channel, uint8_t note, uint8_t velocity){
    CHSV color = AddrLED::get().getColor(instrument, channel, note, velocity);
    AddrLED::get().turnLedsOn(instrument*5, instrument*5+4, color);
}

//Set an Instrument Led to off
void StepperSynth::setInstrumentLedOff(uint8_t instrument){
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