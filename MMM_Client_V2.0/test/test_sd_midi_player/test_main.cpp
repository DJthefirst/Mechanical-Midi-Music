#define ARDUINO_TEENSY41
#define CFG_EXTRA_SD_MIDI_PLAYER
#define CFG_SD_MIDI_DELAY_SECONDS 2
#include <unity.h>
#include "Utility/SdCard/Teensy41SdCardUtility.cpp"
#include "Extras/SdMidiPlayer.cpp"

static std::vector<MidiMessage> dispatched;
static std::vector<uint8_t> errors;
static uint32_t stops;
static SdMidiPlayer* activePlayer;
static bool stopFromFile;

static void dispatch(const MidiMessage& message) {
    dispatched.push_back(message);
    if (stopFromFile && message.buffer[0] == 0xF0) {
        const uint8_t payload[] = {MidiPlayerSubCmd::Stop};
        activePlayer->processCommand(MidiMessage(1, 2, SysEx::ExtraPlayback, payload, 1), 0);
    }
}
static void stopOutputs() { ++stops; }
static void report(uint8_t error) { errors.push_back(error); }
static uint32_t clockMs() { return TestSd::clock; }

static std::vector<uint8_t> midi(std::initializer_list<std::vector<uint8_t>> tracks) {
    std::vector<uint8_t> bytes = {'M','T','h','d',0,0,0,6,0,
        static_cast<uint8_t>(tracks.size() > 1),0,static_cast<uint8_t>(tracks.size()),0,96};
    for (const auto& track : tracks) {
        const uint32_t n = static_cast<uint32_t>(track.size());
        const uint8_t header[] = {'M','T','r','k',static_cast<uint8_t>(n >> 24),
            static_cast<uint8_t>(n >> 16),static_cast<uint8_t>(n >> 8),static_cast<uint8_t>(n)};
        bytes.insert(bytes.end(), header, header + sizeof(header));
        bytes.insert(bytes.end(), track.begin(), track.end());
    }
    return bytes;
}
static void addSong(const char* name, const std::vector<uint8_t>& bytes) {
    TestSd::files[std::string("/midi/") + name] = std::make_shared<std::vector<uint8_t>>(bytes);
}
static std::optional<MidiMessage> command(SdMidiPlayer& player, std::initializer_list<uint8_t> payload,
                                         uint32_t now = 0) {
    const std::vector<uint8_t> data(payload);
    return player.processCommand(MidiMessage(123, 42, SysEx::ExtraPlayback,
        data.data(), static_cast<uint8_t>(data.size())), now);
}
static void assertError(const std::optional<MidiMessage>& response, uint8_t error) {
    TEST_ASSERT_TRUE(response.has_value());
    TEST_ASSERT_EQUAL_UINT8(SysEx::Error, response->sysExCommand());
    TEST_ASSERT_EQUAL_UINT8(error, response->sysExCmdPayload()[1]);
}

void setUp() {
    TestSd::reset();
    TestSd::directories.insert("/midi");
    dispatched.clear();
    errors.clear();
    stops = 0;
    activePlayer = nullptr;
    stopFromFile = false;
    Device::SetDeviceID(42);
    const auto song = midi({{0,0x90,60,100,96,0x80,60,0,0,0xFF,0x2F,0}});
    addSong("b.midi", song);
    addSong("a.MID", song);
}
void tearDown() {}

