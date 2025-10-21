#pragma once

#include "Constants.h"
#include <cstdint>
#include <array>
#include <bitset>

// Forward declarations
class DistributionStrategy;

class InstrumentControllerBase{
public:
    static constexpr Instrument Type = Instrument::None;
    
    // Friend class for access to private members
    friend class DistributionStrategy;

protected:
    //Distributor Tracking Attributes
    std::array<void*, NUM_Instruments> _lastDistributor = {nullptr};
    std::array<uint8_t, NUM_Instruments> _lastChannel = {NONE};
    std::bitset<NUM_Instruments> _activeInstruments = 0;
    std::array<uint32_t, NUM_Instruments> _noteStartTime = {0}; // When each note started (for timeout and longest-playing tracking)

    //Local CC Effect Atributes
    uint16_t _ModulationWheel = 0;
    uint16_t _FootPedal = 0;
    uint16_t _Volume = 127;
    uint16_t _Expression = 0;
    uint16_t _EffectCrtl_1 = 0;
    uint16_t _EffectCrtl_2 = 0;

public:
    //Required Functions
    virtual void reset(uint8_t instrument) = 0;
    virtual void resetAll() = 0;
    virtual void playNote(uint8_t instrument, uint8_t note, uint8_t velocity, uint8_t channel) = 0;
    // Overloaded version with distributor tracking
    void playNote(uint8_t instrument, uint8_t note, uint8_t velocity, uint8_t channel, void* distributor) {
        // Default implementation just calls the base method
        playNote(instrument, note, velocity, channel);
        // Set the distributor that sent this note
        _lastDistributor[instrument] = distributor;
        _lastChannel[instrument] = channel;
    }
    virtual void stopNote(uint8_t instrument, uint8_t velocity) = 0;
    virtual void stopAll() = 0;

    //Required Getters
    virtual uint8_t getNumActiveNotes(uint8_t instrument) = 0;
    virtual bool isNoteActive(uint8_t instrument, uint8_t note) = 0;
    
    // Get the instrument type at runtime
    virtual Instrument getInstrumentType() const { return Type; }
    
    //Timeout tracking functions
    virtual void checkInstrumentTimeouts();

    //Optional Setters
    virtual void setKeyPressure(uint8_t instrument, uint8_t note, uint8_t velocity);
    virtual void setPitchBend(uint8_t instrument, uint16_t value, uint8_t channel);
    virtual void setProgramChange(uint8_t value);
    virtual void setChannelPressure(uint8_t value);
    virtual void setModulationWheel(uint8_t value);
    virtual void setFootPedal(uint8_t value);
    virtual void setVolume(uint8_t value);
    virtual void setExpression(uint8_t value);
    virtual void setEffectCrtl_1(uint8_t value);
    virtual void setEffectCrtl_2(uint8_t value);

    // Return the last distributor (as a void* value) that sent a note to the
    // given instrument. Use setLastDistributor to update/clear this value.
    void* getLastDistributor(uint8_t instrument) {
        if (instrument < NUM_Instruments) {
            return _lastDistributor[instrument];
        }
        return nullptr;
    }

    uint8_t getLastChannel(uint8_t instrument) {
        if (instrument < NUM_Instruments) {
            return _lastChannel[instrument];
        }
        return NONE;
    }

    void setLastDistributor(uint8_t instrument, void* distributor, uint8_t channel) {
        if (instrument < NUM_Instruments) {
            _lastDistributor[instrument] = distributor;
            _lastChannel[instrument] = channel;
        }
    }

    uint32_t getNoteStartTime(uint8_t instrument) {
        if (instrument < NUM_Instruments) {
            return _noteStartTime[instrument];
        }
        return 0;
    }
};