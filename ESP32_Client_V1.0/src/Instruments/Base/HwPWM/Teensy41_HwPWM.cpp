#include "Config.h"
#ifdef PLATFORM_TEENSY41

#include "Instruments/Base/HwPWM/Teensy41_HwPWM.h"
#include "Instruments/InstrumentController.h"
#include "Instruments/Utility/NoteTable.h"
#include "Arduino.h"
#include <cmath>

// Static member definitions - properly scoped as class members
std::array<uint8_t, HardwareConfig::MAX_NUM_INSTRUMENTS> Teensy41_HwPWM::m_activeNotes = {};
uint8_t Teensy41_HwPWM::m_numActiveNotes = 0;
// std::array<double, HardwareConfig::MAX_NUM_INSTRUMENTS> Teensy41_HwPWM::lastFrequency = {};
std::array<double, HardwareConfig::MAX_NUM_INSTRUMENTS> Teensy41_HwPWM::m_noteFrequency = {};
std::array<double, HardwareConfig::MAX_NUM_INSTRUMENTS> Teensy41_HwPWM::m_activeFrequency = {};

Teensy41_HwPWM::Teensy41_HwPWM() : InstrumentControllerBase()
{
    // Initialize PWM pins for each instrument
    const auto& pins = HardwareConfig::PINS;
    for(uint8_t i = 0; i < pins.size() && i < HardwareConfig::MAX_NUM_INSTRUMENTS; ++i){
        initializePwmPin(i, pins[i]);
    }

    delay(500); // Wait a half second for safety

    // Initialize default values
    std::fill_n(m_pitchBend, Midi::NUM_CH, Midi::CTRL_CENTER);
    m_noteStartTime.fill(0); // No notes started initially
}

void Teensy41_HwPWM::initializePwmPin(uint8_t instrument, uint8_t pin)
{
    // Configure PWM pin
    pinMode(pin, OUTPUT);
    analogWriteResolution(PWM_RESOLUTION); // Set PWM resolution to 8-bit
    analogWriteFrequency(pin, 1000.0); // Start with 1kHz, will be changed when notes play
    analogWrite(pin, 0); // Start with output off
}

void Teensy41_HwPWM::setFrequency(uint8_t instrument, double frequency)
{
    if (instrument >= HardwareConfig::MAX_NUM_INSTRUMENTS) return;
    
    // Clamp frequency to valid range (10Hz - 20kHz)
    if (frequency < 10.0) frequency = 10.0;
    if (frequency > 20000.0) frequency = 20000.0;
    
    const auto& pins = HardwareConfig::PINS;
    
    // // Only update if frequency changed significantly (>0.5Hz difference for better precision)
    // if (abs(lastFrequency[instrument] - frequency) > 0.5) {
    //     // Use analogWriteFrequency to set the PWM frequency for this pin
    //     analogWriteFrequency(pins[instrument], frequency);
    //     lastFrequency[instrument] = frequency;
    // }
    
    // Set duty cycle - 50% for square wave audio output
    analogWrite(pins[instrument], DUTY_CYCLE_50_PERCENT);
    m_activeInstruments.set(instrument);
}

void Teensy41_HwPWM::stopChannel(uint8_t instrument)
{
    if (instrument >= HardwareConfig::MAX_NUM_INSTRUMENTS) return;
    
    const auto& pins = HardwareConfig::PINS;
    // Set duty cycle to 0 to stop PWM output
    analogWrite(pins[instrument], 0);
    m_activeInstruments.reset(instrument);
}

void Teensy41_HwPWM::reset(uint8_t instrument)
{
    //Not Yet Implemented
}

void Teensy41_HwPWM::resetAll()
{
    stopAll();
}

