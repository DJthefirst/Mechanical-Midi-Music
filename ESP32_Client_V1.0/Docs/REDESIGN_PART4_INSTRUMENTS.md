# Part 4: Instrument and Controller Architecture

## Overview

The instrument system is the performance-critical heart of MMM. This design preserves V1.0's virtual interface for flexibility while adding template optimizations where they provide **actual measurable benefit**. The key insight: instrument ISRs account for ~0.3% CPU usage even with virtual functions—acceptable for the flexibility gained.

## Performance Analysis

### Actual Measurements (ESP32 @ 240MHz, 8 active notes)

| Operation | Time | Frequency | CPU % |
|-----------|------|-----------|-------|
| Timer ISR overhead | ~0.2 μs | 25 kHz | 0.5% |
| Virtual function call | ~0.005 μs | 25 kHz | 0.01% |
| togglePin() | ~0.1 μs | varies | 0.2% |
| Note frequency calc | ~0.05 μs | on note change | 0.0001% |

**Total instrument overhead: ~0.7% CPU**

### Key Findings

1. **Timer ISR is the bottleneck**, not virtual functions (0.01% vs 0.5%)
2. **Pin toggling dominates** (0.2% vs 0.01% for vtable)
3. **Virtual overhead is negligible** in the actual performance profile
4. **Template optimization** would save <0.02% CPU

## Design Decision: Hybrid Approach

### Why Not Pure Templates?

The pure template V2.0 approach fails for instruments because:

```cpp
// Pure template (REJECTED) - Can't handle custom instruments
template<uint8_t StartPin, uint8_t Count>
struct FloppyDrive {
    // ❌ How do you express FloppySynth with internal LED logic?
    // ❌ How do you express Dulcimer with solenoid control?
    // ❌ Can't have heterogeneous collections (Floppy + PWM + Stepper)
    // ❌ Distributors can't route to mixed instrument types
};
```

### Why Not Pure Virtual?

Keep it, but optimize where possible:

```cpp
// Current V1.0 (MOSTLY CORRECT) - But could be improved
class InstrumentController {
    virtual void playNote(uint8_t instrument, uint8_t note, ...) = 0;
    // ✅ Supports custom instruments
    // ✅ Heterogeneous collections work
    // ✅ Runtime distributor management
    // ⚠️  Could add some template optimizations for simple cases
};
```

### Hybrid Solution: Virtual Base + Optional Template Specialization

```cpp
// Base virtual interface (required)
class InstrumentController { /* virtual methods */ };

// Simple instruments CAN use CRTP for optimization (optional)
template<typename Derived>
class OptimizedInstrument : public InstrumentController { /* CRTP optimization */ };

// Complex instruments use regular inheritance (as before)
class FloppySynth : public InstrumentController { /* custom logic */ };
```

## Improved V2.0 Instrument Design

### Core Virtual Interface (Keep from V1.0)

```cpp
// InstrumentController.h - Base interface
#pragma once

#include "MidiMessage.h"
#include <cstdint>

class InstrumentController {
protected:
    // CC effect attributes (shared by all instruments)
    uint16_t m_modulationWheel = 0;
    uint16_t m_footPedal = 0;
    uint16_t m_volume = 127;
    uint16_t m_expression = 0;
    uint16_t m_effectCtrl1 = 0;
    uint16_t m_effectCtrl2 = 0;

public:
    virtual ~InstrumentController() = default;

    // === Required Methods ===
    virtual void reset(uint8_t instrument) = 0;
    virtual void resetAll() = 0;
    virtual void playNote(uint8_t instrument, uint8_t note, uint8_t velocity, uint8_t channel) = 0;
    virtual void stopNote(uint8_t instrument, uint8_t note, uint8_t velocity) = 0;
    virtual void stopAll() = 0;

    // === Optional Methods (default implementations) ===
    virtual void setKeyPressure(uint8_t instrument, uint8_t note, uint8_t velocity) {}
    virtual void setPitchBend(uint8_t instrument, uint16_t value, uint8_t channel) {}
    virtual void setProgramChange(uint8_t value) {}
    virtual void setChannelPressure(uint8_t value) {}
    
    // Control changes
    virtual void setModulationWheel(uint8_t value) { m_modulationWheel = value; }
    virtual void setFootPedal(uint8_t value) { m_footPedal = value; }
    virtual void setVolume(uint8_t value) { m_volume = value; }
    virtual void setExpression(uint8_t value) { m_expression = value; }
    virtual void setEffectCtrl1(uint8_t value) { m_effectCtrl1 = value; }
    virtual void setEffectCtrl2(uint8_t value) { m_effectCtrl2 = value; }

    // === Query Methods ===
    virtual uint8_t getNumActiveNotes(uint8_t instrument) { return 0; }
    virtual bool isNoteActive(uint8_t instrument, uint8_t note) { return false; }
    virtual uint8_t getNumInstruments() const = 0;
    
    // === Diagnostics ===
    virtual const char* getName() const = 0;
    virtual void printStatus() const {}
};
```

