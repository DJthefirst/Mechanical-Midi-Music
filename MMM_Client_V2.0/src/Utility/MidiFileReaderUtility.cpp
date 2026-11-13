#include "MidiFileReaderUtility.h"
#include <cstring>
#include <limits>

namespace Utility {
namespace {
uint16_t be16(const uint8_t* p) {
    return (static_cast<uint16_t>(p[0]) << 8) | p[1];
}
uint32_t be32(const uint8_t* p) {
    return (static_cast<uint32_t>(p[0]) << 24) | (static_cast<uint32_t>(p[1]) << 16) |
           (static_cast<uint32_t>(p[2]) << 8) | p[3];
}
}

void MidiFileReaderUtility::close() {
    m_file = nullptr;
    m_format = m_trackCount = m_division = 0;
    m_result = Result::NotOpen;
}

bool MidiFileReaderUtility::fail(Result result) {
    m_result = result;
    return false;
}

bool MidiFileReaderUtility::read(void* buffer, size_t length, uint32_t end) {
    const uint32_t position = m_file->position();
    if (position > end || length > end - position) return fail(Result::InvalidFile);
    if (m_file->read(buffer, length) != static_cast<int>(length)) return fail(Result::IoError);
    return true;
}

bool MidiFileReaderUtility::vlq(uint32_t& value, uint32_t end) {
    value = 0;
    for (uint8_t i = 0; i < 4; ++i) {
        uint8_t b;
        if (!byte(b, end)) return false;
        value = (value << 7) | (b & 0x7F);
        if (!(b & 0x80)) return true;
    }
    return fail(Result::InvalidFile);
}

bool MidiFileReaderUtility::open(IReadFile& file) {
    close();
    m_file = &file;
    uint8_t header[14];
    if (!file.seek(0)) return fail(Result::IoError);
    if (!read(header, sizeof(header), file.size())) return false;
    if (std::memcmp(header, "MThd", 4) || be32(header + 4) < 6)
        return fail(Result::InvalidFile);
    const uint32_t headerLength = be32(header + 4);
    const uint16_t format = be16(header + 8), count = be16(header + 10);
    const uint16_t division = be16(header + 12);
    if (format > 1 || (division & 0x8000)) return fail(Result::UnsupportedFormat);
    if (!count || !division || (format == 0 && count != 1)) return fail(Result::InvalidFile);
    if (count > MaxTracks) return fail(Result::CapacityExceeded);
    if (headerLength > file.size() - 8) return fail(Result::InvalidFile);
    if (!file.seek(8 + headerLength)) return fail(Result::IoError);
    uint16_t found = 0;
    while (found < count) {
        uint8_t chunk[8];
        if (!read(chunk, sizeof(chunk), file.size())) return false;
        const uint32_t start = file.position(), length = be32(chunk + 4);
        if (length > file.size() - start) return fail(Result::InvalidFile);
        if (!std::memcmp(chunk, "MTrk", 4)) {
            m_tracks[found++] = {start, start + length, start, 0, 0, false};
        }
        if (!file.seek(start + length)) return fail(Result::IoError);
    }
    m_format = format;
    m_trackCount = count;
    m_division = division;
    m_result = Result::Event;
    return true;
}

bool MidiFileReaderUtility::rewind(uint16_t track) {
    if (!m_file || !m_trackCount) return fail(Result::NotOpen);
    if (track >= m_trackCount) return fail(Result::InvalidFile);
    auto& t = m_tracks[track];
    t.cursor = t.start;
    t.ticks = 0;
    t.runningStatus = 0;
    t.ended = false;
    m_result = Result::Event;
    return true;
}

MidiFileReaderUtility::Result MidiFileReaderUtility::next(uint16_t track, Event& event) {
    event = {};
    if (!m_file || !m_trackCount) { fail(Result::NotOpen); return m_result; }
    if (track >= m_trackCount) { fail(Result::InvalidFile); return m_result; }
    auto& t = m_tracks[track];
    if (t.ended) return m_result = Result::EndOfTrack;
    if (!m_file->seek(t.cursor)) return m_result = Result::IoError;
    if (!vlq(event.deltaTicks, t.end)) return m_result;
    if (event.deltaTicks > std::numeric_limits<uint32_t>::max() - t.ticks)
        return m_result = Result::InvalidFile;
    event.absoluteTicks = t.ticks + event.deltaTicks;
    uint8_t status;
    if (!byte(status, t.end)) return m_result;
    const bool running = status < 0x80;
    if (running) {
        if (!t.runningStatus) return m_result = Result::InvalidFile;
        event.data[0] = status;
        status = t.runningStatus;
    }
    event.status = status;
    uint8_t nextRunning = 0;
    bool ended = false;
    if (status < 0xF0) {
        nextRunning = status;
        event.dataLength = (status & 0xE0) == 0xC0 ? 1 : 2;
        for (uint8_t i = running ? 1 : 0; i < event.dataLength; ++i) {
            if (!byte(event.data[i], t.end)) return m_result;
            if (event.data[i] & 0x80) return m_result = Result::InvalidFile;
        }
    } else if (status == 0xFF || status == 0xF0 || status == 0xF7) {
        if (status == 0xFF) {
            if (!byte(event.metaType, t.end)) return m_result;
            if (event.metaType & 0x80) return m_result = Result::InvalidFile;
        }
        if (!vlq(event.payloadLength, t.end)) return m_result;
        event.payloadOffset = m_file->position();
        if (event.payloadLength > t.end - event.payloadOffset) return m_result = Result::InvalidFile;
        if (status == 0xFF && event.metaType == 0x51) {
            uint8_t tempo[3];
            if (event.payloadLength != 3) return m_result = Result::InvalidFile;
            if (!read(tempo, sizeof(tempo), t.end)) return m_result;
            event.tempoUsPerQuarter = (static_cast<uint32_t>(tempo[0]) << 16) |
                                     (static_cast<uint32_t>(tempo[1]) << 8) | tempo[2];
            if (!event.tempoUsPerQuarter) return m_result = Result::InvalidFile;
        }
        if (status == 0xFF && event.metaType == 0x2F) {
            if (event.payloadLength) return m_result = Result::InvalidFile;
            ended = true;
        }
        if (!m_file->seek(event.payloadOffset + event.payloadLength)) return m_result = Result::IoError;
    } else {
        return m_result = Result::InvalidFile;
    }
    t.cursor = m_file->position();
    t.ticks = event.absoluteTicks;
    t.runningStatus = nextRunning;
    t.ended = ended;
    return m_result = Result::Event;
}
}
