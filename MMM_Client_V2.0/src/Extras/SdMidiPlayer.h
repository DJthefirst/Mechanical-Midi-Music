#pragma once

#include "Config.h"

#if defined(CFG_EXTRA_SD_MIDI_PLAYER)
#include "MsgHandling/MidiMessage.h"
#include "Utility/MidiFileReaderUtility.h"
#include "Utility/SdCard/Teensy41SdCardUtility.h"
#include <optional>
#include <string>
#include <vector>

class SdMidiPlayer {
public:
    enum class State : uint8_t { Reset = 0, Stopped = 1, Playing = 2, Paused = 3 };
    static constexpr uint16_t MaxSongs = 256;
    static constexpr uint32_t MaxPositionMs = 0x0FFFFFFF;
    static constexpr uint32_t MaxScanEvents = 1000000;
    static constexpr uint16_t EventsPerUpdate = 128;
    using Dispatch = void (*)(const MidiMessage&);
    using StopOutputs = void (*)();
    using ReportError = void (*)(uint8_t);
    using Clock = uint32_t (*)();

    SdMidiPlayer(Dispatch dispatch, StopOutputs stopOutputs, ReportError reportError, Clock clock = nullptr);
    bool begin();
    void update(uint32_t nowMs);
    std::optional<MidiMessage> processCommand(const MidiMessage& message, uint32_t nowMs);
    State state() const { return m_state; }
    uint16_t songIndex() const { return m_index; }
    uint16_t songCount() const { return static_cast<uint16_t>(m_songs.size()); }
    uint32_t durationMs() const { return static_cast<uint32_t>(m_durationUs / 1000); }
    uint32_t positionMs() const { return static_cast<uint32_t>(m_positionUs / 1000); }
    uint8_t lastError() const { return m_error; }

private:
    using Reader = Utility::MidiFileReaderUtility;
    struct Pending {
        Reader::Event event{};
        bool valid = false;
    };
    bool fail(uint8_t error);
    bool readerFailure();
    bool select(uint16_t index);
    bool rewind();
    bool advance(uint16_t track);
    int nextTrack() const;
    uint64_t eventTime(const Reader::Event& event) const;
    bool consume(uint16_t track, bool dispatch);
    bool processFileSysEx(const Reader::Event& event, bool dispatch);
    bool play(uint32_t nowMs);
    bool seek(uint32_t position, uint32_t nowMs);
    bool seekTo(uint64_t positionUs, uint32_t nowMs);
    uint32_t afterPreparation(uint32_t nowMs, uint32_t started) const;
    void stop(State state);
    void complete(uint32_t nowMs);
    MidiMessage errorResponse(const MidiMessage& request, uint8_t error) const;

    Utility::Teensy41SdReadFile m_file;
    Reader m_reader;
    std::array<Pending, Reader::MaxTracks> m_pending{};
    MidiMessage m_fileSysEx;
    std::vector<std::string> m_songs;
    Dispatch m_dispatch;
    StopOutputs m_stopOutputs;
    ReportError m_reportError;
    Clock m_clock;
    State m_state = State::Reset;
    uint16_t m_index = 0;
    uint8_t m_error = 0;
    bool m_available = false, m_loaded = false;
    bool m_repeat = false, m_autoPlay = CFG_SD_MIDI_AUTOPLAY, m_loop = CFG_SD_MIDI_LOOP;
    bool m_waiting = false;
    uint32_t m_lastClock = 0, m_waitStart = 0, m_generation = 0;
    uint32_t m_tick = 0, m_tempo = 500000, m_fraction = 0;
    uint64_t m_timeUs = 0, m_durationUs = 0, m_positionUs = 0;
};
#endif
