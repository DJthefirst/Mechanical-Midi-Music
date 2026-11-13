#include "Teensy41SDLocalStorage.h"

#if defined(PLATFORM_TEENSY41)
#include "Device.h"
#include "Distributors/DistributorManager.h"
#include "Utility/Crc8.h"
#include <algorithm>
#include <cstring>

namespace {
// Reserved files only; reset and commits never remove MIDI files.
constexpr const char* SlotPaths[] = {"/mmm-config-0.bin", "/mmm-config-1.bin"};
}

bool Teensy41SDLocalStorage::error(Error value) {
    m_error = value;
    ++m_errorCount;
    return false;
}

void Teensy41SDLocalStorage::defaults() {
    m_data.fill(0);
    std::fill_n(m_data.begin(), DEVICE_NUM_NAME_BYTES, 0x20);
    const uint16_t id = Device::GetDeviceID();
    m_data[IdOffset] = (id >> 7) & 0x7F;
    m_data[IdOffset + 1] = id & 0x7F;
}

bool Teensy41SDLocalStorage::inspectSlot(uint8_t slot, uint32_t& generation) {
    auto file = Utility::Teensy41SdCardUtility::getInstance().openRead(SlotPaths[slot]);
    if (!file) return error(Error::ReadFailed);
    uint8_t header[HeaderBytes];
    if (file.fileSize() != HeaderBytes + SnapshotBytes) return error(Error::CorruptData);
    if (file.read(header, sizeof(header)) != sizeof(header)) return error(Error::ReadFailed);
    if (std::memcmp(header, "MMMS", 4) || header[4] != 1 ||
        ((static_cast<size_t>(header[9]) << 8) | header[10]) != SnapshotBytes ||
        Utility::crc8(header, 11) != header[11]) {
        return error(Error::CorruptData);
    }
    uint8_t buffer[64], crc = 0;
    size_t remaining = SnapshotBytes;
    while (remaining) {
        const size_t length = std::min(remaining, sizeof(buffer));
        if (file.read(buffer, length) != static_cast<int>(length)) return error(Error::ReadFailed);
        if (remaining == SnapshotBytes && buffer[CountOffset] > MAX_DISTRIBUTORS)
            return error(Error::CorruptData);
        crc = Utility::crc8(buffer, length, crc);
        remaining -= length;
    }
    if (crc != header[12]) return error(Error::CorruptData);
    generation = (static_cast<uint32_t>(header[5]) << 24) |
                 (static_cast<uint32_t>(header[6]) << 16) |
                 (static_cast<uint32_t>(header[7]) << 8) | header[8];
    return true;
}

bool Teensy41SDLocalStorage::loadSlot(uint8_t slot) {
    auto file = Utility::Teensy41SdCardUtility::getInstance().openRead(SlotPaths[slot]);
    uint8_t header[HeaderBytes];
    if (!file || file.read(header, sizeof(header)) != sizeof(header) ||
        file.read(m_data.data(), m_data.size()) != static_cast<int>(m_data.size())) {
        m_initialized = false;
        return error(Error::ReadFailed);
    }
    if (Utility::crc8(m_data.data(), m_data.size()) != header[12] ||
        m_data[CountOffset] > MAX_DISTRIBUTORS) {
        m_initialized = false;
        return error(Error::CorruptData);
    }
    return true;
}

bool Teensy41SDLocalStorage::initialize() {
    if (m_initialized) return true;
    auto& card = Utility::Teensy41SdCardUtility::getInstance();
    if (!card.initialize()) return error(Error::CardUnavailable);
    const bool exists0 = SD.exists(SlotPaths[0]), exists1 = SD.exists(SlotPaths[1]);
    uint32_t generation0 = 0, generation1 = 0;
    const bool valid0 = exists0 && inspectSlot(0, generation0);
    const bool valid1 = exists1 && inspectSlot(1, generation1);
    if (valid0 || valid1) {
        // Unsigned distance compares wrapping generations without signed overflow.
        const uint32_t distance = generation1 - generation0;
        m_activeSlot = valid1 && (!valid0 || (distance && distance < 0x80000000UL)) ? 1 : 0;
        m_generation = m_activeSlot == 0 ? generation0 : generation1;
        if (!loadSlot(static_cast<uint8_t>(m_activeSlot))) return false;
        m_initialized = true;
        return true;
    }
    // Like the EEPROM backend's magic/schema check: unreadable config falls back to defaults.
    if (exists0 || exists1) error(Error::CorruptData);
    defaults();
    m_activeSlot = -1;
    m_generation = 0;
    m_initialized = true;
    if (!commit()) {
        m_initialized = false;
        return false;
    }
    return true;
}

bool Teensy41SDLocalStorage::commit() {
    const uint8_t slot = m_activeSlot == 0 ? 1 : 0;
    const uint32_t generation = m_generation + 1;
    uint8_t header[HeaderBytes] = {'M', 'M', 'M', 'S', 1};
    header[5] = generation >> 24;
    header[6] = generation >> 16;
    header[7] = generation >> 8;
    header[8] = generation;
    header[9] = SnapshotBytes >> 8;
    header[10] = SnapshotBytes & 0xFF;
    header[11] = Utility::crc8(header, 11);
    header[12] = Utility::crc8(m_data.data(), m_data.size());
    auto file = Utility::Teensy41SdCardUtility::getInstance().openWrite(SlotPaths[slot]);
    bool success = file && file.write(header, sizeof(header)) == sizeof(header) &&
                   file.write(m_data.data(), m_data.size()) == m_data.size() &&
                   file.sync() && !file.getWriteError();
    if (file && !file.close()) success = false;
    if (!success) {
        // Keep getters consistent with the last durable configuration on write failure.
        if (m_activeSlot >= 0) loadSlot(static_cast<uint8_t>(m_activeSlot));
        return error(Error::WriteFailed);
    }
    m_activeSlot = slot;
    m_generation = generation;
    m_error = Error::None;
    return true;
}

