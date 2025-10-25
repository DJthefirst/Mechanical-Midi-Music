#pragma once

#include "Instruments/InstrumentControllerBase.h"
#include "Config.h"
#include <cstdint>
#include <array>


/**
 * ESP32-specific PWM implementation using LedC peripheral
 * Provides PWM functionality for ESP32 microcontrollers
 * 
 * Channel Mapping Strategy:
 * - Uses LEDC channels 0, 2, 4, 6, 8, 10, 12, 14 (even channels only)
 * - This ensures each channel uses a unique timer (8 timers total)
 * - Prevents timer sharing which causes frequency interference
 * - Maximum 8 simultaneous instruments supported
 */
class ESP32_HwPWM : public InstrumentControllerBase {
public:
    static constexpr Instrument Type = Instrument::HW_PWM;
private:
    // [Instrument][ActiveNote] MSB is set if note is active, the 7 LSBs are the note value 
    static std::array<uint8_t, HardwareConfig::MAX_NUM_INSTRUMENTS> m_activeNotes;
    static uint8_t m_numActiveNotes;

    // Instrument attributes for tracking active notes
    static std::array<uint8_t, HardwareConfig::MAX_NUM_INSTRUMENTS> lastFrequency; // Active note (MSB is set if note is active)
    static std::array<double, HardwareConfig::MAX_NUM_INSTRUMENTS> m_noteFrequency;  // Base note frequency
    static std::array<double, HardwareConfig::MAX_NUM_INSTRUMENTS> m_activeFrequency; // Note played with bend

    // LedC channel management (moved from header due to Config dependency)
    static std::array<uint8_t, HardwareConfig::MAX_NUM_INSTRUMENTS> m_ledcChannels;
    

    void initializeLedcChannel(uint8_t instrument, uint8_t pin);
    inline void setFrequency(uint8_t instrument, double frequency);
    inline void stopChannel(uint8_t instrument);

    //Local MIDI Device Atributes
    uint8_t m_program = 0;
    uint8_t m_channelPressure = 0;
    uint16_t m_pitchBend[Midi::NUM_CH];

    // LedC configuration constants
    static constexpr uint8_t LEDC_RESOLUTION = 12; // 12-bit resolution (0-4095)
    static constexpr uint32_t LEDC_BASE_FREQ = 80000000; // 80MHz base frequency
    static constexpr uint16_t DUTY_CYCLE_50_PERCENT = (1 << (LEDC_RESOLUTION - 1)); // 50% duty cycle

public: 
    ESP32_HwPWM();
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