void test_catalog_queries_and_protocol_limits() {
    SdMidiPlayer player(dispatch, stopOutputs, report);
    TEST_ASSERT_TRUE(player.begin());
    TEST_ASSERT_EQUAL_UINT16(2, player.songCount());
    auto name = command(player, {MidiPlayerSubCmd::GetSongName});
    TEST_ASSERT_EQUAL_UINT16(42, name->SourceID());
    TEST_ASSERT_EQUAL_UINT16(123, name->DestinationID());
    TEST_ASSERT_EQUAL_UINT8(6, name->sysExPayloadLength());
    TEST_ASSERT_EQUAL_MEMORY("a.MID", name->sysExCmdPayload() + 1, 5);
    auto duration = command(player, {MidiPlayerSubCmd::GetSongDuration});
    TEST_ASSERT_EQUAL_UINT8(5, duration->sysExPayloadLength());
    TEST_ASSERT_EQUAL_UINT32(500, Utility::decodeFrom7Bit<28>(duration->sysExCmdPayload() + 1).to_ulong());
    TEST_ASSERT_EQUAL(SdMidiPlayer::State::Reset, player.state());
    TEST_ASSERT_EQUAL_UINT8(1, command(player, {MidiPlayerSubCmd::AutoPlay})->sysExCmdPayload()[1]);
    TEST_ASSERT_EQUAL_UINT8(1, command(player, {MidiPlayerSubCmd::Loop})->sysExCmdPayload()[1]);
    TEST_ASSERT_EQUAL_UINT8(0, command(player, {MidiPlayerSubCmd::Repeat})->sysExCmdPayload()[1]);
    command(player, {MidiPlayerSubCmd::Play});
    assertError(command(player, {MidiPlayerSubCmd::PlaySongAt,0x7F,0x7F}), SysExError::InvalidSongIndex);
    TEST_ASSERT_EQUAL(SdMidiPlayer::State::Playing, player.state());
    assertError(command(player, {MidiPlayerSubCmd::AutoPlay,2}), SysExError::InvalidPayload);
    assertError(command(player, {MidiPlayerSubCmd::Navigate,0,0}), SysExError::InvalidPayload);
    assertError(command(player, {MidiPlayerSubCmd::Play,0x80}), SysExError::InvalidPayload);
    assertError(command(player, {0x06}), SysExError::UnsupportedCommand);
    assertError(command(player, {}), SysExError::InvalidPayload);
}

void test_tempo_merge_fractional_time_and_eot_duration() {
    TestSd::files.clear();
    addSong("tempo.mid", midi({
        {96,0xFF,0x51,3,3,0xD0,0x90,96,0xFF,0x2F,0},
        {0,0x90,60,100,96,0x80,60,0,96,0xFF,0x2F,0}}));
    SdMidiPlayer player(dispatch, stopOutputs, report);
    TEST_ASSERT_TRUE(player.begin());
    command(player, {MidiPlayerSubCmd::AutoPlay,0});
    command(player, {MidiPlayerSubCmd::Loop,0});
    command(player, {MidiPlayerSubCmd::Play});
    TEST_ASSERT_EQUAL_UINT32(750, player.durationMs());
    player.update(0);
    player.update(499);
    TEST_ASSERT_EQUAL_UINT32(1, dispatched.size());
    player.update(500);
    TEST_ASSERT_EQUAL_UINT32(2, dispatched.size());
    player.update(749);
    TEST_ASSERT_EQUAL(SdMidiPlayer::State::Playing, player.state());
    player.update(750);
    TEST_ASSERT_EQUAL(SdMidiPlayer::State::Stopped, player.state());
    TEST_ASSERT_EQUAL_UINT32(750, player.positionMs());
    TestSd::files.clear();
    addSong("fraction.mid", midi({{1,0xC0,1,1,0xC0,2,1,0xFF,0x2F,0}}));
    TEST_ASSERT_TRUE(player.begin());
    command(player, {MidiPlayerSubCmd::Play});
    TEST_ASSERT_EQUAL_UINT32(15, player.durationMs());
    player.update(5);
    TEST_ASSERT_TRUE(dispatched.size() == 2);
    player.update(6);
    TEST_ASSERT_EQUAL_UINT32(3, dispatched.size());
}

