// #include "Extras/AddrLED.h"
// #include "Instruments/Default/StepperA4988.h"
// #include "Instruments/InterruptTimer.h"
// #include "Arduino.h"

// enum PIN_Connnections{
//     PIN_SHIFTREG_Data = 25,
//     PIN_SHIFTREG_Clock = 27,
//     PIN_SHIFTREG_Load = 26,
//     PIN_LED_Data = 18
// };

// const uint8_t OFFSET_PINS = 4;

// //[Instrument][ActiveNote] MSB is set if note is Active the 7 LSBs are the Notes Value 
// static std::array<uint8_t,MAX_NUM_INSTRUMENTS> m_activeNotes;
// static uint8_t m_numActiveNotes;

// //Instrument Attributes
// static std::array<uint16_t,MAX_NUM_INSTRUMENTS> m_notePeriod;  //Base Note
// static std::array<uint16_t,MAX_NUM_INSTRUMENTS> m_activePeriod;//Note Played
// static std::array<uint16_t,MAX_NUM_INSTRUMENTS> m_currentTick; //Timeing
// static std::array<bool,MAX_NUM_INSTRUMENTS> m_currentState; //IO
// static std::array<bool,MAX_NUM_INSTRUMENTS> m_outputenabled; //Output Enabled

// StepperA4988::StepperA4988()
// {
//     //Setup pins
//     for(uint8_t i=0; i < pins.size(); i++){
//         pinMode(pins[i], OUTPUT);
//     }

//     //Setup FAST LED
//     setupLEDs();

//     delay(500); // Wait a half second for safety

//     // Setup timer to handle interrupts for driving the instrument
//     InterruptTimer::initialize(TIMER_RESOLUTION, Tick);

//     //Initalize Default values
//     std::fill_n(m_pitchBend, MAX_NUM_INSTRUMENTS, MIDI_CTRL_CENTER);
// }

// void StepperA4988::reset(uint8_t instrument)
// {
//     //Not Yet Implemented
// }

// void StepperA4988::resetAll()
// {
//     stopAll();
// }

// void StepperA4988::playNote(uint8_t instrument, uint8_t note, uint8_t velocity,  uint8_t channel)
// {

//     //Use MSB in note to indicate if a note is active.
//     //If Note inactive (MSB == 0) Update active notes, notePeriod and activePeriod
    
//     //Remove this if statement condition for note overwrite
//     //if((m_activeNotes[instrument] & MSB_BITMASK) == 0){
//         m_outputenabled[instrument] = true;
//         updateShiftRegister();
//         m_activeNotes[instrument] = (MSB_BITMASK | note);
//         m_notePeriod[instrument] = NOTE_TICKS_DOUBLE[note];
//         double bendDeflection = ((double)m_pitchBend[instrument] - (double)MIDI_CTRL_CENTER) / (double)MIDI_CTRL_CENTER;
//         m_activePeriod[instrument] = NOTE_TICKS_DOUBLE[note] / pow(2.0, BEND_OCTAVES * bendDeflection);
//         m_numActiveNotes++;
//         setInstumentLedOn(instrument, channel, note, velocity);
//         return;
//     //}
// }

// void StepperA4988::stopNote(uint8_t instrument, uint8_t note, uint8_t velocity)
// {
//     if((m_activeNotes[instrument] & (~MSB_BITMASK)) == note){
//         m_outputenabled[instrument] = false;
//         updateShiftRegister();
//         m_activeNotes[instrument] = 0;
//         m_notePeriod[instrument] = 0;
//         m_activePeriod[instrument] = 0;
//         digitalWrite(pins[instrument + OFFSET_PINS], 0);
//         m_numActiveNotes--;
//         setInstumentLedOff(instrument);
//         return;
//     }
// }

// void StepperA4988::stopAll(){
//     std::fill_n(m_pitchBend, MAX_NUM_INSTRUMENTS, MIDI_CTRL_CENTER);
//     m_numActiveNotes = 0;
//     m_activeNotes = {};
//     m_notePeriod = {};
//     m_activePeriod = {};
//     m_currentTick = {};
//     m_currentState = {};
//     m_outputenabled = {};

//     for(uint8_t i = 0; i < MAX_NUM_INSTRUMENTS; i++){
//         digitalWrite(pins[i + OFFSET_PINS], LOW);
//     }
//     updateShiftRegister();
//     resetLEDs();
// }

// ////////////////////////////////////////////////////////////////////////////////////////////////////
// //Tick
// ////////////////////////////////////////////////////////////////////////////////////////////////////

// /*
// Called by the timer interrupt at the specified resolution.  Because this is called extremely often,
// it's crucial that any computations here be kept to a minimum!

