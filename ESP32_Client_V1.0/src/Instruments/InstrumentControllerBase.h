#pragma once

#include "Constants.h"
#include <cstdint>
#include <array>
#include <bitset>

// Forward declarations
class DistributionStrategy;

class InstrumentControllerBase{
public:
    // Friend class for access to private members
    friend class DistributionStrategy;

protected:
    //Distributor Tracking Attributes
    std::array<void*, NUM_Instruments> m_lastDistributor = {nullptr};
    std::array<uint8_t, NUM_Instruments> m_lastChannel = {NONE};
    std::bitset<NUM_Instruments> m_activeInstruments = 0;
    std::array<uint32_t, NUM_Instruments> m_noteStartTime = {0}; // When each note started (for timeout and longest-playing tracking)

    //Local CC Effect Atributes
    uint16_t m_pitchBend[Midi::NUM_CH]; 
    uint8_t m_modulationWheel[Midi::NUM_CH];
    uint8_t m_footPedal[Midi::NUM_CH];
    uint8_t m_volume[Midi::NUM_CH];
    uint8_t m_expression[Midi::NUM_CH];
    uint8_t m_effectCrtl_1[Midi::NUM_CH];
    uint8_t m_effectCrtl_2[Midi::NUM_CH];



public:
    //Required Functions
    virtual void reset(uint8_t instrument) = 0;
    virtual void resetAll() = 0;
    virtual void playNote(uint8_t instrument, uint8_t note, uint8_t velocity, uint8_t channel) = 0;
    // Overloaded version with distributor tracking
    void playNote(uint8_t instrument, uint8_t note, uint8_t velocity, uint8_t channel, void* distributor) {

        // Set the distributor that sent this note
        m_lastDistributor[instrument] = distributor;
        m_lastChannel[instrument] = channel;
        
        // Default implementation just calls the base method
        playNote(instrument, note, velocity, channel);
    }
    virtual void stopNote(uint8_t instrument, uint8_t velocity) = 0;
    virtual void stopAll() = 0;

    //Required Getters
    virtual uint8_t getNumActiveNotes(uint8_t instrument) = 0;
    virtual bool isNoteActive(uint8_t instrument, uint8_t note) = 0;
    
    // Get the instrument type at runtime
    virtual Instrument getInstrumentType() const { return Instrument::None; };
    
    //Timeout tracking functions
    virtual void checkInstrumentTimeouts();

    //Optional Setters
    virtual void setKeyPressure(uint8_t channel,uint8_t note, uint8_t velocity);
    virtual void setPitchBend(uint8_t channel, uint16_t value);
    virtual void setProgramChange(uint8_t channel, uint8_t value);
    virtual void setChannelPressure(uint8_t channel, uint8_t value);
    virtual void setModulationWheel(uint8_t channel, uint8_t value);
    virtual void setFootPedal(uint8_t channel, uint8_t value);
    virtual void setVolume(uint8_t channel, uint8_t value);
    virtual void setExpression(uint8_t channel, uint8_t value);
    virtual void setEffectCrtl_1(uint8_t channel, uint8_t value);
    virtual void setEffectCrtl_2(uint8_t channel, uint8_t value);

    // Return the last distributor (as a void* value) that sent a note to the
    // given instrument. Use setLastDistributor to update/clear this value.
    void* getLastDistributor(uint8_t instrument) {
        if (instrument < NUM_Instruments) {
            return m_lastDistributor[instrument];
        }
        return nullptr;
    }

    uint8_t getLastChannel(uint8_t instrument) {
        if (instrument < NUM_Instruments) {
            return m_lastChannel[instrument];
        }
        return NONE;
    }

    void setLastDistributor(uint8_t instrument, void* distributor, uint8_t channel) {
        if (instrument < NUM_Instruments) {
            m_lastDistributor[instrument] = distributor;
            m_lastChannel[instrument] = channel;
        }
    }

    uint32_t getNoteStartTime(uint8_t instrument) {
        if (instrument < NUM_Instruments) {
            return m_noteStartTime[instrument];
        }
        return 0;
    }
};