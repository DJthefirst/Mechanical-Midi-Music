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
    std::shared_ptr<DistributorManager> distributorManager;
    std::shared_ptr<SysExMsgHandler> sysExHandler;
    std::shared_ptr<InstrumentControllerBase> instrumentController;

public:

    MidiMsgHandler(std::shared_ptr<DistributorManager> distributorManager,
                   std::shared_ptr<SysExMsgHandler> sysExHandler,
                   std::shared_ptr<InstrumentControllerBase> instrumentController);

    std::optional<MidiMessage> processMessage(MidiMessage& message);

private:

    void distributeMessage(MidiMessage& message);
    void processCC(MidiMessage& message);
    
    std::optional<MidiMessage> processSystemMessage(MidiMessage& message);
};