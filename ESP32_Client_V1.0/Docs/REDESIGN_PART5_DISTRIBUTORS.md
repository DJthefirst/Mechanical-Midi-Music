# Part 5: Distributor System with Runtime Management

## Overview

The distributor system is **THE CRITICAL FEATURE** that enables runtime flexibility. The pure template V2.0's biggest flaw was eliminating runtime add/remove/modify of distributors. This design preserves V1.0's runtime management while adding optional polymorphic distributor types for custom algorithms.

## Why Runtime Management is Non-Negotiable

### Real-World Use Cases

1. **Field Configuration** - Musician adjusts note routing between songs without laptop
2. **Performance Experimentation** - Try different mappings in real-time during rehearsal
3. **Remote Management** - Server software sends distributor configs to devices
4. **Learning/Education** - Students experiment with different routing algorithms
5. **Dynamic Performances** - Change instrument mappings during a performance via MIDI

### What Pure Template V2.0 Lost

```cpp
// Pure template V2.0 (REJECTED) - Compile-time only
namespace DistributorConfig {
    using Dist0 = Distributor<Channels<0>, Instruments<0,1,2>, RoundRobin, ...>;
    using Dist1 = Distributor<Channels<1>, Instruments<3,4>, Sequential, ...>;
    using AllDistributors = DistributorList<Dist0, Dist1>;
}

// Problems:
// ❌ Can't add new distributor without recompiling
// ❌ Can't remove distributor without recompiling
// ❌ Can't modify channels/instruments without recompiling
// ❌ Can't save/load configurations at runtime
// ❌ SysEx distributor commands become useless
```

## Improved V2.0 Distributor Design

### Core Principles

1. **Runtime Management** - Add, remove, modify distributors at runtime
2. **Polymorphic Algorithms** - Support custom distribution algorithms
3. **Efficient Storage** - Compact binary serialization for NVS storage
4. **Type Safety** - Compile-time validation where possible
5. **Backward Compatible** - Keep V1.0 API working

### Distributor Base Class (Keep V1.0 Design)

```cpp
// Distributor.h - Runtime-manageable distributor
#pragma once

#include "InstrumentController.h"
#include "MidiMessage.h"
#include "Constants.h"
#include <array>
#include <cstdint>

// Serialized distributor size (24 bytes)
static constexpr uint8_t DISTRIBUTOR_NUM_CFG_BYTES = 24;

// Boolean flags
static constexpr uint8_t DISTRIBUTOR_BOOL_MUTED         = 0x01;
static constexpr uint8_t DISTRIBUTOR_BOOL_DAMPERPEDAL   = 0x02;
static constexpr uint8_t DISTRIBUTOR_BOOL_POLYPHONIC    = 0x04;
static constexpr uint8_t DISTRIBUTOR_BOOL_NOTEOVERWRITE = 0x08;

// Distribution methods
enum class DistributionMethod : uint8_t {
    RoundRobinBalance = 0,  // Balance active notes across instruments
    RoundRobin = 1,         // Simple round-robin
    Sequential = 2,         // Fill instruments in order
    Random = 3,             // Random instrument selection
    StraightThrough = 4,    // 1:1 channel to instrument mapping
    Custom = 255            // User-defined algorithm
};

/* Routes MIDI notes to instrument groups via configurable algorithms */
class Distributor {
protected:
    // Routing state
    uint8_t m_currentChannel = 0;
    uint8_t m_currentInstrument = 0;
    InstrumentController* m_ptrInstrumentController;

    // Configuration (bitmasks for efficiency)
    uint16_t m_channels = 0;      // 16-bit: enabled MIDI channels (max 16)
    uint32_t m_instruments = 0;   // 32-bit: enabled instruments (max 32)

    // Settings
    bool m_muted = false;
    bool m_damperPedal = false;
    bool m_polyphonic = true;
    bool m_noteOverwrite = false;
    uint8_t m_minNote = 0;
    uint8_t m_maxNote = 127;
    uint8_t m_numPolyphonicNotes = 1;
    DistributionMethod m_distributionMethod = DistributionMethod::RoundRobinBalance;

public:
    explicit Distributor(InstrumentController* ptrInstrumentController);
    virtual ~Distributor() = default;

    // === Core Processing ===
    virtual void processMessage(const MidiMessage& message);

    // === Serialization ===
    std::array<uint8_t, DISTRIBUTOR_NUM_CFG_BYTES> toSerial() const;
    void fromSerial(const uint8_t data[DISTRIBUTOR_NUM_CFG_BYTES]);

    // === Getters ===
    bool getMuted() const { return m_muted; }
    uint16_t getChannels() const { return m_channels; }
    uint32_t getInstruments() const { return m_instruments; }
    uint8_t getMinNote() const { return m_minNote; }
    uint8_t getMaxNote() const { return m_maxNote; }
    DistributionMethod getMethod() const { return m_distributionMethod; }

    // === Setters ===
    void setDistributor(const uint8_t profile[DISTRIBUTOR_NUM_CFG_BYTES]);
    void setDistributionMethod(DistributionMethod method);
    void setMuted(bool muted);
    void setDamperPedal(bool enable);
    void setPolyphonic(bool enable);
    void setNoteOverwrite(bool noteOverwrite);
    void setMinMaxNote(uint8_t minNote, uint8_t maxNote);
    void setNumPolyphonicNotes(uint8_t numPolyphonicNotes);
    void setChannels(uint16_t channels);
    void setInstruments(uint32_t instruments);
    void toggleMuted();

    // === Diagnostics ===
    virtual const char* getName() const { return "Distributor"; }
    virtual void printStatus() const;

protected:
    // === MIDI Event Handlers ===
    virtual void noteOnEvent(uint8_t note, uint8_t velocity, uint8_t channel);
    virtual void noteOffEvent(uint8_t note, uint8_t velocity);
    void keyPressureEvent(uint8_t note, uint8_t velocity);
    void programChangeEvent(uint8_t program);
    void channelPressureEvent(uint8_t velocity);
    void pitchBendEvent(uint16_t pitchBend, uint8_t channel);

    // === Routing Logic (overridable for custom algorithms) ===
    virtual uint8_t nextInstrument();
    uint8_t checkForNote(uint8_t note);
    
    // === Helper Functions ===
    bool distributorHasInstrument(uint8_t instrumentId) const;
    bool channelEnabled(uint8_t channel) const;
    bool noteInRange(uint8_t note) const;
};
```

