/*
 * DistributionStrategies.cpp
 *
 * Implementation of concrete distribution strategies
 */

#include "DistributionStrategies.h"
#include "../Constants.h"
#include "../Device.h"
#include "../Instruments/InstrumentControllerBase.h"

// Shared by every strategy's stopActiveInstrument()
bool DistributionStrategy::stopIfPlaying(uint8_t instrument, uint8_t note, uint8_t velocity, uint8_t channel) {
    if (m_distributor->getInstruments()[instrument]
        && m_instrumentController->getLastDistributor(instrument) == m_distributor
        && m_instrumentController->getLastChannel(instrument) == channel
        && m_instrumentController->isNoteActive(instrument, note)) {
        m_instrumentController->stopNote(instrument, note, velocity, channel);
        return true;
    }
    return false;
}

// Round Robin with Load Balancing Strategy
void RoundRobinBalanceStrategy::playNextInstrument(uint8_t note, uint8_t velocity, uint8_t channel) {
    const std::bitset<NUM_Instruments> instruments = m_distributor->getInstruments();
    uint8_t instrumentLeastActive = NONE;
    uint8_t leastActiveNotes = 0;

    for (int i = 0; i < HardwareConfig::MAX_NUM_INSTRUMENTS; i++) {
        // Increment current Instrument
        m_currentInstrument++;
        
        // Loop to first instrument if end is reached
        m_currentInstrument = (m_currentInstrument >= HardwareConfig::MAX_NUM_INSTRUMENTS) ? 0 : m_currentInstrument;
        
        // Check if valid instrument
        if (!instruments[m_currentInstrument]) continue;
        
        // If there are no active notes this must be the least active Instrument return
        uint8_t activeNotes = m_instrumentController->getNumActiveNotes(m_currentInstrument);
        if (activeNotes == 0) {
            m_instrumentController->playNote(m_currentInstrument, note, velocity, channel, m_distributor);
            return;
        }
        
        // Remember the first instrument found with the fewest active notes
        if (instrumentLeastActive == NONE || activeNotes < leastActiveNotes) {
            instrumentLeastActive = m_currentInstrument;
            leastActiveNotes = activeNotes;
        }
    }
    if(instrumentLeastActive != NONE) {
        m_instrumentController->playNote(instrumentLeastActive, note, velocity, channel, m_distributor);
    }
}

void RoundRobinBalanceStrategy::stopActiveInstrument(uint8_t note, uint8_t velocity, uint8_t channel) {
    // Iterate through each instrument in reverse
    uint8_t instrument = m_currentInstrument;
    for(int i = 0; i < HardwareConfig::MAX_NUM_INSTRUMENTS; ++i){
        // Decrement instrument with underflow protection
        if(instrument == 0) instrument = HardwareConfig::MAX_NUM_INSTRUMENTS;
        --instrument;

        if(stopIfPlaying(instrument, note, velocity, channel)) return;
    }
}

// Simple Round Robin Strategy
void RoundRobinStrategy::playNextInstrument(uint8_t note, uint8_t velocity, uint8_t channel) {
    const std::bitset<NUM_Instruments> instruments = m_distributor->getInstruments();

    for (int i = 0; i < HardwareConfig::MAX_NUM_INSTRUMENTS; i++) {
        m_currentInstrument++;
        
        // Loop to first instrument if end is reached
        m_currentInstrument = (m_currentInstrument >= HardwareConfig::MAX_NUM_INSTRUMENTS) ? 0 : m_currentInstrument;
        
        // Check if valid instrument
        if (!instruments[m_currentInstrument]) continue;
        m_instrumentController->playNote(m_currentInstrument, note, velocity, channel, m_distributor);
        return;
    }
}

void RoundRobinStrategy::stopActiveInstrument(uint8_t note, uint8_t velocity, uint8_t channel) {
    // Iterate through each instrument in reverse
    uint8_t instrument = m_currentInstrument;
    for(int i = 0; i < HardwareConfig::MAX_NUM_INSTRUMENTS; ++i){
        // Decrement instrument with underflow protection
        if(instrument == 0) instrument = HardwareConfig::MAX_NUM_INSTRUMENTS;
        --instrument;

        if(stopIfPlaying(instrument, note, velocity, channel)) return;
    }
}

