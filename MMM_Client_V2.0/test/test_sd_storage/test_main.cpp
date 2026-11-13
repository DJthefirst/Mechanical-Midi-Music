#define ARDUINO_TEENSY41
#include <unity.h>
#include "Utility/SdCard/Teensy41SdCardUtility.cpp"
#include "Extras/LocalStorage/Teensy41SDLocalStorage.cpp"

using Storage = Teensy41SDLocalStorage;
static Storage* restoringStorage = nullptr;
static uint8_t restoredCount = 0;

Distributor::~Distributor() = default;
DistributorManager::DistributorManager(std::shared_ptr<InstrumentControllerBase>) {}
std::shared_ptr<DistributorManager> DistributorManager::getInstance(std::shared_ptr<InstrumentControllerBase>) {
    return std::shared_ptr<DistributorManager>(new DistributorManager(nullptr));
}
bool DistributorManager::addDistributor(const uint8_t* data, size_t length) {
    ++restoredCount;
    if (restoringStorage) {
        restoringStorage->setDistributorConstruct(restoredCount - 1, data);
        restoringStorage->setNumOfDistributors(restoredCount);
    }
    return length == DISTRIBUTOR_NUM_CFG_BYTES;
}

void setUp() {
    TestSd::reset();
    Device::Name = "Default";
    Device::SetDeviceID(123);
    Device::Muted = Device::OmniMode = Device::DamperPedal = Device::Vibrato = false;
    restoredCount = 0;
    restoringStorage = nullptr;
}
void tearDown() {}

void test_missing_card_fails_without_flash_fallback() {
    TestSd::mountFails = true;
    Storage storage;
    TEST_ASSERT_FALSE(storage.initialize());
    TEST_ASSERT_FALSE(storage.isAvailable());
    TEST_ASSERT_EQUAL(Storage::Error::CardUnavailable, storage.lastError());
    TEST_ASSERT_TRUE(TestSd::files.empty());
}

void test_round_trip_all_fields_and_restore_does_not_write() {
    Storage storage;
    TEST_ASSERT_TRUE(storage.initialize());
    storage.setDeviceName("SD instrument");
    storage.setDeviceID(0x1234);
    storage.setDeviceBoolean(15);
    uint8_t record[DISTRIBUTOR_NUM_CFG_BYTES] = {};
    record[5] = 77;
    storage.setDistributorConstruct(0, record);
    storage.setDistributorConstruct(MAX_DISTRIBUTORS - 1, record);
    storage.setNumOfDistributors(1);
    Storage rebooted;
    TEST_ASSERT_TRUE(rebooted.initialize());
    TEST_ASSERT_EQUAL_STRING("SD instrument", rebooted.getDeviceName().c_str());
    TEST_ASSERT_EQUAL_UINT16(0x1234, rebooted.getDeviceID());
    TEST_ASSERT_EQUAL_UINT16(15, rebooted.getDeviceBoolean());
    uint8_t readback[DISTRIBUTOR_NUM_CFG_BYTES];
    TEST_ASSERT_TRUE(rebooted.getDistributorConstruct(MAX_DISTRIBUTORS - 1, readback));
    TEST_ASSERT_EQUAL_UINT8_ARRAY(record, readback, sizeof(record));
    const auto writes = TestSd::writes;
    restoringStorage = &rebooted;
    auto manager = DistributorManager::getInstance();
    TEST_ASSERT_TRUE(rebooted.initializeDeviceConfiguration(*manager));
    TEST_ASSERT_EQUAL_UINT8(1, restoredCount);
    TEST_ASSERT_EQUAL_UINT32(writes, TestSd::writes);
    TEST_ASSERT_TRUE(Device::Muted && Device::OmniMode && Device::DamperPedal && Device::Vibrato);
}

void test_identical_values_skip_writes_and_names_are_bounded() {
    Storage storage;
    TEST_ASSERT_TRUE(storage.initialize());
    storage.setDeviceName("1234567890123456789012345");
    TEST_ASSERT_EQUAL_STRING("12345678901234567890", storage.getDeviceName().c_str());
    const auto writes = TestSd::writes;
    storage.setDeviceName("1234567890123456789012345");
    storage.setDeviceID(123);
    storage.setDeviceBoolean(0);
    storage.setNumOfDistributors(0);
    TEST_ASSERT_EQUAL_UINT32(writes, TestSd::writes);
    storage.setDeviceName("");
    TEST_ASSERT_EQUAL_STRING("Default", storage.getDeviceName().c_str());
}