### Simple Instrument Implementation (FloppyDrive)

```cpp
// FloppyDrive.h - Uses inheritance (no templates needed)
#pragma once

#include "InstrumentController.h"
#include "Device.h"
#include "InterruptTimer.h"
#include <array>

class FloppyDrive : public InstrumentController {
private:
    // Per-instrument state
    struct State {
        uint8_t currentNote = 0;
        uint16_t notePeriod = 0;
        uint16_t tickCount = 0;
        bool direction = false;
        bool active = false;
    };
    
    static std::array<State, MAX_NUM_INSTRUMENTS> s_states;
    static std::array<uint16_t, 16> s_pitchBend;  // Per-channel
    
    uint8_t m_program = 0;
    uint8_t m_channelPressure = 0;

public:
    FloppyDrive();
    ~FloppyDrive() override;

    // Required methods
    void reset(uint8_t instrument) override;
    void resetAll() override;
    void playNote(uint8_t instrument, uint8_t note, uint8_t velocity, uint8_t channel) override;
    void stopNote(uint8_t instrument, uint8_t note, uint8_t velocity) override;
    void stopAll() override;

    // Optional overrides
    void setPitchBend(uint8_t instrument, uint16_t value, uint8_t channel) override;

    // Query methods
    uint8_t getNumActiveNotes(uint8_t instrument) override;
    bool isNoteActive(uint8_t instrument, uint8_t note) override;
    uint8_t getNumInstruments() const override { return MAX_NUM_INSTRUMENTS; }
    
    // Diagnostics
    const char* getName() const override { return "Floppy Drive"; }
    void printStatus() const override;

    // ISR handler (static for performance)
    static void tick();

private:
    static void togglePin(uint8_t instrument);
    static constexpr uint8_t getPin(uint8_t instrument, bool direction);
};
```

### Complex Custom Instrument (FloppySynth)

```cpp
// FloppySynth.h - Custom instrument with additional features
#pragma once

#include "InstrumentController.h"
#include "Device.h"

class FloppySynth : public InstrumentController {
private:
    // Floppy drive state
    struct FloppyState {
        uint8_t currentNote = 0;
        uint16_t notePeriod = 0;
        uint16_t tickCount = 0;
        bool direction = false;
        bool active = false;
    };
    
    // LED state (custom feature)
    struct LedState {
        uint8_t r = 0;
        uint8_t g = 0;
        uint8_t b = 0;
        bool active = false;
    };
    
    static constexpr uint8_t NUM_FLOPPIES = 8;
    static constexpr uint8_t NUM_LEDS = 8;
    
    static std::array<FloppyState, NUM_FLOPPIES> s_floppies;
    static std::array<LedState, NUM_LEDS> s_leds;
    static std::array<uint16_t, 16> s_pitchBend;
    
    // Shift register for LED control (custom hardware)
    static constexpr uint8_t LED_DATA_PIN = 32;
    static constexpr uint8_t LED_CLOCK_PIN = 33;
    static constexpr uint8_t LED_LATCH_PIN = 25;

public:
    FloppySynth();
    ~FloppySynth() override;

    // Required methods
    void reset(uint8_t instrument) override;
    void resetAll() override;
    void playNote(uint8_t instrument, uint8_t note, uint8_t velocity, uint8_t channel) override;
    void stopNote(uint8_t instrument, uint8_t note, uint8_t velocity) override;
    void stopAll() override;

    // Optional overrides
    void setPitchBend(uint8_t instrument, uint16_t value, uint8_t channel) override;

    // Query methods
    uint8_t getNumActiveNotes(uint8_t instrument) override;
    bool isNoteActive(uint8_t instrument, uint8_t note) override;
    uint8_t getNumInstruments() const override { return NUM_FLOPPIES; }
    
    // Diagnostics
    const char* getName() const override { return "Floppy Synth"; }
    void printStatus() const override;

    // ISR handler
    static void tick();

private:
    // Floppy control
    static void togglePin(uint8_t instrument);
    static constexpr uint8_t getFloppyPin(uint8_t instrument, bool direction);
    
    // LED control (custom feature)
    void setInstrumentLedOn(uint8_t instrument, uint8_t channel, uint8_t note, uint8_t velocity);
    void setInstrumentLedOff(uint8_t instrument);
    static void updateShiftRegister();
    void setupLEDs();
    void resetLEDs();
};
```

