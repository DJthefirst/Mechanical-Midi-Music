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
void RoundRobinBalanceStrategy::playNextInstrument(uint8_t note, uint8_t velocity, uint8_t channel) {
    static uint8_t currentInstrument = 0; // Make currentInstrument static
    uint8_t instrumentLeastActive = NONE;
    
    for (int i = 0; i < HardwareConfig::MAX_NUM_INSTRUMENTS; i++) {
        // Increment current Instrument
        currentInstrument++;
        
        // Loop to first instrument if end is reached
        currentInstrument = (currentInstrument >= HardwareConfig::MAX_NUM_INSTRUMENTS) ? 0 : currentInstrument;
        
        // Check if valid instrument
        if (!m_distributor->getInstruments()[currentInstrument]) continue;
        
        // If there are no active notes this must be the least active Instrument return
        uint8_t activeNotes = instrumentController->getNumActiveNotes(currentInstrument);
            instrumentController->playNote(currentInstrument, note, velocity, channel, static_cast<void*>(m_distributor));
        if (activeNotes == 0) return;
        
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
    if(instrumentLeastActive != NONE) {
        instrumentController->playNote(instrumentLeastActive, note, velocity, channel, static_cast<void*>(m_distributor));
    }
}

void RoundRobinBalanceStrategy::stopActiveInstrument(uint8_t note, uint8_t velocity, uint8_t channel) {
    // Iterate through each instrument in reverse
    uint8_t instrument = currentInstrument;
    for(int i = 0; i < HardwareConfig::MAX_NUM_INSTRUMENTS; ++i){
        // Decrement instrument with underflow protection
        if(instrument == 0) instrument = HardwareConfig::MAX_NUM_INSTRUMENTS;
        --instrument;

        // Check if valid instrument
        if(m_distributor->getInstruments()[i] 
            && instrumentController->getLastDistributor(i) == static_cast<void*>(m_distributor)
            && instrumentController->getLastChannel(i) == channel 
            && instrumentController->isNoteActive(i, note)) {
            instrumentController->stopNote(instrument, velocity);
            return;
        }
    }
}

// Simple Round Robin Strategy
void RoundRobinStrategy::playNextInstrument(uint8_t note, uint8_t velocity, uint8_t channel) {
    for (int i = 0; i < HardwareConfig::MAX_NUM_INSTRUMENTS; i++) {
        currentInstrument++;
        
        // Loop to first instrument if end is reached
        currentInstrument = (currentInstrument >= HardwareConfig::MAX_NUM_INSTRUMENTS) ? 0 : currentInstrument;
        
        // Check if valid instrument
        if (!m_distributor->getInstruments()[currentInstrument]) continue;
            instrumentController->playNote(currentInstrument, note, velocity, channel, static_cast<void*>(m_distributor));
        return;
    }
    return;
}

void RoundRobinStrategy::stopActiveInstrument(uint8_t note, uint8_t velocity, uint8_t channel) {
    // Iterate through each instrument in reverse
    uint8_t instrument = currentInstrument;
    for(int i = 0; i < HardwareConfig::MAX_NUM_INSTRUMENTS; ++i){
        // Decrement instrument with underflow protection
        if(instrument == 0) instrument = HardwareConfig::MAX_NUM_INSTRUMENTS;
        --instrument;

        // Check if valid instrument
        if(m_distributor->getInstruments()[i] 
            && instrumentController->getLastDistributor(i) == static_cast<void*>(m_distributor)
            && instrumentController->getLastChannel(i) == channel 
            && instrumentController->isNoteActive(i, note)) {
            instrumentController->stopNote(instrument, velocity);
            return;
        }
    }
}

// Ascending Strategy
void AscendingStrategy::playNextInstrument(uint8_t note, uint8_t velocity, uint8_t channel) {
    bool validInstrument = false;
    uint8_t instrumentLeastActive = NONE;
    uint8_t leastActiveNotes = 255;
    
    for (int i = 0; i < HardwareConfig::MAX_NUM_INSTRUMENTS; i++) {
        // Check if valid instrument
        if (!m_distributor->getInstruments()[i]) continue;
        validInstrument = true;
        
        // Check if instrument has the least active notes
        uint8_t activeNotes = instrumentController->getNumActiveNotes(i);
        
        // If there are no active notes this must be the least active instrument return
        if (activeNotes == 0){
            instrumentController->playNote(i, note, velocity, channel, static_cast<void*>(m_distributor));
            return;
        }
        
        // Update least active instrument
        if (activeNotes < leastActiveNotes) {
            leastActiveNotes = activeNotes;
            instrumentLeastActive = i;
        }
    }
    if(validInstrument) {
        instrumentController->playNote(instrumentLeastActive, note, velocity, channel, static_cast<void*>(m_distributor));
    }
}

void AscendingStrategy::stopActiveInstrument(uint8_t note, uint8_t velocity, uint8_t channel) {
    for(uint8_t i = 0; i < HardwareConfig::MAX_NUM_INSTRUMENTS; ++i){
        if(m_distributor->getInstruments()[i] 
            && instrumentController->getLastDistributor(i) == static_cast<void*>(m_distributor)
            && instrumentController->getLastChannel(i) == channel 
            && instrumentController->isNoteActive(i, note)) {
            instrumentController->stopNote(i, velocity);
            return;
        }
    }
}

// Descending Strategy
void DescendingStrategy::playNextInstrument(uint8_t note, uint8_t velocity, uint8_t channel) {
    bool validInstrument = false;
    uint8_t instrumentLeastActive = NONE;
    uint8_t leastActiveNotes = 255;
    
    for (int i = (HardwareConfig::MAX_NUM_INSTRUMENTS - 1); i >= 0; i--) {
        // Check if valid instrument
        if (!m_distributor->getInstruments()[i]) continue;
        validInstrument = true;
        
        // Check if instrument has the least active notes
        uint8_t activeNotes = instrumentController->getNumActiveNotes(i);
        
        // If there are no active notes this must be the least active Instrument return
        if (activeNotes == 0){
            instrumentController->playNote(i, note, velocity, channel, static_cast<void*>(m_distributor));
            return;
        }
        
        // Update least active instrument
        if (activeNotes < leastActiveNotes) {
            leastActiveNotes = activeNotes;
            instrumentLeastActive = i;
        }
    }
    if(validInstrument) {
        instrumentController->playNote(instrumentLeastActive, note, velocity, channel, static_cast<void*>(m_distributor));
    }
}

void DescendingStrategy::stopActiveInstrument(uint8_t note, uint8_t velocity, uint8_t channel) {
    for(int i = (HardwareConfig::MAX_NUM_INSTRUMENTS - 1); i >= 0; --i){
        // Check if valid instrument
        if(m_distributor->getInstruments()[i] 
            && instrumentController->getLastDistributor(i) == static_cast<void*>(m_distributor)
            && instrumentController->getLastChannel(i) == channel 
            && instrumentController->isNoteActive(i, note)) {
            instrumentController->stopNote(i, velocity);
            return;
        }
    }
}

// Straight Through Strategy
void StraightThroughStrategy::playNextInstrument(uint8_t note, uint8_t velocity, uint8_t channel) {
   
    uint8_t instrumentId = channel; // Map channel directly to instrument ID
    if (instrumentId < HardwareConfig::MAX_NUM_INSTRUMENTS &&
        m_distributor->getInstruments()[instrumentId]) {
            instrumentController->playNote(instrumentId, note, velocity, channel, static_cast<void*>(m_distributor));
    }
}

void StraightThroughStrategy::stopActiveInstrument(uint8_t note, uint8_t velocity, uint8_t channel) {
    if(m_distributor->getInstruments()[channel] 
            && instrumentController->getLastDistributor(channel) == static_cast<void*>(m_distributor)
            && instrumentController->getLastChannel(channel) == channel
            && instrumentController->isNoteActive(channel, note)){
        instrumentController->stopNote(channel, velocity);
    }
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