### Implementation Details

```cpp
// Distributor.cpp - Key methods

Distributor::Distributor(InstrumentController* ptrInstrumentController)
    : m_ptrInstrumentController(ptrInstrumentController) {}

void Distributor::processMessage(const MidiMessage& message) {
    m_currentChannel = message.channel();

    // Check if this distributor handles this channel
    if (!channelEnabled(m_currentChannel)) return;
    
    // Check if muted
    if (m_muted) return;

    switch (message.type()) {
        case MIDI_NoteOff:
            noteOffEvent(message.data1, message.data2);
            break;
        case MIDI_NoteOn:
            noteOnEvent(message.data1, message.data2, message.channel());
            break;
        case MIDI_KeyPressure:
            keyPressureEvent(message.data1, message.data2);
            break;
        case MIDI_ProgramChange:
            programChangeEvent(message.data1);
            break;
        case MIDI_ChannelPressure:
            channelPressureEvent(message.data1);
            break;
        case MIDI_PitchBend:
            pitchBendEvent((message.data2 << 7) | message.data1, m_currentChannel);
            break;
        default:
            break;
    }
}

void Distributor::noteOnEvent(uint8_t note, uint8_t velocity, uint8_t channel) {
    // Note On with velocity 0 is Note Off
    if (velocity == 0) {
        noteOffEvent(note, velocity);
        return;
    }

    // Check note range
    if (!noteInRange(note)) return;

    // Get next instrument to play
    uint8_t instrument = nextInstrument();
    if (instrument != NONE) {
        m_ptrInstrumentController->playNote(instrument, note, velocity, channel);
    }
}

void Distributor::noteOffEvent(uint8_t note, uint8_t velocity) {
    // Find instrument playing this note
    uint8_t instrument = checkForNote(note);
    if (instrument != NONE) {
        m_ptrInstrumentController->stopNote(instrument, note, velocity);
    }
}

uint8_t Distributor::nextInstrument() {
    // Implement distribution algorithm
    switch (m_distributionMethod) {
        case DistributionMethod::RoundRobinBalance:
            return nextInstrumentRoundRobinBalance();
        case DistributionMethod::RoundRobin:
            return nextInstrumentRoundRobin();
        case DistributionMethod::Sequential:
            return nextInstrumentSequential();
        case DistributionMethod::Random:
            return nextInstrumentRandom();
        case DistributionMethod::StraightThrough:
            return nextInstrumentStraightThrough();
        default:
            return NONE;
    }
}

// Helper functions
bool Distributor::channelEnabled(uint8_t channel) const {
    return (m_channels & (1 << channel)) != 0;
}

bool Distributor::noteInRange(uint8_t note) const {
    return note >= m_minNote && note <= m_maxNote;
}

bool Distributor::distributorHasInstrument(uint8_t instrumentId) const {
    return (m_instruments & (1 << instrumentId)) != 0;
}
```

