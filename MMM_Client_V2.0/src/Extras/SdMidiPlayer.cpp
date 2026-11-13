#include "SdMidiPlayer.h"

#if defined(CFG_EXTRA_SD_MIDI_PLAYER)
#include "Device.h"
#include "Utility/BitManipulation.h"
#include <algorithm>
#include <cstring>

SdMidiPlayer::SdMidiPlayer(Dispatch dispatch, StopOutputs stopOutputs, ReportError reportError, Clock clock)
    : m_dispatch(dispatch), m_stopOutputs(stopOutputs), m_reportError(reportError), m_clock(clock) {}

bool SdMidiPlayer::fail(uint8_t error) {
    m_error = error;
    stop(State::Stopped);
    m_loaded = false;
    m_reader.close();
    m_file.close();
    if (m_reportError) m_reportError(error);
    return false;
}

bool SdMidiPlayer::readerFailure() {
    switch (m_reader.lastResult()) {
        case Reader::Result::IoError: return fail(SysExError::IoError);
        case Reader::Result::UnsupportedFormat: return fail(SysExError::UnsupportedCommand);
        case Reader::Result::CapacityExceeded: return fail(SysExError::CapacityExceeded);
        default: return fail(SysExError::InvalidMidiFile);
    }
}

bool SdMidiPlayer::begin() {
    stop(State::Reset);
    m_reader.close();
    m_file.close();
    m_loaded = m_available = false;
    m_positionUs = m_durationUs = 0;
    m_index = 0;
    m_songs.clear();
    auto& card = Utility::Teensy41SdCardUtility::getInstance();
    if (!card.initialize()) return fail(SysExError::SdUnavailable);
    FsFile directory = card.openRead("/midi");
    if (!directory || !directory.isDirectory()) return fail(SysExError::NoSong);
    FsFile entry;
    while (entry.openNext(&directory, O_RDONLY)) {
        if (!entry.isDirectory()) {
            char name[256];
            const size_t length = entry.getName(name, sizeof(name));
            if (!length) return fail(SysExError::IoError);
            std::string filename(name);
            std::string extension = filename.substr(filename.find_last_of('.') == std::string::npos
                ? filename.size() : filename.find_last_of('.'));
            std::transform(extension.begin(), extension.end(), extension.begin(), [](char c) {
                return c >= 'A' && c <= 'Z' ? static_cast<char>(c + ('a' - 'A')) : c;
            });
            if (extension == ".mid" || extension == ".midi") {
                if (length > MAX_PACKET_LENGTH - SYSEX_HeaderSize - 2)
                    return fail(SysExError::CapacityExceeded);
                for (unsigned char c : filename) {
                    if (c >= 0x80) return fail(SysExError::InvalidPayload);
                }
                if (m_songs.size() == MaxSongs) return fail(SysExError::CapacityExceeded);
                m_songs.push_back(std::move(filename));
            }
        }
        entry.close();
    }
    if (directory.getError()) return fail(SysExError::IoError);
    std::sort(m_songs.begin(), m_songs.end());
    m_available = true;
    m_error = 0;
    return true;
}

void SdMidiPlayer::stop(State state) {
    ++m_generation;
    m_waiting = false;
    m_state = state;
    if (m_stopOutputs) m_stopOutputs();
}

bool SdMidiPlayer::advance(uint16_t track) {
    const auto result = m_reader.next(track, m_pending[track].event);
    m_pending[track].valid = result == Reader::Result::Event;
    if (result == Reader::Result::Event || result == Reader::Result::EndOfTrack) return true;
    return readerFailure();
}

bool SdMidiPlayer::rewind() {
    m_pending = {};
    m_fileSysEx = {};
    m_tick = m_fraction = 0;
    m_timeUs = 0;
    m_tempo = 500000;
    for (uint16_t track = 0; track < m_reader.trackCount(); ++track) {
        if (!m_reader.rewind(track)) return readerFailure();
        if (!advance(track)) return false;
    }
    return true;
}

