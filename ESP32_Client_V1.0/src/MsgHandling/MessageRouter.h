/*
 * MessageRouter.h
 *
 * Handles message routing between networks and message handlers.
 * Encapsulates the main message processing loop logic.
 */

#pragma once

#include "Networks/NetworkManager.h"
#include "Distributors/DistributorManager.h"
#include "SysExMsgHandler.h"
#include "MidiMsgHandler.h"
#include "MidiMessage.h"
#include <memory>
#include <functional>

/**
 * MessageRouter handles the core message routing logic between networks and handlers.
 * This class encapsulates the main loop functionality for processing MIDI messages.
 */
class MessageRouter {
private:
    std::shared_ptr<NetworkManager> m_networkManager;
    std::shared_ptr<MidiMsgHandler> m_midiMsgHandler;
    std::shared_ptr<SysExMsgHandler> m_sysExMsgHandler;
    
    std::function<void(const MidiMessage&, INetwork*)> m_deviceChangedCallback;

public:
    /**
     * Constructor
     * @param networkManager Shared pointer to network manager
     * @param midiMsgHandler Shared pointer to MIDI message handler
     * @param sysExMsgHandler Shared pointer to SysEx message handler
     */
    MessageRouter(std::shared_ptr<NetworkManager> networkManager,
                  std::shared_ptr<MidiMsgHandler> midiMsgHandler,
                  std::shared_ptr<SysExMsgHandler> sysExMsgHandler);

    /**
     * Process messages from all networks.
     * This method should be called repeatedly in the main loop.
     */
    void processMessages();

    /**
     * Set callback for device changed notifications
     * @param callback Function to call when device changes occur
     */
    void setDeviceChangedCallback(std::function<void(const MidiMessage&, INetwork*)> callback);

    /**
     * Broadcast a device changed message to all networks except the source
     * @param sourceNetwork The network that originated the change (can be nullptr)
     */
    void broadcastDeviceChanged(INetwork* sourceNetwork = nullptr);

private:
    /**
     * Process a single message from a specific network
     * @param message The MIDI message to process
     * @param sourceNetwork The network that provided the message
     */
    void processMessage(MidiMessage& message, INetwork* sourceNetwork);
};