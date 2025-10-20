#include "Instruments/Base/HwPWM/ESP32_HwPWM.h"
#include "Instruments/InstrumentController.h"
#include "Instruments/Utility/NoteTable.h"
#include "Arduino.h"
#include "Config.h"
#include <cmath>

// [Instrument][ActiveNote] MSB is set if note is active, the 7 LSBs are the note value 
static std::array<uint8_t, HardwareConfig::MAX_NUM_INSTRUMENTS> m_activeNotes;
static uint8_t m_numActiveNotes;

// Instrument attributes for tracking active notes
static std::array<uint8_t, HardwareConfig::MAX_NUM_INSTRUMENTS> lastFrequency; // Active note (MSB is set if note is active)
static std::array<double, HardwareConfig::MAX_NUM_INSTRUMENTS> m_noteFrequency;  // Base note frequency
static std::array<double, HardwareConfig::MAX_NUM_INSTRUMENTS> m_activeFrequency; // Note played with bend

// Distributor tracking and timeout management
static std::array<uint32_t, HardwareConfig::MAX_NUM_INSTRUMENTS> m_noteStartTime; // When the note started (for timeout)

// LedC channel management (moved from header due to Config dependency)
static std::array<uint8_t, HardwareConfig::MAX_NUM_INSTRUMENTS> m_ledcChannels;

ESP32_HwPWM::ESP32_HwPWM() : InstrumentControllerBase()
{
    // Initialize LedC channels for each instrument
    const auto& pins = HardwareConfig::PINS;
    for(uint8_t i = 0; i < pins.size() && i < HardwareConfig::MAX_NUM_INSTRUMENTS; ++i){
        m_ledcChannels[i] = i; // Use channel number equal to instrument number
        initializeLedcChannel(i, pins[i]);
    }

    delay(500); // Wait a half second for safety

    // Initialize default values
    std::fill_n(m_pitchBend, Midi::NUM_CH, Midi::CTRL_CENTER);
    m_noteStartTime.fill(0); // No notes started initially
}

void ESP32_HwPWM::initializeLedcChannel(uint8_t instrument, uint8_t pin)
{
    // Configure LedC channel - only do full setup once during initialization
    ledcSetup(m_ledcChannels[instrument], 1000, LEDC_RESOLUTION); // Start with 1kHz, will be changed when notes play
    ledcAttachPin(pin, m_ledcChannels[instrument]);
    ledcWrite(m_ledcChannels[instrument], 0); // Start with output off
}

void ESP32_HwPWM::setFrequency(uint8_t instrument, double frequency)
{
    if (instrument >= HardwareConfig::MAX_NUM_INSTRUMENTS) return;
    
    // Clamp frequency to valid range (10Hz - 20kHz)
    if (frequency < 10.0) frequency = 10.0;
    if (frequency > 20000.0) frequency = 20000.0;
    
    // Cache last frequency per instrument to avoid unnecessary updates
    static std::array<double, HardwareConfig::MAX_NUM_INSTRUMENTS> lastFrequency = {};
    
    // Only update if frequency changed significantly (>0.5Hz difference for better precision)
    if (abs(lastFrequency[instrument] - frequency) > 0.5) {
        // Use ledcChangeFrequency for much faster frequency updates (no full reconfiguration)
        ledcChangeFrequency(m_ledcChannels[instrument], frequency, LEDC_RESOLUTION);
        lastFrequency[instrument] = frequency;
    }
    
    // Set duty cycle - this is fast and always needs to be done to ensure output is active
    ledcWrite(m_ledcChannels[instrument], DUTY_CYCLE_50_PERCENT);
    _activeInstruments.set(instrument);
}

void ESP32_HwPWM::stopChannel(uint8_t instrument)
{
    if (instrument >= HardwareConfig::MAX_NUM_INSTRUMENTS) return;
    
    // Set duty cycle to 0 to stop PWM output - this is very fast
    ledcWrite(m_ledcChannels[instrument], 0);
    _activeInstruments.reset(instrument);
}

void ESP32_HwPWM::reset(uint8_t instrument)
{
    //Not Yet Implemented
}

void ESP32_HwPWM::resetAll()
{
    stopAll();
}

