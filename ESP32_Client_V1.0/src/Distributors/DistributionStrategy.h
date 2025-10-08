/*
 * DistributionStrategy.h
 *
 * Abstract base class for different distribution strategies.
 * This replaces the switch statement in Distributor.cpp with a modular
 * strategy pattern approach.
 */

#pragma once

#include "../Constants.h"
#include <cstdint>

class InstrumentController; // Forward declaration

/**
 * Abstract base class for distribution strategies
 */
class DistributionStrategy {
public:
    virtual ~DistributionStrategy() = default;
    
    /**
     * Get the next instrument to play a note on
     * @param instrumentController Pointer to the instrument controller
     * @param currentInstrument Reference to the current instrument index (may be modified)
     * @param instruments Bitmask of enabled instruments
     * @return Instrument ID to play on, or NONE if no valid instrument
     */
    virtual uint8_t getNextInstrument(
        InstrumentController* instrumentController,
        uint8_t& currentInstrument,
        uint32_t instruments
    ) = 0;
    
    /**
     * Get the distribution method type for this strategy
     */
    virtual DistributionMethod getMethodType() const = 0;

protected:
    /**
     * Helper function to check if a distributor has a specific instrument
     */
    bool distributorHasInstrument(int instrumentId, uint32_t instruments) {
        return (instruments & (1 << instrumentId)) != 0;
    }
};