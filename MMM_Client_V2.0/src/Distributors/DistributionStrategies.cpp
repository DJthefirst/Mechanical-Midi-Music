/*
 * DistributionStrategies.cpp
 *
 * Implementation of concrete distribution strategies
 */

#include "DistributionStrategies.h"
#include "../Constants.h"
#include "../Device.h"
#include "../Instruments/InstrumentControllerBase.h"

// Round Robin with Load Balancing Strategy
void RoundRobinBalanceStrategy::playNextInstrument(uint8_t note, uint8_t velocity, uint8_t channel) {
    uint8_t instrumentLeastActive = NONE;

    for (int i = 0; i < HardwareConfig::MAX_NUM_INSTRUMENTS; i++) {
        // Increment current Instrument
        m_currentInstrument++;
        
        // Loop to first instrument if end is reached
        m_currentInstrument = (m_currentInstrument >= HardwareConfig::MAX_NUM_INSTRUMENTS) ? 0 : m_currentInstrument;
        
        // Check if valid instrument
        if (!m_distributor->getInstruments()[m_currentInstrument]) continue;
        
        // If there are no active notes this must be the least active Instrument return
        uint8_t activeNotes = m_instrumentController->getNumActiveNotes(m_currentInstrument);
        if (activeNotes == 0) {
            m_instrumentController->playNote(m_currentInstrument, note, velocity, channel, static_cast<void*>(m_distributor));
            return;
        }
        
        // Set this to Least Active Instrument if instrumentLeastActive is not yet set.
        if (instrumentLeastActive == NONE) {
            instrumentLeastActive = m_currentInstrument;
            continue;
        }
        
        // Update the Least Active Instrument if needed.
        if (activeNotes < m_instrumentController->getNumActiveNotes(instrumentLeastActive)) {
            instrumentLeastActive = m_currentInstrument;
        }   
    }
    if(instrumentLeastActive != NONE) {
        m_instrumentController->playNote(instrumentLeastActive, note, velocity, channel, static_cast<void*>(m_distributor));
    }
}

void RoundRobinBalanceStrategy::stopActiveInstrument(uint8_t note, uint8_t velocity, uint8_t channel) {
    // Iterate through each instrument in reverse
    uint8_t instrument = m_currentInstrument;
    for(int i = 0; i < HardwareConfig::MAX_NUM_INSTRUMENTS; ++i){
        // Decrement instrument with underflow protection
        if(instrument == 0) instrument = HardwareConfig::MAX_NUM_INSTRUMENTS;
        --instrument;

        // Check if valid instrument
        if(m_distributor->getInstruments()[instrument] 
            && m_instrumentController->getLastDistributor(instrument) == static_cast<void*>(m_distributor)
            && m_instrumentController->getLastChannel(instrument) == channel 
            && m_instrumentController->isNoteActive(instrument, note)) {
            m_instrumentController->stopNote(instrument, note, velocity, channel);
            return;
        }
    }
}

// Simple Round Robin Strategy
void RoundRobinStrategy::playNextInstrument(uint8_t note, uint8_t velocity, uint8_t channel) {
    for (int i = 0; i < HardwareConfig::MAX_NUM_INSTRUMENTS; i++) {
        m_currentInstrument++;
        
        // Loop to first instrument if end is reached
        m_currentInstrument = (m_currentInstrument >= HardwareConfig::MAX_NUM_INSTRUMENTS) ? 0 : m_currentInstrument;
        
        // Check if valid instrument
        if (!m_distributor->getInstruments()[m_currentInstrument]) continue;
        m_instrumentController->playNote(m_currentInstrument, note, velocity, channel, static_cast<void*>(m_distributor));
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

        // Check if valid instrument
        if(m_distributor->getInstruments()[instrument] 
            && m_instrumentController->getLastDistributor(instrument) == static_cast<void*>(m_distributor)
            && m_instrumentController->getLastChannel(instrument) == channel 
            && m_instrumentController->isNoteActive(instrument, note)) {
            m_instrumentController->stopNote(instrument, note, velocity, channel);
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
        uint8_t activeNotes = m_instrumentController->getNumActiveNotes(i);
        
        // If there are no active notes this must be the least active instrument return
        if (activeNotes == 0){
            m_instrumentController->playNote(i, note, velocity, channel, static_cast<void*>(m_distributor));
            return;
        }
        
        // Update least active instrument
        if (activeNotes < leastActiveNotes) {
            leastActiveNotes = activeNotes;
            instrumentLeastActive = i;
        }
    }
    if(validInstrument) {
        m_instrumentController->playNote(instrumentLeastActive, note, velocity, channel, static_cast<void*>(m_distributor));
    }
}

void AscendingStrategy::stopActiveInstrument(uint8_t note, uint8_t velocity, uint8_t channel) {
    for(uint8_t i = 0; i < HardwareConfig::MAX_NUM_INSTRUMENTS; ++i){
        if(m_distributor->getInstruments()[i] 
            && m_instrumentController->getLastDistributor(i) == static_cast<void*>(m_distributor)
            && m_instrumentController->getLastChannel(i) == channel 
            && m_instrumentController->isNoteActive(i, note)) {
            m_instrumentController->stopNote(i, note, velocity, channel);
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
        uint8_t activeNotes = m_instrumentController->getNumActiveNotes(i);
        
        // If there are no active notes this must be the least active Instrument return
        if (activeNotes == 0){
            m_instrumentController->playNote(i, note, velocity, channel, static_cast<void*>(m_distributor));
            return;
        }
        
        // Update least active instrument
        if (activeNotes < leastActiveNotes) {
            leastActiveNotes = activeNotes;
            instrumentLeastActive = i;
        }
    }
    if(validInstrument) {
        m_instrumentController->playNote(instrumentLeastActive, note, velocity, channel, static_cast<void*>(m_distributor));
    }
}

void DescendingStrategy::stopActiveInstrument(uint8_t note, uint8_t velocity, uint8_t channel) {
    for(int i = (HardwareConfig::MAX_NUM_INSTRUMENTS - 1); i >= 0; --i){
        // Check if valid instrument
        if(m_distributor->getInstruments()[i] 
            && m_instrumentController->getLastDistributor(i) == static_cast<void*>(m_distributor)
            && m_instrumentController->getLastChannel(i) == channel 
            && m_instrumentController->isNoteActive(i, note)) {
            m_instrumentController->stopNote(i, note, velocity, channel);
            return;
        }
    }
}

// Straight Through Strategy
void StraightThroughStrategy::playNextInstrument(uint8_t note, uint8_t velocity, uint8_t channel) {
   
    uint8_t instrumentId = channel; // Map channel directly to instrument ID
    if (instrumentId < HardwareConfig::MAX_NUM_INSTRUMENTS &&
        m_distributor->getInstruments()[instrumentId]) {
            m_instrumentController->playNote(instrumentId, note, velocity, channel, static_cast<void*>(m_distributor));
    }
}

void StraightThroughStrategy::stopActiveInstrument(uint8_t note, uint8_t velocity, uint8_t channel) {
    int instrumentId = channel % HardwareConfig::MAX_NUM_INSTRUMENTS; // Map channel directly to instrument ID
    if(m_distributor->getInstruments()[instrumentId] 
            && m_instrumentController->getLastDistributor(instrumentId) == static_cast<void*>(m_distributor)
            && m_instrumentController->getLastChannel(instrumentId) == channel
            && m_instrumentController->isNoteActive(instrumentId, note)){
        m_instrumentController->stopNote(instrumentId, note, velocity, channel);
    }
}
