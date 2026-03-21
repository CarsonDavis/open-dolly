#include <unity.h>
#include <cstring>
#include "dji_protocol.h"
#include "dji_crc.h"

void setUp(void) {}
void tearDown(void) {}

// --- Frame building tests ---

void test_build_enable_push_frame() {
    uint8_t frame[64];
    uint8_t payload[] = {0x01}; // enable
    size_t len = dji::buildFrame(0x03, 0x0E, 0x07, payload, 1, 0x0003, frame);

    // Total = 18 + 1 = 19 bytes
    TEST_ASSERT_EQUAL(19, len);
    TEST_ASSERT_EQUAL_HEX8(0xAA, frame[0]); // SOF
    TEST_ASSERT_EQUAL_HEX8(19, frame[1]);    // length low byte
    TEST_ASSERT_EQUAL_HEX8(0x00, frame[2]);  // version=0, length high bits=0
    TEST_ASSERT_EQUAL_HEX8(0x03, frame[3]);  // CmdType
    TEST_ASSERT_EQUAL_HEX8(0x00, frame[4]);  // ENC
    TEST_ASSERT_EQUAL_HEX8(0x00, frame[5]);  // RES
    TEST_ASSERT_EQUAL_HEX8(0x00, frame[6]);  // RES
    TEST_ASSERT_EQUAL_HEX8(0x00, frame[7]);  // RES
    TEST_ASSERT_EQUAL_HEX8(0x03, frame[8]);  // SEQ low
    TEST_ASSERT_EQUAL_HEX8(0x00, frame[9]);  // SEQ high
    // CRC16 at [10-11]
    TEST_ASSERT_EQUAL_HEX8(0x0E, frame[12]); // CmdSet
    TEST_ASSERT_EQUAL_HEX8(0x07, frame[13]); // CmdID
    TEST_ASSERT_EQUAL_HEX8(0x01, frame[14]); // payload

    // Verify CRC16
    uint16_t expected_crc16 = dji::crc16(frame, 10);
    uint16_t actual_crc16 = frame[10] | (frame[11] << 8);
    TEST_ASSERT_EQUAL_HEX16(expected_crc16, actual_crc16);

    // Verify CRC32
    uint32_t expected_crc32 = dji::crc32(frame, len - 4);
    uint32_t actual_crc32 = frame[len-4] | (frame[len-3] << 8) |
                            (frame[len-2] << 16) | (frame[len-1] << 24);
    TEST_ASSERT_EQUAL_HEX32(expected_crc32, actual_crc32);
}

void test_build_position_command() {
    uint8_t frame[64];
    // yaw=3.2deg (32), roll=4.8deg (48), pitch=6.4deg (64), absolute, 2.0s
    uint8_t payload[] = {0x20, 0x00, 0x30, 0x00, 0x40, 0x00, 0x01, 0x14};
    size_t len = dji::buildFrame(0x03, 0x0E, 0x00, payload, 8, 0x1122, frame);

    TEST_ASSERT_EQUAL(26, len);
    TEST_ASSERT_EQUAL_HEX8(0xAA, frame[0]);
    TEST_ASSERT_EQUAL_HEX8(0x1A, frame[1]); // 26 = 0x1A
    TEST_ASSERT_EQUAL_HEX8(0x03, frame[3]);
    TEST_ASSERT_EQUAL_HEX8(0x22, frame[8]); // SEQ low
    TEST_ASSERT_EQUAL_HEX8(0x11, frame[9]); // SEQ high
    TEST_ASSERT_EQUAL_HEX8(0x0E, frame[12]);
    TEST_ASSERT_EQUAL_HEX8(0x00, frame[13]);

    // Verify CRC16 matches the SDK example
    uint16_t actual_crc16 = frame[10] | (frame[11] << 8);
    TEST_ASSERT_EQUAL_HEX16(0x42A2, actual_crc16);
}

void test_build_frame_no_payload() {
    uint8_t frame[64];
    size_t len = dji::buildFrame(0x03, 0x0E, 0x02, nullptr, 0, 0x0005, frame);
    TEST_ASSERT_EQUAL(18, len); // minimum frame size
}