### Binary Serialization Format

```cpp
// Distributor serialization (24 bytes total)
std::array<uint8_t, DISTRIBUTOR_NUM_CFG_BYTES> Distributor::toSerial() const {
    std::array<uint8_t, DISTRIBUTOR_NUM_CFG_BYTES> data{};
    
    // Bytes 0-1: Channel mask (16-bit)
    data[0] = m_channels & 0xFF;
    data[1] = (m_channels >> 8) & 0xFF;
    
    // Bytes 2-5: Instrument mask (32-bit)
    data[2] = m_instruments & 0xFF;
    data[3] = (m_instruments >> 8) & 0xFF;
    data[4] = (m_instruments >> 16) & 0xFF;
    data[5] = (m_instruments >> 24) & 0xFF;
    
    // Byte 6: Boolean flags
    data[6] = 0;
    if (m_muted) data[6] |= DISTRIBUTOR_BOOL_MUTED;
    if (m_damperPedal) data[6] |= DISTRIBUTOR_BOOL_DAMPERPEDAL;
    if (m_polyphonic) data[6] |= DISTRIBUTOR_BOOL_POLYPHONIC;
    if (m_noteOverwrite) data[6] |= DISTRIBUTOR_BOOL_NOTEOVERWRITE;
    
    // Byte 7: Distribution method
    data[7] = static_cast<uint8_t>(m_distributionMethod);
    
    // Byte 8: Min note
    data[8] = m_minNote;
    
    // Byte 9: Max note
    data[9] = m_maxNote;
    
    // Byte 10: Num polyphonic notes
    data[10] = m_numPolyphonicNotes;
    
    // Bytes 11-23: Reserved for future use
    
    return data;
}

void Distributor::fromSerial(const uint8_t data[DISTRIBUTOR_NUM_CFG_BYTES]) {
    // Channels
    m_channels = data[0] | (data[1] << 8);
    
    // Instruments
    m_instruments = data[2] | (data[3] << 8) | (data[4] << 16) | (data[5] << 24);
    
    // Boolean flags
    m_muted = (data[6] & DISTRIBUTOR_BOOL_MUTED) != 0;
    m_damperPedal = (data[6] & DISTRIBUTOR_BOOL_DAMPERPEDAL) != 0;
    m_polyphonic = (data[6] & DISTRIBUTOR_BOOL_POLYPHONIC) != 0;
    m_noteOverwrite = (data[6] & DISTRIBUTOR_BOOL_NOTEOVERWRITE) != 0;
    
    // Distribution method
    m_distributionMethod = static_cast<DistributionMethod>(data[7]);
    
    // Note range
    m_minNote = data[8];
    m_maxNote = data[9];
    
    // Polyphony
    m_numPolyphonicNotes = data[10];
}
```

## Custom Distributor Algorithms (Polymorphic Extension)

### Arpeggio Distributor Example

