/*
 * DistributionStrategy.h
 *
 * Abstract base class for different distribution strategies.
 * This replaces the switch statement in Distributor.cpp with a modular
 * strategy pattern approach.
 */

#pragma once

#include "../Constants.h"
#include "Device.h"
#include "../Instruments/InstrumentController.h"
#include <cstdint>
#include <bitset>
#include <optional>
#include <memory>

// Forward declarations
class InstrumentControllerBase;
class Distributor;

class DistributionStrategy {
protected:
    Distributor* m_distributor;
    std::shared_ptr<InstrumentControllerBase> instrumentController = InstrumentController<InstrumentType>::getInstance();

public:
    DistributionStrategy(Distributor* distributor) : m_distributor(distributor) {};
    virtual ~DistributionStrategy() = default;
    
    virtual uint8_t getNextInstrument() = 0;
    virtual std::optional<uint8_t> checkForNote() = 0;

    virtual DistributionMethod getMethodType() const = 0;

protected:
    bool distributorHasInstrument(int instrumentId);  // Implementation in .cpp file



    void setLastDistributor(uint8_t instrument, void* distributor) {
        // if (instrument < HardwareConfig::MAX_NUM_INSTRUMENTS) {
        //     _lastDistributor[instrument] = distributor;
        // }
    }

    void* getLastDistributor(uint8_t instrument) {
        // if (instrument < HardwareConfig::MAX_NUM_INSTRUMENTS) {
        //     return _lastDistributor[instrument];
        // }
        return nullptr;
    }

    void clearDistributorFromInstrument(void* distributor) {
        // When a distributor is destroyed, clear it from all instruments
        // for (uint8_t i = 0; i < HardwareConfig::MAX_NUM_INSTRUMENTS; i++) {
        //     if (_lastDistributor[i] == distributor) {
        //         // If this distributor was the last to send a note, stop the note to prevent hanging
        //         if (m_activeNotes[i] != 0) {
        //             stopNote(i, 0);
        //         }
        //         _lastDistributor[i] = nullptr;
        //     }
        // }
    }
};