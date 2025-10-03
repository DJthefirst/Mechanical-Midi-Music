#include "Extras/AddrLED.h"
#include "Instruments/ESP32/ESP32_PwmTimer.h"
#include "Instruments/InterruptTimer.h"
#include "Arduino.h"
#include <cmath>

//[Instrument][ActiveNote] MSB is set if note is Active the 7 LSBs are the Notes Value 
static std::array<uint8_t,MAX_NUM_INSTRUMENTS> m_activeNotes;
static uint8_t m_numActiveNotes;

//Instrument Attributes for tracking active notes
static std::array<double,MAX_NUM_INSTRUMENTS> m_noteFrequency;  //Base Note Frequency
static std::array<double,MAX_NUM_INSTRUMENTS> m_activeFrequency;//Note Played with bend
static std::array<uint8_t,MAX_NUM_INSTRUMENTS> m_noteCh; //Midi Channel

ESP32_PwmTimer::ESP32_PwmTimer()
{
    // Initialize LedC channels for each instrument
    for(uint8_t i = 0; i < pins.size() && i < MAX_NUM_INSTRUMENTS; i++){
        m_ledcChannels[i] = i; // Use channel number equal to instrument number
        m_channelActive[i] = false;
        initializeLedcChannel(i, pins[i]);
    }

    //Setup FAST LED
    setupLEDs();

    delay(500); // Wait a half second for safety

    //Initialize Default values
    std::fill_n(m_pitchBend, NUM_MIDI_CH, MIDI_CTRL_CENTER);
    m_noteCh.fill(255); // 255 indicates no channel assigned
}

void ESP32_PwmTimer::initializeLedcChannel(uint8_t instrument, uint8_t pin)
{
    // Configure LedC channel
    ledcSetup(m_ledcChannels[instrument], 1000, LEDC_RESOLUTION); // Start with 1kHz, will be changed when notes play
    ledcAttachPin(pin, m_ledcChannels[instrument]);
    ledcWrite(m_ledcChannels[instrument], 0); // Start with output off
}

void ESP32_PwmTimer::setFrequency(uint8_t instrument, double frequency)
{
    if (instrument >= MAX_NUM_INSTRUMENTS) return;
    
    // Clamp frequency to valid range (50Hz - 20kHz)
    if (frequency < 50.0) frequency = 50.0;
    if (frequency > 20000.0) frequency = 20000.0;
    
    // Set up the channel with new frequency and 50% duty cycle
    ledcSetup(m_ledcChannels[instrument], frequency, LEDC_RESOLUTION);
    ledcWrite(m_ledcChannels[instrument], DUTY_CYCLE_50_PERCENT);
    m_channelActive[instrument] = true;
}

void ESP32_PwmTimer::stopChannel(uint8_t instrument)
{
    if (instrument >= MAX_NUM_INSTRUMENTS) return;
    
    // Set duty cycle to 0 to stop PWM output
    ledcWrite(m_ledcChannels[instrument], 0);
    m_channelActive[instrument] = false;
}

void ESP32_PwmTimer::reset(uint8_t instrument)
{
    //Not Yet Implemented
}

void ESP32_PwmTimer::resetAll()
{
    stopAll();
}

void ESP32_PwmTimer::playNote(uint8_t instrument, uint8_t note, uint8_t velocity,  uint8_t channel)
{
    if (instrument >= MAX_NUM_INSTRUMENTS || note >= 128) return;

    // Store note information
    m_activeNotes[instrument] = (MSB_BITMASK | note);
    m_noteFrequency[instrument] = noteFrequency[note];
    
    // Calculate pitch bend effect
    double bendDeflection = ((double)m_pitchBend[channel] - (double)MIDI_CTRL_CENTER) / (double)MIDI_CTRL_CENTER;
    m_activeFrequency[instrument] = noteFrequency[note] * pow(2.0, BEND_OCTAVES * bendDeflection);
    
    m_noteCh[instrument] = channel;
    m_numActiveNotes++;
    
    // Set the frequency using LedC
    setFrequency(instrument, m_activeFrequency[instrument]);
    
    setInstumentLedOn(instrument, channel, note, velocity);
}