```cpp
// ArpeggioDistributor.h - Custom algorithm
#pragma once

#include "Distributor.h"
#include <vector>

class ArpeggioDistributor : public Distributor {
private:
    struct NoteEvent {
        uint8_t note;
        uint32_t timestamp;
    };
    
    std::vector<NoteEvent> m_noteBuffer;
    uint32_t m_arpeggioPeriod = 200;  // ms between notes
    uint32_t m_lastNoteTime = 0;
    uint8_t m_currentStep = 0;

public:
    explicit ArpeggioDistributor(InstrumentController* controller)
        : Distributor(controller) {}

    const char* getName() const override { 
        return "Arpeggio Distributor"; 
    }

    void setArpeggioPeriod(uint32_t periodMs) {
        m_arpeggioPeriod = periodMs;
    }

protected:
    void noteOnEvent(uint8_t note, uint8_t velocity, uint8_t channel) override {
        // Buffer note instead of playing immediately
        m_noteBuffer.push_back({note, millis()});
    }

    void noteOffEvent(uint8_t note, uint8_t velocity) override {
        // Remove from buffer
        m_noteBuffer.erase(
            std::remove_if(m_noteBuffer.begin(), m_noteBuffer.end(),
                [note](const NoteEvent& e) { return e.note == note; }),
            m_noteBuffer.end()
        );
    }

public:
    void update() {
        // Called from main loop - plays buffered notes as arpeggio
        uint32_t now = millis();
        if (now - m_lastNoteTime < m_arpeggioPeriod) return;
        if (m_noteBuffer.empty()) return;

        m_lastNoteTime = now;
        
        // Play current step
        uint8_t instrument = nextInstrument();
        if (instrument != NONE) {
            const NoteEvent& event = m_noteBuffer[m_currentStep];
            m_ptrInstrumentController->playNote(instrument, event.note, 127, 0);
        }

        // Advance step
        m_currentStep = (m_currentStep + 1) % m_noteBuffer.size();
    }
};
```

## Message Handler Integration

### Runtime Distributor Management

```cpp
// MessageHandler.h - Manages distributor collection
#pragma once

#include "Distributor.h"
#include "InstrumentController.h"
#include "MidiMessage.h"
#include <vector>
#include <memory>

class MessageHandler {
private:
    // Polymorphic distributor storage - supports any distributor type
    std::vector<std::unique_ptr<Distributor>> m_distributors;
    InstrumentController* m_ptrInstrumentController;
    
    uint16_t m_srcDeviceId;
    uint16_t m_destDeviceId;

public:
    explicit MessageHandler(InstrumentController* controller);

    // Process incoming MIDI message
    void processMessage(const MidiMessage& message);

    // === Runtime Distributor Management ===
    
    // Add distributor (takes ownership)
    void addDistributor(std::unique_ptr<Distributor> distributor);
    
    // Add default distributor
    void addDistributor();
    
    // Add distributor from serialized data
    void addDistributor(const uint8_t data[DISTRIBUTOR_NUM_CFG_BYTES]);
    
    // Set/update distributor configuration
    void setDistributor(uint8_t id, const uint8_t data[DISTRIBUTOR_NUM_CFG_BYTES]);
    
    // Remove distributor
    void removeDistributor(uint8_t id);
    
    // Remove all distributors
    void removeAllDistributors();
    
    // Get distributor reference
    Distributor& getDistributor(uint8_t id);
    const Distributor& getDistributor(uint8_t id) const;
    
    // Get distributor serialized data
    std::array<uint8_t, DISTRIBUTOR_NUM_CFG_BYTES> getDistributorSerial(uint8_t id) const;
    
    // Get distributor count
    size_t getDistributorCount() const { return m_distributors.size(); }

    // === Distributor Factory (for custom types) ===
    template<typename DistributorType, typename... Args>
    void addCustomDistributor(Args&&... args) {
        m_distributors.push_back(
            std::make_unique<DistributorType>(
                m_ptrInstrumentController,
                std::forward<Args>(args)...
            )
        );
    }

private:
    void distributeMessage(const MidiMessage& message);
    void processCC(const MidiMessage& message);
    MidiMessage processSysEx(const MidiMessage& message);
};
```

### Implementation

