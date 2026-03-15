/*
 * MidiMsgHandler.h
 *
 * Handles core MIDI message processing and routing.
 * Extracted from the original MessageHandler to follow single responsibility principle.
 */

#pragma once

#include "MidiMessage.h"
#include "Constants.h"

#include <cstdint>
#include <optional>
#include <functional>

// Forward declarations
class DistributorManager;
class SysExMsgHandler;
class InstrumentControllerBase;

class MidiMsgHandler {
private:
    DistributorManager* m_distributorManager;
    SysExMsgHandler* m_sysExHandler;
    InstrumentControllerBase* m_instrumentController;

public:

    MidiMsgHandler(DistributorManager& distributorManager,
                   SysExMsgHandler& sysExHandler,
                   InstrumentControllerBase& instrumentController);

    std::optional<MidiMessage> processMessage(const MidiMessage& message);

private:

    void distributeMessage(const MidiMessage& message);
    void processCC(const MidiMessage& message);
    
    std::optional<MidiMessage> processSystemMessage(const MidiMessage& message);
};