// Ascending Strategy
void AscendingStrategy::playNextInstrument(uint8_t note, uint8_t velocity, uint8_t channel) {
    const std::bitset<NUM_Instruments> instruments = m_distributor->getInstruments();
    bool validInstrument = false;
    uint8_t instrumentLeastActive = NONE;
    uint8_t leastActiveNotes = 255;
    
    for (int i = 0; i < HardwareConfig::MAX_NUM_INSTRUMENTS; i++) {
        // Check if valid instrument
        if (!instruments[i]) continue;
        validInstrument = true;
        
        // Check if instrument has the least active notes
        uint8_t activeNotes = m_instrumentController->getNumActiveNotes(i);
        
        // If there are no active notes this must be the least active instrument return
        if (activeNotes == 0){
            m_instrumentController->playNote(i, note, velocity, channel, m_distributor);
            return;
        }
        
        // Update least active instrument
        if (activeNotes < leastActiveNotes) {
            leastActiveNotes = activeNotes;
            instrumentLeastActive = i;
        }
    }
    if(validInstrument) {
        m_instrumentController->playNote(instrumentLeastActive, note, velocity, channel, m_distributor);
    }
}

void AscendingStrategy::stopActiveInstrument(uint8_t note, uint8_t velocity, uint8_t channel) {
    for(uint8_t i = 0; i < HardwareConfig::MAX_NUM_INSTRUMENTS; ++i){
        if(stopIfPlaying(i, note, velocity, channel)) return;
    }
}

// Descending Strategy
void DescendingStrategy::playNextInstrument(uint8_t note, uint8_t velocity, uint8_t channel) {
    const std::bitset<NUM_Instruments> instruments = m_distributor->getInstruments();
    bool validInstrument = false;
    uint8_t instrumentLeastActive = NONE;
    uint8_t leastActiveNotes = 255;
    
    for (int i = (HardwareConfig::MAX_NUM_INSTRUMENTS - 1); i >= 0; i--) {
        // Check if valid instrument
        if (!instruments[i]) continue;
        validInstrument = true;
        
        // Check if instrument has the least active notes
        uint8_t activeNotes = m_instrumentController->getNumActiveNotes(i);
        
        // If there are no active notes this must be the least active Instrument return
        if (activeNotes == 0){
            m_instrumentController->playNote(i, note, velocity, channel, m_distributor);
            return;
        }
        
        // Update least active instrument
        if (activeNotes < leastActiveNotes) {
            leastActiveNotes = activeNotes;
            instrumentLeastActive = i;
        }
    }
    if(validInstrument) {
        m_instrumentController->playNote(instrumentLeastActive, note, velocity, channel, m_distributor);
    }
}

void DescendingStrategy::stopActiveInstrument(uint8_t note, uint8_t velocity, uint8_t channel) {
    for(int i = (HardwareConfig::MAX_NUM_INSTRUMENTS - 1); i >= 0; --i){
        if(stopIfPlaying(i, note, velocity, channel)) return;
    }
}

// Straight Through Strategy
void StraightThroughStrategy::playNextInstrument(uint8_t note, uint8_t velocity, uint8_t channel) {
   
    uint8_t instrumentId = channel; // Map channel directly to instrument ID
    if (instrumentId < HardwareConfig::MAX_NUM_INSTRUMENTS &&
        m_distributor->getInstruments()[instrumentId]) {
            m_instrumentController->playNote(instrumentId, note, velocity, channel, m_distributor);
    }
}

void StraightThroughStrategy::stopActiveInstrument(uint8_t note, uint8_t velocity, uint8_t channel) {
    int instrumentId = channel % HardwareConfig::MAX_NUM_INSTRUMENTS; // Map channel directly to instrument ID
    stopIfPlaying(instrumentId, note, velocity, channel);
}