void test_pause_resume_stop_reset_seek_and_clock_wrap() {
    SdMidiPlayer player(dispatch, stopOutputs, report);
    TEST_ASSERT_TRUE(player.begin());
    command(player, {MidiPlayerSubCmd::AutoPlay,0});
    command(player, {MidiPlayerSubCmd::Loop,0});
    command(player, {MidiPlayerSubCmd::Play}, 0xFFFFFF00);
    player.update(0xFFFFFF00);
    command(player, {MidiPlayerSubCmd::Pause}, 0xFFFFFF64);
    TEST_ASSERT_EQUAL_UINT32(100, player.positionMs());
    const auto beforePause = dispatched.size();
    player.update(1000);
    TEST_ASSERT_EQUAL_UINT32(beforePause, dispatched.size());
    command(player, {MidiPlayerSubCmd::Play}, 1000);
    player.update(1399);
    TEST_ASSERT_EQUAL(SdMidiPlayer::State::Playing, player.state());
    player.update(1400);
    TEST_ASSERT_EQUAL(SdMidiPlayer::State::Stopped, player.state());
    command(player, {MidiPlayerSubCmd::SetPosition,0x7F,0x7F,0x7F,0x7F});
    TEST_ASSERT_EQUAL_UINT32(500, player.positionMs());
    command(player, {MidiPlayerSubCmd::Navigate,0x40,0,0,0});
    TEST_ASSERT_EQUAL_UINT32(0, player.positionMs());
    command(player, {MidiPlayerSubCmd::Stop});
    TEST_ASSERT_EQUAL_UINT32(0, player.positionMs());
    command(player, {MidiPlayerSubCmd::PlaySongAt,0,1});
    TEST_ASSERT_EQUAL_UINT16(1, player.songIndex());
    command(player, {MidiPlayerSubCmd::Reset});
    TEST_ASSERT_EQUAL_UINT16(0, player.songIndex());
    TEST_ASSERT_EQUAL(SdMidiPlayer::State::Reset, player.state());
    TEST_ASSERT_EQUAL_UINT8(0, command(player, {MidiPlayerSubCmd::Loop})->sysExCmdPayload()[1]);
    TEST_ASSERT_GREATER_THAN_UINT32(0, stops);
    command(player, {MidiPlayerSubCmd::PlaySongAt,0,1}, 0xFFFFFF00);
    player.update(244);
    TEST_ASSERT_EQUAL_UINT32(500, player.positionMs());
}

void test_completion_flags_delay_and_explicit_navigation() {
    SdMidiPlayer player(dispatch, stopOutputs, report);
    TEST_ASSERT_TRUE(player.begin());
    command(player, {MidiPlayerSubCmd::Play});
    player.update(500);
    TEST_ASSERT_EQUAL_UINT16(1, player.songIndex());
    TEST_ASSERT_EQUAL(SdMidiPlayer::State::Stopped, player.state());
    player.update(2499);
    TEST_ASSERT_EQUAL(SdMidiPlayer::State::Stopped, player.state());
    player.update(2500);
    TEST_ASSERT_EQUAL(SdMidiPlayer::State::Playing, player.state());
    command(player, {MidiPlayerSubCmd::Repeat,1});
    player.update(3000);
    TEST_ASSERT_EQUAL_UINT16(1, player.songIndex());
    command(player, {MidiPlayerSubCmd::AutoPlay,0});
    player.update(5000);
    TEST_ASSERT_EQUAL(SdMidiPlayer::State::Stopped, player.state());
    command(player, {MidiPlayerSubCmd::Next}, 5000);
    TEST_ASSERT_EQUAL_UINT16(0, player.songIndex());
    TEST_ASSERT_EQUAL(SdMidiPlayer::State::Playing, player.state());
    command(player, {MidiPlayerSubCmd::Loop,0});
    assertError(command(player, {MidiPlayerSubCmd::Prev}), SysExError::InvalidSongIndex);
    TEST_ASSERT_EQUAL(SdMidiPlayer::State::Playing, player.state());
    command(player, {MidiPlayerSubCmd::Repeat,0});
    player.update(5500);
    TEST_ASSERT_EQUAL_UINT16(1, player.songIndex());
    TEST_ASSERT_EQUAL_UINT32(0, player.positionMs());
    TEST_ASSERT_EQUAL(SdMidiPlayer::State::Stopped, player.state());
}

