/*
 * DistributionStrategy.cpp
 *
 * Implementation file for DistributionStrategy base class
 */

#include "DistributionStrategy.h"
#include "Distributor.h"

bool DistributionStrategy::distributorHasInstrument(int instrumentId) {
    return m_distributor->distributorHasInstrument(instrumentId);
}