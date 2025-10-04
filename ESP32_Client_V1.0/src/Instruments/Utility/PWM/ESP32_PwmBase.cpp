#include "Instruments/Utility/PWM/ESP32_PwmBase.h"
#include "Arduino.h"
#include "Device.h"
#include "Instruments/Utility/NoteTable.h"
#include <cmath>

//[Instrument][ActiveNote] MSB is set if note is Active the 7 LSBs are the Notes Value 
static std::array<uint8_t,Config::MAX_NUM_INSTRUMENTS> m_activeNotes;
static uint8_t m_numActiveNotes;

//Instrument Attributes for tracking active notes
static std::array<double,Config::MAX_NUM_INSTRUMENTS> m_noteFrequency;  //Base Note Frequency
static std::array<double,Config::MAX_NUM_INSTRUMENTS> m_activeFrequency;//Note Played with bend
static std::array<uint8_t,Config::MAX_NUM_INSTRUMENTS> m_noteCh; //Midi Channel

// LedC channel management (moved from header due to Config dependency)
static std::array<uint8_t, Config::MAX_NUM_INSTRUMENTS> m_ledcChannels;
static std::array<bool, Config::MAX_NUM_INSTRUMENTS> m_channelActive;

ESP32_PwmBase::ESP32_PwmBase()
{
    // Initialize LedC channels for each instrument
    for(uint8_t i = 0; i < Config::PINS.size() && i < Config::MAX_NUM_INSTRUMENTS; i++){
        m_ledcChannels[i] = i; // Use channel number equal to instrument number
        m_channelActive[i] = false;
        initializeLedcChannel(i, Config::PINS[i]);
    }

    delay(500); // Wait a half second for safety

    //Initialize Default values
    std::fill_n(m_pitchBend, NUM_MIDI_CH, MIDI_CTRL_CENTER);
    m_noteCh.fill(255); // 255 indicates no channel assigned
}

void ESP32_PwmBase::initializeLedcChannel(uint8_t instrument, uint8_t pin)
{
    // Configure LedC channel
    ledcSetup(m_ledcChannels[instrument], 1000, LEDC_RESOLUTION); // Start with 1kHz, will be changed when notes play
    ledcAttachPin(pin, m_ledcChannels[instrument]);
    ledcWrite(m_ledcChannels[instrument], 0); // Start with output off
}

void ESP32_PwmBase::setFrequency(uint8_t instrument, double frequency)
{
    if (instrument >= Config::MAX_NUM_INSTRUMENTS) return;
    
    // Clamp frequency to valid range (10Hz - 20kHz)
    if (frequency < 50.0) frequency = 10.0;
    if (frequency > 20000.0) frequency = 20000.0;
    
    // Set up the channel with new frequency and 50% duty cycle
    ledcSetup(m_ledcChannels[instrument], frequency, LEDC_RESOLUTION);
    ledcWrite(m_ledcChannels[instrument], DUTY_CYCLE_50_PERCENT);
    m_channelActive[instrument] = true;
}

void ESP32_PwmBase::stopChannel(uint8_t instrument)
{
    if (instrument >= Config::MAX_NUM_INSTRUMENTS) return;
    
    // Set duty cycle to 0 to stop PWM output
    ledcWrite(m_ledcChannels[instrument], 0);
    m_channelActive[instrument] = false;
}

void ESP32_PwmBase::reset(uint8_t instrument)
{
    //Not Yet Implemented
}

void ESP32_PwmBase::resetAll()
{
    stopAll();
}

void ESP32_PwmBase::playNote(uint8_t instrument, uint8_t note, uint8_t velocity,  uint8_t channel)
{
    if (instrument >= Config::MAX_NUM_INSTRUMENTS || note >= 128) return;

    // Store note information
    m_activeNotes[instrument] = (MSB_BITMASK | note);
    m_noteFrequency[instrument] = NoteTables::noteFrequency[note];
    
    // Calculate pitch bend effect
    m_activeFrequency[instrument] = NoteTables::applyPitchBend(NoteTables::noteFrequency[note], m_pitchBend[channel]);
    
    m_noteCh[instrument] = channel;
    m_numActiveNotes++;
    
    // Set the frequency using LedC
    setFrequency(instrument, m_activeFrequency[instrument]);
}

void ESP32_PwmBase::stopNote(uint8_t instrument, uint8_t note, uint8_t velocity)
{
    if (instrument >= Config::MAX_NUM_INSTRUMENTS) return;
    
    // Check if this instrument is playing the specified note
    if((m_activeNotes[instrument] & (~MSB_BITMASK)) == note){
        m_activeNotes[instrument] = 0;
        m_noteFrequency[instrument] = 0;
        m_activeFrequency[instrument] = 0;
        
        // Stop the LedC channel
        stopChannel(instrument);
        
        m_noteCh[instrument] = 255; // 255 indicates no channel
        m_numActiveNotes--;
    }
}

void ESP32_PwmBase::stopAll(){
    std::fill_n(m_pitchBend, NUM_MIDI_CH, MIDI_CTRL_CENTER);
    m_noteCh.fill(255); // 255 indicates no channel
    m_numActiveNotes = 0;
    m_activeNotes = {};
    m_noteFrequency = {};
    m_activeFrequency = {};

    // Stop all LedC channels
    for(uint8_t i = 0; i < Config::MAX_NUM_INSTRUMENTS; i++){
        stopChannel(i);
    }
}

// Tick() and togglePin() functions removed - LedC handles PWM generation in hardware

////////////////////////////////////////////////////////////////////////////////////////////////////
//Getters and Setters
////////////////////////////////////////////////////////////////////////////////////////////////////

uint8_t ESP32_PwmBase::getNumActiveNotes(uint8_t instrument)
{
    return (m_activeNotes[instrument] != 0) ? 1 : 0;
}
 
bool ESP32_PwmBase::isNoteActive(uint8_t instrument, uint8_t note)
{
    //Mask lower 7bits and return true if the instument is playing the respective note.
    return ((m_activeNotes[instrument] & (~ MSB_BITMASK)) == note);
}

void ESP32_PwmBase::setPitchBend(uint8_t instrument, uint16_t bend, uint8_t channel){
    m_pitchBend[channel] = bend; 
    
    // Only apply pitch bend if instrument is active and on the correct channel
    if(m_noteFrequency[instrument] == 0) return;
    if(m_noteCh[instrument] != channel) return;
    
    // Calculate pitch bend effect
    m_activeFrequency[instrument] = NoteTables::applyPitchBend(m_noteFrequency[instrument], bend);
    // Update the LedC frequency with the bent frequency
    setFrequency(instrument, m_activeFrequency[instrument]);
}