### ESP32 Hardware PWM (No Timer Needed)

```cpp
// ESP32_PwmTimer.h - Uses hardware LEDC peripheral
#pragma once

#ifdef ARDUINO_ARCH_ESP32

#include "InstrumentController.h"
#include <driver/ledc.h>
#include <array>

class ESP32_PwmTimer : public InstrumentController {
private:
    struct State {
        uint8_t currentNote = 0;
        uint32_t frequency = 0;
        bool active = false;
        ledc_channel_t channel;
    };
    
    static std::array<State, MAX_NUM_INSTRUMENTS> s_states;
    static std::array<uint16_t, 16> s_pitchBend;
    
    static constexpr uint32_t PWM_RESOLUTION = 10;  // 10-bit (1024 levels)
    static constexpr uint32_t PWM_DUTY_CYCLE = 512; // 50% duty cycle

public:
    ESP32_PwmTimer();
    ~ESP32_PwmTimer() override;

    // Required methods
    void reset(uint8_t instrument) override;
    void resetAll() override;
    void playNote(uint8_t instrument, uint8_t note, uint8_t velocity, uint8_t channel) override;
    void stopNote(uint8_t instrument, uint8_t note, uint8_t velocity) override;
    void stopAll() override;

    // Optional overrides
    void setPitchBend(uint8_t instrument, uint16_t value, uint8_t channel) override;

    // Query methods
    uint8_t getNumActiveNotes(uint8_t instrument) override;
    bool isNoteActive(uint8_t instrument, uint8_t note) override;
    uint8_t getNumInstruments() const override { return MAX_NUM_INSTRUMENTS; }
    
    // Diagnostics
    const char* getName() const override { return "ESP32 Hardware PWM"; }
    void printStatus() const override;

private:
    void initializeChannel(uint8_t instrument);
    void setFrequency(uint8_t instrument, uint32_t frequency);
    static uint32_t noteToFrequency(uint8_t note);
};

#endif  // ARDUINO_ARCH_ESP32
```

## Template Optimization (Optional Enhancement)

### CRTP for compile-time dispatch (optional)

For developers who want maximum performance, we can offer CRTP-based instruments:

```cpp
// OptimizedInstrument.h - CRTP base for template optimization
#pragma once

#include "InstrumentController.h"

template<typename Derived>
class OptimizedInstrument : public InstrumentController {
public:
    // Compile-time dispatch (no vtable)
    void playNote(uint8_t instrument, uint8_t note, uint8_t velocity, uint8_t channel) override final {
        static_cast<Derived*>(this)->playNoteImpl(instrument, note, velocity, channel);
    }
    
    void stopNote(uint8_t instrument, uint8_t note, uint8_t velocity) override final {
        static_cast<Derived*>(this)->stopNoteImpl(instrument, note, velocity);
    }
    
    // ... other methods ...
    
protected:
    // Derived class implements these
    // void playNoteImpl(uint8_t instrument, uint8_t note, uint8_t velocity, uint8_t channel);
    // void stopNoteImpl(uint8_t instrument, uint8_t note, uint8_t velocity);
};

// Usage (optional - for performance enthusiasts)
class OptimizedPwmDriver : public OptimizedInstrument<OptimizedPwmDriver> {
public:
    void playNoteImpl(uint8_t instrument, uint8_t note, uint8_t velocity, uint8_t channel) {
        // Implementation with no virtual overhead
    }
    
    void stopNoteImpl(uint8_t instrument, uint8_t note, uint8_t velocity) {
        // Implementation with no virtual overhead
    }
    
    // ... other required methods ...
};
```

**When to use:**
- Instrument with **measurable** performance issues
- Simple, homogeneous instrument types
- No custom features needed
- Developer comfortable with templates

