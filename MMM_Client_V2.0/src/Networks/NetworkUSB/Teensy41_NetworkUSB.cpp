/*
 * Teensy41_NetworkUSB.cpp
 *
 * Teensy 4.1 USB MIDI network implementation using the built-in usbMIDI library.
 * Translates between raw MIDI byte buffers (MidiMessage) and the Teensy usbMIDI API.
 */

#include "Teensy41_NetworkUSB.h"

#if defined(CFG_MMM_NETWORK_USB) && (defined(ARDUINO_TEENSY41) || defined(__IMXRT1062__))

#include "Constants.h"
#include <algorithm>

// ─── Lifecycle ───────────────────────────────────────────────────────────────

void Teensy41_NetworkUSB::begin() {
    // The Teensy USB stack is initialised automatically during startup.
    // Nothing additional is required here.
}

// ─── Receive ─────────────────────────────────────────────────────────────────

std::optional<MidiMessage> Teensy41_NetworkUSB::readMessage() {
    if (!usbMIDI.read()) {
        return std::nullopt;
    }

    const uint8_t type = usbMIDI.getType();
    MidiMessage message;

    // ── System Exclusive: copy the raw byte array directly ──────────
    if (type == usbMIDI.SystemExclusive) {
        const uint8_t* sysex = usbMIDI.getSysExArray();
        const uint16_t len   = usbMIDI.getSysExArrayLength();
        const uint16_t copyLen = std::min(len, static_cast<uint16_t>(MAX_PACKET_LENGTH));
        std::copy(sysex, sysex + copyLen, message.buffer.begin());
        message.length = static_cast<uint8_t>(copyLen);
        return message;
    }

    // ── Channel Messages: reconstruct raw MIDI bytes ────────────────
    // usbMIDI.getType() returns the status high nibble (0x80–0xE0).
    // Combine with channel (1-16 → 0-15) to form the full status byte.
    const uint8_t status = type | ((usbMIDI.getChannel() - 1) & 0x0F);
    message.buffer[0] = status;
    message.buffer[1] = usbMIDI.getData1();

    // ProgramChange (0xC0) and ChannelPressure (0xD0) are 2-byte messages
    if (type == 0xC0 || type == 0xD0) {
        message.length = 2;
    } else {
        message.buffer[2] = usbMIDI.getData2();
        message.length = 3;
    }

    return message;
}

// ─── Transmit ────────────────────────────────────────────────────────────────

void Teensy41_NetworkUSB::sendMessage(const MidiMessage& message) {
    if (message.length == 0) return;

    const uint8_t status  = message.buffer[0];
    const uint8_t type    = status & 0xF0;
    const uint8_t channel = (status & 0x0F) + 1; // usbMIDI channels are 1-16

    switch (type) {
        case Midi::NoteOff:
            usbMIDI.sendNoteOff(message.buffer[1], message.buffer[2], channel);
            break;

        case Midi::NoteOn:
            usbMIDI.sendNoteOn(message.buffer[1], message.buffer[2], channel);
            break;

        case Midi::KeyPressure:
            usbMIDI.sendAfterTouchPoly(message.buffer[1], message.buffer[2], channel);
            break;

        case Midi::ControlChange:
            usbMIDI.sendControlChange(message.buffer[1], message.buffer[2], channel);
            break;

        case Midi::ProgramChange:
            usbMIDI.sendProgramChange(message.buffer[1], channel);
            break;

        case Midi::ChannelPressure:
            usbMIDI.sendAfterTouch(message.buffer[1], channel);
            break;

        case Midi::PitchBend: {
            // Raw MIDI: data1 = LSB (7-bit), data2 = MSB (7-bit)
            // usbMIDI expects a signed value from -8192 to +8191
            const int rawValue  = (static_cast<int>(message.buffer[2]) << 7) | message.buffer[1];
            const int pitchBend = rawValue - 8192;
            usbMIDI.sendPitchBend(pitchBend, channel);
            break;
        }

        case Midi::SysCommon:
            if (status == 0xF0 && message.length > 1) {
                // sendSysEx expects data WITHOUT the leading 0xF0.
                // hasTerm = true  → the caller has already included 0xF7 at the end.
                const bool hasTerm = (message.buffer[message.length - 1] == 0xF7);
                usbMIDI.sendSysEx(
                    message.length - 1,           // length without 0xF0
                    message.buffer.data() + 1,    // skip the 0xF0
                    hasTerm
                );
            }
            break;

        default:
            break;
    }

    usbMIDI.send_now(); // Flush the USB MIDI buffer for low-latency output
}

// ─── Debug String ────────────────────────────────────────────────────────────

void Teensy41_NetworkUSB::sendString(const String& message) {
    // USB MIDI has no text channel.
    // When USB_MIDI_SERIAL (or similar) is configured, Serial is still available.
    #if defined(USB_MIDI_SERIAL) || defined(USB_MIDI4_SERIAL) || defined(USB_MIDI16_SERIAL)
        Serial.println(message);
    #endif
}

#endif /* CFG_MMM_NETWORK_USB && TEENSY41 */
