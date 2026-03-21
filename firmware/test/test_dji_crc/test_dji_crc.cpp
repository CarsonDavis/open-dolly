#include <unity.h>
#include "dji_crc.h"

void setUp(void) {}
void tearDown(void) {}

// --- CRC16 tests ---

// Test vector from SDK documentation: position command header
void test_crc16_position_header() {
    uint8_t header[] = {0xAA, 0x1A, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x22, 0x11};
    uint16_t crc = dji::crc16(header, 10);
    TEST_ASSERT_EQUAL_HEX16(0x42A2, crc);
}

// Single byte
void test_crc16_single_byte() {
    uint8_t data[] = {0xAA};
    uint16_t crc = dji::crc16(data, 1);
    // Verify it produces a non-zero result (basic sanity)
    TEST_ASSERT_NOT_EQUAL(0, crc);
}

// Empty input
void test_crc16_empty() {
    uint16_t crc = dji::crc16(nullptr, 0);
    // Empty input with init 0x3AA3 should return init value
    TEST_ASSERT_EQUAL_HEX16(0x3AA3, crc);
}

// Deterministic: same input = same output
void test_crc16_deterministic() {
    uint8_t data[] = {0x01, 0x02, 0x03, 0x04, 0x05};
    uint16_t crc1 = dji::crc16(data, 5);
    uint16_t crc2 = dji::crc16(data, 5);
    TEST_ASSERT_EQUAL_HEX16(crc1, crc2);
}

// --- CRC32 tests ---

// Test vector from SDK documentation: position command (full frame minus CRC32)
void test_crc32_position_frame() {
    uint8_t frame[] = {
        0xAA, 0x1A, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00,
        0x22, 0x11, 0xA2, 0x42, 0x0E, 0x00, 0x20, 0x00,
        0x30, 0x00, 0x40, 0x00, 0x01, 0x14
    };
    uint32_t crc = dji::crc32(frame, 22);
    // Expected CRC32 bytes in the SDK example: 7B 40 97 BE (little-endian)
    // So the value is 0xBE97407B
    TEST_ASSERT_EQUAL_HEX32(0xBE97407B, crc);
}

// Empty input
void test_crc32_empty() {
    uint32_t crc = dji::crc32(nullptr, 0);
    TEST_ASSERT_EQUAL_HEX32(0x00003AA3, crc);
}

// Deterministic
void test_crc32_deterministic() {
    uint8_t data[] = {0xAA, 0xBB, 0xCC, 0xDD};
    uint32_t crc1 = dji::crc32(data, 4);
    uint32_t crc2 = dji::crc32(data, 4);
    TEST_ASSERT_EQUAL_HEX32(crc1, crc2);
}

// Verify CRC32 changes with different data
void test_crc32_different_data() {
    uint8_t data1[] = {0x01, 0x02};
    uint8_t data2[] = {0x01, 0x03};
    uint32_t crc1 = dji::crc32(data1, 2);
    uint32_t crc2 = dji::crc32(data2, 2);
    TEST_ASSERT_NOT_EQUAL(crc1, crc2);
}

int main() {
    UNITY_BEGIN();

    // CRC16
    RUN_TEST(test_crc16_position_header);
    RUN_TEST(test_crc16_single_byte);
    RUN_TEST(test_crc16_empty);
    RUN_TEST(test_crc16_deterministic);

    // CRC32
    RUN_TEST(test_crc32_position_frame);
    RUN_TEST(test_crc32_empty);
    RUN_TEST(test_crc32_deterministic);
    RUN_TEST(test_crc32_different_data);

    return UNITY_END();
}
