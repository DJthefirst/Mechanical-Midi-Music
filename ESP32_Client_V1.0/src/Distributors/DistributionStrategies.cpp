/*
 * DistributionStrategies.cpp
 *
 * Implementation of concrete distribution strategies
 */

#include "DistributionStrategies.h"
#include "../Constants.h"
#include "../Device.h"
#include "../Instruments/InstrumentController.h"

// Round Robin with Load Balancing Strategy
uint8_t RoundRobinBalanceStrategy::getNextInstrument() {
    static uint8_t currentInstrument = 0; // Make currentInstrument static
    uint8_t instrumentLeastActive = NONE;
    
    for (int i = 0; i < HardwareConfig::MAX_NUM_INSTRUMENTS; i++) {
        // Increment current Instrument
        currentInstrument++;
        
        // Loop to first instrument if end is reached
        currentInstrument = (currentInstrument >= HardwareConfig::MAX_NUM_INSTRUMENTS) ? 0 : currentInstrument;
        
        // Check if valid instrument
        if (!distributorHasInstrument(currentInstrument)) continue;
        
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

std::optional<uint8_t> RoundRobinBalanceStrategy::checkForNote() {
    // For now, return empty optional as this method needs to check for specific note tracking
    // This method should be implemented to check if a specific note is being played
    return std::nullopt;
}

// Simple Round Robin Strategy
uint8_t RoundRobinStrategy::getNextInstrument() {
    static uint8_t currentInstrument = 0; // Make currentInstrument static
    for (int i = 0; i < HardwareConfig::MAX_NUM_INSTRUMENTS; i++) {
        currentInstrument++;
        
        // Loop to first instrument if end is reached
        currentInstrument = (currentInstrument >= HardwareConfig::MAX_NUM_INSTRUMENTS) ? 0 : currentInstrument;
        
        // Check if valid instrument
        if (!distributorHasInstrument(currentInstrument)) continue;
        return currentInstrument;
    }
    return NONE;
}

std::optional<uint8_t> RoundRobinStrategy::checkForNote() {
    // For now, return empty optional as this method needs to check for specific note tracking
    return std::nullopt;
}

// Ascending Strategy
uint8_t AscendingStrategy::getNextInstrument() {
    bool validInstrument = false;
    uint8_t instrumentLeastActive = NONE;
    uint8_t leastActiveNotes = 255;
    
    for (int i = 0; i < HardwareConfig::MAX_NUM_INSTRUMENTS; i++) {
        // Check if valid instrument
        if (!distributorHasInstrument(i)) continue;
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

std::optional<uint8_t> AscendingStrategy::checkForNote() {
    // For now, return empty optional as this method needs to check for specific note tracking
    return std::nullopt;
}

// Descending Strategy
uint8_t DescendingStrategy::getNextInstrument() {
    bool validInstrument = false;
    uint8_t instrumentLeastActive = NONE;
    uint8_t leastActiveNotes = 255;
    
    for (int i = (HardwareConfig::MAX_NUM_INSTRUMENTS - 1); i >= 0; i--) {
        // Check if valid instrument
        if (!distributorHasInstrument(i)) continue;
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

std::optional<uint8_t> DescendingStrategy::checkForNote() {
    // For now, return empty optional as this method needs to check for specific note tracking
    return std::nullopt;
}

// Straight Through Strategy
uint8_t StraightThroughStrategy::getNextInstrument() {
    static uint8_t currentInstrument = 0; // Make currentInstrument static
    uint8_t instrumentLeastActive = NONE;
    
    for (int i = 0; i < HardwareConfig::MAX_NUM_INSTRUMENTS; i++) {
        // Increment current Instrument
        currentInstrument++;
        
        // Loop to first instrument if end is reached
        currentInstrument = (currentInstrument >= HardwareConfig::MAX_NUM_INSTRUMENTS) ? 0 : currentInstrument;
        
        // Check if valid instrument
        if (!distributorHasInstrument(currentInstrument)) continue;
        
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

std::optional<uint8_t> StraightThroughStrategy::checkForNote() {
    // For now, return empty optional as this method needs to check for specific note tracking
    return std::nullopt;
}

// //Returns the instument which the first note is played or NONE.
// //*Note this function is optimised for nonpolyphonic playback and the Distribution method.
// uint8_t Distributor::checkForNote(uint8_t note)
// {
//     uint8_t instrument = m_currentInstrument;

//     switch(m_distributionMethod){

//     // Check for note being played on the instrument in the current channel position
//     case(DistributionMethod::StraightThrough):
//         if((*m_ptrInstrumentController).isNoteActive(m_currentChannel, note)){
//             return m_currentChannel;
//         }
//         break;

//     // Check for note being played on the instrument iterating backwards through all instruments
//     case(DistributionMethod::RoundRobin):
//     case(DistributionMethod::RoundRobinBalance):
//         // Iterate through each instrument in reverse
//         for(int i = 0; i < HardwareConfig::MAX_NUM_INSTRUMENTS; ++i){
//             // Decrement instrument with underflow protection
//             if(instrument == 0) instrument = HardwareConfig::MAX_NUM_INSTRUMENTS;
//             --instrument;

//             // Check if valid instrument
//             if(!distributorHasInstrument(instrument)) continue;
//             if(m_ptrinstrumentController->isNoteActive(instrument, note)) return instrument;
//         }
//         break;

//     // Check for note being played on the instrument starting at instrument 0
//     case(DistributionMethod::Ascending):
//         // Iterate over enabled instruments
//         {
//             for(uint8_t i = 0; i < HardwareConfig::MAX_NUM_INSTRUMENTS; ++i){
//                 if(m_instruments[i] && m_ptrinstrumentController->isNoteActive(i, note)){
//                     return i;
//                 }
//             }
//         }
//         break;

//     // Check for note being played on the instrument starting at the last instrument iterating in reverse
//     case(DistributionMethod::Descending):
//         for(int i = (HardwareConfig::MAX_NUM_INSTRUMENTS - 1); i >= 0; --i){
//             // Check if valid instrument
//             if(!distributorHasInstrument(i)) continue;
//             if(m_ptrinstrumentController->isNoteActive(i, note)) return i;
//         }
//         break;

//     default:
//         // TODO: Handle error
//         break;
//     }
//     return NONE;
// }
