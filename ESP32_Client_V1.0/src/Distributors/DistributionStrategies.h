/*
 * DistributionStrategies.h
 *
 * Concrete implementations of distribution strategies
 */

#pragma once

#include "DistributionStrategy.h"

// Forward declaration
class InstrumentControllerBase;

/**
 * Round Robin with Load Balancing Strategy
 * Distributes notes in a circular manner, preferring instruments with fewer active notes
 */
class RoundRobinBalanceStrategy : public DistributionStrategy {   
public:
    RoundRobinBalanceStrategy(Distributor* distributor) : DistributionStrategy(distributor) {}
    void playNextInstrument(uint8_t note, uint8_t velocity, uint8_t channel) override;
    void stopActiveInstrument(uint8_t note, uint8_t velocity, uint8_t channel) override;

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
    RoundRobinStrategy(Distributor* distributor) : DistributionStrategy(distributor) {}
    void playNextInstrument(uint8_t note, uint8_t velocity, uint8_t channel) override;
    void stopActiveInstrument(uint8_t note, uint8_t velocity, uint8_t channel) override;

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
    AscendingStrategy(Distributor* distributor) : DistributionStrategy(distributor) {}
    void playNextInstrument(uint8_t note, uint8_t velocity, uint8_t channel) override;
    void stopActiveInstrument(uint8_t note, uint8_t velocity, uint8_t channel) override;

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
    DescendingStrategy(Distributor* distributor) : DistributionStrategy(distributor) {}
    void playNextInstrument(uint8_t note, uint8_t velocity, uint8_t channel) override;
    void stopActiveInstrument(uint8_t note, uint8_t velocity, uint8_t channel) override;

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
    StraightThroughStrategy(Distributor* distributor) : DistributionStrategy(distributor) {}
    void playNextInstrument(uint8_t note, uint8_t velocity, uint8_t channel) override;
    void stopActiveInstrument(uint8_t note, uint8_t velocity, uint8_t channel) override;

    DistributionMethod getMethodType() const override {
        return DistributionMethod::StraightThrough;
    }
}; 