#include <unity.h>

#include "MsgHandling/MidiMessage.h"
#include "Networks/NetworkUSB/ESP32_S3_USBMidiCodec.h"
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

void test_usb_midi_codec_round_trips_note_on() {
    MidiMessage message;
    message.buffer[0] = 0x92;
    message.buffer[1] = 60;
    message.buffer[2] = 100;
    message.length = 3;

    uint8_t offset = 0;
    const auto packet = ESP32_S3_USBMidiCodec::encodePacket(message, offset);
    TEST_ASSERT_EQUAL_UINT8(0x09, packet[0]);
    TEST_ASSERT_EQUAL_UINT8(0x92, packet[1]);
    TEST_ASSERT_EQUAL_UINT8(60, packet[2]);
    TEST_ASSERT_EQUAL_UINT8(100, packet[3]);
    TEST_ASSERT_EQUAL_UINT8(message.length, offset);

    ESP32_S3_USBMidiCodec codec;
    const auto result = codec.decodePacket(packet);
    TEST_ASSERT_TRUE(result.message.has_value());
    TEST_ASSERT_EQUAL_UINT8(message.length, result.message->length);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(message.buffer.data(), result.message->buffer.data(), message.length);
}

void test_usb_midi_codec_reassembles_fragmented_sysex() {
    ESP32_S3_USBMidiCodec codec;
    const ESP32_S3_USBMidiCodec::Packet first = {0x04, 0xF0, 0x7D, 0x01};
    const ESP32_S3_USBMidiCodec::Packet middle = {0x04, 0x02, 0x03, 0x04};
    const ESP32_S3_USBMidiCodec::Packet last = {0x06, 0x05, 0xF7, 0x00};

    TEST_ASSERT_FALSE(codec.decodePacket(first).message.has_value());
    TEST_ASSERT_FALSE(codec.decodePacket(middle).message.has_value());
    const auto result = codec.decodePacket(last);

    TEST_ASSERT_TRUE(result.message.has_value());
    TEST_ASSERT_EQUAL_UINT8(8, result.message->length);
    const uint8_t expected[] = {0xF0, 0x7D, 0x01, 0x02, 0x03, 0x04, 0x05, 0xF7};
    TEST_ASSERT_EQUAL_UINT8_ARRAY(expected, result.message->buffer.data(), sizeof(expected));
}

void test_usb_midi_codec_preserves_sysex_across_realtime_message() {
    ESP32_S3_USBMidiCodec codec;
    const ESP32_S3_USBMidiCodec::Packet first = {0x04, 0xF0, 0x7D, 0x01};
    const ESP32_S3_USBMidiCodec::Packet realtime = {0x0F, 0xF8, 0x00, 0x00};
    const ESP32_S3_USBMidiCodec::Packet last = {0x06, 0x02, 0xF7, 0x00};

    TEST_ASSERT_FALSE(codec.decodePacket(first).message.has_value());
    const auto clock = codec.decodePacket(realtime);
    TEST_ASSERT_TRUE(clock.message.has_value());
    TEST_ASSERT_EQUAL_UINT8(0xF8, clock.message->buffer[0]);
    const auto result = codec.decodePacket(last);
    TEST_ASSERT_TRUE(result.message.has_value());
    TEST_ASSERT_EQUAL_UINT8(5, result.message->length);
}

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_sysex_constructor_creates_complete_packet);
    RUN_TEST(test_incomplete_sysex_has_no_payload);
    RUN_TEST(test_sysex_payload_is_bounded_to_packet_capacity);
    RUN_TEST(test_crc_detects_record_corruption);
    RUN_TEST(test_usb_midi_codec_round_trips_note_on);
    RUN_TEST(test_usb_midi_codec_reassembles_fragmented_sysex);
    RUN_TEST(test_usb_midi_codec_preserves_sysex_across_realtime_message);
    return UNITY_END();
}