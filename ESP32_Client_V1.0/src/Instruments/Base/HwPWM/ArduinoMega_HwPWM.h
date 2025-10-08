//////////////////////////////////////////////////////////////
// WARNING UNTESTED CODE - WILL NOT COMPILE! - EXAMPLE ONLY //
//////////////////////////////////////////////////////////////

#pragma once

#include "Instruments/InstrumentController.h"
#include <cstdint>
#include <array>

/**
 * Arduino Mega-specific PWM implementation using Timer PWM
 * Provides PWM functionality for Arduino Mega microcontrollers
 */
class ArduinoMega_HwPWM : public InstrumentController {
public:
    static constexpr Instrument Type = Instrument::PWM;
private:
    // LedC channel management (arrays moved to .cpp file due to Config dependency)
    
    void initializeLedcChannel(uint8_t instrument, uint8_t pin);
    void setFrequency(uint8_t instrument, double frequency);
    void stopChannel(uint8_t instrument);

    //Local MIDI Device Atributes
    uint8_t m_program = 0;
    uint8_t m_channelPressure = 0;
    uint16_t m_pitchBend[Midi::NUM_CH];

    // LedC configuration constants
    static constexpr uint8_t LEDC_RESOLUTION = 12; // 12-bit resolution (0-4095)
    static constexpr uint32_t LEDC_BASE_FREQ = 80000000; // 80MHz base frequency
    static constexpr uint16_t DUTY_CYCLE_50_PERCENT = (1 << (LEDC_RESOLUTION - 1)); // 50% duty cycle

public: 
    ArduinoMega_HwPWM();
    void reset(uint8_t instrument) override;
    void resetAll() override;
    void playNote(uint8_t instrument, uint8_t note, uint8_t velocity,  uint8_t channel) override;
    void stopNote(uint8_t instrument, uint8_t note, uint8_t velocity) override;
    void stopAll() override;

    void setPitchBend(uint8_t instrument, uint16_t value, uint8_t channel) override;

    uint8_t getNumActiveNotes(uint8_t instrument) override;
    bool isNoteActive(uint8_t instrument, uint8_t note) override;


};