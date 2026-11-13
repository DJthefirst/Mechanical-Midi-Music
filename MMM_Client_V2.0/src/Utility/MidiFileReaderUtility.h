#pragma once

#include "SdCard/IReadFile.h"
#include <array>

namespace Utility {
class MidiFileReaderUtility {
public:
    static constexpr uint16_t MaxTracks = 32;
    enum class Result : uint8_t {
        Event, EndOfTrack, NotOpen, IoError, InvalidFile, UnsupportedFormat, CapacityExceeded
    };
    struct Event {
        uint32_t deltaTicks = 0;
        uint32_t absoluteTicks = 0;
        uint8_t status = 0;
        uint8_t data[2] = {};
        uint8_t dataLength = 0;
        uint8_t metaType = 0;
        uint32_t payloadOffset = 0;
        uint32_t payloadLength = 0;
        uint32_t tempoUsPerQuarter = 0;
    };

    bool open(IReadFile& file);
    void close();
    Result next(uint16_t track, Event& event);
    bool rewind(uint16_t track);
    Result lastResult() const { return m_result; }
    uint16_t format() const { return m_format; }
    uint16_t trackCount() const { return m_trackCount; }
    uint16_t ticksPerQuarter() const { return m_division; }

private:
    struct Track {
        uint32_t start = 0, end = 0, cursor = 0, ticks = 0;
        uint8_t runningStatus = 0;
        bool ended = false;
    };
    bool fail(Result result);
    bool read(void* buffer, size_t length, uint32_t end);
    bool byte(uint8_t& value, uint32_t end) { return read(&value, 1, end); }
    bool vlq(uint32_t& value, uint32_t end);
    IReadFile* m_file = nullptr;
    std::array<Track, MaxTracks> m_tracks{};
    uint16_t m_format = 0, m_trackCount = 0, m_division = 0;
    Result m_result = Result::NotOpen;
};
}
