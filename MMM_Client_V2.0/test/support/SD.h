#pragma once

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <map>
#include <memory>
#include <string>
#include <vector>

using oflag_t = uint8_t;
constexpr oflag_t O_RDONLY = 1, O_RDWR = 2, O_CREAT = 4, O_TRUNC = 8;
constexpr uint8_t BUILTIN_SDCARD = 254;

namespace TestSd {
inline std::map<std::string, std::shared_ptr<std::vector<uint8_t>>> files;
inline bool mountFails = false, syncFails = false, closeFails = false, readFails = false;
inline int writeBudget = -1;
inline uint32_t writes = 0;
inline void reset() {
    files.clear();
    mountFails = syncFails = closeFails = readFails = false;
    writeBudget = -1;
    writes = 0;
}
}

class FsFile {
public:
    FsFile() = default;
    explicit FsFile(std::shared_ptr<std::vector<uint8_t>> data) : m_data(std::move(data)) {}
    explicit operator bool() const { return static_cast<bool>(m_data); }
    uint64_t fileSize() const { return m_data ? m_data->size() : 0; }
    uint64_t curPosition() const { return m_position; }
    bool seekSet(uint64_t offset) {
        if (!m_data || offset > m_data->size()) return false;
        m_position = static_cast<size_t>(offset);
        return true;
    }
    int read(void* buffer, size_t length) {
        if (!m_data || TestSd::readFails) return -1;
        length = std::min(length, m_data->size() - m_position);
        std::memcpy(buffer, m_data->data() + m_position, length);
        m_position += length;
        return static_cast<int>(length);
    }
    size_t write(const void* buffer, size_t length) {
        if (!m_data) return 0;
        ++TestSd::writes;
        if (TestSd::writeBudget >= 0) {
            const size_t allowed = std::min(length, static_cast<size_t>(TestSd::writeBudget));
            m_writeError = allowed != length;
            length = allowed;
            TestSd::writeBudget -= static_cast<int>(length);
        }
        m_data->resize(m_position + length);
        std::memcpy(m_data->data() + m_position, buffer, length);
        m_position += length;
        return length;
    }
    bool sync() { return !TestSd::syncFails; }
    bool getWriteError() const { return m_writeError; }
    bool close() {
        m_data.reset();
        return !TestSd::closeFails;
    }

private:
    std::shared_ptr<std::vector<uint8_t>> m_data;
    size_t m_position = 0;
    bool m_writeError = false;
};

struct TestSdFs {
    FsFile open(const char* path, oflag_t flags) {
        auto found = TestSd::files.find(path);
        if (found == TestSd::files.end()) {
            if (!(flags & O_CREAT)) return {};
            found = TestSd::files.emplace(path, std::make_shared<std::vector<uint8_t>>()).first;
        }
        if (flags & O_TRUNC) found->second->clear();
        return FsFile(found->second);
    }
};

struct TestSdClass {
    TestSdFs sdfs;
    bool begin(uint8_t) { return !TestSd::mountFails; }
    bool exists(const char* path) { return TestSd::files.count(path) != 0; }
};
inline TestSdClass SD;
