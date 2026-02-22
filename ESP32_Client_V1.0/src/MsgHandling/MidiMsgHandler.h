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
#include <memory>
#include <functional>

// Forward declarations
class DistributorManager;
class SysExMsgHandler;
class InstrumentControllerBase;

class MidiMsgHandler {
private:
    std::shared_ptr<DistributorManager> m_distributorManager;
    std::shared_ptr<SysExMsgHandler> m_sysExHandler;
    std::shared_ptr<InstrumentControllerBase> m_instrumentController;

public:

    MidiMsgHandler(std::shared_ptr<DistributorManager> distributorManager,
                   std::shared_ptr<SysExMsgHandler> sysExHandler,
                   std::shared_ptr<InstrumentControllerBase> instrumentController);

    std::optional<MidiMessage> processMessage(const MidiMessage& message);

private:

    void distributeMessage(const MidiMessage& message);
    void processCC(const MidiMessage& message);
    
    std::optional<MidiMessage> processSystemMessage(const MidiMessage& message);
};