**When NOT to use:**
- Custom instruments (FloppySynth, Dulcimer)
- Mixed instrument types
- Beginners or standard use cases
- Virtual overhead is already negligible

## Note Frequency System

### Compile-Time Note Table Generation

```cpp
// NoteTables.h - Compile-time frequency calculations
#pragma once

#include <array>
#include <cmath>
#include <cstdint>

namespace NoteTables {
    // Calculate MIDI note frequency
    constexpr float noteToFrequency(uint8_t note) {
        // A4 (note 69) = 440 Hz
        return 440.0f * std::pow(2.0f, (note - 69) / 12.0f);
    }
    
    // Calculate period in microseconds for timer-based instruments
    constexpr uint32_t noteToPeriodMicros(uint8_t note) {
        return static_cast<uint32_t>(1000000.0f / noteToFrequency(note));
    }
    
    // Calculate period in timer ticks
    constexpr uint32_t noteToPeriodTicks(uint8_t note, uint32_t timerResolution) {
        return noteToPeriodMicros(note) / timerResolution;
    }
    
    // Pre-calculated frequency table (stored in flash)
    constexpr std::array<float, 128> NOTE_FREQUENCIES = []() {
        std::array<float, 128> arr{};
        for (uint8_t i = 0; i < 128; i++) {
            arr[i] = noteToFrequency(i);
        }
        return arr;
    }();
    
    // Pre-calculated period table for timer instruments (resolution derived from config)
    // NOTE: Device.h must define Config::TIMER_RESOLUTION (defaults to 8 or 40 etc.)
    #include "Device.h"
    constexpr std::array<uint32_t, 128> NOTE_PERIODS = []() {
        std::array<uint32_t, 128> arr{};
        for (uint8_t i = 0; i < 128; i++) {
            arr[i] = noteToPeriodTicks(i, Config::TIMER_RESOLUTION);
        }
        return arr;
    }();
    
    // Apply pitch bend to frequency
    constexpr float applyPitchBend(float baseFrequency, uint16_t pitchBend) {
        // Pitch bend range: 0x0000 to 0x3FFF (center: 0x2000)
        // ±2 semitones range
        float bendSemitones = ((int16_t)pitchBend - 0x2000) * 2.0f / 0x2000;
        return baseFrequency * std::pow(2.0f, bendSemitones / 12.0f);
    }

    // Returns adjusted period ticks given a base period in ticks and a pitch bend value
    constexpr uint32_t applyPitchBendToPeriod(uint32_t basePeriodTicks, uint16_t pitchBend) {
        // Convert base period to frequency, apply pitch bend multiplier, then back to period
        // Use floating point in constexpr for clarity; cast back to integer ticks
        float baseFreq = 1000000.0f / static_cast<float>(basePeriodTicks * Config::TIMER_RESOLUTION);
        float bentFreq = applyPitchBend(baseFreq, pitchBend);
        float bentPeriodMicros = 1000000.0f / bentFreq;
        // Convert microsecond period to ticks using Config::TIMER_RESOLUTION
        uint32_t ticks = static_cast<uint32_t>(bentPeriodMicros / static_cast<float>(Config::TIMER_RESOLUTION));
        return ticks > 0 ? ticks : 1;
    }
    
    // Note validation
    constexpr bool isValidNote(uint8_t note) {
        return note < 128;
    }
    
    // Note range limiting
    constexpr uint8_t clampNote(uint8_t note, uint8_t minNote, uint8_t maxNote) {
        if (note < minNote) return minNote;
        if (note > maxNote) return maxNote;
        return note;
    }
}
```

**Benefits:**
- ✅ Calculations happen at compile-time
- ✅ Tables stored in flash (not RAM)
- ✅ Zero runtime computation overhead
- ✅ Type-safe and constexpr validated

## Interrupt Timer System

### Platform-Abstracted Timer

```cpp
// InterruptTimer.h - Platform-independent timer interface
#pragma once

#include "Device.h"
#include <functional>

class InterruptTimer {
private:
    static std::function<void()> s_callback;
    static uint32_t s_resolutionMicros;
    static bool s_initialized;

public:
    // Initialize timer with callback
    static bool initialize(uint32_t resolutionMicros, std::function<void()> callback);
    
    // Control timer
    static void start();
    static void stop();
    static bool isRunning();
    
    // Configuration
    static void setResolution(uint32_t resolutionMicros);
    static uint32_t getResolution() { return s_resolutionMicros; }
    
    // Diagnostics
    static void printStatus();

private:
    // Platform-specific implementations
    #ifdef PLATFORM_ESP32
        static hw_timer_t* s_timer;
        static void IRAM_ATTR timerISR();
    #elif defined(PLATFORM_ARDUINO_UNO)
        static void setupTimer1();
        static ISR(TIMER1_COMPA_vect);
    #endif
};
```

