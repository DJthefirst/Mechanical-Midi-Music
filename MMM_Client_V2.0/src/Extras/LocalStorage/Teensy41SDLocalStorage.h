#pragma once

#include "Config.h"
#include "ILocalStorage.h"
#include "Distributors/Distributor.h"

#if defined(PLATFORM_TEENSY41)
#include "Utility/SdCard/Teensy41SdCardUtility.h"
#include <array>

class DistributorManager;

class Teensy41SDLocalStorage : public ILocalStorage {
public:
    enum class Error : uint8_t { None, CardUnavailable, ReadFailed, CorruptData, WriteFailed, InvalidArgument, RestoreFailed };
    bool initialize() override;
    bool isAvailable() override { return m_initialized; }
    void resetDeviceConfig() override;
    bool initializeDeviceConfiguration(DistributorManager& manager) override;
    std::string getDeviceName() override;
    void setDeviceName(const std::string& name) override;
    uint16_t getDeviceID() override;
    void setDeviceID(uint16_t id) override;
    uint16_t getDeviceBoolean() override;
    void setDeviceBoolean(uint16_t flags) override;
    uint8_t getNumOfDistributors() override;
    void setNumOfDistributors(uint8_t count) override;
    bool getDistributorConstruct(uint16_t index, uint8_t* construct) override;
    void setDistributorConstruct(uint16_t index, const uint8_t* construct) override;
    Error lastError() const { return m_error; }
    uint32_t errorCount() const { return m_errorCount; }

private:
    static constexpr size_t NameOffset = 0;
    static constexpr size_t IdOffset = DEVICE_NUM_NAME_BYTES;
    static constexpr size_t FlagsOffset = IdOffset + 2;
    static constexpr size_t CountOffset = FlagsOffset + 2;
    static constexpr size_t RecordsOffset = CountOffset + 1;
    static constexpr size_t RecordBytes = DISTRIBUTOR_NUM_CFG_BYTES + 1;
    static constexpr size_t SnapshotBytes = RecordsOffset + MAX_DISTRIBUTORS * RecordBytes;
    static constexpr size_t HeaderBytes = 13;
    bool error(Error value);
    void defaults();
    bool inspectSlot(uint8_t slot, uint32_t& generation);
    bool loadSlot(uint8_t slot);
    bool commit();
    void update(size_t offset, const uint8_t* data, size_t length);
    uint16_t getWord(size_t offset) const;
    void setWord(size_t offset, uint16_t value);
    std::array<uint8_t, SnapshotBytes> m_data{};
    uint32_t m_generation = 0, m_errorCount = 0;
    int8_t m_activeSlot = -1;
    bool m_initialized = false, m_restoring = false;
    Error m_error = Error::None;
};
#endif
