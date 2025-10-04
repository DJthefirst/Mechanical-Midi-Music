/*
 * Network.h - V2.0 Network Base Interface
 *
 * Part of the Mechanical MIDI Music V2.0 redesign
 * Based on REDESIGN_PART3_NETWORK.md specifications
 *
 * Key improvements over V1.0:
 * - Added diagnostics and status methods
 * - Better const correctness
 * - Enhanced documentation
 * - Backward compatible with V1.0 interface
 */

#pragma once

#include "core/MidiMessage.h"
#include <Arduino.h>
#include <optional>
#include <cstdint>

////////////////////////////////////////////////////////////////////////////////////////////////////
// Network Base Interface
////////////////////////////////////////////////////////////////////////////////////////////////////

class Network {
protected:
    static constexpr uint8_t MIN_MSG_BYTES = 3;
    
public:
    virtual ~Network() = default;
    
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    // Core Interface (V1.0 Compatible)
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    
    // Initialize network connection
    virtual void begin() = 0;
    
    // Send MIDI message
    virtual void sendMessage(const MidiMessage& message) = 0;
    
    // Read incoming MIDI message
    virtual std::optional<MidiMessage> readMessage() = 0;
    
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    // V2.0 Enhancements
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    
    // Connection status
    virtual bool isConnected() const { return true; }
    
    // Network identification
    virtual const char* getName() const = 0;
    
    // Status and diagnostics
    virtual void printStatus() const {}
    
    // Optional: Advanced features
    virtual void sendString(const String& str) {}
    virtual void flush() {}
    
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    // V1.0 Compatibility Methods
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    
    // Legacy string message sending (kept for V1.0 compatibility)
    virtual void sendMessage(const String& message) {
        // Convert string to MIDI if needed
        // Default implementation does nothing
        sendString(message);
    }
    
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    // Utility Methods
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    
    // Get connection type as string
    virtual const char* getConnectionType() const {
        return "Unknown";
    }
    
    // Get data rate information
    virtual uint32_t getDataRate() const {
        return 0; // Unknown/Variable
    }
    
    // Get maximum message size
    virtual uint16_t getMaxMessageSize() const {
        return 256; // Default MIDI buffer size
    }
};