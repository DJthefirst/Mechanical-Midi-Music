/*
 * DistributionStrategies.h
 *
 * Concrete implementations of distribution strategies
 */

#pragma once

#include "DistributionStrategy.h"
#include "../Instruments/InstrumentController.h"

/**
 * Round Robin with Load Balancing Strategy
 * Distributes notes in a circular manner, preferring instruments with fewer active notes
 */
class RoundRobinBalanceStrategy : public DistributionStrategy {
public:
    uint8_t getNextInstrument(
        InstrumentController* instrumentController,
        uint8_t& currentInstrument,
        uint32_t instruments
    ) override;
    
    DistributionMethod getMethodType() const override {
        return DistributionMethod::RoundRobinBalance;
    }
};

/**
 * Simple Round Robin Strategy
 * Distributes notes in a circular manner without load balancing
 */
class RoundRobinStrategy : public DistributionStrategy {
public:
    uint8_t getNextInstrument(
        InstrumentController* instrumentController,
        uint8_t& currentInstrument,
        uint32_t instruments
    ) override;
    
    DistributionMethod getMethodType() const override {
        return DistributionMethod::RoundRobin;
    }
};

/**
 * Ascending Strategy
 * Always chooses the lowest-numbered available instrument with the fewest active notes
 */
class AscendingStrategy : public DistributionStrategy {
public:
    uint8_t getNextInstrument(
        InstrumentController* instrumentController,
        uint8_t& currentInstrument,
        uint32_t instruments
    ) override;
    
    DistributionMethod getMethodType() const override {
        return DistributionMethod::Ascending;
    }
};

/**
 * Descending Strategy
 * Always chooses the highest-numbered available instrument with the fewest active notes
 */
class DescendingStrategy : public DistributionStrategy {
public:
    uint8_t getNextInstrument(
        InstrumentController* instrumentController,
        uint8_t& currentInstrument,
        uint32_t instruments
    ) override;
    
    DistributionMethod getMethodType() const override {
        return DistributionMethod::Descending;
    }
};

/**
 * Straight Through Strategy
 * Maps channels to corresponding instrument IDs with load balancing
 */
class StraightThroughStrategy : public DistributionStrategy {
public:
    uint8_t getNextInstrument(
        InstrumentController* instrumentController,
        uint8_t& currentInstrument,
        uint32_t instruments
    ) override;
    
    DistributionMethod getMethodType() const override {
        return DistributionMethod::StraightThrough;
    }
};