void ESP32_PwmTimer::stopNote(uint8_t instrument, uint8_t note, uint8_t velocity)
{
    if (instrument >= MAX_NUM_INSTRUMENTS) return;
    
    // Check if this instrument is playing the specified note
    if((m_activeNotes[instrument] & (~MSB_BITMASK)) == note){
        m_activeNotes[instrument] = 0;
        m_noteFrequency[instrument] = 0;
        m_activeFrequency[instrument] = 0;
        
        // Stop the LedC channel
        stopChannel(instrument);
        
        m_noteCh[instrument] = 255; // 255 indicates no channel
        m_numActiveNotes--;
        setInstumentLedOff(instrument);
    }
}

void ESP32_PwmTimer::stopAll(){
    std::fill_n(m_pitchBend, NUM_MIDI_CH, MIDI_CTRL_CENTER);
    m_noteCh.fill(255); // 255 indicates no channel
    m_numActiveNotes = 0;
    m_activeNotes = {};
    m_noteFrequency = {};
    m_activeFrequency = {};

    // Stop all LedC channels
    for(uint8_t i = 0; i < MAX_NUM_INSTRUMENTS; i++){
        stopChannel(i);
    }
    
    resetLEDs();
}

// Tick() and togglePin() functions removed - LedC handles PWM generation in hardware

////////////////////////////////////////////////////////////////////////////////////////////////////
//Getters and Setters
////////////////////////////////////////////////////////////////////////////////////////////////////

uint8_t ESP32_PwmTimer::getNumActiveNotes(uint8_t instrument)
{
    return (m_activeNotes[instrument] != 0) ? 1 : 0;
}
 
bool ESP32_PwmTimer::isNoteActive(uint8_t instrument, uint8_t note)
{
    //Mask lower 7bits and return true if the instument is playing the respective note.
    return ((m_activeNotes[instrument] & (~ MSB_BITMASK)) == note);
}

void ESP32_PwmTimer::setPitchBend(uint8_t instrument, uint16_t bend, uint8_t channel){
    m_pitchBend[channel] = bend; 
    
    // Only apply pitch bend if instrument is active and on the correct channel
    if(m_noteFrequency[instrument] == 0) return;
    if(m_noteCh[instrument] != channel) return;
    
    // Calculate pitch bend effect
    double bendDeflection = ((double)bend - (double)MIDI_CTRL_CENTER) / (double)MIDI_CTRL_CENTER;
    m_activeFrequency[instrument] = m_noteFrequency[instrument] * pow(2.0, BEND_OCTAVES * bendDeflection);
    
    // Update the LedC frequency with the bent frequency
    setFrequency(instrument, m_activeFrequency[instrument]);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//FAST LED Helper Functions
////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef EXTRA_ADDRESSABLE_LEDS

    void ESP32_PwmTimer::setupLEDs(){
        AddrLED::get().setup();
    }

    //Set an Instrument Led to on
    void ESP32_PwmTimer::setInstumentLedOn(uint8_t instrument, uint8_t channel, uint8_t note, uint8_t velocity){
        CHSV color = AddrLED::get().getColor(instrument, channel, note, velocity);
        AddrLED::get().turnLedOn(instrument, color);
    }

    //Set an Instrument Led to off
    void ESP32_PwmTimer::setInstumentLedOff(uint8_t instrument){
        AddrLED::get().turnLedOff(instrument);
    }

    //Reset Leds
    void ESP32_PwmTimer::resetLEDs(){
        AddrLED::get().reset();
    }

#else
void ESP32_PwmTimer::setupLEDs(){}
void ESP32_PwmTimer::setInstumentLedOn(uint8_t instrument, uint8_t channel, uint8_t note, uint8_t velocity){}
void ESP32_PwmTimer::setInstumentLedOff(uint8_t instrument){}
void ESP32_PwmTimer::resetLEDs(){}
#endif