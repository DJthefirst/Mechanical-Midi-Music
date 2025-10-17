#pragma once
#include <array>
#include <bitset>


namespace Utility{

    // Convert std::bitset<N> to byte array
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
}