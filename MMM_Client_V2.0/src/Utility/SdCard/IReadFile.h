#pragma once

#include <cstddef>
#include <cstdint>

namespace Utility {
class IReadFile {
public:
    virtual ~IReadFile() = default;
    virtual int read(void* buffer, size_t length) = 0;
    virtual bool seek(uint32_t offset) = 0;
    virtual uint32_t position() const = 0;
    virtual uint32_t size() const = 0;
};
}
