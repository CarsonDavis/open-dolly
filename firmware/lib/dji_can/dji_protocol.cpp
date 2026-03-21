#include "dji_protocol.h"
#include "dji_crc.h"
#include <cstring>

namespace dji {

size_t buildFrame(uint8_t cmd_type, uint8_t cmd_set, uint8_t cmd_id,
                  const uint8_t* payload, size_t payload_len,
                  uint16_t seq, uint8_t* frame) {
    // Total length = 12 (header) + 2 (CmdSet + CmdID) + payload_len + 4 (CRC32)
    size_t total_len = FRAME_OVERHEAD + payload_len;
    if (total_len > MAX_FRAME_SIZE) {
        return 0;
    }

    // -- Header (bytes 0..11) --

    // [0] SOF
    frame[0] = SOF_BYTE;

    // [1..2] Ver/Length (little-endian)
    // Bits [9:0] = total frame length, bits [15:10] = version (0)
    frame[1] = static_cast<uint8_t>(total_len & 0xFF);
    frame[2] = static_cast<uint8_t>((total_len >> 8) & 0x03);  // version 0, upper 6 bits = 0

    // [3] CmdType
    frame[3] = cmd_type;

    // [4] ENC (no encryption, no supplementary bytes)
    frame[4] = 0x00;

    // [5..7] Reserved
    frame[5] = 0x00;
    frame[6] = 0x00;
    frame[7] = 0x00;

    // [8..9] Sequence number (little-endian)
    frame[8] = static_cast<uint8_t>(seq & 0xFF);
    frame[9] = static_cast<uint8_t>((seq >> 8) & 0xFF);

    // [10..11] CRC16 over header bytes [0..9]
    uint16_t crc16_val = crc16(frame, 10);
    frame[10] = static_cast<uint8_t>(crc16_val & 0xFF);
    frame[11] = static_cast<uint8_t>((crc16_val >> 8) & 0xFF);

    // -- Payload --

    // [12] CmdSet
    frame[12] = cmd_set;

    // [13] CmdID
    frame[13] = cmd_id;

    // [14..14+payload_len-1] Payload data
    if (payload_len > 0 && payload != nullptr) {
        memcpy(&frame[14], payload, payload_len);
    }

    // -- CRC32 over everything except the CRC32 field itself --
    size_t crc32_offset = total_len - 4;
    uint32_t crc32_val = crc32(frame, crc32_offset);
    frame[crc32_offset + 0] = static_cast<uint8_t>(crc32_val & 0xFF);
    frame[crc32_offset + 1] = static_cast<uint8_t>((crc32_val >> 8) & 0xFF);
    frame[crc32_offset + 2] = static_cast<uint8_t>((crc32_val >> 16) & 0xFF);
    frame[crc32_offset + 3] = static_cast<uint8_t>((crc32_val >> 24) & 0xFF);

    return total_len;
}

bool validateFrame(const uint8_t* frame, size_t length) {
    if (length < FRAME_OVERHEAD) {
        return false;
    }
    if (frame[0] != SOF_BYTE) {
        return false;
    }

    // Validate CRC16 over header (bytes 0..9)
    uint16_t expected_crc16 = static_cast<uint16_t>(frame[10]) |
                              (static_cast<uint16_t>(frame[11]) << 8);
    uint16_t computed_crc16 = crc16(frame, 10);
    if (expected_crc16 != computed_crc16) {
        return false;
    }

    // Validate CRC32 over everything except the last 4 bytes
    size_t crc32_offset = length - 4;
    uint32_t expected_crc32 = static_cast<uint32_t>(frame[crc32_offset + 0]) |
                              (static_cast<uint32_t>(frame[crc32_offset + 1]) << 8) |
                              (static_cast<uint32_t>(frame[crc32_offset + 2]) << 16) |
                              (static_cast<uint32_t>(frame[crc32_offset + 3]) << 24);
    uint32_t computed_crc32 = crc32(frame, crc32_offset);
    if (expected_crc32 != computed_crc32) {
        return false;
    }

    return true;
}

bool parseFrame(const uint8_t* frame, size_t length,
                uint8_t& cmd_type, uint16_t& seq,
                uint8_t& cmd_set, uint8_t& cmd_id,
                const uint8_t*& payload, size_t& payload_len) {
    if (length < FRAME_OVERHEAD) {
        return false;
    }

    cmd_type = frame[3];
    seq = static_cast<uint16_t>(frame[8]) | (static_cast<uint16_t>(frame[9]) << 8);
    cmd_set = frame[12];
    cmd_id = frame[13];

    // Payload starts after CmdSet + CmdID (byte 14), ends before CRC32 (last 4 bytes)
    payload_len = length - FRAME_OVERHEAD;
    payload = (payload_len > 0) ? &frame[14] : nullptr;

    return true;
}

size_t extractFrameLength(const uint8_t* header3) {
    if (header3[0] != SOF_BYTE) {
        return 0;
    }
    size_t len = static_cast<size_t>(header3[1]) |
                 (static_cast<size_t>(header3[2] & 0x03) << 8);
    if (len < FRAME_OVERHEAD) {
        return 0;
    }
    return len;
}

}  // namespace dji
