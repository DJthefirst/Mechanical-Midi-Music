/*
 * main.cpp - V2.0 M// Message handling
#include "core/MessageHandler.h"

// Optional features
#ifdef ENABLE_LOCAL_STORAGE
#include "extras/LocalStorage.h"
#endif

// Use configuration namespace
using namespace Config;

////////////////////////////////////////////////////////////////////////////////////////////////////
// Global Objects (configured by Device.h)
////////////////////////////////////////////////////////////////////////////////////////////////////

// Network and instrument instances (types defined in config.h)
NetworkType network;
InstrumentType instrumentController;Music Main Program
 *
 * Part of the Mechanical MIDI Music V2.0 redesign
 * Based on all REDESIGN_PART documentation
 *
 * This is the V2.0 main program with simplified configuration
 * and enhanced architecture while maintaining V1.0 compatibility.
 */

#include <Arduino.h>

// Configuration selection - use V2.0 configuration system
#define USE_V2_CONFIG

// Configuration and V2.0 Core System
#include "core/Device.h"
#include "core/MidiMessage.h"

// Network and Instrument (configured in config.h)
#include "networks/Network.h"
#include "instruments/InstrumentController.h"

// Message handling
#include "core/MessageHandler.h"

// Optional features
#ifdef ENABLE_LOCAL_STORAGE
    // #include "extras/LocalStorage.h"
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////
// Global Objects (configured by Device.h)
////////////////////////////////////////////////////////////////////////////////////////////////////

// Network and instrument instances (types defined in config.h)
NetworkType network;
InstrumentType instrumentController;

// Message handler with runtime distributor management
MessageHandler messageHandler(&instrumentController);

////////////////////////////////////////////////////////////////////////////////////////////////////
// Setup Function
////////////////////////////////////////////////////////////////////////////////////////////////////

void setup() {
    Serial.begin(115200);
    while (!Serial && millis() < 3000) {
        delay(10);
    }
    
    // Debug output disabled for MIDI serial communication
    // Uncomment only when debugging over a separate debug port
    /*
    Serial.println();
    Serial.println("=====================================");
    Serial.println(" Mechanical MIDI Music V2.0");
    Serial.println("=====================================");
    */
    
    // Initialize and validate configuration
    Device::validateConfiguration();
    
    // Initialize network
    // Serial.println("Initializing network...");
    network.begin();
    delay(100);
    
    // Print network status (disabled for MIDI mode)
    // network.printStatus();
    
    // Initialize instruments
    // Serial.println("Initializing instruments...");
    instrumentController.resetAll();
    
    // Print instrument status (disabled for MIDI mode)
    // instrumentController.printStatus();
    
    #ifdef ENABLE_LOCAL_STORAGE
    // TODO: Load saved distributors from NVS in future enhancement
    // Serial.println("Local storage enabled (future feature)");
    #endif
    
    // Create default distributor configuration
    // Serial.println("Creating default distributor...");
    messageHandler.addDistributor();
    
    // Configure default distributor: All channels -> All instruments, RoundRobinBalance
    auto& defaultDistributor = messageHandler.getDistributor(0);
    defaultDistributor.setChannels(0xFFFF);  // All MIDI channels (1-16)
    defaultDistributor.setInstruments((1UL << Config::NUM_INSTRUMENTS) - 1);  // All instruments
    defaultDistributor.setDistributionMethod(DistributionMethod::RoundRobinBalance);
    
    // Serial.println("Default distributor configured:");
    // defaultDistributor.printStatus();
    
    // Serial.println("Ready for MIDI messages!");
    
    // Send device ready message (V1.0 compatibility)
    MidiMessage ready(SYSEX_DEV_ID, SYSEX_Server, SYSEX_DeviceReady, nullptr, 0);
    network.sendMessage(ready);
    
    // Serial.println("Setup complete!");
    // Serial.println("=====================================");
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Main Loop - V2.0 Complete Implementation
////////////////////////////////////////////////////////////////////////////////////////////////////

void loop() {
    // Read incoming MIDI messages
    std::optional<MidiMessage> message = network.readMessage();
    
    if (!message.has_value()) {
        return;  // No message available
    }
    
    // Process message through the V2.0 message handler
    std::optional<MidiMessage> response = messageHandler.processMessage(*message);
    
    // Send response if there is one (mainly for SysEx commands)
    if (response.has_value()) {
        network.sendMessage(*response);
    }
    
    // Debug output disabled when using Serial for MIDI communication
    // (Uncomment these lines only when debugging over a separate debug port)
    /*
    if (message->isSysEx()) {
        Serial.println("SysEx: " + message->toString());
    } else {
        // Serial.println("MIDI: " + message->toString());
    }
    */
}