void Teensy41SDLocalStorage::update(size_t offset, const uint8_t* data, size_t length) {
    if (m_restoring) return;
    if (!m_initialized) { error(Error::CardUnavailable); return; }
    if (!std::memcmp(m_data.data() + offset, data, length)) return;
    std::memcpy(m_data.data() + offset, data, length);
    commit();
}

uint16_t Teensy41SDLocalStorage::getWord(size_t offset) const {
    return (static_cast<uint16_t>(m_data[offset]) << 7) | m_data[offset + 1];
}

void Teensy41SDLocalStorage::setWord(size_t offset, uint16_t value) {
    const uint8_t bytes[] = {static_cast<uint8_t>((value >> 7) & 0x7F), static_cast<uint8_t>(value & 0x7F)};
    update(offset, bytes, sizeof(bytes));
}

void Teensy41SDLocalStorage::resetDeviceConfig() {
    if (!m_initialized) { error(Error::CardUnavailable); return; }
    defaults();
    commit();
}

std::string Teensy41SDLocalStorage::getDeviceName() {
    if (!m_initialized) { error(Error::CardUnavailable); return Device::Name; }
    size_t length = DEVICE_NUM_NAME_BYTES;
    while (length && (m_data[length - 1] == 0x20 || m_data[length - 1] == 0)) --length;
    return length ? std::string(reinterpret_cast<const char*>(m_data.data()), length) : Device::Name;
}

void Teensy41SDLocalStorage::setDeviceName(const std::string& name) {
    uint8_t bytes[DEVICE_NUM_NAME_BYTES];
    std::fill_n(bytes, sizeof(bytes), 0x20);
    std::memcpy(bytes, name.data(), std::min(name.size(), sizeof(bytes)));
    update(NameOffset, bytes, sizeof(bytes));
}

uint16_t Teensy41SDLocalStorage::getDeviceID() {
    if (!m_initialized) { error(Error::CardUnavailable); return Device::GetDeviceID(); }
    return getWord(IdOffset);
}

void Teensy41SDLocalStorage::setDeviceID(uint16_t id) { setWord(IdOffset, id); }

uint16_t Teensy41SDLocalStorage::getDeviceBoolean() {
    if (!m_initialized) { error(Error::CardUnavailable); return 0; }
    return getWord(FlagsOffset);
}

void Teensy41SDLocalStorage::setDeviceBoolean(uint16_t flags) { setWord(FlagsOffset, flags); }

uint8_t Teensy41SDLocalStorage::getNumOfDistributors() {
    if (!m_initialized) { error(Error::CardUnavailable); return 0; }
    return m_data[CountOffset];
}

void Teensy41SDLocalStorage::setNumOfDistributors(uint8_t count) {
    count = std::min(count, MAX_DISTRIBUTORS);
    update(CountOffset, &count, 1);
}

bool Teensy41SDLocalStorage::getDistributorConstruct(uint16_t index, uint8_t* construct) {
    if (!m_initialized) return error(Error::CardUnavailable);
    if (!construct || index >= MAX_DISTRIBUTORS) return error(Error::InvalidArgument);
    const auto* record = m_data.data() + RecordsOffset + index * RecordBytes;
    if (Utility::crc8(record, DISTRIBUTOR_NUM_CFG_BYTES) != record[DISTRIBUTOR_NUM_CFG_BYTES])
        return error(Error::CorruptData);
    std::memcpy(construct, record, DISTRIBUTOR_NUM_CFG_BYTES);
    return true;
}

void Teensy41SDLocalStorage::setDistributorConstruct(uint16_t index, const uint8_t* construct) {
    if (!construct || index >= MAX_DISTRIBUTORS) { error(Error::InvalidArgument); return; }
    uint8_t record[RecordBytes];
    std::memcpy(record, construct, DISTRIBUTOR_NUM_CFG_BYTES);
    record[DISTRIBUTOR_NUM_CFG_BYTES] = Utility::crc8(construct, DISTRIBUTOR_NUM_CFG_BYTES);
    update(RecordsOffset + index * RecordBytes, record, sizeof(record));
}

bool Teensy41SDLocalStorage::initializeDeviceConfiguration(DistributorManager& manager) {
    if (!m_initialized) return error(Error::CardUnavailable);
    Device::Name = getDeviceName();
    Device::SetDeviceID(getDeviceID());
    const uint16_t flags = getDeviceBoolean();
    Device::Muted = (flags & DEVICE_BOOL_MASK::MUTED) != 0;
    Device::OmniMode = (flags & DEVICE_BOOL_MASK::OMNIMODE) != 0;
    Device::DamperPedal = (flags & DEVICE_BOOL_MASK::DAMPER_PEDAL) != 0;
    Device::Vibrato = (flags & DEVICE_BOOL_MASK::VIBRATO) != 0;
    // DistributorManager's add path persists; suppress those writes during restore.
    m_restoring = true;
    bool success = true;
    const uint8_t count = getNumOfDistributors();
    for (uint8_t i = 0; i < count; ++i) {
        uint8_t record[DISTRIBUTOR_NUM_CFG_BYTES];
        if (!getDistributorConstruct(i, record) || !manager.addDistributor(record, sizeof(record))) {
            success = false;
            break;
        }
    }
    m_restoring = false;
    return success || error(Error::RestoreFailed);
}
#endif
