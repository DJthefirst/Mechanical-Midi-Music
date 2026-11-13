#include <unity.h>
#include "Utility/MidiFileReaderUtility.h"
#include <algorithm>
#include <cstring>
#include <vector>

using Reader = Utility::MidiFileReaderUtility;

class MemoryFile : public Utility::IReadFile {
public:
    std::vector<uint8_t> bytes;
    uint32_t cursor = 0;
    bool failRead = false;
    int read(void* buffer, size_t length) override {
        if (failRead) return -1;
        length = std::min(length, bytes.size() - cursor);
        std::memcpy(buffer, bytes.data() + cursor, length);
        cursor += static_cast<uint32_t>(length);
        return static_cast<int>(length);
    }
    bool seek(uint32_t offset) override {
        if (offset > bytes.size()) return false;
        cursor = offset;
        return true;
    }
    uint32_t position() const override { return cursor; }
    uint32_t size() const override { return static_cast<uint32_t>(bytes.size()); }
};

MemoryFile midi(std::initializer_list<std::vector<uint8_t>> tracks) {
    MemoryFile file;
    file.bytes = {'M','T','h','d',0,0,0,6,0, static_cast<uint8_t>(tracks.size() > 1),
                  0, static_cast<uint8_t>(tracks.size()), 0,96};
    for (const auto& track : tracks) {
        const uint32_t n = static_cast<uint32_t>(track.size());
        const uint8_t header[] = {'M','T','r','k',static_cast<uint8_t>(n >> 24),
            static_cast<uint8_t>(n >> 16),static_cast<uint8_t>(n >> 8),static_cast<uint8_t>(n)};
        file.bytes.insert(file.bytes.end(), header, header + sizeof(header));
        file.bytes.insert(file.bytes.end(), track.begin(), track.end());
    }
    return file;
}

void setUp() {}
void tearDown() {}

void test_streams_running_status_and_rewinds() {
    auto file = midi({{0,0x90,60,100,0x81,0,61,0,0,0xFF,0x2F,0}});
    Reader reader;
    Reader::Event event;
    TEST_ASSERT_TRUE(reader.open(file));
    TEST_ASSERT_EQUAL_UINT16(96, reader.ticksPerQuarter());
    TEST_ASSERT_EQUAL(Reader::Result::Event, reader.next(0, event));
    TEST_ASSERT_EQUAL_UINT8(60, event.data[0]);
    TEST_ASSERT_EQUAL(Reader::Result::Event, reader.next(0, event));
    TEST_ASSERT_EQUAL_UINT32(128, event.deltaTicks);
    TEST_ASSERT_EQUAL_UINT32(128, event.absoluteTicks);
    TEST_ASSERT_EQUAL_UINT8(0x90, event.status);
    TEST_ASSERT_EQUAL_UINT8(61, event.data[0]);
    TEST_ASSERT_EQUAL_UINT8(0, event.data[1]);
    TEST_ASSERT_EQUAL(Reader::Result::Event, reader.next(0, event));
    TEST_ASSERT_EQUAL_UINT8(0x2F, event.metaType);
    TEST_ASSERT_EQUAL(Reader::Result::EndOfTrack, reader.next(0, event));
    TEST_ASSERT_TRUE(reader.rewind(0));
    TEST_ASSERT_EQUAL(Reader::Result::Event, reader.next(0, event));
    TEST_ASSERT_EQUAL_UINT32(0, event.absoluteTicks);
}

void test_tracks_have_independent_cursors_and_payloads() {
    auto file = midi({{0,0xFF,0x51,3,7,0xA1,0x20,0,0xFF,0x2F,0},
                      {0,0xC2,7,2,0xF0,3,1,2,0xF7,0,0xFF,0x2F,0}});
    Reader reader;
    Reader::Event event;
    TEST_ASSERT_TRUE(reader.open(file));
    TEST_ASSERT_EQUAL_UINT16(1, reader.format());
    TEST_ASSERT_EQUAL(Reader::Result::Event, reader.next(1, event));
    TEST_ASSERT_EQUAL_UINT8(1, event.dataLength);
    TEST_ASSERT_EQUAL_UINT8(7, event.data[0]);
    TEST_ASSERT_EQUAL(Reader::Result::Event, reader.next(0, event));
    TEST_ASSERT_EQUAL_UINT32(500000, event.tempoUsPerQuarter);
    TEST_ASSERT_EQUAL(Reader::Result::Event, reader.next(1, event));
    TEST_ASSERT_EQUAL_UINT32(3, event.payloadLength);
    TEST_ASSERT_EQUAL_UINT8(1, file.bytes[event.payloadOffset]);
    TEST_ASSERT_TRUE(file.seek(event.payloadOffset));
    TEST_ASSERT_EQUAL(Reader::Result::Event, reader.next(1, event));
    TEST_ASSERT_EQUAL_UINT8(0x2F, event.metaType);
}