```cpp
// MessageHandler.cpp

MessageHandler::MessageHandler(InstrumentController* controller)
    : m_ptrInstrumentController(controller)
    , m_srcDeviceId(Config::DEVICE_ID)
    , m_destDeviceId(0xFFFF) {}

void MessageHandler::addDistributor() {
    m_distributors.push_back(
        std::make_unique<Distributor>(m_ptrInstrumentController)
    );
}

void MessageHandler::addDistributor(std::unique_ptr<Distributor> distributor) {
    m_distributors.push_back(std::move(distributor));
}

void MessageHandler::addDistributor(const uint8_t data[DISTRIBUTOR_NUM_CFG_BYTES]) {
    auto dist = std::make_unique<Distributor>(m_ptrInstrumentController);
    dist->fromSerial(data);
    m_distributors.push_back(std::move(dist));
}

void MessageHandler::setDistributor(uint8_t id, const uint8_t data[DISTRIBUTOR_NUM_CFG_BYTES]) {
    if (id < m_distributors.size()) {
        m_distributors[id]->fromSerial(data);
    } else {
        addDistributor(data);
    }
}

void MessageHandler::removeDistributor(uint8_t id) {
    if (id < m_distributors.size()) {
        m_distributors.erase(m_distributors.begin() + id);
    }
}

void MessageHandler::removeAllDistributors() {
    m_distributors.clear();
}

Distributor& MessageHandler::getDistributor(uint8_t id) {
    if (id >= m_distributors.size()) {
        throw std::out_of_range("Distributor ID out of range");
    }
    return *m_distributors[id];
}

void MessageHandler::processMessage(const MidiMessage& message) {
    if (message.isSysEx()) {
        processSysEx(message);
    } else {
        distributeMessage(message);
    }
}

void MessageHandler::distributeMessage(const MidiMessage& message) {
    // Route message through all distributors
    for (auto& distributor : m_distributors) {
        distributor->processMessage(message);
    }
}
```

## SysEx Protocol for Runtime Configuration

### Distributor Management Commands

```cpp
// SysEx format: F0 7D <DevID_H> <DevID_L> <Command> <Data...> F7

// Commands:
constexpr uint8_t SYSEX_CMD_ADD_DISTRIBUTOR    = 0x10;
constexpr uint8_t SYSEX_CMD_REMOVE_DISTRIBUTOR = 0x11;
constexpr uint8_t SYSEX_CMD_SET_DISTRIBUTOR    = 0x12;
constexpr uint8_t SYSEX_CMD_GET_DISTRIBUTOR    = 0x13;
constexpr uint8_t SYSEX_CMD_GET_NUM_DISTRIBUTORS = 0x14;

// Example: Add distributor
// F0 7D 00 42 10 <24 bytes of distributor data> F7

void MessageHandler::processSysEx(const MidiMessage& message) {
    if (message.sysExLength < 5) return;
    
    // Parse device ID
    uint16_t deviceId = (message.sysExData[0] << 7) | message.sysExData[1];
    if (deviceId != m_srcDeviceId && deviceId != 0x7F) return;  // Not for us
    
    uint8_t command = message.sysExData[2];
    
    switch (command) {
        case SYSEX_CMD_ADD_DISTRIBUTOR:
            if (message.sysExLength >= DISTRIBUTOR_NUM_CFG_BYTES + 3) {
                addDistributor(&message.sysExData[3]);
            }
            break;
            
        case SYSEX_CMD_REMOVE_DISTRIBUTOR:
            if (message.sysExLength >= 4) {
                removeDistributor(message.sysExData[3]);
            }
            break;
            
        case SYSEX_CMD_SET_DISTRIBUTOR:
            if (message.sysExLength >= DISTRIBUTOR_NUM_CFG_BYTES + 4) {
                uint8_t id = message.sysExData[3];
                setDistributor(id, &message.sysExData[4]);
            }
            break;
            
        case SYSEX_CMD_GET_DISTRIBUTOR:
            if (message.sysExLength >= 4) {
                uint8_t id = message.sysExData[3];
                auto data = getDistributorSerial(id);
                // Send response SysEx
                sendDistributorData(id, data);
            }
            break;
            
        case SYSEX_CMD_GET_NUM_DISTRIBUTORS:
            // Send response with distributor count
            sendDistributorCount();
            break;
    }
}
```

## NVS Storage Integration

### Save/Load Distributor Configurations

