#pragma once

#include "MsgHandling/MidiMessage.h"
#include <optional>

class ESP32_S3_USBMidiCodec {
public:
    using Packet = std::array<uint8_t, 4>;
    enum class Error { None, InvalidPacket, SysExOverflow };

    struct Result {
        std::optional<MidiMessage> message;
        Error error = Error::None;
    };

    static uint8_t messageLength(uint8_t status) {
        if (status >= 0x80 && status < 0xF0) {
            return (status < 0xC0 || status >= 0xE0) ? 3 : 2;
        }
        switch (status) {
            case 0xF1:
            case 0xF3: return 2;
            case 0xF2: return 3;
            case 0xF6: return 1;
            default: return status >= 0xF8 ? 1 : 0;
        }
    }

    static bool isValid(const MidiMessage& message) {
        if (message.length == 0 || message.length > MAX_PACKET_LENGTH) return false;
        const bool sysex = message.buffer[0] == 0xF0;
        if (sysex) {
            if (message.length < 2 || message.buffer[message.length - 1] != 0xF7) return false;
        } else if (message.length != messageLength(message.buffer[0])) {
            return false;
        }
        const uint8_t end = message.length - (sysex ? 1 : 0);
        for (uint8_t i = 1; i < end; ++i) {
            if (message.buffer[i] >= 0x80) return false;
        }
        return true;
    }

    // The caller validates the complete message once before encoding its packets.
    static Packet encodePacket(const MidiMessage& message, uint8_t& offset) {
        Packet packet{};
        uint8_t count = message.length - offset;
        if (message.buffer[0] == 0xF0) {
            packet[0] = count > 3 ? 0x04 : static_cast<uint8_t>(0x04 + count);
            count = std::min<uint8_t>(count, 3);
        } else {
            const uint8_t status = message.buffer[0];
            packet[0] = status < 0xF0 ? status >> 4
                : status >= 0xF8 ? 0x0F
                : status == 0xF6 ? 0x05
                : count == 2 ? 0x02 : 0x03;
        }
        std::copy_n(message.buffer.begin() + offset, count, packet.begin() + 1);
        offset += count;
        return packet;
    }

    void reset() {
        m_sysex.length = 0;
        m_discardingSysEx = false;
    }

    Result decodePacket(const Packet& packet) {
        // This descriptor exposes only cable zero.
        if ((packet[0] >> 4) != 0) return {};
        const uint8_t cin = packet[0] & 0x0F;
        const uint8_t status = packet[1];
        const bool sysex = cin == 4 || cin == 6 || cin == 7 || (cin == 5 && status == 0xF7);
        if (sysex) {
            const uint8_t count = cin == 4 ? 3 : cin - 4;
            const bool ending = cin != 4;
            if (ending && packet[count] != 0xF7) return invalidPacket();
            for (uint8_t i = 1; i <= count; ++i) {
                if ((i == 1 && packet[i] == 0xF0) || (ending && i == count)) continue;
                if (packet[i] >= 0x80) return invalidPacket();
            }
            if (status == 0xF0) reset();
            if (m_discardingSysEx) {
                if (ending) reset();
                return {};
            }
            if (m_sysex.length == 0 && status != 0xF0) return invalidPacket();
            if (m_sysex.length + count > MAX_PACKET_LENGTH) {
                reset();
                m_discardingSysEx = !ending;
                return {std::nullopt, Error::SysExOverflow};
            }
            std::copy_n(packet.begin() + 1, count, m_sysex.buffer.begin() + m_sysex.length);
            m_sysex.length += count;
            if (!ending) return {};
            MidiMessage complete = m_sysex;
            reset();
            return {complete, Error::None};
        }

        const uint8_t count = messageLength(status);
        const uint8_t expectedCin = status < 0xF0 ? status >> 4
            : status >= 0xF8 ? 0x0F
            : status == 0xF6 ? 0x05
            : count == 2 ? 0x02 : 0x03;
        if (count == 0 || cin != expectedCin) return invalidPacket();
        MidiMessage message;
        message.length = count;
        std::copy_n(packet.begin() + 1, count, message.buffer.begin());
        if (!isValid(message)) return invalidPacket();
        // Real-time events may be interleaved with SysEx without ending it.
        if (status < 0xF8) reset();
        return {message, Error::None};
    }

private:
    MidiMessage m_sysex;
    bool m_discardingSysEx = false;

    Result invalidPacket() {
        reset();
        return {std::nullopt, Error::InvalidPacket};
    }
};
