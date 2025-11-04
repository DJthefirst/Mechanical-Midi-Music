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
    std::shared_ptr<InstrumentControllerBase> m_instrumentController;
    
    std::function<void(const MidiMessage&, INetwork*)> m_deviceChangedCallback;

public:
    /**
     * Constructor
     * @param networkManager Shared pointer to network manager
     * @param midiMsgHandler Shared pointer to MIDI message handler
     * @param sysExMsgHandler Shared pointer to SysEx message handler
     * @param instrumentController Shared pointer to instrument controller
     */
    MessageRouter(std::shared_ptr<NetworkManager> networkManager,
                  std::shared_ptr<MidiMsgHandler> midiMsgHandler,
                  std::shared_ptr<SysExMsgHandler> sysExMsgHandler,
                  std::shared_ptr<InstrumentControllerBase> instrumentController);

    void setDeviceChangedCallback(const std::function<void(const MidiMessage&, INetwork*)>& callback);

    void broadcastDeviceChanged(INetwork* sourceNetwork = nullptr);

    void processMessages();

private:

    void processMessage(MidiMessage& message, INetwork* sourceNetwork);
};