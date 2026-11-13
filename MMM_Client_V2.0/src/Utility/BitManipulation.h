#pragma once
#include <array>
#include <bitset>
#include <cstdint>


namespace Utility{

    // Encode N-bit value into MIDI 7-bit format
    // MIDI SysEx requires MSB=0 for all bytes, so each byte holds only 7 bits of data
    // Packs bits MSB-first: 16 bits -> 3 MIDI bytes (2+7+7 bits), 32 bits -> 5 MIDI bytes (4+7+7+7+7 bits)
    // Example: 0x0003 (binary: 0000000000000011) -> [0x00, 0x00, 0x03]
    //          First byte has bits 14-15, second has bits 7-13, third has bits 0-6
    template <size_t NumBits>
    std::array<uint8_t, (NumBits + 6) / 7> encodeTo7Bit(const std::bitset<NumBits>& bits) {
        static_assert(NumBits <= 32, "encodeTo7Bit supports at most 32 bits");
        std::array<uint8_t, (NumBits + 6) / 7> result{};

        // Fill from the last byte backwards, taking the lowest 7 bits each time
        uint32_t value = static_cast<uint32_t>(bits.to_ulong());
        for (size_t i = result.size(); i-- > 0;) {
            result[i] = value & 0x7F;
            value >>= 7;
        }

        return result;
    }

    // Decode MIDI 7-bit format back to N-bit value
    // Unpacks bits MSB-first
    template <size_t NumBits>
    std::bitset<NumBits> decodeFrom7Bit(const uint8_t* midiBytes) {
        static_assert(NumBits <= 32, "decodeFrom7Bit supports at most 32 bits");
        constexpr size_t midiByteCount = (NumBits + 6) / 7;

        // Shift each 7-bit group in; std::bitset keeps only the lowest NumBits
        uint32_t value = 0;
        for (size_t i = 0; i < midiByteCount; i++) {
            value = (value << 7) | (midiBytes[i] & 0x7F);
        }

        return std::bitset<NumBits>(value);
    }
}