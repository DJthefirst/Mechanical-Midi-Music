#pragma once

#include "Config.h"

#if defined(PLATFORM_TEENSY41)
#include <SD.h>
#include "IReadFile.h"

namespace Utility {
// Own one instance per reader. Handles have independent positions on the shared card.
class Teensy41SdReadFile : public IReadFile {
public:
    Teensy41SdReadFile() = default;
    Teensy41SdReadFile(const Teensy41SdReadFile&) = delete;
    Teensy41SdReadFile& operator=(const Teensy41SdReadFile&) = delete;
    bool open(const char* path);
    void close() { m_file.close(); }
    int read(void* buffer, size_t length) override { return m_file.read(buffer, length); }
    bool seek(uint32_t offset) override { return m_file.seekSet(offset); }
    uint32_t position() const override { return static_cast<uint32_t>(m_file.curPosition()); }
    uint32_t size() const override { return static_cast<uint32_t>(m_file.fileSize()); }

private:
    FsFile m_file;
};

class Teensy41SdCardUtility {
public:
    enum class Error : uint8_t { None, MountFailed, InvalidPath, OpenFailed, FileTooLarge };
    static Teensy41SdCardUtility& getInstance();
    bool initialize();
    bool isAvailable() const { return m_initialized; }
    Error lastError() const { return m_error; }
    FsFile openRead(const char* path);
    FsFile openWrite(const char* path);
    Teensy41SdCardUtility(const Teensy41SdCardUtility&) = delete;
    Teensy41SdCardUtility& operator=(const Teensy41SdCardUtility&) = delete;

private:
    Teensy41SdCardUtility() = default;
    FsFile open(const char* path, oflag_t flags);
    bool m_initialized = false;
    Error m_error = Error::None;
};
}
#endif
