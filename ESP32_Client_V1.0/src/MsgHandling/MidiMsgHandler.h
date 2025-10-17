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

/**
 * Handles core MIDI message processing and routing.
 * Coordinates with DistributorManager and SysExMsgHandler for specialized processing.
 */
class MidiMsgHandler {
private:
    std::shared_ptr<DistributorManager> distributorManager;
    std::shared_ptr<SysExMsgHandler> sysExHandler;
    std::shared_ptr<InstrumentControllerBase> instrumentController;

public:
    /**
     * Constructor with dependency injection
     * @param distributorManager Shared pointer to DistributorManager
     * @param sysExHandler Shared pointer to SysExMsgHandler
     * @param instrumentController Shared pointer to InstrumentControllerBase
     */
    MidiMsgHandler(std::shared_ptr<DistributorManager> distributorManager,
                   std::shared_ptr<SysExMsgHandler> sysExHandler,
                   std::shared_ptr<InstrumentControllerBase> instrumentController);

    /**
     * Process a MIDI message and return optional response
     * @param message The MIDI message to process
     * @return Optional response message (typically for SysEx commands)
     */
    std::optional<MidiMessage> processMessage(MidiMessage& message);

private:
    /**
     * Route message to appropriate distributors based on channel
     * @param message The MIDI message to distribute
     */
    void distributeMessage(MidiMessage& message);

    /**
     * Process Control Change messages
     * @param message The CC message to process
     */
    void processCC(MidiMessage& message);

    /**
     * Process System Common messages (SysEx, System Stop, System Reset)
     * @param message The system message to process
     * @return Optional response message for SysEx commands
     */
    std::optional<MidiMessage> processSystemMessage(MidiMessage& message);
};