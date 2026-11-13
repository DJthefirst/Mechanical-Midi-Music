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
#elif defined(CFG_ESP32_S3)
    #include "Networks/NetworkUSB/ESP32_S3_NetworkUSB.h"
    #define NetworkUSB ESP32_S3_NetworkUSB
#elif defined(ESP32) || defined(ARDUINO_ARCH_ESP32)
    #error "USB MIDI network requires an ESP32-S3; other ESP32 targets are not implemented"
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