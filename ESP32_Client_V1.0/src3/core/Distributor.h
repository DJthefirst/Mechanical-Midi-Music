/*
 * Distributor.h - V2.0 Runtime-Manageable Distributor System
 *
 * Part of the Mechanical MIDI Music V2.0 redesign
 * Based on REDESIGN_PART5_DISTRIBUTORS.md specifications
 *
 * Key improvements over V1.0:
 * - Enhanced polymorphic support for custom algorithms
 * - Better serialization/deserialization 
 * - Runtime management preserved (CRITICAL)
 * - Improved diagnostics and status reporting
 */

#pragma once

#include "core/Constants.h"
#include "core/MidiMessage.h"
#include "instruments/InstrumentController.h"
#include <array>
#include <cstdint>

////////////////////////////////////////////////////////////////////////////////////////////////////
// Distributor Class - Routes MIDI to Instruments
////////////////////////////////////////////////////////////////////////////////////////////////////

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

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    // Core Processing
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    
    // Process incoming MIDI message
    virtual void processMessage(const MidiMessage& message);

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    // Serialization for Runtime Management
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    
    // Convert to binary format for storage/transmission
    std::array<uint8_t, DISTRIBUTOR_NUM_CFG_BYTES> toSerial() const;
    
    // Load from binary format
    void fromSerial(const uint8_t data[DISTRIBUTOR_NUM_CFG_BYTES]);

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    // Configuration Getters
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    
    bool getMuted() const { return m_muted; }
    uint16_t getChannels() const { return m_channels; }
    uint32_t getInstruments() const { return m_instruments; }
    uint8_t getMinNote() const { return m_minNote; }
    uint8_t getMaxNote() const { return m_maxNote; }
    uint8_t getNumPolyphonicNotes() const { return m_numPolyphonicNotes; }
    DistributionMethod getMethod() const { return m_distributionMethod; }
    bool getDamperPedal() const { return m_damperPedal; }
    bool getPolyphonic() const { return m_polyphonic; }
    bool getNoteOverwrite() const { return m_noteOverwrite; }

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    // Configuration Setters
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    
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

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    // Diagnostics - V2.0 Enhancement
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    
    virtual const char* getName() const { return "Distributor"; }
    // virtual void printStatus() const;

protected:
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    // MIDI Event Handlers
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    
    virtual void noteOnEvent(uint8_t note, uint8_t velocity, uint8_t channel);
    virtual void noteOffEvent(uint8_t note, uint8_t velocity);
    void keyPressureEvent(uint8_t note, uint8_t velocity);
    void programChangeEvent(uint8_t program);
    void channelPressureEvent(uint8_t velocity);
    void pitchBendEvent(uint16_t pitchBend, uint8_t channel);

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    // Routing Logic (overridable for custom algorithms)
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    
    virtual uint8_t nextInstrument();
    uint8_t checkForNote(uint8_t note);
    
    // Specific distribution methods
    uint8_t nextInstrumentRoundRobin();
    uint8_t nextInstrumentRoundRobinBalance();
    uint8_t nextInstrumentSequential();
    uint8_t nextInstrumentRandom();
    uint8_t nextInstrumentStraightThrough();

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    // Helper Functions
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    
    bool distributorHasInstrument(uint8_t instrumentId) const;
    bool channelEnabled(uint8_t channel) const;
    bool noteInRange(uint8_t note) const;
    uint8_t countEnabledInstruments() const;
    uint8_t getEnabledInstrument(uint8_t index) const;
};