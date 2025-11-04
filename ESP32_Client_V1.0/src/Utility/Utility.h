#pragma once
#include <array>
#include <bitset>
#include <cstring>


namespace Utility{

    // Convert std::bitset<N> to byte array (returns little-endian 8-bit bytes)
    template <size_t N>
    std::array<uint8_t, (N + 7) / 8> bitsetToBytes(const std::bitset<N>& bits) {
        std::array<uint8_t, (N + 7) / 8> bytes{};
        // Copy raw storage directly
        constexpr size_t blockCount = (N + 8 * sizeof(unsigned long) - 1) / (8 * sizeof(unsigned long));
        static_assert(sizeof(unsigned long) % 1 == 0, "Unexpected block size");

        // Access internal storage (standard guarantees contiguous storage in C++11+)
        const unsigned long* dataPtr = reinterpret_cast<const unsigned long*>(&bits);
        memcpy(bytes.data(), dataPtr, bytes.size());
        return bytes;
    }

    // Encode N-bit value into MIDI 7-bit format
    // MIDI SysEx requires MSB=0 for all bytes, so each byte holds only 7 bits of data
    // Packs bits MSB-first: 16 bits -> 3 MIDI bytes (2+7+7 bits), 32 bits -> 5 MIDI bytes (4+7+7+7+7 bits)
    // Example: 0x0003 (binary: 0000000000000011) -> [0x00, 0x00, 0x03]
    //          First byte has bits 14-15, second has bits 7-13, third has bits 0-6
    template <size_t NumBits>
    std::array<uint8_t, (NumBits + 6) / 7> encodeTo7Bit(const std::bitset<NumBits>& bits) {
        constexpr size_t midiBytes = (NumBits + 6) / 7;
        std::array<uint8_t, midiBytes> result{};
        
        // Get native byte representation (little-endian)
        auto nativeBytes = bitsetToBytes(bits);
        
        // Pack bits into 7-bit MIDI bytes, MSB-first
        // Calculate how many bits go in first byte (the remainder after dividing by 7)
        size_t firstByteBits = (NumBits % 7 == 0) ? 7 : (NumBits % 7);
        
        for (size_t i = 0; i < midiBytes; i++) {
            uint8_t midiByte = 0;
            size_t bitsInThisByte = (i == 0) ? firstByteBits : 7;
            
            // Calculate starting bit position (from MSB)
            size_t startBit = (i == 0) ? (NumBits - firstByteBits) : (NumBits - firstByteBits - i * 7);
            
            // Extract bits
            for (size_t bit = 0; bit < bitsInThisByte; bit++) {
                size_t bitPos = startBit + bit;
                size_t byteIdx = bitPos / 8;
                size_t bitIdx = bitPos % 8;
                if (nativeBytes[byteIdx] & (1 << bitIdx)) {
                    midiByte |= (1 << bit);
                }
            }
            result[i] = midiByte & 0x7F;
        }
        
        return result;
    }

    // Decode MIDI 7-bit format back to N-bit value
    // Unpacks bits MSB-first
    template <size_t NumBits>
    std::bitset<NumBits> decodeFrom7Bit(const uint8_t* midiBytes) {
        std::bitset<NumBits> result;
        constexpr size_t midiByteCount = (NumBits + 6) / 7;
        
        // Calculate how many bits are in first byte
        size_t firstByteBits = (NumBits % 7 == 0) ? 7 : (NumBits % 7);
        
        for (size_t i = 0; i < midiByteCount; i++) {
            uint8_t midiByte = midiBytes[i] & 0x7F;
            size_t bitsInThisByte = (i == 0) ? firstByteBits : 7;
            
            // Calculate starting bit position (from MSB)
            size_t startBit = (i == 0) ? (NumBits - firstByteBits) : (NumBits - firstByteBits - i * 7);
            
            // Extract bits and place in result
            for (size_t bit = 0; bit < bitsInThisByte; bit++) {
                if (midiByte & (1 << bit)) {
                    result[startBit + bit] = 1;
                }
            }
        }
        
        return result;
    }
}