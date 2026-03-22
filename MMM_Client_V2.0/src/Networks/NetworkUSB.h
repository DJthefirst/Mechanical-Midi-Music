/*
 * NetworkUSB.h
 *
 * Platform dispatcher for USB MIDI network implementations.
 * Includes the correct platform-specific header and #defines NetworkUSB
 * to the concrete class, following the same pattern as the Instruments.
 */

#pragma once

#ifdef CFG_MMM_NETWORK_USB

// Platform-specific includes
#if defined(__IMXRT1062__) || defined(ARDUINO_TEENSY41)
    #include "Networks/NetworkUSB/Teensy41_NetworkUSB.h"
    #define NetworkUSB Teensy41_NetworkUSB
#elif defined(ESP32) || defined(ARDUINO_ARCH_ESP32)
    // TODO: Add ESP32 USB MIDI implementation
    // #include "Networks/NetworkUSB/ESP32_NetworkUSB.h"
    // #define NetworkUSB ESP32_NetworkUSB
    #error "USB MIDI network is not yet implemented for ESP32"
#else
    // Unsupported platform: compile a no-op stub so the project still builds.
    #include "Networks/INetwork.h"
    class UnsupportedNetworkUSB : public INetwork {
    public:
        void begin() override {}
        void sendMessage(const MidiMessage& message) override {}
        void sendString(const String& message) override {}
        std::optional<MidiMessage> readMessage() override { return std::nullopt; }
    };
    #define NetworkUSB UnsupportedNetworkUSB
#endif

#endif /* CFG_MMM_NETWORK_USB */