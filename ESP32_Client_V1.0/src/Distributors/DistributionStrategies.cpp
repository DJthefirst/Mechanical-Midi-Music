/*
 * DistributionStrategies.cpp
 *
 * Implementation of concrete distribution strategies
 */

#include "DistributionStrategies.h"
#include "../Constants.h"
#include "../Device.h"

// Round Robin with Load Balancing Strategy
uint8_t RoundRobinBalanceStrategy::getNextInstrument(
    InstrumentController* instrumentController,
    uint8_t& currentInstrument,
    uint32_t instruments
) {
    uint8_t instrumentLeastActive = NONE;
    
    for (int i = 0; i < HardwareConfig::MAX_NUM_INSTRUMENTS; i++) {
        // Increment current Instrument
        currentInstrument++;
        
        // Loop to first instrument if end is reached
        currentInstrument = (currentInstrument >= HardwareConfig::MAX_NUM_INSTRUMENTS) ? 0 : currentInstrument;
        
        // Check if valid instrument
        if (!distributorHasInstrument(currentInstrument, instruments)) continue;
        
        // If there are no active notes this must be the least active Instrument return
        uint8_t activeNotes = instrumentController->getNumActiveNotes(currentInstrument);
        if (activeNotes == 0) return currentInstrument;
        
        // Set this to Least Active Instrument if instrumentLeastActive is not yet set.
        if (instrumentLeastActive == NONE) {
            instrumentLeastActive = currentInstrument;
            continue;
        }
        
        // Update the Least Active Instrument if needed.
        if (activeNotes < instrumentController->getNumActiveNotes(instrumentLeastActive)) {
            instrumentLeastActive = currentInstrument;
        }
    }
    return instrumentLeastActive;
}

// Simple Round Robin Strategy
uint8_t RoundRobinStrategy::getNextInstrument(
    InstrumentController* instrumentController,
    uint8_t& currentInstrument,
    uint32_t instruments
) {
    for (int i = 0; i < HardwareConfig::MAX_NUM_INSTRUMENTS; i++) {
        currentInstrument++;
        
        // Loop to first instrument if end is reached
        currentInstrument = (currentInstrument >= HardwareConfig::MAX_NUM_INSTRUMENTS) ? 0 : currentInstrument;
        
        // Check if valid instrument
        if (!distributorHasInstrument(currentInstrument, instruments)) continue;
        return currentInstrument;
    }
    return NONE;
}

// Ascending Strategy
uint8_t AscendingStrategy::getNextInstrument(
    InstrumentController* instrumentController,
    uint8_t& currentInstrument,
    uint32_t instruments
) {
    bool validInstrument = false;
    uint8_t instrumentLeastActive = NONE;
    uint8_t leastActiveNotes = 255;
    
    for (int i = 0; i < HardwareConfig::MAX_NUM_INSTRUMENTS; i++) {
        // Check if valid instrument
        if (!distributorHasInstrument(i, instruments)) continue;
        validInstrument = true;
        
        // Check if instrument has the least active notes
        uint8_t activeNotes = instrumentController->getNumActiveNotes(i);
        
        // If there are no active notes this must be the least active instrument return
        if (activeNotes == 0) return i;
        
        // Update least active instrument
        if (activeNotes < leastActiveNotes) {
            leastActiveNotes = activeNotes;
            instrumentLeastActive = i;
        }
    }
    return validInstrument ? instrumentLeastActive : NONE;
}

// Descending Strategy
uint8_t DescendingStrategy::getNextInstrument(
    InstrumentController* instrumentController,
    uint8_t& currentInstrument,
    uint32_t instruments
) {
    bool validInstrument = false;
    uint8_t instrumentLeastActive = NONE;
    uint8_t leastActiveNotes = 255;
    
    for (int i = (HardwareConfig::MAX_NUM_INSTRUMENTS - 1); i >= 0; i--) {
        // Check if valid instrument
        if (!distributorHasInstrument(i, instruments)) continue;
        validInstrument = true;
        
        // Check if instrument has the least active notes
        uint8_t activeNotes = instrumentController->getNumActiveNotes(i);
        
        // If there are no active notes this must be the least active Instrument return
        if (activeNotes == 0) return i;
        
        // Update least active instrument
        if (activeNotes < leastActiveNotes) {
            leastActiveNotes = activeNotes;
            instrumentLeastActive = i;
        }
    }
    return validInstrument ? instrumentLeastActive : NONE;
}

// Straight Through Strategy
uint8_t StraightThroughStrategy::getNextInstrument(
    InstrumentController* instrumentController,
    uint8_t& currentInstrument,
    uint32_t instruments
) {
    uint8_t instrumentLeastActive = NONE;
    
    for (int i = 0; i < HardwareConfig::MAX_NUM_INSTRUMENTS; i++) {
        // Increment current Instrument
        currentInstrument++;
        
        // Loop to first instrument if end is reached
        currentInstrument = (currentInstrument >= HardwareConfig::MAX_NUM_INSTRUMENTS) ? 0 : currentInstrument;
        
        // Check if valid instrument
        if (!distributorHasInstrument(currentInstrument, instruments)) continue;
        
        // If there are no active notes this must be the least active Instrument return
        uint8_t activeNotes = instrumentController->getNumActiveNotes(currentInstrument);
        if (activeNotes == 0) return currentInstrument;
        
        // Set this to Least Active Instrument if instrumentLeastActive is not yet set.
        if (instrumentLeastActive == NONE) {
            instrumentLeastActive = currentInstrument;
            continue;
        }
        
        // Update the Least Active Instrument if needed.
        if (activeNotes < instrumentController->getNumActiveNotes(instrumentLeastActive)) {
            instrumentLeastActive = currentInstrument;
        }
    }
    return instrumentLeastActive;
}