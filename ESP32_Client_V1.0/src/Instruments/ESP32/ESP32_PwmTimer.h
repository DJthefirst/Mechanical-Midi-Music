#pragma once

#include "Constants.h"
#include "Instruments/InstrumentController.h"
#include "Device.h"

#include <cstdint>
#include <array>
using std::int8_t;

/* Outputs a PWM signal at the Notes Frequency on each Digital IO Pin using LedC */
class ESP32_PwmTimer : public InstrumentController{
public:

private:
    // LedC channel management
    std::array<uint8_t, MAX_NUM_INSTRUMENTS> m_ledcChannels;
    std::array<bool, MAX_NUM_INSTRUMENTS> m_channelActive;
    
    void initializeLedcChannel(uint8_t instrument, uint8_t pin);
    void setFrequency(uint8_t instrument, double frequency);
    void stopChannel(uint8_t instrument);

    //Local MIDI Device Atributes
    uint8_t m_program = 0;
    uint8_t m_channelPressure = 0;
    uint16_t m_pitchBend[NUM_MIDI_CH];

    // LedC configuration constants
    static constexpr uint8_t LEDC_RESOLUTION = 12; // 12-bit resolution (0-4095)
    static constexpr uint32_t LEDC_BASE_FREQ = 80000000; // 80MHz base frequency
    static constexpr uint16_t DUTY_CYCLE_50_PERCENT = (1 << (LEDC_RESOLUTION - 1)); // 50% duty cycle

public: 
    ESP32_PwmTimer();

    void reset(uint8_t instrument) override;
    void resetAll() override;
    void playNote(uint8_t instrument, uint8_t note, uint8_t velocity,  uint8_t channel) override;
    void stopNote(uint8_t instrument, uint8_t note, uint8_t velocity) override;
    void stopAll() override;

    void setPitchBend(uint8_t instrument, uint16_t value, uint8_t channel) override;

    uint8_t getNumActiveNotes(uint8_t instrument) override;
    bool isNoteActive(uint8_t instrument, uint8_t note) override;

private:
    //LEDs
    void setInstumentLedOn(uint8_t instrument, uint8_t channel, uint8_t note, uint8_t velocity);
    void setInstumentLedOff(uint8_t instrument);
    void resetLEDs();
    void setupLEDs();
};