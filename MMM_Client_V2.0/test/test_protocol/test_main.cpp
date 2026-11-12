#include <unity.h>

#include "MsgHandling/MidiMessage.h"
#include "Utility/Crc8.h"

void setUp() {}
void tearDown() {}

void test_sysex_constructor_creates_complete_packet() {
    const uint8_t payload[] = {0x12, 0x34};
    MidiMessage message(0x0123, 0x0234, SysEx::DistributorConstruct, payload, sizeof(payload));

    TEST_ASSERT_TRUE(message.isCompleteSysEx());
    TEST_ASSERT_EQUAL_UINT8(sizeof(payload), message.sysExPayloadLength());
    TEST_ASSERT_EQUAL_UINT16(0x0123, message.SourceID());
    TEST_ASSERT_EQUAL_UINT16(0x0234, message.DestinationID());
    TEST_ASSERT_EQUAL_UINT8(0xF7, message.buffer[message.length - 1]);
}

void test_incomplete_sysex_has_no_payload() {
    MidiMessage message;
    message.buffer[0] = 0xF0;
    message.buffer[1] = SysEx::ID;
    message.length = SYSEX_HeaderSize;

    TEST_ASSERT_FALSE(message.isCompleteSysEx());
    TEST_ASSERT_EQUAL_UINT8(0, message.sysExPayloadLength());
}

void test_sysex_payload_is_bounded_to_packet_capacity() {
    uint8_t payload[MAX_PACKET_LENGTH] = {};
    MidiMessage message(1, 2, SysEx::Message, payload, sizeof(payload));

    TEST_ASSERT_EQUAL_UINT16(MAX_PACKET_LENGTH, message.length);
    TEST_ASSERT_EQUAL_UINT8(MAX_PACKET_LENGTH - SYSEX_HeaderSize - 1, message.sysExPayloadLength());
    TEST_ASSERT_TRUE(message.isCompleteSysEx());
}

void test_crc_detects_record_corruption() {
    uint8_t record[] = {0x00, 0x7F, 0x12, 0x34, 0x56};
    const uint8_t checksum = Utility::crc8(record, sizeof(record));

    record[2] ^= 0x01;
    TEST_ASSERT_NOT_EQUAL(checksum, Utility::crc8(record, sizeof(record)));
}

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_sysex_constructor_creates_complete_packet);
    RUN_TEST(test_incomplete_sysex_has_no_payload);
    RUN_TEST(test_sysex_payload_is_bounded_to_packet_capacity);
    RUN_TEST(test_crc_detects_record_corruption);
    return UNITY_END();
}