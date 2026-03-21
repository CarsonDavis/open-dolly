#pragma once
#include "dji_types.h"

namespace dji {

/// Build a complete DJI R SDK frame.
/// @param cmd_type   CmdType byte (e.g., 0x03 for command with reply required)
/// @param cmd_set    Command set (e.g., 0x0E for gimbal)
/// @param cmd_id     Command ID
/// @param payload    Command payload data (may be nullptr if payload_len == 0)
/// @param payload_len Length of payload in bytes
/// @param seq        Sequence number
/// @param[out] frame Output buffer (must be >= FRAME_OVERHEAD + payload_len bytes)
/// @return Total frame length, or 0 on error
size_t buildFrame(uint8_t cmd_type, uint8_t cmd_set, uint8_t cmd_id,
                  const uint8_t* payload, size_t payload_len,
                  uint16_t seq, uint8_t* frame);

/// Validate a received frame's CRC16 (header) and CRC32 (full frame).
/// @param frame      Complete frame bytes
/// @param length     Total frame length
/// @return true if both CRCs are valid
bool validateFrame(const uint8_t* frame, size_t length);

/// Extract fields from a validated frame.
/// @return true if extraction succeeded
bool parseFrame(const uint8_t* frame, size_t length,
                uint8_t& cmd_type, uint16_t& seq,
                uint8_t& cmd_set, uint8_t& cmd_id,
                const uint8_t*& payload, size_t& payload_len);

/// Extract the total frame length from the first 3 bytes of a frame.
/// Returns 0 if the SOF byte is not 0xAA or the length is invalid.
size_t extractFrameLength(const uint8_t* header3);

/// Manage a sequence number counter. Thread-safe if called from a single thread.
class SequenceCounter {
public:
    SequenceCounter() : _seq(SEQ_MIN) {}

    uint16_t next() {
        uint16_t val = _seq;
        _seq++;
        if (_seq > SEQ_MAX) {
            _seq = SEQ_MIN;
        }
        return val;
    }

    void reset() { _seq = SEQ_MIN; }

private:
    uint16_t _seq;
};

}  // namespace dji
