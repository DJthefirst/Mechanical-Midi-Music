/*
 * Teensy41_NetworkUSB.h
 *
 * Teensy 4.1 USB MIDI network implementation using the built-in usbMIDI library.
 * The device appears as a USB MIDI output device with the name set by CFG_DEVICE_NAME.
 *
 * Requirements:
 *   - Build flag: -D USB_MIDI_SERIAL (enables USB MIDI + Serial on Teensy)
 *   - Build flag: -D CFG_MMM_NETWORK_USB (enables the USB network)
 */

#pragma once

#include "Networks/INetwork.h"

#if defined(CFG_MMM_NETWORK_USB) && (defined(ARDUINO_TEENSY41) || defined(__IMXRT1062__))

class Teensy41_NetworkUSB : public INetwork {
public:
    Teensy41_NetworkUSB() = default;
    void begin() override;
    void sendMessage(const MidiMessage& message) override;
    void sendString(const String& message) override;
    std::optional<MidiMessage> readMessage() override;
};

#endif /* CFG_MMM_NETWORK_USB && TEENSY41 */