int SdMidiPlayer::nextTrack() const {
    int selected = -1;
    for (uint16_t track = 0; track < m_reader.trackCount(); ++track) {
        if (!m_pending[track].valid) continue;
        const auto& event = m_pending[track].event;
        if (selected < 0 || event.absoluteTicks < m_pending[selected].event.absoluteTicks ||
            (event.absoluteTicks == m_pending[selected].event.absoluteTicks &&
             event.tempoUsPerQuarter && !m_pending[selected].event.tempoUsPerQuarter)) {
            selected = track;
        }
    }
    return selected;
}

uint64_t SdMidiPlayer::eventTime(const Reader::Event& event) const {
    const uint64_t scaled = static_cast<uint64_t>(event.absoluteTicks - m_tick) * m_tempo + m_fraction;
    return m_timeUs + scaled / m_reader.ticksPerQuarter();
}

bool SdMidiPlayer::processFileSysEx(const Reader::Event& event, bool dispatch) {
    if (event.status == 0xF0) {
        if (m_fileSysEx.length) return fail(SysExError::InvalidMidiFile);
        m_fileSysEx.buffer[0] = 0xF0;
        m_fileSysEx.length = 1;
    } else if (!m_fileSysEx.length) {
        // Standalone F7 escape events are outside the supported SysEx profile.
        return fail(SysExError::UnsupportedCommand);
    }
    if (event.payloadLength > static_cast<uint32_t>(MAX_PACKET_LENGTH - m_fileSysEx.length))
        return fail(SysExError::CapacityExceeded);
    if (!m_file.seek(event.payloadOffset) ||
        m_file.read(m_fileSysEx.buffer.data() + m_fileSysEx.length, event.payloadLength)
            != static_cast<int>(event.payloadLength)) return fail(SysExError::IoError);
    m_fileSysEx.length += static_cast<uint8_t>(event.payloadLength);
    for (uint16_t i = 1; i < m_fileSysEx.length; ++i) {
        if (m_fileSysEx.buffer[i] >= 0x80 &&
            !(i == m_fileSysEx.length - 1 && m_fileSysEx.buffer[i] == 0xF7))
            return fail(SysExError::InvalidMidiFile);
    }
    if (m_fileSysEx.buffer[m_fileSysEx.length - 1] == 0xF7) {
        const MidiMessage message = m_fileSysEx;
        m_fileSysEx = {};
        if (dispatch && m_dispatch) m_dispatch(message);
    }
    return true;
}

bool SdMidiPlayer::consume(uint16_t track, bool dispatch) {
    const auto event = m_pending[track].event;
    const uint64_t scaled = static_cast<uint64_t>(event.absoluteTicks - m_tick) * m_tempo + m_fraction;
    m_timeUs += scaled / m_reader.ticksPerQuarter();
    m_fraction = static_cast<uint32_t>(scaled % m_reader.ticksPerQuarter());
    m_tick = event.absoluteTicks;
    if (m_timeUs / 1000 > MaxPositionMs) return fail(SysExError::CapacityExceeded);
    if (event.tempoUsPerQuarter) m_tempo = event.tempoUsPerQuarter;
    // Advance before dispatch: an embedded playback command can replace the reader.
    if (!advance(track)) return false;
    if (event.status == 0xF0 || event.status == 0xF7) return processFileSysEx(event, dispatch);
    if (event.status < 0xF0 && dispatch && m_dispatch) {
        MidiMessage message;
        message.length = event.dataLength + 1;
        message.buffer[0] = event.status;
        std::copy(event.data, event.data + event.dataLength, message.buffer.begin() + 1);
        m_dispatch(message);
    }
    return true;
}

