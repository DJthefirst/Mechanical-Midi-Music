#pragma once

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <map>
#include <memory>
#include <string>
#include <set>
#include <vector>

using oflag_t = uint8_t;
constexpr oflag_t O_RDONLY = 1, O_RDWR = 2, O_CREAT = 4, O_TRUNC = 8;
constexpr uint8_t BUILTIN_SDCARD = 254;

namespace TestSd {
inline std::map<std::string, std::shared_ptr<std::vector<uint8_t>>> files;
inline std::set<std::string> directories;
inline bool mountFails = false, syncFails = false, closeFails = false, readFails = false;
inline int writeBudget = -1;
inline uint32_t writes = 0;
inline uint32_t clock = 0, readDelayMs = 0;
inline void reset() {
    files.clear();
    directories.clear();
    mountFails = syncFails = closeFails = readFails = false;
    writeBudget = -1;
    writes = 0;
    clock = readDelayMs = 0;
}
}

class FsFile {
public:
    FsFile() = default;
    explicit FsFile(std::shared_ptr<std::vector<uint8_t>> data, std::string name = {})
        : m_data(std::move(data)), m_name(std::move(name)) {}
    explicit FsFile(std::string directory) : m_name(std::move(directory)), m_directory(true) {
        const std::string prefix = m_name + "/";
        for (const auto& file : TestSd::files) {
            if (file.first.compare(0, prefix.size(), prefix) == 0 &&
                file.first.find('/', prefix.size()) == std::string::npos)
                m_entries.push_back(file.first);
        }
    }
    explicit operator bool() const { return m_directory || static_cast<bool>(m_data); }
    bool isDirectory() const { return m_directory; }
    uint8_t getError() const { return TestSd::readFails ? 1 : 0; }
    size_t getName(char* buffer, size_t length) {
        const auto name = m_name.substr(m_name.find_last_of('/') + 1);
        const size_t count = std::min(name.size(), length - 1);
        std::memcpy(buffer, name.data(), count);
        buffer[count] = 0;
        return count;
    }
    bool openNext(FsFile* directory, oflag_t) {
        close();
        if (TestSd::readFails || !directory->m_directory ||
            directory->m_next == directory->m_entries.size()) return false;
        m_name = directory->m_entries[directory->m_next++];
        m_data = TestSd::files.at(m_name);
        m_position = 0;
        return true;
    }
    uint64_t fileSize() const { return m_data ? m_data->size() : 0; }
    uint64_t curPosition() const { return m_position; }
    bool seekSet(uint64_t offset) {
        if (!m_data || offset > m_data->size()) return false;
        m_position = static_cast<size_t>(offset);
        return true;
    }
    int read(void* buffer, size_t length) {
        if (!m_data || TestSd::readFails) return -1;
        TestSd::clock += TestSd::readDelayMs;
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
        m_directory = false;
        return !TestSd::closeFails;
    }

private:
    std::shared_ptr<std::vector<uint8_t>> m_data;
    size_t m_position = 0;
    bool m_writeError = false;
    std::string m_name;
    bool m_directory = false;
    std::vector<std::string> m_entries;
    size_t m_next = 0;
};

struct TestSdFs {
    FsFile open(const char* path, oflag_t flags) {
        if (TestSd::directories.count(path)) return FsFile(std::string(path));
        auto found = TestSd::files.find(path);
        if (found == TestSd::files.end()) {
            if (!(flags & O_CREAT)) return {};
            found = TestSd::files.emplace(path, std::make_shared<std::vector<uint8_t>>()).first;
        }
        if (flags & O_TRUNC) found->second->clear();
        return FsFile(found->second, path);
    }
};

struct TestSdClass {
    TestSdFs sdfs;
    bool begin(uint8_t) { return !TestSd::mountFails; }
    bool exists(const char* path) { return TestSd::files.count(path) != 0; }
};
inline TestSdClass SD;
