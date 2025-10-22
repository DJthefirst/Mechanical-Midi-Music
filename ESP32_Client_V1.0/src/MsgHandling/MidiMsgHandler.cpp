/*
 * MidiMsgHandler.cpp
 *
 * Handles core MIDI message processing and routing.
 * Extracted from the original MessageHandler to follow single responsibility principle.
 */

#include "MidiMsgHandler.h"
#include "Distributors/DistributorManager.h"
#include "SysExMsgHandler.h"
#include "../Instruments/InstrumentController.h"
#include <Arduino.h>

// Constructor with dependency injection
MidiMsgHandler::MidiMsgHandler(std::shared_ptr<DistributorManager> distributorManager,
                               std::shared_ptr<SysExMsgHandler> sysExHandler,
                               std::shared_ptr<InstrumentControllerBase> instrumentController)
    : distributorManager(distributorManager)
    , sysExHandler(sysExHandler)
    , instrumentController(instrumentController)
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Main Message Processing
////////////////////////////////////////////////////////////////////////////////////////////////////

// Process a MIDI message and return optional response
std::optional<MidiMessage> MidiMsgHandler::processMessage(MidiMessage& message)
{
    // Handle messages based on type - optimize for most common cases first
    const uint8_t msgType = message.type();
    
    if (msgType == Midi::ControlChange) {
        // Handle Control Change Messages
        processCC(message);
    } else if (msgType == Midi::SysCommon) {
        // Handle System Common Messages
        return processSystemMessage(message);
    } else {
        // Handle other MIDI messages (route to distributors)
        distributeMessage(message);
    }
    return std::nullopt;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Message Routing and Processing
////////////////////////////////////////////////////////////////////////////////////////////////////

// Route message to appropriate distributors based on channel
void MidiMsgHandler::distributeMessage(MidiMessage& message)
{
    if (distributorManager) {
        distributorManager->distributeMessage(message);
    }
}

// Process Control Change messages
void MidiMsgHandler::processCC(MidiMessage& message)
{
    if (distributorManager) {
        distributorManager->processCC(message);
    }
}

// Process System Common messages (SysEx, System Stop, System Reset)
std::optional<MidiMessage> MidiMsgHandler::processSystemMessage(MidiMessage& message)
{
    switch (message.sysCommonType()) {

        case(Midi::SysEx):
            if (sysExHandler) {
                return sysExHandler->processSysExMessage(message);
            }
            break;

        case(Midi::SysStop):
            if (instrumentController) {
                instrumentController->stopAll();
            }
            break;

        case(Midi::SysReset):
            if (instrumentController) {
                instrumentController->resetAll();
            }
            break;

        default:
            break;
    }
    
    return {};
}