void test_build_frame_validates_frame() {
    uint8_t frame[64];
    uint8_t payload[] = {0x01};
    size_t len = dji::buildFrame(0x03, 0x0E, 0x07, payload, 1, 0x0003, frame);

    // The frame we just built should pass validation
    TEST_ASSERT_TRUE(dji::validateFrame(frame, len));
}

// --- Frame validation tests ---

void test_validate_corrupted_crc16() {
    uint8_t frame[64];
    uint8_t payload[] = {0x01};
    size_t len = dji::buildFrame(0x03, 0x0E, 0x07, payload, 1, 0x0003, frame);

    // Corrupt CRC16
    frame[10] ^= 0xFF;
    TEST_ASSERT_FALSE(dji::validateFrame(frame, len));
}

void test_validate_corrupted_crc32() {
    uint8_t frame[64];
    uint8_t payload[] = {0x01};
    size_t len = dji::buildFrame(0x03, 0x0E, 0x07, payload, 1, 0x0003, frame);

    // Corrupt CRC32
    frame[len - 1] ^= 0xFF;
    TEST_ASSERT_FALSE(dji::validateFrame(frame, len));
}

void test_validate_too_short() {
    uint8_t frame[] = {0xAA, 0x00, 0x00};
    TEST_ASSERT_FALSE(dji::validateFrame(frame, 3));
}

// --- Frame parsing tests ---

void test_parse_extracts_fields() {
    uint8_t frame[64];
    uint8_t payload_data[] = {0x01, 0x02, 0x03};
    size_t len = dji::buildFrame(0x03, 0x0E, 0x07, payload_data, 3, 0x0042, frame);

    uint8_t cmd_type;
    uint16_t seq;
    uint8_t cmd_set, cmd_id;
    const uint8_t* payload;
    size_t payload_len;

    TEST_ASSERT_TRUE(dji::parseFrame(frame, len, cmd_type, seq, cmd_set, cmd_id, payload, payload_len));
    TEST_ASSERT_EQUAL_HEX8(0x03, cmd_type);
    TEST_ASSERT_EQUAL_HEX16(0x0042, seq);
    TEST_ASSERT_EQUAL_HEX8(0x0E, cmd_set);
    TEST_ASSERT_EQUAL_HEX8(0x07, cmd_id);
    TEST_ASSERT_EQUAL(3, payload_len);
    TEST_ASSERT_EQUAL_HEX8(0x01, payload[0]);
    TEST_ASSERT_EQUAL_HEX8(0x02, payload[1]);
    TEST_ASSERT_EQUAL_HEX8(0x03, payload[2]);
}

void test_parse_no_payload() {
    uint8_t frame[64];
    size_t len = dji::buildFrame(0x03, 0x0E, 0x02, nullptr, 0, 0x0005, frame);

    uint8_t cmd_type;
    uint16_t seq;
    uint8_t cmd_set, cmd_id;
    const uint8_t* payload;
    size_t payload_len;

    TEST_ASSERT_TRUE(dji::parseFrame(frame, len, cmd_type, seq, cmd_set, cmd_id, payload, payload_len));
    TEST_ASSERT_EQUAL(0, payload_len);
}

// --- Sequence number tests ---

void test_sequence_starts_at_min() {
    dji::SequenceCounter seq;
    uint16_t val = seq.next();
    TEST_ASSERT_EQUAL(dji::SEQ_MIN, val);
}

void test_sequence_increments() {
    dji::SequenceCounter seq;
    uint16_t seq1 = seq.next();
    uint16_t seq2 = seq.next();
    TEST_ASSERT_EQUAL(seq1 + 1, seq2);
}

int main() {
    UNITY_BEGIN();

    // Frame building
    RUN_TEST(test_build_enable_push_frame);
    RUN_TEST(test_build_position_command);
    RUN_TEST(test_build_frame_no_payload);
    RUN_TEST(test_build_frame_validates_frame);

    // Frame validation
    RUN_TEST(test_validate_corrupted_crc16);
    RUN_TEST(test_validate_corrupted_crc32);
    RUN_TEST(test_validate_too_short);

    // Frame parsing
    RUN_TEST(test_parse_extracts_fields);
    RUN_TEST(test_parse_no_payload);

    // Sequence numbers
    RUN_TEST(test_sequence_starts_at_min);
    RUN_TEST(test_sequence_increments);

    return UNITY_END();
}
