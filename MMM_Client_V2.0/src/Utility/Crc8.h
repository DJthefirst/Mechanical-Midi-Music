#pragma once

#include <cstddef>
#include <cstdint>

namespace Utility {
inline uint8_t crc8(const uint8_t* data, size_t length) noexcept {
    uint8_t crc = 0;
    for (size_t index = 0; index < length; ++index) {
        crc ^= data[index];
        for (uint8_t bit = 0; bit < 8; ++bit) {
            crc = (crc & 0x80) ? static_cast<uint8_t>((crc << 1) ^ 0x07) : static_cast<uint8_t>(crc << 1);
        }
    }
    return crc;
}
}