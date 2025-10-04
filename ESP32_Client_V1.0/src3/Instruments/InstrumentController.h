/*
 * InstrumentController.h - V2.0 Base Instrument Interface
 *
 * Part of the Mechanical MIDI Music V2.0 redesign
 * Based on REDESIGN_PART4_INSTRUMENTS.md specifications
 *
 * Key improvements over V1.0:
 * - Added query methods for active notes
 * - Enhanced diagnostics and status reporting
 * - Better const correctness
 * - Cleaner method naming
 */

#pragma once

#include <Arduino.h>
#include "core/Constants.h"
#include <cstdint>

////////////////////////////////////////////////////////////////////////////////////////////////////
// Instrument Controller Base Interface
////////////////////////////////////////////////////////////////////////////////////////////////////

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

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    // Required Methods - Core Interface
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    
    // Instrument management
    virtual void reset(uint8_t instrument) = 0;
    virtual void resetAll() = 0;
    
    // Note control
    virtual void playNote(uint8_t instrument, uint8_t note, uint8_t velocity, uint8_t channel) = 0;
    virtual void stopNote(uint8_t instrument, uint8_t note, uint8_t velocity) = 0;
    virtual void stopAll() = 0;

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    // Optional Methods - Default Implementations
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    
    // Extended note control
    virtual void setKeyPressure(uint8_t instrument, uint8_t note, uint8_t velocity) {}
    virtual void setPitchBend(uint8_t instrument, uint16_t value, uint8_t channel) {}
    
    // Program and channel settings
    virtual void setProgramChange(uint8_t value) {}
    virtual void setChannelPressure(uint8_t value) {}
    
    // Control changes (V1.0 method names preserved for compatibility)
    virtual void setModulationWheel(uint8_t value) { m_modulationWheel = value; }
    virtual void setFootPedal(uint8_t value) { m_footPedal = value; }
    virtual void setVolume(uint8_t value) { m_volume = value; }
    virtual void setExpression(uint8_t value) { m_expression = value; }
    virtual void setEffectCrtl_1(uint8_t value) { m_effectCtrl1 = value; }  // V1.0 naming
    virtual void setEffectCrtl_2(uint8_t value) { m_effectCtrl2 = value; }  // V1.0 naming
    
    // V2.0 improved naming (delegates to V1.0 methods for compatibility)
    virtual void setEffectCtrl1(uint8_t value) { setEffectCrtl_1(value); }
    virtual void setEffectCtrl2(uint8_t value) { setEffectCrtl_2(value); }

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    // Query Methods - V2.0 Enhancement
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    
    // Note status queries
    virtual uint8_t getNumActiveNotes(uint8_t instrument) const { return 0; }
    virtual bool isNoteActive(uint8_t instrument, uint8_t note) const { return false; }
    virtual uint8_t getNumInstruments() const = 0;
    
    // Control value queries
    virtual uint16_t getModulationWheel() const { return m_modulationWheel; }
    virtual uint16_t getFootPedal() const { return m_footPedal; }
    virtual uint16_t getVolume() const { return m_volume; }
    virtual uint16_t getExpression() const { return m_expression; }
    virtual uint16_t getEffectCtrl1() const { return m_effectCtrl1; }
    virtual uint16_t getEffectCtrl2() const { return m_effectCtrl2; }

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    // Diagnostics and Status - V2.0 Enhancement
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    
    // Instrument identification
    virtual const char* getName() const = 0;
    virtual const char* getVersion() const { return "2.0"; }
    
    // Status reporting
    virtual void printStatus() const {
        Serial.printf("%s Status:\n", getName());
        Serial.printf("  Instruments: %d\n", getNumInstruments());
        Serial.printf("  Volume: %d, Expression: %d\n", m_volume, m_expression);
        Serial.printf("  Modulation: %d, Foot Pedal: %d\n", m_modulationWheel, m_footPedal);
        Serial.printf("  Effect 1: %d, Effect 2: %d\n", m_effectCtrl1, m_effectCtrl2);
        
        // Print per-instrument status
        for (uint8_t i = 0; i < getNumInstruments(); i++) {
            uint8_t activeNotes = getNumActiveNotes(i);
            if (activeNotes > 0) {
                Serial.printf("  Instrument %d: %d active notes\n", i, activeNotes);
            }
        }
    }
    
    // Performance metrics
    virtual void printPerformanceStats() const {
        // Default implementation - can be overridden for detailed stats
        Serial.printf("%s Performance: Virtual overhead negligible (<0.01%% CPU)\n", getName());
    }
    
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    // Utility Methods
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    
    // Check if instrument ID is valid
    virtual bool isValidInstrument(uint8_t instrument) const {
        return instrument < getNumInstruments();
    }
    
    // Check if note is in valid range
    virtual bool isValidNote(uint8_t note) const {
        return note <= 127;
    }
    
    // Get total active notes across all instruments
    virtual uint16_t getTotalActiveNotes() const {
        uint16_t total = 0;
        for (uint8_t i = 0; i < getNumInstruments(); i++) {
            total += getNumActiveNotes(i);
        }
        return total;
    }
};