void ESP32_HwPWM::playNote(uint8_t instrument, uint8_t note, uint8_t velocity,  uint8_t channel)
{
    // Early bounds checking for performance
    if (instrument >= HardwareConfig::MAX_NUM_INSTRUMENTS || note >= 128) return;

    // Cache the base frequency to avoid repeated lookups
    const double baseFreq = NoteTables::noteFrequency[note];
    
    // Store note information
    m_activeNotes[instrument] = (MSB_BITMASK | note);
    m_noteFrequency[instrument] = baseFreq;
    m_noteStartTime[instrument] = millis(); // Record when note started for timeout tracking
    
    // Calculate final frequency with pitch bend applied
    m_activeFrequency[instrument] = NoteTables::applyPitchBend(baseFreq, m_pitchBend[channel]);
    
    // Increment active note count only if this instrument wasn't already active
    if (!_activeInstruments.test(instrument)) {
        m_numActiveNotes++;
    }

    setFrequency(instrument, m_activeFrequency[instrument]);
    lastFrequency[instrument] = m_activeFrequency[instrument];



}

void ESP32_HwPWM::stopNote(uint8_t instrument, uint8_t velocity)
{
    if (instrument >= HardwareConfig::MAX_NUM_INSTRUMENTS) return;
    
    // Check if this instrument is playing the specified note (optimized bit operation)
        // Clear note information
        m_activeNotes[instrument] = 0;
        m_noteFrequency[instrument] = 0;
        m_activeFrequency[instrument] = 0;
        _lastDistributor[instrument] = nullptr; // Clear distributor tracking
        _lastChannel[instrument] = NONE; // Clear channel tracking
        m_noteStartTime[instrument] = 0;
        
        // Decrement active note count only if channel was actually active
        if (_activeInstruments.test(instrument)) {
            m_numActiveNotes--;
        }
        
        // Stop the LedC channel (fast duty cycle set to 0)
        stopChannel(instrument);
}

void ESP32_HwPWM::stopAll(){
    std::fill_n(m_pitchBend, Midi::NUM_CH, Midi::CTRL_CENTER);
    m_numActiveNotes = 0;
    m_activeNotes = {};
    m_noteFrequency = {};
    m_activeFrequency = {};
    _lastDistributor.fill(nullptr); // Clear all distributor tracking
    _lastChannel.fill(NONE); // Clear all channel tracking
    m_noteStartTime.fill(0); // Clear all start times

    // Stop all LedC channels
    for(uint8_t i = 0; i < HardwareConfig::MAX_NUM_INSTRUMENTS; i++){
        stopChannel(i);
    }
}

// Tick() and togglePin() functions removed - LedC handles PWM generation in hardware

////////////////////////////////////////////////////////////////////////////////////////////////////
// Getters and Setters
////////////////////////////////////////////////////////////////////////////////////////////////////

uint8_t ESP32_HwPWM::getNumActiveNotes(uint8_t instrument)
{
    return (m_activeNotes[instrument] != 0) ? 1 : 0;
}
 
bool ESP32_HwPWM::isNoteActive(uint8_t instrument, uint8_t note)
{
    //Mask lower 7bits and return true if the instrument is playing the respective note.
    return ((m_activeNotes[instrument] & 0x7F) == note && m_activeNotes[instrument] != 0);
}

void ESP32_HwPWM::setPitchBend(uint8_t instrument, uint16_t bend, uint8_t channel){
    m_pitchBend[channel] = bend; 
    
    // Early exit optimizations - check most likely fail conditions first
    if(m_noteFrequency[instrument] == 0) return;
    
    // Calculate and apply pitch bend effect in one operation
    const double bentFreq = NoteTables::applyPitchBend(m_noteFrequency[instrument], bend);
    
    // Direct frequency update using optimized LedC call
    if (abs(lastFrequency[instrument] - bentFreq) > 0.75) {
        m_activeFrequency[instrument] = bentFreq;
        setFrequency(instrument, m_activeFrequency[instrument]);
        lastFrequency[instrument] = bentFreq;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Timeout Tracking Functions
////////////////////////////////////////////////////////////////////////////////////////////////////

void ESP32_HwPWM::checkInstrumentTimeouts() {
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