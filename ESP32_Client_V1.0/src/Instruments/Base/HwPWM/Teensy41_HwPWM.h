#pragma once

#include "Instruments/InstrumentControllerBase.h"
#include "Config.h"
#include <cstdint>
#include <array>

/**
 * Teensy 4.1-specific PWM implementation using analogWrite with PWM frequency control
 * Provides PWM functionality for Teensy 4.1 microcontrollers
 * 
 * Teensy 4.1 has multiple FlexPWM modules, each with multiple channels
 * Uses analogWriteFrequency() to set per-pin frequencies
 * Maximum 15 PWM pins available on Teensy 4.1
 */
class Teensy41_HwPWM : public InstrumentControllerBase {
public:
    static constexpr Instrument Type = Instrument::HW_PWM;
private:
    // [Instrument][ActiveNote] MSB is set if note is active, the 7 LSBs are the note value 
    static std::array<uint8_t, HardwareConfig::MAX_NUM_INSTRUMENTS> m_activeNotes;
    static uint8_t m_numActiveNotes;

    // Instrument attributes for tracking active notes
    // static std::array<double, HardwareConfig::MAX_NUM_INSTRUMENTS> lastFrequency; // Last set frequency for caching
    static std::array<double, HardwareConfig::MAX_NUM_INSTRUMENTS> m_noteFrequency;  // Base note frequency
    static std::array<double, HardwareConfig::MAX_NUM_INSTRUMENTS> m_activeFrequency; // Note played with bend

    void initializePwmPin(uint8_t instrument, uint8_t pin);
    inline void setFrequency(uint8_t instrument, double frequency);
    inline void stopChannel(uint8_t instrument);

    //Local MIDI Device Attributes
    uint8_t m_program = 0;
    uint8_t m_channelPressure = 0;

    // PWM configuration constants
    static constexpr uint8_t PWM_RESOLUTION = 8; // 8-bit resolution (0-255)
    static constexpr uint8_t DUTY_CYCLE_50_PERCENT = 128; // 50% duty cycle

public: 
    Teensy41_HwPWM();
    void reset(uint8_t instrument) override;
    void resetAll() override;
    void playNote(uint8_t instrument, uint8_t note, uint8_t velocity,  uint8_t channel) override;
    void stopNote(uint8_t instrument, uint8_t velocity) override;
    void stopAll() override;

    void setPitchBend(uint8_t channel, uint16_t value) override;

    Instrument getInstrumentType() const override { return Instrument::HW_PWM; }
    uint8_t getNumActiveNotes(uint8_t instrument) override;
    bool isNoteActive(uint8_t instrument, uint8_t note) override;
    
    //Timeout tracking functions
    void checkInstrumentTimeouts() override;
};