void test_file_sysex_preserved_fragmented_and_reentrant_stop_safe() {
    TestSd::files.clear();
    addSong("config.mid", midi({{0,0xF0,4,0x7D,0,1,0,
        0,0xF7,4,42,0x23,'X',0xF7,0,0x90,60,100,96,0xFF,0x2F,0}}));
    SdMidiPlayer player(dispatch, stopOutputs, report);
    activePlayer = &player;
    TEST_ASSERT_TRUE(player.begin());
    command(player, {MidiPlayerSubCmd::Play});
    TEST_ASSERT_TRUE(dispatched.empty());
    stopFromFile = true;
    player.update(0);
    TEST_ASSERT_EQUAL_UINT32(1, dispatched.size());
    TEST_ASSERT_TRUE(dispatched[0].isCompleteSysEx());
    TEST_ASSERT_EQUAL_UINT8(SysEx::DeviceName, dispatched[0].sysExCommand());
    TEST_ASSERT_EQUAL_UINT8('X', dispatched[0].sysExCmdPayload()[0]);
    TEST_ASSERT_EQUAL(SdMidiPlayer::State::Stopped, player.state());
    stopFromFile = false;
    command(player, {MidiPlayerSubCmd::SetPosition,0,0,0,100});
    command(player, {MidiPlayerSubCmd::Play});
    player.update(0);
    TEST_ASSERT_EQUAL_UINT32(1, dispatched.size());
}

void test_malformed_io_capacity_and_empty_catalog_errors() {
    TestSd::files.clear();
    addSong("broken.mid", midi({{0,0x90,60,100}}));
    SdMidiPlayer player(dispatch, stopOutputs, report);
    TEST_ASSERT_TRUE(player.begin());
    assertError(command(player, {MidiPlayerSubCmd::Play}), SysExError::InvalidMidiFile);
    TEST_ASSERT_EQUAL(SdMidiPlayer::State::Stopped, player.state());
    TEST_ASSERT_FALSE(errors.empty());
    TestSd::files.clear();
    TEST_ASSERT_TRUE(player.begin());
    assertError(command(player, {MidiPlayerSubCmd::Play}), SysExError::NoSong);
    TestSd::directories.clear();
    TEST_ASSERT_FALSE(player.begin());
    TestSd::directories.insert("/midi");
    std::vector<uint8_t> track = {0,0xF0,0x81,0};
    track.insert(track.end(), 127, 1);
    track.push_back(0xF7);
    track.insert(track.end(), {96,0xFF,0x2F,0});
    addSong("large.mid", midi({track}));
    TEST_ASSERT_TRUE(player.begin());
    assertError(command(player, {MidiPlayerSubCmd::Play}), SysExError::CapacityExceeded);
    TestSd::files.clear();
    addSong("ok.mid", midi({{0,0x90,60,100,96,0xFF,0x2F,0}}));
    TEST_ASSERT_TRUE(player.begin());
    command(player, {MidiPlayerSubCmd::Play});
    TestSd::readFails = true;
    player.update(500);
    TEST_ASSERT_EQUAL_UINT8(SysExError::IoError, player.lastError());
    TEST_ASSERT_EQUAL(SdMidiPlayer::State::Stopped, player.state());
}

void test_playlist_and_update_budget_are_bounded() {
    TestSd::files.clear();
    for (uint16_t i = 0; i <= SdMidiPlayer::MaxSongs; ++i) {
        const auto name = std::to_string(i) + ".mid";
        addSong(name.c_str(), midi({{96,0xFF,0x2F,0}}));
    }
    SdMidiPlayer player(dispatch, stopOutputs, report);
    TEST_ASSERT_FALSE(player.begin());
    TEST_ASSERT_EQUAL_UINT8(SysExError::CapacityExceeded, player.lastError());
    TestSd::files.clear();
    std::vector<uint8_t> track;
    for (int i = 0; i < 300; ++i) track.insert(track.end(), {0,0xC0,1});
    track.insert(track.end(), {96,0xFF,0x2F,0});
    addSong("dense.mid", midi({track}));
    TEST_ASSERT_TRUE(player.begin());
    command(player, {MidiPlayerSubCmd::Play});
    player.update(0);
    TEST_ASSERT_EQUAL_UINT32(SdMidiPlayer::EventsPerUpdate, dispatched.size());
    player.update(0);
    TEST_ASSERT_EQUAL_UINT32(2 * SdMidiPlayer::EventsPerUpdate, dispatched.size());
    player.update(0);
    TEST_ASSERT_EQUAL_UINT32(300, dispatched.size());
}

