#pragma once

#include "Constants.h"
#include <cstdint>
using std::int8_t;
using std::int16_t;


class InstrumentController{
public:
    static constexpr Instrument Type = Instrument::None;
    
    // Forward declare Distributor for distributor tracking
    class Distributor; 
    
protected:
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
    virtual void playNote(uint8_t instrument, uint8_t note, uint8_t velocity, uint8_t channel, void* distributor) {
        // Default implementation just calls the base method
        playNote(instrument, note, velocity, channel);
        // Set the distributor that sent this note
        setLastDistributor(instrument, distributor);
    }
    virtual void stopNote(uint8_t instrument, uint8_t note, uint8_t velocity) = 0;
    virtual void stopAll() = 0;

    //Required Getters
    virtual uint8_t getNumActiveNotes(uint8_t instrument) = 0;
    virtual bool isNoteActive(uint8_t instrument, uint8_t note) = 0;
    
    //Distributor tracking functions
    virtual void setLastDistributor(uint8_t instrument, void* distributor);
    virtual void* getLastDistributor(uint8_t instrument);
    virtual void clearDistributorFromInstrument(void* distributor);
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
};