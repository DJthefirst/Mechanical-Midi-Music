/*
 * ESP32_PwmTimer.h - V2.0 ESP32 Hardware PWM Implementation
 *
 * Part of the Mechanical MIDI Music V2.0 redesign
 * Based on REDESIGN_PART4_INSTRUMENTS.md specifications
 *
 * Uses ESP32 LEDC peripheral for hardware PWM generation
 * No timer interrupts needed - hardware handles everything
 */

#pragma once

#ifdef ARDUINO_ARCH_ESP32

#include "InstrumentController.h"
#include <driver/ledc.h>
#include <array>

////////////////////////////////////////////////////////////////////////////////////////////////////
// ESP32 Hardware PWM Instrument Controller
////////////////////////////////////////////////////////////////////////////////////////////////////

class ESP32_PwmTimer : public InstrumentController {
private:
    struct State {
        uint8_t currentNote = 0;
        uint32_t frequency = 0;
        bool active = false;
        ledc_channel_t channel;
        ledc_mode_t mode;
        uint8_t pin;
    };
    
    // Static instrument state arrays for ESP32 PWM channels (max 12 instruments)
    static std::array<State, 12> s_states;
    static std::array<uint16_t, 16> s_pitchBend;  // Per-channel pitch bend
    
    // Default pin configuration
    static constexpr std::array<uint8_t, 12> INSTRUMENT_PINS = { 2, 4, 18, 19, 21, 22, 23, 32, 33, 25, 26, 27 };
    
    static constexpr uint32_t PWM_RESOLUTION = 10;  // 10-bit (1024 levels)
    static constexpr uint32_t PWM_DUTY_CYCLE = 512; // 50% duty cycle
    static constexpr uint32_t PWM_FREQUENCY_MAX = 20000; // 20kHz max frequency
    
    bool m_initialized = false;

public:
    ESP32_PwmTimer() {
        // Initialize pitch bend to center (no bend)
        for (uint16_t& bend : s_pitchBend) {
            bend = 0x2000;  // MIDI pitch bend center
        }
    }
    
