#include "Extras/AddrLED.h"
#include "Instruments/DJthefirst/StepperSynthHw.h"
#include "Arduino.h"

#include "Device.h"

std::array<bool,HardwareConfig::NUM_INSTRUMENTS> StepperSynthHw::m_outputenabled = {};

enum PIN_Connnections{
    PIN_SHIFTREG_Data = 25,
    PIN_SHIFTREG_Clock = 27,
    PIN_SHIFTREG_Load = 26,
    PIN_LED_Data = 18
};

StepperSynthHw::StepperSynthHw() : HwPWM()
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

void StepperSynthHw::playNote(uint8_t instrument, uint8_t note, uint8_t velocity,  uint8_t channel)
{
    m_outputenabled[instrument] = true;
    updateShiftRegister();
    HwPWM::playNote(instrument, note, velocity, channel);
    setInstrumentLedOn(instrument, channel, note, velocity);
    return;
}

void StepperSynthHw::stopNote(uint8_t instrument, uint8_t velocity)
{
   HwPWM::stopNote(instrument, velocity);
    m_outputenabled[instrument] = false;
    updateShiftRegister();
    setInstrumentLedOff(instrument);
}

void StepperSynthHw::reset(uint8_t instrument){
    HwPWM::reset(instrument);
    m_outputenabled[instrument] = false;
    updateShiftRegister();
    setInstrumentLedOff(instrument);
}

void StepperSynthHw::resetAll(){
    stopAll();
}

void StepperSynthHw::stopAll(){
    HwPWM::stopAll();
    m_outputenabled = {};
    updateShiftRegister();
    resetLEDs();
}

void StepperSynthHw::updateShiftRegister() {

    // Write and Shift Data
    // For 74HC595-style shift registers: last bit shifted in appears at Q7
    // Shift MSB first (instrument 9) so it ends up at Q7, LSB last (instrument 0) ends up at Q0
    for(int8_t i = HardwareConfig::NUM_INSTRUMENTS - 1; i >= 0; i-- ){
        // Note: Using ! for active-low enable logic (LOW = enabled)
        // If your drivers are active-high, change to: m_outputenabled[i]
        digitalWrite(PIN_SHIFTREG_Data, !m_outputenabled[i]);
        digitalWrite(PIN_SHIFTREG_Clock, HIGH); //Serial Clock
        delayMicroseconds(1); //Stabilize 
        digitalWrite(PIN_SHIFTREG_Clock,  LOW); //Serial Clock
    }
    // Toggle Load to transfer shift register to output latches
    digitalWrite(PIN_SHIFTREG_Load, HIGH); //Register Load
    delayMicroseconds(1); //Stabilize 
    digitalWrite(PIN_SHIFTREG_Load, LOW); //Register Load
    digitalWrite(PIN_SHIFTREG_Data, LOW); //Leave data line low when idle
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//FAST LED Helper Functions
////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef EXTRA_ADDRESSABLE_LEDS

void StepperSynthHw::setupLEDs(){
    AddrLED::get().setup();
}

//Set an Instrument Led to on
void StepperSynthHw::setInstrumentLedOn(uint8_t instrument, uint8_t channel, uint8_t note, uint8_t velocity){
    CHSV color = AddrLED::get().getColor(instrument, channel, note, velocity);
    AddrLED::get().turnLedsOn(instrument*5, instrument*5+4, color);
}

//Set an Instrument Led to off
void StepperSynthHw::setInstrumentLedOff(uint8_t instrument){
    AddrLED::get().turnLedsOn(instrument*5, instrument*5+4, CHSV(0,0,0));
}

//Reset Leds
void StepperSynthHw::resetLEDs(){
    AddrLED::get().reset();
}

#else
void StepperSynthHw::setupLEDs(){}
void StepperSynthHw::setInstrumentLedOn(uint8_t instrument, uint8_t channel, uint8_t note, uint8_t velocity){}
void StepperSynthHw::setInstrumentLedOff(uint8_t instrument){}
void StepperSynthHw::resetLEDs(){}
#endif