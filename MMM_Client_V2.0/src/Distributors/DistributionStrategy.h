/*
 * DistributionStrategy.h
 *
 * Abstract base class for different distribution strategies.
 * This replaces the switch statement in Distributor.cpp with a modular
 * strategy pattern approach.
 */

#pragma once

#include "../Constants.h"
#include "../Device.h"
#include "../Instruments/InstrumentControllerBase.h"
#include "../Distributors/Distributor.h"
#include <cstdint>
#include <bitset>
#include <memory>

// Forward declarations
class InstrumentControllerBase;
class Distributor;

class DistributionStrategy {
protected:
    // Non-owning: the strategy is owned by its Distributor, which keeps the controller alive
    Distributor* m_distributor;
    InstrumentControllerBase* m_instrumentController;

    uint8_t m_currentInstrument = 0;

    // Stops the note on an instrument if this distributor is the one playing it there.
    // Returns true if the note was stopped.
    bool stopIfPlaying(uint8_t instrument, uint8_t note, uint8_t velocity, uint8_t channel);

public:
    DistributionStrategy(Distributor* distributor, InstrumentControllerBase* instrController) 
        : m_distributor(distributor), m_instrumentController(instrController) {};
    virtual ~DistributionStrategy() = default;

    virtual void playNextInstrument(uint8_t note, uint8_t velocity, uint8_t channel) = 0;
    virtual void stopActiveInstrument(uint8_t note, uint8_t velocity, uint8_t channel) = 0;

    virtual DistributionMethod getMethodType() const = 0;

};