    ~ESP32_PwmTimer() override {
        stopAll();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    // Required Methods
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    
    void reset(uint8_t instrument) override {
        if (!isValidInstrument(instrument)) return;
        
        stopInstrument(instrument);
        s_states[instrument].currentNote = 0;
        s_states[instrument].frequency = 0;
        s_states[instrument].active = false;
    }
    
    void resetAll() override {
        if (!m_initialized) {
            initialize();
        }
        
        for (uint8_t i = 0; i < 12; i++) {
            reset(i);
        }
    }
    
    void playNote(uint8_t instrument, uint8_t note, uint8_t velocity, uint8_t channel) override {
        if (!isValidInstrument(instrument) || !isValidNote(note)) return;
        if (velocity == 0) {
            stopNote(instrument, note, velocity);
            return;
        }
        
        // Calculate frequency for this note
        uint32_t frequency = noteToFrequency(note);
        
        // Apply pitch bend if available
        if (channel < 16) {
            frequency = applyPitchBend(frequency, s_pitchBend[channel]);
        }
        
        // Apply volume scaling
        uint32_t dutyCycle = (PWM_DUTY_CYCLE * velocity * m_volume) / (127 * 127);
        
        // Update state
        s_states[instrument].currentNote = note;
        s_states[instrument].frequency = frequency;
        s_states[instrument].active = true;
        
        // Set hardware PWM
        setFrequency(instrument, frequency);
        setDutyCycle(instrument, dutyCycle);
    }
    
    void stopNote(uint8_t instrument, uint8_t note, uint8_t velocity) override {
        if (!isValidInstrument(instrument)) return;
        
        // Only stop if this instrument is playing the requested note
        if (s_states[instrument].active && s_states[instrument].currentNote == note) {
            stopInstrument(instrument);
        }
    }
    
    void stopAll() override {
        for (uint8_t i = 0; i < 12; i++) {
            stopInstrument(i);
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    // Optional Overrides
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    
    void setPitchBend(uint8_t instrument, uint16_t value, uint8_t channel) override {
        if (channel >= 16) return;
        
        s_pitchBend[channel] = value;
        
        // Update frequency for instruments on this channel
        // (This is a simplification - real implementation would track channel per instrument)
        for (uint8_t i = 0; i < 12; i++) {
            if (s_states[i].active) {
                uint32_t newFreq = applyPitchBend(noteToFrequency(s_states[i].currentNote), value);
                s_states[i].frequency = newFreq;
                setFrequency(i, newFreq);
            }
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    // Query Methods
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    
    uint8_t getNumActiveNotes(uint8_t instrument) const override {
        if (!isValidInstrument(instrument)) return 0;
        return s_states[instrument].active ? 1 : 0;
    }
    
    bool isNoteActive(uint8_t instrument, uint8_t note) const override {
        if (!isValidInstrument(instrument)) return false;
        return s_states[instrument].active && s_states[instrument].currentNote == note;
    }
    
    uint8_t getNumInstruments() const override {
        return 12;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    // Diagnostics
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    
    const char* getName() const override {
        return "ESP32 Hardware PWM";
    }

private:
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    // Hardware Interface
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    
    void initialize() {
        if (m_initialized) return;
        
        // Configure LEDC timers for both speed modes so we can support >8 channels
        ledc_timer_config_t timer_low = {
            .speed_mode = LEDC_LOW_SPEED_MODE,
            .duty_resolution = static_cast<ledc_timer_bit_t>(PWM_RESOLUTION),
            .timer_num = LEDC_TIMER_0,
            .freq_hz = 1000,
            .clk_cfg = LEDC_AUTO_CLK
        };
        ledc_timer_config(&timer_low);

        ledc_timer_config_t timer_high = {
            .speed_mode = LEDC_HIGH_SPEED_MODE,
            .duty_resolution = static_cast<ledc_timer_bit_t>(PWM_RESOLUTION),
            .timer_num = LEDC_TIMER_1,
            .freq_hz = 1000,
            .clk_cfg = LEDC_AUTO_CLK
        };
        ledc_timer_config(&timer_high);

        // Initialize channels and pins. Use low-speed channels 0-7, high-speed 0-3 for instruments 8-11
        for (uint8_t i = 0; i < 12; i++) {
            s_states[i].pin = INSTRUMENT_PINS[i];
            if (i < 8) {
                s_states[i].mode = LEDC_LOW_SPEED_MODE;
                s_states[i].channel = static_cast<ledc_channel_t>(i); // low-speed channel i
            } else {
                s_states[i].mode = LEDC_HIGH_SPEED_MODE;
                s_states[i].channel = static_cast<ledc_channel_t>(i - 8); // high-speed channel 0..3
            }

            ledc_channel_config_t channel_conf = {
                .gpio_num = s_states[i].pin,
                .speed_mode = s_states[i].mode,
                .channel = s_states[i].channel,
                .intr_type = LEDC_INTR_DISABLE,
                .timer_sel = (s_states[i].mode == LEDC_LOW_SPEED_MODE) ? LEDC_TIMER_0 : LEDC_TIMER_1,
                .duty = 0,
                .hpoint = 0
            };
            ledc_channel_config(&channel_conf);
        }
        
        m_initialized = true;
    }
    
    void setFrequency(uint8_t instrument, uint32_t frequency) {
        if (!isValidInstrument(instrument)) return;
        
        // Clamp frequency to valid range
        if (frequency > PWM_FREQUENCY_MAX) frequency = PWM_FREQUENCY_MAX;
        if (frequency < 1) frequency = 1;
        
        // Set frequency for the timer used by this instrument
        ledc_mode_t mode = s_states[instrument].mode;
        ledc_timer_t timer = (mode == LEDC_LOW_SPEED_MODE) ? LEDC_TIMER_0 : LEDC_TIMER_1;
        ledc_set_freq(mode, timer, frequency);
    }
    
    void setDutyCycle(uint8_t instrument, uint32_t dutyCycle) {
        if (!isValidInstrument(instrument)) return;
        ledc_mode_t mode = s_states[instrument].mode;
        ledc_set_duty(mode, s_states[instrument].channel, dutyCycle);
        ledc_update_duty(mode, s_states[instrument].channel);
    }
    
    void stopInstrument(uint8_t instrument) {
        if (!isValidInstrument(instrument)) return;
        ledc_mode_t mode = s_states[instrument].mode;
        ledc_set_duty(mode, s_states[instrument].channel, 0);
        ledc_update_duty(mode, s_states[instrument].channel);
        
        s_states[instrument].active = false;
        s_states[instrument].currentNote = 0;
        s_states[instrument].frequency = 0;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    // Note Frequency Calculations
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    
    static uint32_t noteToFrequency(uint8_t note) {
        // A4 (note 69) = 440 Hz
        // Formula: freq = 440 * 2^((note - 69) / 12)
        if (note > 127) return 440;
        
        float frequency = 440.0f * powf(2.0f, (note - 69) / 12.0f);
        return static_cast<uint32_t>(frequency + 0.5f);  // Round to nearest integer
    }
    
    static uint32_t applyPitchBend(uint32_t baseFrequency, uint16_t pitchBend) {
        // Pitch bend range: ±2 semitones
        // Center: 0x2000, Range: 0x0000 to 0x3FFF
        float bendSemitones = ((int16_t)pitchBend - 0x2000) * 2.0f / 0x2000;
        float bentFrequency = baseFrequency * powf(2.0f, bendSemitones / 12.0f);
        return static_cast<uint32_t>(bentFrequency + 0.5f);
    }
};

// Static member declarations (definitions in ESP32_PwmTimer.cpp)

#endif  // ARDUINO_ARCH_ESP32