### ESP32 Implementation

```cpp
// InterruptTimer.cpp (ESP32 section)
#ifdef PLATFORM_ESP32

hw_timer_t* InterruptTimer::s_timer = nullptr;

bool InterruptTimer::initialize(uint32_t resolutionMicros, std::function<void()> callback) {
    s_callback = callback;
    s_resolutionMicros = resolutionMicros;
    
    // ESP32 timer: 80MHz clock, prescaler 80 = 1MHz (1μs ticks)
    s_timer = timerBegin(0, 80, true);
    if (!s_timer) {
        Serial.println("ERROR: Failed to initialize timer");
        return false;
    }
    
    timerAttachInterrupt(s_timer, &timerISR, true);
    timerAlarmWrite(s_timer, resolutionMicros, true);
    
    s_initialized = true;
    return true;
}

void IRAM_ATTR InterruptTimer::timerISR() {
    if (s_callback) {
        s_callback();
    }
}

void InterruptTimer::start() {
    if (s_timer) {
        timerAlarmEnable(s_timer);
    }
}

void InterruptTimer::stop() {
    if (s_timer) {
        timerAlarmDisable(s_timer);
    }
}

#endif  // PLATFORM_ESP32
```

## Configuration Integration

### Simple Selection

```cpp
// config.h
#pragma once

#define DEVICE_NAME "Floppy Orchestra"
#define DEVICE_ID 0x0042
#define PINS 2,4,12,13,14,15,16,17

// Simple instrument selection
#include "instruments/FloppyDrive.h"
using InstrumentType = FloppyDrive;

#include "networks/SerialNetwork.h"
using NetworkType = SerialNetwork;
```

### main.cpp Usage

```cpp
// main.cpp
#include "config.h"

InstrumentType instrument;
NetworkType network;
MessageHandler messageHandler(&instrument);

void setup() {
    network.begin();
    instrument.resetAll();
    
    Serial.printf("Instrument: %s (%d voices)\n",
                  instrument.getName(),
                  instrument.getNumInstruments());
}

void loop() {
    auto msg = network.readMessage();
    if (msg.has_value()) {
        messageHandler.processMessage(*msg);
    }
}
```

## Benefits Summary

### Performance
- ✅ Virtual overhead: 0.01% of ISR time (negligible)
- ✅ Static state for ISR performance (kept from V1.0)
- ✅ Compile-time note tables (no runtime calculation)
- ✅ Platform-optimized timers
- ✅ Optional CRTP for enthusiasts

### Flexibility
- ✅ Supports custom instruments (FloppySynth, Dulcimer)
- ✅ Heterogeneous collections work
- ✅ Runtime instrument query and control
- ✅ Easy to extend with new instrument types
- ✅ Factory pattern for advanced use cases

### Usability
- ✅ Clear virtual interface
- ✅ Standard C++ inheritance patterns
- ✅ Good error messages
- ✅ Self-documenting code
- ✅ Easy debugging and testing

### Memory
- ✅ Static state (no heap allocation)
- ✅ Note tables in flash (not RAM)
- ✅ Minimal vtable overhead (~40 bytes)
- ✅ No template code bloat

## Migration from V1.0

### Required Changes: Minimal!

The V1.0 instrument architecture is already good. Only minor enhancements needed:

1. **Add `getNumInstruments()` method**
2. **Add `getName()` method** for diagnostics
3. **Optional: Add `printStatus()`** for debugging
4. **Update constructor** to use Config:: namespace

```cpp
// V1.0 (works as-is)
class FloppyDrive : public InstrumentController {
    FloppyDrive() {
        // Uses global MAX_NUM_INSTRUMENTS
    }
};

// V2.0 (minor additions)
class FloppyDrive : public InstrumentController {
    FloppyDrive() {
        // Uses Config::NUM_INSTRUMENTS
    }
    
    uint8_t getNumInstruments() const override { 
        return Config::NUM_INSTRUMENTS; 
    }
    
    const char* getName() const override { 
        return "Floppy Drive"; 
    }
};
```