void Teensy41_HwPWM::playNote(uint8_t instrument, uint8_t note, uint8_t velocity,  uint8_t channel)
{
    // Early bounds checking for performance
    if (instrument >= HardwareConfig::MAX_NUM_INSTRUMENTS || note >= 128) return;

    const double baseFreq = NoteTables::noteFrequency[note];
    
    // Store note information
    m_activeNotes[instrument] = (MSB_BITMASK | note);
    m_noteFrequency[instrument] = baseFreq;
    m_noteStartTime[instrument] = millis(); // Record when note started for timeout tracking
    
    // Calculate final frequency with pitch bend applied
    m_activeFrequency[instrument] = NoteTables::applyPitchBend(baseFreq, m_pitchBend[channel]);
    
    // Increment active note count only if this instrument wasn't already active
    if (!m_activeInstruments.test(instrument)) {
        m_numActiveNotes++;
    }

    setFrequency(instrument, m_activeFrequency[instrument]);
}

void Teensy41_HwPWM::stopNote(uint8_t instrument, uint8_t velocity)
{
    if (instrument >= HardwareConfig::MAX_NUM_INSTRUMENTS) return;
    
    // Clear note information
    m_activeNotes[instrument] = 0;
    m_noteFrequency[instrument] = 0;
    m_activeFrequency[instrument] = 0;
    m_lastDistributor[instrument] = nullptr; // Clear distributor tracking
    m_lastChannel[instrument] = NONE; // Clear channel tracking
    m_noteStartTime[instrument] = 0;
    
    // Decrement active note count only if channel was actually active
    if (m_activeInstruments.test(instrument)) {
        m_numActiveNotes--;
    }
    
    // Stop the PWM channel
    stopChannel(instrument);
}

void Teensy41_HwPWM::stopAll(){
    std::fill_n(m_pitchBend, Midi::NUM_CH, Midi::CTRL_CENTER);
    m_numActiveNotes = 0;
    m_activeNotes = {};
    m_noteFrequency = {};
    m_activeFrequency = {};
    m_lastDistributor.fill(nullptr); // Clear all distributor tracking
    m_lastChannel.fill(NONE); // Clear all channel tracking
    m_noteStartTime.fill(0); // Clear all start times

    // Stop all PWM channels
    for(uint8_t i = 0; i < HardwareConfig::MAX_NUM_INSTRUMENTS; i++){
        stopChannel(i);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Getters and Setters
////////////////////////////////////////////////////////////////////////////////////////////////////

uint8_t Teensy41_HwPWM::getNumActiveNotes(uint8_t instrument)
{
    return (m_activeNotes[instrument] != 0) ? 1 : 0;
}
 
bool Teensy41_HwPWM::isNoteActive(uint8_t instrument, uint8_t note)
{
    //Mask lower 7bits and return true if the instrument is playing the respective note.
    return ((m_activeNotes[instrument] & 0x7F) == note && m_activeNotes[instrument] != 0);
}

void Teensy41_HwPWM::setPitchBend(uint8_t channel, uint16_t bend){
    m_pitchBend[channel] = bend; 
    
    for (uint8_t i = 0; i < HardwareConfig::MAX_NUM_INSTRUMENTS; i++){
        if(m_lastChannel[i] == channel){
            if(m_activeNotes[i] == 0) continue; // Skip if no active note
            // Mask off the MSB flag bit to get the actual note value (0-127)
            uint8_t note = m_activeNotes[i] & (~MSB_BITMASK);

            m_noteFrequency[i] = NoteTables::noteFrequency[note];

            uint16_t newFrequency = NoteTables::applyPitchBend(m_noteFrequency[i], bend);
            if (abs(m_activeFrequency[i] - newFrequency) > 0.75) {
                m_activeFrequency[i] = newFrequency;
                setFrequency(i, newFrequency);
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Timeout Tracking Functions
////////////////////////////////////////////////////////////////////////////////////////////////////

void Teensy41_HwPWM::checkInstrumentTimeouts() {
    // Only check timeouts if a timeout is configured
    if (HardwareConfig::INSTRUMENT_TIMEOUT_MS == 0) return;
    
    uint32_t currentTime = millis();
    for (uint8_t i = 0; i < HardwareConfig::MAX_NUM_INSTRUMENTS; i++) {
        // Check if instrument is active and has timed out
        if (m_activeNotes[i] != 0 && 
            (currentTime - m_noteStartTime[i]) > HardwareConfig::INSTRUMENT_TIMEOUT_MS) {
            // Stop the note due to timeout
            stopNote(i, 0);
        }
    }
}

#endif