// Additionally, the ICACHE_RAM_ATTR helps avoid crashes with WiFi libraries, but may increase speed generally anyway
//  */
// #pragma GCC push_options
// #pragma GCC optimize("Ofast") // Required to unroll this loop, but useful to try to keep this speedy
// #ifdef ARDUINO_ARCH_ESP32
// void ICACHE_RAM_ATTR StepperA4988::Tick()
// #else
// void StepperA4988::tick()
// #endif
// {
//     // Go through every Instrument
//     for (int i = 0; i < MAX_NUM_INSTRUMENTS; i++) {
//         if(m_numActiveNotes == 0)continue;

//         //If note active increase tick until period reset and toggle pin
//         if (m_activePeriod[i] > 0){
//             if (m_currentTick[i] >= m_activePeriod[i]) {
//                 togglePin(i);

//                 m_currentTick[i] = 0;
//                 continue;
//             }
//             m_currentTick[i]++;
//         }
//     }
// }


// #ifdef ARDUINO_ARCH_ESP32
// void ICACHE_RAM_ATTR StepperA4988::togglePin(uint8_t instrument)
// #else
// void StepperA4988::togglePin(uint8_t instrument)
// #endif
// {
//     //Pulse the control pin
//     m_currentState[instrument] = !m_currentState[instrument];
//     digitalWrite(pins[instrument + OFFSET_PINS], m_currentState[instrument]);
        
// }

// #ifdef ARDUINO_ARCH_ESP32
// void ICACHE_RAM_ATTR StepperA4988::updateShiftRegister() {
// #else
// void StepperA4988::updateShiftRegister(uint8_t instrument) {
// #endif

//     // Write and Shift Data
//     for(uint8_t i=1; i <= MAX_NUM_INSTRUMENTS; i++ ){
//         digitalWrite(PIN_SHIFTREG_Data, !m_outputenabled[MAX_NUM_INSTRUMENTS - i]); //Serial Data (active high)
//         digitalWrite(PIN_SHIFTREG_Clock, HIGH); //Serial Clock
//         delayMicroseconds(1); //Stabilize 
//         digitalWrite(PIN_SHIFTREG_Clock,  LOW); //Serial Clock
//     }
//     // Toggle Load
//     digitalWrite(PIN_SHIFTREG_Load, HIGH); //Register Load
//     delayMicroseconds(1); //Stabilize 
//     digitalWrite(PIN_SHIFTREG_Load, LOW); //Register Load
//     digitalWrite(PIN_SHIFTREG_Data, HIGH); //Serial Data
        
// }
// #pragma GCC pop_options

// ////////////////////////////////////////////////////////////////////////////////////////////////////
// //Getters and Setters
// ////////////////////////////////////////////////////////////////////////////////////////////////////

// uint8_t StepperA4988::getNumActiveNotes(uint8_t instrument)
// {
//     return (m_activeNotes[instrument] != 0) ? 1 : 0;
// }
 
// bool StepperA4988::isNoteActive(uint8_t instrument, uint8_t note)
// {
//     return ((m_activeNotes[instrument] & (~ MSB_BITMASK)) == note);
// }

// void StepperA4988::setPitchBend(uint8_t instrument, uint16_t bend){
//     m_pitchBend[instrument] = bend; 
//     if(m_notePeriod[instrument] == 0) return;
//     //Calculate Pitch Bend
//     double bendDeflection = ((double)bend - (double)MIDI_CTRL_CENTER) / (double)MIDI_CTRL_CENTER;
//     m_activePeriod[instrument] = m_notePeriod[instrument] / pow(2.0, BEND_OCTAVES * bendDeflection);

// }

// ////////////////////////////////////////////////////////////////////////////////////////////////////
// //FAST LED Helper Functions
// ////////////////////////////////////////////////////////////////////////////////////////////////////
// #ifdef ADDRESSABLE_LEDS

// // Addressable LED Strip
// AddrLED addrLEDs;

// void StepperA4988::setupLEDs(){
//     addrLEDs.setup();
// }

// //Set an Instrument Led to on
// void StepperA4988::setInstumentLedOn(uint8_t instrument, uint8_t channel, uint8_t note, uint8_t velocity){
//     CHSV color = addrLEDs.getColor(instrument, channel, note, velocity);
//     addrLEDs.setLedOn(instrument, color);
// }

// //Set an Instrument Led to off
// void StepperA4988::setInstumentLedOff(uint8_t instrument){
//     addrLEDs.setLedOff(instrument);
// }

// //Reset Leds
// void StepperA4988::resetLEDs(){
//     addrLEDs.reset();
// }

// #else
// void StepperA4988::setupLEDs(){}
// void StepperA4988::setInstumentLedOn(uint8_t instrument, uint8_t channel, uint8_t note, uint8_t velocity){}
// void StepperA4988::setInstumentLedOff(uint8_t instrument){}
// void StepperA4988::resetLEDs(){}
// #endif