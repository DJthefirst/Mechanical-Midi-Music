#include "Config.h"
#ifdef SHIFTREG_TYPE_74HC595

#include "Extras/AddrLED.h"
#include "Instruments/DJthefirst/StepperSynthSw.h"
#include "Arduino.h"

#include "Device.h"

std::array<bool,HardwareConfig::NUM_INSTRUMENTS> StepperSynthSw::m_outputenabled = {};

StepperSynthSw::StepperSynthSw() : SwPWM()
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

void StepperSynthSw::playNote(uint8_t instrument, uint8_t note, uint8_t velocity,  uint8_t channel)
{
    m_outputenabled[instrument] = true;
    updateShiftRegister();
    SwPWM::playNote(instrument, note, velocity, channel);
    setInstrumentLedOn(instrument, channel, note, velocity);
    return;
}

void StepperSynthSw::stopNote(uint8_t instrument, uint8_t velocity)
{
   SwPWM::stopNote(instrument, velocity);
    m_outputenabled[instrument] = false;
    updateShiftRegister();
    setInstrumentLedOff(instrument);
}

void StepperSynthSw::reset(uint8_t instrument){
    SwPWM::reset(instrument);
    m_outputenabled[instrument] = false;
    updateShiftRegister();
    setInstrumentLedOff(instrument);
}

void StepperSynthSw::resetAll(){
    stopAll();
}

void StepperSynthSw::stopAll(){
    SwPWM::stopAll();
    m_outputenabled = {};
    updateShiftRegister();
    resetLEDs();
}

void StepperSynthSw::updateShiftRegister() {

    // Write and Shift Data
    // For 74HC595-style shift registers: last bit shifted in appears at Q7
    // Shift MSB first (instrument 9) so it ends up at Q7, LSB last (instrument 0) ends up at Q0
    // Using NOP instructions for tiny delays without blocking SW PWM
    for(int8_t i = HardwareConfig::NUM_INSTRUMENTS - 1; i >= 0; i-- ){
        // Note: Using ! for active-low enable logic (LOW = enabled)
        // If your drivers are active-high, change to: m_outputenabled[i]
        digitalWriteFast(PIN_SHIFTREG_Data, !m_outputenabled[i]);
        delayNanoseconds(SHIFTREG_HOLDTIME_NS);
        digitalWriteFast(PIN_SHIFTREG_Clock, HIGH); //Serial Clock
        delayNanoseconds(SHIFTREG_HOLDTIME_NS);
        digitalWriteFast(PIN_SHIFTREG_Clock, LOW);  //Serial Clock (latch data)
        delayNanoseconds(SHIFTREG_HOLDTIME_NS);
    }
    // Toggle Load to transfer shift register to output latches
    digitalWriteFast(PIN_SHIFTREG_Load, HIGH); //Register Load
    delayNanoseconds(SHIFTREG_HOLDTIME_NS);
    digitalWriteFast(PIN_SHIFTREG_Load, LOW);  //Register Load (latch output)
    digitalWriteFast(PIN_SHIFTREG_Data, LOW);  //Leave data line low when idle
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//FAST LED Helper Functions
////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef EXTRA_ADDRESSABLE_LEDS

void StepperSynthSw::setupLEDs(){
    AddrLED::get().setup();
}

//Set an Instrument Led to on
void StepperSynthSw::setInstrumentLedOn(uint8_t instrument, uint8_t channel, uint8_t note, uint8_t velocity){
    CHSV color = AddrLED::get().getColor(instrument, channel, note, velocity);
    AddrLED::get().turnLedsOn(instrument*5, instrument*5+4, color);
}

//Set an Instrument Led to off
void StepperSynthSw::setInstrumentLedOff(uint8_t instrument){
    AddrLED::get().turnLedsOn(instrument*5, instrument*5+4, CHSV(0,0,0));
}

//Reset Leds
void StepperSynthSw::resetLEDs(){
    AddrLED::get().reset();
}

#else
void StepperSynthSw::setupLEDs(){}
void StepperSynthSw::setInstrumentLedOn(uint8_t instrument, uint8_t channel, uint8_t note, uint8_t velocity){}
void StepperSynthSw::setInstrumentLedOff(uint8_t instrument){}
void StepperSynthSw::resetLEDs(){}
#endif

#endif // SHIFTREG_TYPE_74HC595