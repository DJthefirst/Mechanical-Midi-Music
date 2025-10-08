/*
 * MessageRouter.cpp
 *
 * Handles message routing between networks and message handlers.
 * Encapsulates the main message processing loop logic.
 */

#include "MessageRouter.h"
#include "Constants.h"

// Constructor
MessageRouter::MessageRouter(
    std::shared_ptr<NetworkManager> networkManager,
    std::shared_ptr<DistributorManager> distributorManager,
    std::shared_ptr<SysExMsgHandler> sysExHandler,
    std::shared_ptr<MidiMsgHandler> midiHandler
) : m_networkManager(networkManager),
    m_distributorManager(distributorManager),
    m_sysExHandler(sysExHandler),
    m_midiHandler(midiHandler)
{
}

// Process messages from all networks
void MessageRouter::processMessages()
{
    if (!m_networkManager) return;

    // Loop through each network individually
    for (size_t i = 0; i < m_networkManager->numberOfNetworks(); i++) {
        INetwork* currentNetwork = m_networkManager->getNetwork(i);
        if (!currentNetwork) continue;

        // Read message from this specific network
        std::optional<MidiMessage> message = currentNetwork->readMessage();
        if (!message) continue;

        // Process the message
        processMessage(*message, currentNetwork);
    }
    
    // Check for instrument timeouts (only when configured)
    if (m_distributorManager) {
        m_distributorManager->checkInstrumentTimeouts();
    }
}

// Set callback for device changed notifications
void MessageRouter::setDeviceChangedCallback(std::function<void(const MidiMessage&, INetwork*)> callback)
{
    m_deviceChangedCallback = callback;
}

// Broadcast a device changed message to all networks except the source
void MessageRouter::broadcastDeviceChanged(INetwork* sourceNetwork)
{
    if (!m_networkManager) return;

    MidiMessage deviceChangedMsg(Device::GetDeviceID(), SYSEX_Server, SYSEX_DeviceChanged, nullptr, 0);
    
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

// Process a single message from a specific network
void MessageRouter::processMessage(MidiMessage& message, INetwork* sourceNetwork)
{
    // Store current source network for device changed notifications (thread-local)
    static thread_local INetwork* currentSourceNetwork = nullptr;
    currentSourceNetwork = sourceNetwork;

    // Process message based on type
    if (message.type() == MIDI_SysCommon && message.sysCommonType() == MIDI_SysEX) {
        // Handle SysEx messages with SysExMsgHandler
        if (m_sysExHandler) {
            std::optional<MidiMessage> response = m_sysExHandler->processSysExMessage(message);
            // Send response back to the originating network only
            if (response && sourceNetwork) {
                sourceNetwork->sendMessage(*response);
            }
        }
    } else {
        // Handle other MIDI messages with MidiMsgHandler
        if (m_midiHandler) {
            m_midiHandler->processMessage(message);
        }
    }

    // Clear the source network context
    currentSourceNetwork = nullptr;
}