bool SdMidiPlayer::select(uint16_t index) {
    if (!m_available) { m_error = SysExError::SdUnavailable; return false; }
    if (m_songs.empty()) { m_error = SysExError::NoSong; return false; }
    if (index >= m_songs.size()) { m_error = SysExError::InvalidSongIndex; return false; }
    stop(State::Stopped);
    m_loaded = false;
    m_reader.close();
    const std::string path = "/midi/" + m_songs[index];
    if (!m_file.open(path.c_str())) return fail(SysExError::IoError);
    if (!m_reader.open(m_file)) return readerFailure();
    if (!rewind()) return false;
    uint32_t count = 0;
    for (int track; (track = nextTrack()) >= 0;) {
        if (++count > MaxScanEvents) return fail(SysExError::CapacityExceeded);
        if (!consume(static_cast<uint16_t>(track), false)) return false;
    }
    if (m_fileSysEx.length) return fail(SysExError::InvalidMidiFile);
    m_durationUs = m_timeUs;
    if (!durationMs()) return fail(SysExError::InvalidMidiFile);
    if (!rewind()) return false;
    m_index = index;
    m_positionUs = 0;
    m_loaded = true;
    m_error = 0;
    return true;
}

bool SdMidiPlayer::play(uint32_t nowMs) {
    const uint32_t started = m_clock ? m_clock() : 0;
    if (m_state == State::Playing && !m_waiting) return true;
    if (!m_loaded && !select(m_index)) return false;
    if (m_positionUs >= m_durationUs && !seek(0, nowMs)) return false;
    ++m_generation;
    m_waiting = false;
    m_state = State::Playing;
    m_lastClock = afterPreparation(nowMs, started);
    return true;
}

bool SdMidiPlayer::seek(uint32_t position, uint32_t nowMs) {
    return seekTo(static_cast<uint64_t>(position) * 1000, nowMs);
}

uint32_t SdMidiPlayer::afterPreparation(uint32_t nowMs, uint32_t started) const {
    return m_clock ? nowMs + static_cast<uint32_t>(m_clock() - started) : nowMs;
}

bool SdMidiPlayer::seekTo(uint64_t positionUs, uint32_t nowMs) {
    const uint32_t started = m_clock ? m_clock() : 0;
    const State previous = m_state;
    if (!m_loaded && !select(m_index)) return false;
    stop(previous);
    const uint64_t target = std::min(positionUs, m_durationUs);
    if (!rewind()) return false;
    uint32_t count = 0;
    for (int track; (track = nextTrack()) >= 0 &&
         (target == m_durationUs || eventTime(m_pending[track].event) < target);) {
        if (++count > MaxScanEvents) return fail(SysExError::CapacityExceeded);
        if (!consume(static_cast<uint16_t>(track), false)) return false;
    }
    m_positionUs = target;
    m_lastClock = afterPreparation(nowMs, started);
    return true;
}

void SdMidiPlayer::complete(uint32_t nowMs) {
    const uint32_t started = m_clock ? m_clock() : 0;
    stop(State::Stopped);
    uint16_t next = m_index;
    if (!m_repeat) {
        if (static_cast<size_t>(m_index) + 1 < m_songs.size()) ++next;
        else if (m_loop) next = 0;
        else return;
    }
    if (!select(next)) return;
    if (m_autoPlay) {
        m_waiting = true;
        m_waitStart = afterPreparation(nowMs, started);
    }
}

void SdMidiPlayer::update(uint32_t nowMs) {
    const uint32_t started = m_clock ? m_clock() : 0;
    if (m_waiting) {
        const uint32_t delay = static_cast<uint32_t>(CFG_SD_MIDI_DELAY_SECONDS * 1000);
        if (static_cast<uint32_t>(nowMs - m_waitStart) < delay) return;
        if (!play(nowMs)) return;
        nowMs = m_lastClock;
    }
    if (m_state != State::Playing) return;
    m_positionUs = std::min(m_durationUs,
        m_positionUs + static_cast<uint64_t>(static_cast<uint32_t>(nowMs - m_lastClock)) * 1000);
    m_lastClock = nowMs;
    for (uint16_t count = 0; count < EventsPerUpdate; ++count) {
        const int track = nextTrack();
        if (track < 0) {
            if (m_fileSysEx.length) fail(SysExError::InvalidMidiFile);
            else complete(afterPreparation(nowMs, started));
            return;
        }
        if (eventTime(m_pending[track].event) > m_positionUs) return;
        const uint32_t generation = m_generation;
        if (!consume(static_cast<uint16_t>(track), true)) return;
        if (generation != m_generation) return;
    }
}