```cpp
// LocalStorage.h additions

class LocalStorage {
public:
    // Save all distributors
    static bool saveDistributors(const std::vector<std::unique_ptr<Distributor>>& distributors) {
        if (!initialize()) return false;
        
        // Save count
        uint8_t count = distributors.size();
        if (!nvs_set_u8(s_handle, "dist_count", count)) {
            return false;
        }
        
        // Save each distributor
        for (size_t i = 0; i < distributors.size(); i++) {
            char key[16];
            snprintf(key, sizeof(key), "dist_%zu", i);
            
            auto data = distributors[i]->toSerial();
            if (!nvs_set_blob(s_handle, key, data.data(), data.size())) {
                return false;
            }
        }
        
        nvs_commit(s_handle);
        return true;
    }
    
    // Load all distributors
    static bool loadDistributors(std::vector<std::unique_ptr<Distributor>>& distributors,
                                  InstrumentController* controller) {
        if (!initialize()) return false;
        
        // Load count
        uint8_t count = 0;
        if (!nvs_get_u8(s_handle, "dist_count", &count)) {
            return false;
        }
        
        // Load each distributor
        distributors.clear();
        for (uint8_t i = 0; i < count; i++) {
            char key[16];
            snprintf(key, sizeof(key), "dist_%d", i);
            
            uint8_t data[DISTRIBUTOR_NUM_CFG_BYTES];
            size_t length = sizeof(data);
            
            if (nvs_get_blob(s_handle, key, data, &length)) {
                auto dist = std::make_unique<Distributor>(controller);
                dist->fromSerial(data);
                distributors.push_back(std::move(dist));
            }
        }
        
        return !distributors.empty();
    }
};
```

### Usage in main.cpp

```cpp
// main.cpp

void setup() {
    network.begin();
    instrument.resetAll();
    
    // Try to load saved distributors
    if (!LocalStorage::loadDistributors(messageHandler.getDistributors(), &instrument)) {
        Serial.println("No saved distributors, creating default");
        
        // Create default distributor: All channels -> All instruments
        auto defaultDist = std::make_unique<Distributor>(&instrument);
        defaultDist->setChannels(0xFFFF);  // All channels
        defaultDist->setInstruments((1 << Config::NUM_INSTRUMENTS) - 1);  // All instruments
        messageHandler.addDistributor(std::move(defaultDist));
    }
    
    Serial.printf("Loaded %zu distributors\n", messageHandler.getDistributorCount());
}

void loop() {
    auto msg = network.readMessage();
    if (msg.has_value()) {
        messageHandler.processMessage(*msg);
        
        // Save distributors if configuration changed (debounced)
        if (configChanged) {
            LocalStorage::saveDistributors(messageHandler.getDistributors());
            configChanged = false;
        }
    }
}
```

## Benefits Summary

### Runtime Flexibility
- ✅ **Add/remove distributors** at runtime (essential feature preserved)
- ✅ **Modify configurations** without recompiling
- ✅ **Save/load from NVS** for persistent configurations
- ✅ **Remote configuration** via SysEx commands
- ✅ **Custom algorithms** via polymorphism

### Performance
- ✅ **Efficient bitmask routing** - O(1) channel/instrument checks
- ✅ **Compact storage** - 24 bytes per distributor
- ✅ **Minimal overhead** - virtual dispatch negligible vs MIDI I/O
- ✅ **Static distribution logic** - hot path optimized

### Usability
- ✅ **Standard MIDI protocol** - SysEx commands for management
- ✅ **Type-safe API** - Compiler catches errors
- ✅ **Clear interface** - Easy to understand and extend
- ✅ **Backward compatible** - V1.0 API preserved

### Extensibility
- ✅ **Polymorphic distributors** - Custom algorithms supported
- ✅ **Factory pattern** - Easy to add new types
- ✅ **Plugin architecture** - Users can create custom distributors
- ✅ **Future-proof** - Reserved bytes in serialization

## Migration from V1.0

### Required Changes: MINIMAL!

V1.0 distributor architecture is already correct! Only enhancements needed:

1. **Change storage to unique_ptr** (for polymorphism support)
2. **Add `fromSerial()` method** (for better API)
3. **Add `getName()` method** (for diagnostics)

```cpp
// V1.0 (works, but not polymorphic)
class MessageHandler {
    std::vector<Distributor> m_distributors;  // By value
};

// V2.0 (polymorphic support)
class MessageHandler {
    std::vector<std::unique_ptr<Distributor>> m_distributors;  // By pointer
};
```

That's it! The core V1.0 design was already solid.

## Next Steps

With distributor architecture defined, we can proceed to:
- **Part 6**: Implementation strategy and migration path

---

*This design preserves V1.0's critical runtime management capabilities while adding polymorphic distributor support for custom algorithms—the best of both worlds.*
