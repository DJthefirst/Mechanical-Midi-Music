/*
 * MidiMsgHandler.cpp
 *
 * Handles core MIDI message processing and routing.
 * Extracted from the original MessageHandler to follow single responsibility principle.
 */

#include "MidiMsgHandler.h"
#include "Distributors/DistributorManager.h"
#include "SysExMsgHandler.h"
#include "../Instruments/InstrumentControllerBase.h"
#include <Arduino.h>

// Constructor with dependency injection
MidiMsgHandler::MidiMsgHandler(DistributorManager& distributorManager,
                               SysExMsgHandler& sysExHandler,
                               InstrumentControllerBase& instrumentController)
    : m_distributorManager(&distributorManager)
    , m_sysExHandler(&sysExHandler)
    , m_instrumentController(&instrumentController)
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Main Message Processing
////////////////////////////////////////////////////////////////////////////////////////////////////

// Process a MIDI message and return optional response
std::optional<MidiMessage> MidiMsgHandler::processMessage(const MidiMessage& message)
{
    // Handle messages based on type - optimize for most common cases first
    const uint8_t msgType = message.type();
    
    switch(msgType){
        case(Midi::ControlChange):
            processCC(message);
            break;
        case (Midi::SysCommon):
            return processSystemMessage(message);
        case(Midi::NoteOff):
        case(Midi::NoteOn):
            // Route message to appropriate distributors based on channel
            if (m_distributorManager) m_distributorManager->distributeMessage(message);
            break;
        case(Midi::KeyPressure):
            m_instrumentController->setKeyPressure(message.channel(), message.buffer[1], message.buffer[2]);
            break;
        case(Midi::ProgramChange):
            m_instrumentController->setProgramChange(message.channel(), message.buffer[1]);
            break;
        case(Midi::ChannelPressure):
            m_instrumentController->setChannelPressure(message.channel(), message.buffer[1]);
            break;
        case(Midi::PitchBend):
            m_instrumentController->setPitchBend(message.channel(), (static_cast<uint16_t>(message.buffer[2]) << 7) | static_cast<uint16_t>(message.buffer[1]));
            break;
        default:
            break;
    }
    return std::nullopt;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// Message Routing and Processing
////////////////////////////////////////////////////////////////////////////////////////////////////

// Process CC MIDI Messages by type
void MidiMsgHandler::processCC(const MidiMessage& message)
{
    uint8_t channel = message.channel();
    m_instrumentController->setControlChange(channel, message.CC_Control(), message.CC_Value());
    switch (message.CC_Control()) {
        case(MidiCC::Mute):
            m_instrumentController->stopAll();
            break;
        case(MidiCC::AllNotesOff):
            m_instrumentController->stopAll();
            break;
        case(MidiCC::Monophonic):
            //Device::setPolyphonic(false);
            break;
        case(MidiCC::Polyphonic):
            //Device::setPolyphonic(true);
            break;
        default:
            break;
    }
}

// Process System Common messages (SysEx, System Stop, System Reset)
std::optional<MidiMessage> MidiMsgHandler::processSystemMessage(const MidiMessage& message)
{
    switch (message.sysCommonType()) {

        case(Midi::SysEx):
            if (m_sysExHandler) {
                return m_sysExHandler->processSysExMessage(message);
            }
            break;

        case(Midi::SysStop):
            if (m_instrumentController) {
                m_instrumentController->stopAll();
            }
            break;

        case(Midi::SysReset):
            if (m_instrumentController) {
                m_instrumentController->resetAll();
            }
            break;

        default:
            break;
    }
    
    return {};
}