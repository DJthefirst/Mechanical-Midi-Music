#include "Teensy41SdCardUtility.h"

#if defined(PLATFORM_TEENSY41)
#include <limits>

namespace Utility {
Teensy41SdCardUtility& Teensy41SdCardUtility::getInstance() {
    static Teensy41SdCardUtility instance;
    return instance;
}

bool Teensy41SdCardUtility::initialize() {
    if (!m_initialized) {
        m_initialized = SD.begin(BUILTIN_SDCARD);
    }
    m_error = m_initialized ? Error::None : Error::MountFailed;
    return m_initialized;
}

FsFile Teensy41SdCardUtility::open(const char* path, oflag_t flags) {
    if (!path || !*path) {
        m_error = Error::InvalidPath;
        return {};
    }
    if (!initialize()) return {};
    FsFile file = SD.sdfs.open(path, flags);
    m_error = file ? Error::None : Error::OpenFailed;
    if (file && file.fileSize() > std::numeric_limits<uint32_t>::max()) {
        file.close();
        m_error = Error::FileTooLarge;
    }
    return file;
}

FsFile Teensy41SdCardUtility::openRead(const char* path) {
    return open(path, O_RDONLY);
}

FsFile Teensy41SdCardUtility::openWrite(const char* path) {
    return open(path, O_RDWR | O_CREAT | O_TRUNC);
}

bool Teensy41SdReadFile::open(const char* path) {
    close();
    m_file = Teensy41SdCardUtility::getInstance().openRead(path);
    return static_cast<bool>(m_file);
}
}
#endif
