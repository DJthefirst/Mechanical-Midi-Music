/*
 * MessageRouter.cpp
 *
 * Handles message routing between networks and message handlers.
 * Encapsulates the main message processing loop logic.
 */

#include "MessageRouter.h"
#include "Instruments/InstrumentController.h"
#include "Constants.h"

// Constructor
MessageRouter::MessageRouter(
    std::shared_ptr<NetworkManager> networkManager,
    std::shared_ptr<MidiMsgHandler> midiMsgHandler,
    std::shared_ptr<SysExMsgHandler> sysExMsgHandler,
    std::shared_ptr<InstrumentControllerBase> instrumentController) 
    : m_networkManager(networkManager)
    , m_midiMsgHandler(midiMsgHandler)
    , m_sysExMsgHandler(sysExMsgHandler)
    , m_instrumentController(instrumentController) {}

// Set callback for device changed notifications
void MessageRouter::setDeviceChangedCallback(std::function<void(const MidiMessage&, INetwork*)> callback)
{
    m_deviceChangedCallback = callback;
}

// Broadcast a device changed message to all networks except the source
void MessageRouter::broadcastDeviceChanged(INetwork* sourceNetwork)
{
    if (!m_networkManager) return;

    MidiMessage deviceChangedMsg(Device::GetDeviceID(), SysEx::Server, SysEx::DeviceChanged, nullptr, 0);
    
    if (sourceNetwork) {
        // Send to all networks except the one that originated the change
        m_networkManager->sendMessageToOthers(deviceChangedMsg, sourceNetwork);
    } else {
        // Send to all networks if no source network (e.g., internal change)
        m_networkManager->sendMessage(deviceChangedMsg);
    }

    // Notify callback if set
    if (m_deviceChangedCallback) {
        m_deviceChangedCallback(deviceChangedMsg, sourceNetwork);
    }
}

// Process messages from all networks
void MessageRouter::processMessages()
{
    if (!m_networkManager) return;

    // Loop through each network individually
    const size_t numNetworks = m_networkManager->numberOfNetworks();
    for (size_t i = 0; i < numNetworks; ++i) {
        INetwork* const net = m_networkManager->getNetwork(i);
        if (!net) continue;

        // Read message from this specific network
        if (auto message = net->readMessage(); message.has_value()) {
            processMessage(*message, net);
        }
    }
    
    // Check for instrument timeouts (only when configured)
    if (m_instrumentController) {
        m_instrumentController->checkInstrumentTimeouts();
    }
    
}

// Process a single message from a specific network
void MessageRouter::processMessage(MidiMessage& message, INetwork* sourceNetwork)
{
    INetwork* currentSourceNetwork = sourceNetwork;

    // Process message based on type - optimize for most common case first
    const uint8_t msgType = message.type();
    if (message.type() == Midi::SysCommon && message.sysCommonType() == Midi::SysEx) {
        // Handle SysEx messages
        auto response = m_sysExMsgHandler->processSysExMessage(message);
        if ( response.has_value() && sourceNetwork != nullptr) {
            sourceNetwork->sendMessage(*response);
        }
        
    } else {
        // Handle other MIDI messages with MidiMsgHandler
        m_midiMsgHandler->processMessage(message);
    }

    // Clear the source network context
    currentSourceNetwork = nullptr;
}