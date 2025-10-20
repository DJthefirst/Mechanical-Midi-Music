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
#include "../Distributors/Distributor.h"
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

    uint8_t currentInstrument = 0;

public:
    DistributionStrategy(Distributor* distributor) : m_distributor(distributor) {};
    virtual ~DistributionStrategy() {};

    virtual void playNextInstrument(uint8_t note, uint8_t velocity, uint8_t channel) = 0;
    virtual void stopActiveInstrument(uint8_t note, uint8_t velocity, uint8_t channel) = 0;

    virtual DistributionMethod getMethodType() const = 0;

};