void test_preparation_latency_is_not_playback_time_or_inter_song_delay() {
    SdMidiPlayer player(dispatch, stopOutputs, report, clockMs);
    TestSd::readDelayMs = 5;
    TEST_ASSERT_TRUE(player.begin());
    command(player, {MidiPlayerSubCmd::PlaySongAt,0,0}, TestSd::clock);
    TEST_ASSERT_GREATER_THAN_UINT32(0, TestSd::clock);
    player.update(TestSd::clock);
    TEST_ASSERT_EQUAL_UINT32(0, player.positionMs());
    TEST_ASSERT_EQUAL_UINT32(1, dispatched.size());
    const uint32_t start = TestSd::clock;
    player.update(start + 500);
    TEST_ASSERT_EQUAL_UINT16(1, player.songIndex());
    const uint32_t ready = TestSd::clock + 500;
    player.update(ready + 1999);
    TEST_ASSERT_EQUAL(SdMidiPlayer::State::Stopped, player.state());
    player.update(ready + 2000);
    TEST_ASSERT_EQUAL(SdMidiPlayer::State::Playing, player.state());
    TEST_ASSERT_EQUAL_UINT32(0, player.positionMs());
}

void test_exact_duration_and_filename_limits() {
    TestSd::files.clear();
    auto bytes = midi({{0,0xFF,0x51,3,0,3,0xE8,0xFF,0xFF,0xFF,0x7F,0xFF,0x2F,0}});
    bytes[13] = 1;
    const std::string longest = std::string(115, 'a') + ".mid";
    addSong(longest.c_str(), bytes);
    addSong((std::string(150, 'z') + ".txt").c_str(), {});
    SdMidiPlayer player(dispatch, stopOutputs, report);
    TEST_ASSERT_TRUE(player.begin());
    command(player, {MidiPlayerSubCmd::Play});
    TEST_ASSERT_EQUAL_UINT32(SdMidiPlayer::MaxPositionMs, player.durationMs());
    auto name = command(player, {MidiPlayerSubCmd::GetSongName});
    TEST_ASSERT_EQUAL_UINT8(128, name->length);
    TestSd::files.clear();
    addSong((std::string(116, 'a') + ".mid").c_str(), bytes);
    TEST_ASSERT_FALSE(player.begin());
    TEST_ASSERT_EQUAL_UINT8(SysExError::CapacityExceeded, player.lastError());
    TestSd::files.clear();
    addSong("too-long.mid", midi({{0xFF,0xFF,0xFF,0x7F,0xFF,0x2F,0}}));
    TEST_ASSERT_TRUE(player.begin());
    assertError(command(player, {MidiPlayerSubCmd::Play}), SysExError::CapacityExceeded);
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_catalog_queries_and_protocol_limits);
    RUN_TEST(test_tempo_merge_fractional_time_and_eot_duration);
    RUN_TEST(test_pause_resume_stop_reset_seek_and_clock_wrap);
    RUN_TEST(test_completion_flags_delay_and_explicit_navigation);
    RUN_TEST(test_file_sysex_preserved_fragmented_and_reentrant_stop_safe);
    RUN_TEST(test_malformed_io_capacity_and_empty_catalog_errors);
    RUN_TEST(test_playlist_and_update_budget_are_bounded);
    RUN_TEST(test_preparation_latency_is_not_playback_time_or_inter_song_delay);
    RUN_TEST(test_exact_duration_and_filename_limits);
    return UNITY_END();
}