void test_rejects_bad_headers_and_disables_failed_open() {
    auto file = midi({{0,0xFF,0x2F,0}});
    Reader reader;
    Reader::Event event;
    TEST_ASSERT_TRUE(reader.open(file));
    file.bytes[13] = 0;
    TEST_ASSERT_FALSE(reader.open(file));
    TEST_ASSERT_EQUAL(Reader::Result::InvalidFile, reader.lastResult());
    TEST_ASSERT_EQUAL(Reader::Result::NotOpen, reader.next(0, event));
    file.bytes[12] = 0x80;
    TEST_ASSERT_FALSE(reader.open(file));
    TEST_ASSERT_EQUAL(Reader::Result::UnsupportedFormat, reader.lastResult());
    file.bytes[12] = 0;
    file.bytes[13] = 96;
    file.bytes[9] = 1;
    file.bytes[11] = Reader::MaxTracks + 1;
    TEST_ASSERT_FALSE(reader.open(file));
    TEST_ASSERT_EQUAL(Reader::Result::CapacityExceeded, reader.lastResult());
}

void test_rejects_malformed_events_and_track_bounds() {
    const std::vector<std::vector<uint8_t>> invalid = {
        {0,60,100},
        {0x81,0x80,0x80,0x80,0,0x90,60,100},
        {0,0x90,60,0x80},
        {0,0xF0,5,1},
        {0,0xFF,0x2F,1,0},
        {0,0xFF,0x80,0},
        {0,0xFF,0x51,2,1,2},
        {0,0xFF,0x51,3,0,0,0}
    };
    for (const auto& track : invalid) {
        auto file = midi({track});
        Reader reader;
        Reader::Event event;
        TEST_ASSERT_TRUE(reader.open(file));
        TEST_ASSERT_EQUAL(Reader::Result::InvalidFile, reader.next(0, event));
    }
    auto file = midi({{0,0x90,60,100}});
    Reader reader;
    Reader::Event event;
    TEST_ASSERT_TRUE(reader.open(file));
    TEST_ASSERT_EQUAL(Reader::Result::Event, reader.next(0, event));
    TEST_ASSERT_EQUAL(Reader::Result::InvalidFile, reader.next(0, event));
}

void test_meta_clears_running_status_and_io_errors_surface() {
    auto file = midi({{0,0x90,60,100,0,0xFF,1,0,0,61,100}});
    Reader reader;
    Reader::Event event;
    TEST_ASSERT_TRUE(reader.open(file));
    TEST_ASSERT_EQUAL(Reader::Result::Event, reader.next(0, event));
    TEST_ASSERT_EQUAL(Reader::Result::Event, reader.next(0, event));
    TEST_ASSERT_EQUAL(Reader::Result::InvalidFile, reader.next(0, event));
    TEST_ASSERT_TRUE(reader.rewind(0));
    file.failRead = true;
    TEST_ASSERT_EQUAL(Reader::Result::IoError, reader.next(0, event));
}

void test_tick_overflow_fails_without_advancing_cursor() {
    std::vector<uint8_t> track;
    for (int i = 0; i < 17; ++i) {
        const uint8_t event[] = {0xFF,0xFF,0xFF,0x7F,0xC0,1};
        track.insert(track.end(), event, event + sizeof(event));
    }
    auto file = midi({track});
    Reader reader;
    Reader::Event event;
    TEST_ASSERT_TRUE(reader.open(file));
    for (int i = 0; i < 16; ++i)
        TEST_ASSERT_EQUAL(Reader::Result::Event, reader.next(0, event));
    TEST_ASSERT_EQUAL(Reader::Result::InvalidFile, reader.next(0, event));
    TEST_ASSERT_EQUAL(Reader::Result::InvalidFile, reader.next(0, event));
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_streams_running_status_and_rewinds);
    RUN_TEST(test_tracks_have_independent_cursors_and_payloads);
    RUN_TEST(test_rejects_bad_headers_and_disables_failed_open);
    RUN_TEST(test_rejects_malformed_events_and_track_bounds);
    RUN_TEST(test_meta_clears_running_status_and_io_errors_surface);
    RUN_TEST(test_tick_overflow_fails_without_advancing_cursor);
    return UNITY_END();
}