MidiMessage SdMidiPlayer::errorResponse(const MidiMessage& request, uint8_t error) const {
    const uint8_t payload[] = {SysEx::ExtraPlayback, error};
    return MidiMessage(Device::GetDeviceID(), request.SourceID(), SysEx::Error, payload, sizeof(payload));
}

std::optional<MidiMessage> SdMidiPlayer::processCommand(const MidiMessage& message, uint32_t nowMs) {
    const uint32_t started = m_clock ? m_clock() : 0;
    const uint32_t generation = m_generation;
    const auto* payload = message.sysExCmdPayload();
    const uint8_t length = message.sysExPayloadLength();
    if (!length) return errorResponse(message, SysExError::InvalidPayload);
    for (uint8_t i = 0; i < length; ++i)
        if (payload[i] & 0x80) return errorResponse(message, SysExError::InvalidPayload);
    const uint8_t cmd = payload[0];
    uint8_t expected = 1;
    if (cmd == MidiPlayerSubCmd::PlaySongAt) expected = 3;
    if (cmd == MidiPlayerSubCmd::SetPosition || cmd == MidiPlayerSubCmd::Navigate) expected = 5;
    const bool flag = cmd == MidiPlayerSubCmd::Repeat || cmd == MidiPlayerSubCmd::AutoPlay ||
                      cmd == MidiPlayerSubCmd::Loop;
    const bool indexedName = cmd == MidiPlayerSubCmd::GetSongName && length == 3;
    if (flag) {
        if (length != 1 && length != 2) return errorResponse(message, SysExError::InvalidPayload);
        if (length == 2 && payload[1] > 1) return errorResponse(message, SysExError::InvalidPayload);
    } else if (!indexedName && length != expected) {
        return errorResponse(message, SysExError::InvalidPayload);
    }
    uint8_t response[MAX_PACKET_LENGTH - SYSEX_HeaderSize - 1] = {cmd};
    uint8_t responseLength = 1;
    const auto number = [&](uint32_t value, bool index) {
        if (index) {
            const auto bytes = Utility::encodeTo7Bit(std::bitset<14>(value));
            std::copy(bytes.begin(), bytes.end(), response + 1);
            responseLength = 3;
        } else {
            const auto bytes = Utility::encodeTo7Bit(std::bitset<28>(value));
            std::copy(bytes.begin(), bytes.end(), response + 1);
            responseLength = 5;
        }
    };
    bool success = true, query = false;
    switch (cmd) {
        case MidiPlayerSubCmd::Reset:
            stop(State::Reset);
            m_index = 0;
            m_positionUs = 0;
            m_loaded = false;
            m_reader.close();
            m_file.close();
            break;
        case MidiPlayerSubCmd::GetState:
            response[1] = static_cast<uint8_t>(m_state);
            responseLength = 2;
            query = true;
            break;
        case MidiPlayerSubCmd::Play: success = play(nowMs); break;
        case MidiPlayerSubCmd::PlaySongAt:
            success = select(static_cast<uint16_t>(Utility::decodeFrom7Bit<14>(payload + 1).to_ulong()));
            if (success) success = play(nowMs);
            break;
        case MidiPlayerSubCmd::Pause:
            if (m_state == State::Playing || m_waiting) {
                if (m_state == State::Playing) {
                    m_positionUs = std::min(m_durationUs, m_positionUs +
                        static_cast<uint64_t>(static_cast<uint32_t>(nowMs - m_lastClock)) * 1000);
                    success = seekTo(m_positionUs, nowMs);
                }
                if (!success) break;
                stop(State::Paused);
            }
            break;
        case MidiPlayerSubCmd::Stop:
            stop(State::Stopped);
            m_positionUs = 0;
            if (m_loaded) success = rewind();
            break;
        case MidiPlayerSubCmd::Next:
        case MidiPlayerSubCmd::Prev: {
            int index = static_cast<int>(m_index) + (cmd == MidiPlayerSubCmd::Next ? 1 : -1);
            if (m_songs.empty()) { m_error = m_available ? SysExError::NoSong : SysExError::SdUnavailable; success = false; }
            else if (index < 0 || index >= static_cast<int>(m_songs.size())) {
                if (!m_loop) { m_error = SysExError::InvalidSongIndex; success = false; }
                else index = index < 0 ? static_cast<int>(m_songs.size()) - 1 : 0;
            }
            if (success) success = select(static_cast<uint16_t>(index)) && play(nowMs);
            break;
        }
        case MidiPlayerSubCmd::Repeat:
        case MidiPlayerSubCmd::AutoPlay:
        case MidiPlayerSubCmd::Loop: {
            bool& value = cmd == MidiPlayerSubCmd::Repeat ? m_repeat :
                          cmd == MidiPlayerSubCmd::AutoPlay ? m_autoPlay : m_loop;
            if (length == 2) {
                value = payload[1];
                if (cmd == MidiPlayerSubCmd::AutoPlay && !value) m_waiting = false;
            } else {
                response[1] = value;
                responseLength = 2;
                query = true;
            }
            break;
        }
        case MidiPlayerSubCmd::GetSongIndex:
            number(m_index, true);
            query = true;
            break;
        case MidiPlayerSubCmd::GetSongName: {
            const uint16_t index = indexedName
                ? static_cast<uint16_t>(Utility::decodeFrom7Bit<14>(payload + 1).to_ulong()) : m_index;
            if (!m_available) return errorResponse(message, SysExError::SdUnavailable);
            if (m_songs.empty()) return errorResponse(message, SysExError::NoSong);
            if (index >= m_songs.size()) return errorResponse(message, SysExError::InvalidSongIndex);
            const auto& name = m_songs[index];
            std::copy(name.begin(), name.end(), response + 1);
            responseLength += static_cast<uint8_t>(name.size());
            query = true;
            break;
        }
        case MidiPlayerSubCmd::GetSongDuration: {
            if (!m_loaded) {
                const State previous = m_state;
                success = select(m_index);
                if (success) m_state = previous;
            }
            if (success) number(durationMs(), false);
            query = true;
            break;
        }
        case MidiPlayerSubCmd::GetPosition:
            if (!m_available) return errorResponse(message, SysExError::SdUnavailable);
            if (m_songs.empty()) return errorResponse(message, SysExError::NoSong);
            number(positionMs(), false);
            query = true;
            break;
        case MidiPlayerSubCmd::SetPosition:
            success = seek(static_cast<uint32_t>(Utility::decodeFrom7Bit<28>(payload + 1).to_ulong()), nowMs);
            break;
        case MidiPlayerSubCmd::Navigate: {
            const uint32_t raw = static_cast<uint32_t>(Utility::decodeFrom7Bit<28>(payload + 1).to_ulong());
            const int64_t delta = raw & 0x08000000 ? static_cast<int64_t>(raw) - 0x10000000 : raw;
            const int64_t target = std::max<int64_t>(0, std::min<int64_t>(MaxPositionMs, positionMs() + delta));
            success = seek(static_cast<uint32_t>(target), nowMs);
            break;
        }
        default: return errorResponse(message, SysExError::UnsupportedCommand);
    }
    if (!success) return errorResponse(message, m_error);
    if (generation != m_generation && m_state == State::Playing)
        m_lastClock = afterPreparation(nowMs, started);
    if (!query) return {};
    return MidiMessage(Device::GetDeviceID(), message.SourceID(), SysEx::ExtraPlayback,
                       response, responseLength);
}
#endif
