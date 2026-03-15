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
#include <functional>

/**
 * MessageRouter handles the core message routing logic between networks and handlers.
 * This class encapsulates the main loop functionality for processing MIDI messages.
 */
class MessageRouter {
private:
    NetworkManager* m_networkManager;
    MidiMsgHandler* m_midiMsgHandler;
    SysExMsgHandler* m_sysExMsgHandler;
    InstrumentControllerBase* m_instrumentController;
    
    std::function<void(const MidiMessage&, INetwork*)> m_deviceChangedCallback;

public:
    /**
     * Constructor
        * @param networkManager Network manager dependency
        * @param midiMsgHandler MIDI message handler dependency
        * @param sysExMsgHandler SysEx message handler dependency
        * @param instrumentController Instrument controller dependency
     */
    MessageRouter(NetworkManager& networkManager,
                  MidiMsgHandler& midiMsgHandler,
                  SysExMsgHandler& sysExMsgHandler,
                  InstrumentControllerBase& instrumentController);

    void setDeviceChangedCallback(const std::function<void(const MidiMessage&, INetwork*)>& callback);

    void broadcastDeviceChanged(INetwork* sourceNetwork = nullptr);

    void processMessages();

private:

    void processMessage(MidiMessage& message, INetwork* sourceNetwork);
};