void test_torn_write_rolls_back_and_survives_reboot() {
    Storage storage;
    TEST_ASSERT_TRUE(storage.initialize());
    storage.setDeviceID(42);
    TestSd::writeBudget = 20;
    storage.setDeviceID(99);
    TEST_ASSERT_EQUAL(Storage::Error::WriteFailed, storage.lastError());
    TEST_ASSERT_EQUAL_UINT16(42, storage.getDeviceID());
    TestSd::writeBudget = -1;
    Storage rebooted;
    TEST_ASSERT_TRUE(rebooted.initialize());
    TEST_ASSERT_EQUAL_UINT16(42, rebooted.getDeviceID());
    TEST_ASSERT_GREATER_THAN_UINT32(0, rebooted.errorCount());
}

void test_corrupt_slots_recover_to_defaults_and_reset_preserves_midi() {
    Storage storage;
    TEST_ASSERT_TRUE(storage.initialize());
    auto song = std::make_shared<std::vector<uint8_t>>(std::initializer_list<uint8_t>{1,2,3});
    TestSd::files["/song.mid"] = song;
    storage.setDeviceName("Custom");
    storage.setNumOfDistributors(2);
    storage.resetDeviceConfig();
    TEST_ASSERT_EQUAL_UINT8(0, storage.getNumOfDistributors());
    TEST_ASSERT_EQUAL_STRING("Default", storage.getDeviceName().c_str());
    TEST_ASSERT_EQUAL_UINT32(3, song->size());
    for (const char* path : {"/mmm-config-0.bin", "/mmm-config-1.bin"})
        (*TestSd::files.at(path))[20] ^= 1;
    Storage rebooted;
    TEST_ASSERT_TRUE(rebooted.initialize());
    TEST_ASSERT_GREATER_THAN_UINT32(0, rebooted.errorCount());
    TEST_ASSERT_EQUAL_STRING("Default", rebooted.getDeviceName().c_str());
    TEST_ASSERT_EQUAL_UINT32(3, song->size());
    Storage again;
    TEST_ASSERT_TRUE(again.initialize());
    TEST_ASSERT_TRUE(again.isAvailable());
}

void test_sync_failure_and_invalid_records_surface_errors() {
    Storage storage;
    TEST_ASSERT_TRUE(storage.initialize());
    TestSd::syncFails = true;
    storage.setDeviceID(10);
    TEST_ASSERT_EQUAL(Storage::Error::WriteFailed, storage.lastError());
    TEST_ASSERT_EQUAL_UINT16(123, storage.getDeviceID());
    uint8_t record[DISTRIBUTOR_NUM_CFG_BYTES] = {};
    TEST_ASSERT_FALSE(storage.getDistributorConstruct(MAX_DISTRIBUTORS, record));
    TEST_ASSERT_EQUAL(Storage::Error::InvalidArgument, storage.lastError());
    storage.setDistributorConstruct(0, nullptr);
    TEST_ASSERT_EQUAL(Storage::Error::InvalidArgument, storage.lastError());
}

void test_invalid_count_in_newer_slot_recovers_older_configuration() {
    Storage storage;
    TEST_ASSERT_TRUE(storage.initialize());
    storage.setDeviceID(42);
    auto& newer = *TestSd::files.at("/mmm-config-1.bin");
    newer[13 + DEVICE_NUM_NAME_BYTES + 4] = MAX_DISTRIBUTORS + 1;
    newer[12] = Utility::crc8(newer.data() + 13, newer.size() - 13);
    Storage rebooted;
    TEST_ASSERT_TRUE(rebooted.initialize());
    TEST_ASSERT_EQUAL_UINT16(123, rebooted.getDeviceID());
    TEST_ASSERT_EQUAL(Storage::Error::CorruptData, rebooted.lastError());
}

void test_midi_reader_keeps_position_across_storage_writes() {
    Storage storage;
    TEST_ASSERT_TRUE(storage.initialize());
    TestSd::files["/song.mid"] = std::make_shared<std::vector<uint8_t>>(
        std::initializer_list<uint8_t>{1,2,3,4});
    Utility::Teensy41SdReadFile reader;
    TEST_ASSERT_TRUE(reader.open("/song.mid"));
    uint8_t byte;
    TEST_ASSERT_EQUAL_INT(1, reader.read(&byte, 1));
    storage.setDeviceID(77);
    TEST_ASSERT_EQUAL_UINT32(1, reader.position());
    TEST_ASSERT_EQUAL_INT(1, reader.read(&byte, 1));
    TEST_ASSERT_EQUAL_UINT8(2, byte);
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_missing_card_fails_without_flash_fallback);
    RUN_TEST(test_round_trip_all_fields_and_restore_does_not_write);
    RUN_TEST(test_identical_values_skip_writes_and_names_are_bounded);
    RUN_TEST(test_torn_write_rolls_back_and_survives_reboot);
    RUN_TEST(test_corrupt_slots_recover_to_defaults_and_reset_preserves_midi);
    RUN_TEST(test_sync_failure_and_invalid_records_surface_errors);
    RUN_TEST(test_invalid_count_in_newer_slot_recovers_older_configuration);
    RUN_TEST(test_midi_reader_keeps_position_across_storage_writes);
    return UNITY_END();
}
