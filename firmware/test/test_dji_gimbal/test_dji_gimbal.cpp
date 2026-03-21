#include <unity.h>
#include <cstring>
#include <cmath>
#include <vector>
#include <queue>
#include "dji_gimbal.h"
#include "dji_protocol.h"
#include "dji_crc.h"

using namespace dji;

// ============================================================================
// Mock CAN Bus
// ============================================================================

class MockCanBus : public CanBusInterface {
public:
    struct CanFrame {
        uint32_t id;
        uint8_t data[8];
        uint8_t length;
    };

    std::vector<CanFrame> tx_log;
    std::queue<CanFrame> rx_queue;

    bool send(uint32_t id, const uint8_t* data, uint8_t length) override {
        CanFrame f;
        f.id = id;
        f.length = length;
        memcpy(f.data, data, length);
        tx_log.push_back(f);
        return true;
    }

    bool receive(uint32_t& id, uint8_t* data, uint8_t& length, uint32_t) override {
        if (rx_queue.empty()) return false;
        auto& f = rx_queue.front();
        id = f.id;
        length = f.length;
        memcpy(data, f.data, f.length);
        rx_queue.pop();
        return true;
    }

    void clear() {
        tx_log.clear();
        while (!rx_queue.empty()) rx_queue.pop();
    }

    void enqueueSdkFrame(const uint8_t* frame, size_t frame_len) {
        size_t offset = 0;
        while (offset < frame_len) {
            CanFrame f;
            f.id = CAN_RX_ID;
            f.length = (frame_len - offset > 8) ? 8 : (uint8_t)(frame_len - offset);
            memcpy(f.data, frame + offset, f.length);
            rx_queue.push(f);
            offset += f.length;
        }
    }

    size_t totalBytesSent() const {
        size_t total = 0;
        for (const auto& f : tx_log) total += f.length;
        return total;
    }

    // Reconstruct the SDK frame from the last batch of sent CAN frames
    std::vector<uint8_t> reconstructLastSdkFrame(size_t start_idx = 0) const {
        std::vector<uint8_t> result;
        for (size_t i = start_idx; i < tx_log.size(); i++) {
            for (uint8_t j = 0; j < tx_log[i].length; j++) {
                result.push_back(tx_log[i].data[j]);
            }
        }
        return result;
    }
};

// Build a telemetry push frame with given attitude values
static size_t buildTelemetryPush(float yaw, float roll, float pitch,
                                 uint16_t seq_num, uint8_t* frame_buf) {
    uint8_t payload[22];
    memset(payload, 0, sizeof(payload));

    payload[0] = 0x07; // ctrl_byte: angles + limits + stiffness valid

    int16_t yaw_i = (int16_t)(yaw * 10.0f);
    int16_t roll_i = (int16_t)(roll * 10.0f);
    int16_t pitch_i = (int16_t)(pitch * 10.0f);

    // Attitude angles (offset 1)
    payload[1] = yaw_i & 0xFF;   payload[2] = (yaw_i >> 8) & 0xFF;
    payload[3] = roll_i & 0xFF;  payload[4] = (roll_i >> 8) & 0xFF;
    payload[5] = pitch_i & 0xFF; payload[6] = (pitch_i >> 8) & 0xFF;

    // Joint angles (offset 7) — same as attitude for testing
    payload[7] = payload[1]; payload[8] = payload[2];
    payload[9] = payload[3]; payload[10] = payload[4];
    payload[11] = payload[5]; payload[12] = payload[6];

    // Limits (offset 13)
    payload[13] = 145; payload[14] = 55;   // pitch max/min
    payload[15] = 179; payload[16] = 179;  // yaw max/min
    payload[17] = 30;  payload[18] = 30;   // roll max/min

    // Stiffness (offset 19)
    payload[19] = 50; payload[20] = 50; payload[21] = 50;

    // Telemetry push uses CmdType 0x00 (command frame from gimbal, not a reply)
    return buildFrame(0x00, CMD_SET_GIMBAL, CMD_ID_PUSH_DATA,
                      payload, 22, seq_num, frame_buf);
}

// Helper: get gimbal to ACTIVE state
static void getToActive(DjiGimbal& gimbal, MockCanBus& can, uint32_t time_ms = 100) {
    gimbal.begin(&can);
    uint8_t frame[64];
    size_t len = buildTelemetryPush(0.0f, 0.0f, 0.0f, 0x0001, frame);
    can.enqueueSdkFrame(frame, len);
    gimbal.update(time_ms);
}

static MockCanBus mockCan;
static DjiGimbal gimbal;

void setUp(void) {
    mockCan.clear();
    gimbal.end();
}

void tearDown(void) {
    gimbal.end();
}

// ============================================================================
// CRC Tests
// ============================================================================

void test_crc16_sdk_example() {
    uint8_t header[] = {0xAA, 0x1A, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x22, 0x11};
    TEST_ASSERT_EQUAL_HEX16(0x42A2, crc16(header, 10));
}

void test_crc16_empty() {
    TEST_ASSERT_EQUAL_HEX16(0x3AA3, crc16(nullptr, 0));
}

void test_crc32_sdk_example() {
    uint8_t frame[] = {
        0xAA, 0x1A, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00,
        0x22, 0x11, 0xA2, 0x42, 0x0E, 0x00, 0x20, 0x00,
        0x30, 0x00, 0x40, 0x00, 0x01, 0x14
    };
    TEST_ASSERT_EQUAL_HEX32(0xBE97407B, crc32(frame, 22));
}

void test_crc32_empty() {
    TEST_ASSERT_EQUAL_HEX32(0x00003AA3, crc32(nullptr, 0));
}

// ============================================================================
// Frame Building Tests
// ============================================================================

void test_build_position_command() {
    uint8_t frame[64];
    uint8_t payload[] = {0x20, 0x00, 0x30, 0x00, 0x40, 0x00, 0x01, 0x14};
    size_t len = buildFrame(0x03, 0x0E, 0x00, payload, 8, 0x1122, frame);

    TEST_ASSERT_EQUAL(26, len);
    TEST_ASSERT_EQUAL_HEX8(0xAA, frame[0]);
    TEST_ASSERT_EQUAL_HEX8(0x1A, frame[1]);
    TEST_ASSERT_EQUAL_HEX8(0x03, frame[3]);
    TEST_ASSERT_EQUAL_HEX8(0x22, frame[8]);
    TEST_ASSERT_EQUAL_HEX8(0x11, frame[9]);
    TEST_ASSERT_EQUAL_HEX8(0xA2, frame[10]);
    TEST_ASSERT_EQUAL_HEX8(0x42, frame[11]);
    TEST_ASSERT_EQUAL_HEX8(0x0E, frame[12]);
    TEST_ASSERT_EQUAL_HEX8(0x00, frame[13]);
    TEST_ASSERT_EQUAL_HEX8(0x7B, frame[22]);
    TEST_ASSERT_EQUAL_HEX8(0x40, frame[23]);
    TEST_ASSERT_EQUAL_HEX8(0x97, frame[24]);
    TEST_ASSERT_EQUAL_HEX8(0xBE, frame[25]);
}

void test_build_enable_push() {
    uint8_t frame[64];
    uint8_t payload[] = {0x01};
    size_t len = buildFrame(0x03, 0x0E, 0x07, payload, 1, 0x0003, frame);

    TEST_ASSERT_EQUAL(19, len);
    TEST_ASSERT_EQUAL_HEX8(0x13, frame[1]);
    TEST_ASSERT_EQUAL_HEX8(0x0E, frame[12]);
    TEST_ASSERT_EQUAL_HEX8(0x07, frame[13]);
    TEST_ASSERT_EQUAL_HEX8(0x01, frame[14]);
}

void test_build_zero_payload() {
    uint8_t frame[64];
    size_t len = buildFrame(0x03, 0x0E, 0x02, nullptr, 0, 0x0005, frame);
    TEST_ASSERT_EQUAL(18, len);
}

void test_build_rejects_oversized() {
    uint8_t frame[MAX_FRAME_SIZE + 1];
    uint8_t payload[MAX_FRAME_SIZE];
    size_t len = buildFrame(0x03, 0x0E, 0x00, payload, MAX_FRAME_SIZE, 0x0002, frame);
    TEST_ASSERT_EQUAL(0, len);
}

// ============================================================================
// Frame Validation Tests
// ============================================================================

void test_validate_valid_frame() {
    uint8_t frame[64];
    uint8_t payload[] = {0x20, 0x00, 0x30, 0x00, 0x40, 0x00, 0x01, 0x14};
    size_t len = buildFrame(0x03, 0x0E, 0x00, payload, 8, 0x1122, frame);
    TEST_ASSERT_TRUE(validateFrame(frame, len));
}

void test_validate_corrupt_crc16() {
    uint8_t frame[64];
    uint8_t payload[] = {0x01};
    size_t len = buildFrame(0x03, 0x0E, 0x07, payload, 1, 0x0003, frame);
    frame[10] ^= 0xFF;
    TEST_ASSERT_FALSE(validateFrame(frame, len));
}

void test_validate_corrupt_crc32() {
    uint8_t frame[64];
    uint8_t payload[] = {0x01};
    size_t len = buildFrame(0x03, 0x0E, 0x07, payload, 1, 0x0003, frame);
    frame[len - 1] ^= 0xFF;
    TEST_ASSERT_FALSE(validateFrame(frame, len));
}

void test_validate_too_short() {
    uint8_t frame[] = {0xAA, 0x12, 0x00};
    TEST_ASSERT_FALSE(validateFrame(frame, 3));
}

// ============================================================================
// Frame Parsing Tests
// ============================================================================

void test_parse_extracts_fields() {
    uint8_t frame[64];
    uint8_t payload[] = {0x20, 0x00, 0x30, 0x00, 0x40, 0x00, 0x01, 0x14};
    size_t len = buildFrame(0x03, 0x0E, 0x00, payload, 8, 0x1122, frame);

    uint8_t cmd_type; uint16_t seq; uint8_t cmd_set, cmd_id;
    const uint8_t* parsed; size_t parsed_len;

    TEST_ASSERT_TRUE(parseFrame(frame, len, cmd_type, seq, cmd_set, cmd_id, parsed, parsed_len));
    TEST_ASSERT_EQUAL_HEX8(0x03, cmd_type);
    TEST_ASSERT_EQUAL_HEX16(0x1122, seq);
    TEST_ASSERT_EQUAL_HEX8(0x0E, cmd_set);
    TEST_ASSERT_EQUAL_HEX8(0x00, cmd_id);
    TEST_ASSERT_EQUAL(8, parsed_len);
    TEST_ASSERT_EQUAL_HEX8(0x20, parsed[0]);
}

// ============================================================================
// extractFrameLength Tests
// ============================================================================

void test_extract_length_valid() {
    uint8_t h1[] = {0xAA, 0x1A, 0x00};
    TEST_ASSERT_EQUAL(26, extractFrameLength(h1));
}

void test_extract_length_large() {
    uint8_t h[] = {0xAA, 0x2C, 0x01};  // 300
    TEST_ASSERT_EQUAL(300, extractFrameLength(h));
}

void test_extract_length_invalid_sof() {
    uint8_t h[] = {0x55, 0x1A, 0x00};
    TEST_ASSERT_EQUAL(0, extractFrameLength(h));
}

void test_extract_length_too_small() {
    uint8_t h[] = {0xAA, 0x05, 0x00};
    TEST_ASSERT_EQUAL(0, extractFrameLength(h));
}

// ============================================================================
// Sequence Counter Tests
// ============================================================================

void test_seq_starts_at_min() {
    SequenceCounter seq;
    TEST_ASSERT_EQUAL(SEQ_MIN, seq.next());
}

void test_seq_increments() {
    SequenceCounter seq;
    uint16_t first = seq.next();
    TEST_ASSERT_EQUAL(first + 1, seq.next());
}

void test_seq_wraps() {
    SequenceCounter seq;
    for (uint32_t i = SEQ_MIN; i <= SEQ_MAX; i++) seq.next();
    TEST_ASSERT_EQUAL(SEQ_MIN, seq.next());
}

void test_seq_reset() {
    SequenceCounter seq;
    seq.next(); seq.next();
    seq.reset();
    TEST_ASSERT_EQUAL(SEQ_MIN, seq.next());
}

// ============================================================================
// State Machine Tests
// ============================================================================

void test_initial_state_disconnected() {
    TEST_ASSERT_EQUAL(GimbalState::DISCONNECTED, gimbal.getState());
}

void test_begin_transitions_to_connecting() {
    gimbal.begin(&mockCan);
    TEST_ASSERT_EQUAL(GimbalState::CONNECTING, gimbal.getState());
}

void test_begin_sends_enable_push() {
    gimbal.begin(&mockCan);
    TEST_ASSERT_TRUE(mockCan.tx_log.size() > 0);
    for (auto& f : mockCan.tx_log) {
        TEST_ASSERT_EQUAL_HEX32(CAN_TX_ID, f.id);
    }
}

void test_telemetry_transitions_to_active() {
    gimbal.begin(&mockCan);
    uint8_t frame[64];
    size_t len = buildTelemetryPush(45.0f, 0.0f, -10.0f, 0x0001, frame);
    mockCan.enqueueSdkFrame(frame, len);
    gimbal.update(100);
    TEST_ASSERT_EQUAL(GimbalState::ACTIVE, gimbal.getState());
}

void test_attitude_values_parsed() {
    gimbal.begin(&mockCan);
    uint8_t frame[64];
    size_t len = buildTelemetryPush(45.0f, 5.0f, -10.0f, 0x0001, frame);
    mockCan.enqueueSdkFrame(frame, len);
    gimbal.update(100);

    Attitude att = gimbal.getAttitude();
    TEST_ASSERT_FLOAT_WITHIN(0.15f, 45.0f, att.yaw);
    TEST_ASSERT_FLOAT_WITHIN(0.15f, 5.0f, att.roll);
    TEST_ASSERT_FLOAT_WITHIN(0.15f, -10.0f, att.pitch);
}

void test_telemetry_limits_and_stiffness() {
    getToActive(gimbal, mockCan);
    GimbalTelemetry tel = gimbal.getTelemetry();
    TEST_ASSERT_TRUE(tel.angles_valid);
    TEST_ASSERT_TRUE(tel.limits_valid);
    TEST_ASSERT_TRUE(tel.stiffness_valid);
    TEST_ASSERT_EQUAL(145, tel.pitch_max);
    TEST_ASSERT_EQUAL(179, tel.yaw_max);
    TEST_ASSERT_EQUAL(30, tel.roll_max);
    TEST_ASSERT_EQUAL(50, tel.pitch_stiffness);
}

void test_connect_timeout_to_error() {
    gimbal.begin(&mockCan);
    gimbal.update(0);
    gimbal.update(CONNECT_TIMEOUT_MS + 1);
    TEST_ASSERT_EQUAL(GimbalState::ERROR, gimbal.getState());
}

void test_telemetry_timeout_from_active() {
    getToActive(gimbal, mockCan, 100);
    TEST_ASSERT_EQUAL(GimbalState::ACTIVE, gimbal.getState());
    gimbal.update(100 + TELEMETRY_TIMEOUT_MS + 1);
    TEST_ASSERT_EQUAL(GimbalState::ERROR, gimbal.getState());
}

void test_retry_after_error() {
    gimbal.begin(&mockCan);
    gimbal.update(0);
    gimbal.update(CONNECT_TIMEOUT_MS + 1);
    TEST_ASSERT_EQUAL(GimbalState::ERROR, gimbal.getState());

    size_t tx_before = mockCan.tx_log.size();
    gimbal.update(CONNECT_TIMEOUT_MS + 1 + RETRY_INTERVAL_MS + 1);
    TEST_ASSERT_EQUAL(GimbalState::CONNECTING, gimbal.getState());
    TEST_ASSERT_TRUE(mockCan.tx_log.size() > tx_before);
}

void test_retry_reconnect_success() {
    getToActive(gimbal, mockCan, 100);
    uint32_t t = 100 + TELEMETRY_TIMEOUT_MS + 1;
    gimbal.update(t);
    TEST_ASSERT_EQUAL(GimbalState::ERROR, gimbal.getState());

    t += RETRY_INTERVAL_MS + 1;
    gimbal.update(t);
    TEST_ASSERT_EQUAL(GimbalState::CONNECTING, gimbal.getState());

    // Send telemetry to reconnect
    uint8_t frame[64];
    size_t len = buildTelemetryPush(10.0f, 0.0f, 0.0f, 0x0002, frame);
    mockCan.enqueueSdkFrame(frame, len);
    gimbal.update(t + 50);
    TEST_ASSERT_EQUAL(GimbalState::ACTIVE, gimbal.getState());
}

void test_max_retries_to_disconnected() {
    gimbal.begin(&mockCan);
    gimbal.update(0);

    uint32_t t = CONNECT_TIMEOUT_MS + 1;
    gimbal.update(t);
    TEST_ASSERT_EQUAL(GimbalState::ERROR, gimbal.getState());

    for (uint32_t i = 0; i <= MAX_RETRIES; i++) {
        t += RETRY_INTERVAL_MS + 1;
        gimbal.update(t);
        if (gimbal.getState() == GimbalState::DISCONNECTED) break;
        if (gimbal.getState() == GimbalState::CONNECTING) {
            t += CONNECT_TIMEOUT_MS + 1;
            gimbal.update(t);
        }
    }
    TEST_ASSERT_EQUAL(GimbalState::DISCONNECTED, gimbal.getState());
}

void test_end_transitions_to_disconnected() {
    getToActive(gimbal, mockCan);
    gimbal.end();
    TEST_ASSERT_EQUAL(GimbalState::DISCONNECTED, gimbal.getState());
}

void test_time_since_telemetry() {
    gimbal.begin(&mockCan);
    TEST_ASSERT_EQUAL(UINT32_MAX, gimbal.getTimeSinceLastTelemetry(0));

    uint8_t frame[64];
    size_t len = buildTelemetryPush(0.0f, 0.0f, 0.0f, 0x0001, frame);
    mockCan.enqueueSdkFrame(frame, len);
    gimbal.update(1000);

    TEST_ASSERT_EQUAL(0, gimbal.getTimeSinceLastTelemetry(1000));
    TEST_ASSERT_EQUAL(250, gimbal.getTimeSinceLastTelemetry(1250));
}

// ============================================================================
// Command Tests
// ============================================================================

void test_commands_not_connected() {
    gimbal.begin(&mockCan);
    TEST_ASSERT_EQUAL(CommandResult::NOT_CONNECTED, gimbal.setPosition(0, 0, 0, 1000));
    TEST_ASSERT_EQUAL(CommandResult::NOT_CONNECTED, gimbal.cameraControl(CameraAction::SHUTTER));
    TEST_ASSERT_EQUAL(CommandResult::NOT_CONNECTED, gimbal.setSpeed(10, 0, 0));
}

void test_set_position_payload() {
    getToActive(gimbal, mockCan);
    mockCan.tx_log.clear();
    TEST_ASSERT_EQUAL(CommandResult::SUCCESS, gimbal.setPosition(45.0f, -10.0f, 5.0f, 2000));

    auto sent = mockCan.reconstructLastSdkFrame();
    TEST_ASSERT_TRUE(validateFrame(sent.data(), sent.size()));

    uint8_t cmd_type; uint16_t seq; uint8_t cmd_set, cmd_id;
    const uint8_t* payload; size_t payload_len;
    parseFrame(sent.data(), sent.size(), cmd_type, seq, cmd_set, cmd_id, payload, payload_len);

    TEST_ASSERT_EQUAL_HEX8(CMD_SET_GIMBAL, cmd_set);
    TEST_ASSERT_EQUAL_HEX8(CMD_ID_POSITION_CONTROL, cmd_id);
    TEST_ASSERT_EQUAL(8, payload_len);

    int16_t yaw = (int16_t)(payload[0] | (payload[1] << 8));
    int16_t roll = (int16_t)(payload[2] | (payload[3] << 8));
    int16_t pitch = (int16_t)(payload[4] | (payload[5] << 8));
    TEST_ASSERT_EQUAL(450, yaw);
    TEST_ASSERT_EQUAL(50, roll);
    TEST_ASSERT_EQUAL(-100, pitch);
    TEST_ASSERT_EQUAL_HEX8(0x01, payload[6]);  // absolute, all valid
    TEST_ASSERT_EQUAL(20, payload[7]);          // 2000ms / 100
}

void test_set_pan_tilt_roll_ignore() {
    getToActive(gimbal, mockCan);
    mockCan.tx_log.clear();
    gimbal.setPanTilt(90.0f, -20.0f, 1000);

    auto sent = mockCan.reconstructLastSdkFrame();
    uint8_t cmd_type; uint16_t seq; uint8_t cmd_set, cmd_id;
    const uint8_t* payload; size_t payload_len;
    parseFrame(sent.data(), sent.size(), cmd_type, seq, cmd_set, cmd_id, payload, payload_len);

    TEST_ASSERT_EQUAL_HEX8(0x05, payload[6]);  // absolute + roll invalid
}

void test_camera_shutter() {
    getToActive(gimbal, mockCan);
    mockCan.tx_log.clear();
    gimbal.cameraControl(CameraAction::SHUTTER);

    auto sent = mockCan.reconstructLastSdkFrame();
    uint8_t cmd_type; uint16_t seq; uint8_t cmd_set, cmd_id;
    const uint8_t* payload; size_t payload_len;
    parseFrame(sent.data(), sent.size(), cmd_type, seq, cmd_set, cmd_id, payload, payload_len);

    TEST_ASSERT_EQUAL_HEX8(CMD_SET_CAMERA, cmd_set);
    TEST_ASSERT_EQUAL_HEX8(CMD_ID_CAMERA_CONTROL, cmd_id);
    TEST_ASSERT_EQUAL(2, payload_len);
    TEST_ASSERT_EQUAL_HEX8(0x01, payload[0]);  // SHUTTER = 0x0001 LE low byte
    TEST_ASSERT_EQUAL_HEX8(0x00, payload[1]);  // SHUTTER = 0x0001 LE high byte
}

void test_set_speed_payload() {
    getToActive(gimbal, mockCan);
    mockCan.tx_log.clear();
    gimbal.setSpeed(30.0f, -15.0f, 0.0f);

    auto sent = mockCan.reconstructLastSdkFrame();
    uint8_t cmd_type; uint16_t seq; uint8_t cmd_set, cmd_id;
    const uint8_t* payload; size_t payload_len;
    parseFrame(sent.data(), sent.size(), cmd_type, seq, cmd_set, cmd_id, payload, payload_len);

    TEST_ASSERT_EQUAL_HEX8(CMD_ID_SPEED_CONTROL, cmd_id);
    int16_t yaw_speed = (int16_t)(payload[0] | (payload[1] << 8));
    int16_t pitch_speed = (int16_t)(payload[4] | (payload[5] << 8));
    TEST_ASSERT_EQUAL(300, yaw_speed);
    TEST_ASSERT_EQUAL(-150, pitch_speed);
    TEST_ASSERT_EQUAL_HEX8(0x80, payload[6]);
}

void test_stop_speed_release() {
    getToActive(gimbal, mockCan);
    mockCan.tx_log.clear();
    gimbal.stopSpeed();

    auto sent = mockCan.reconstructLastSdkFrame();
    uint8_t cmd_type; uint16_t seq; uint8_t cmd_set, cmd_id;
    const uint8_t* payload; size_t payload_len;
    parseFrame(sent.data(), sent.size(), cmd_type, seq, cmd_set, cmd_id, payload, payload_len);

    TEST_ASSERT_EQUAL_HEX8(0x00, payload[6]);  // release
}

void test_set_angle_limits() {
    getToActive(gimbal, mockCan);
    mockCan.tx_log.clear();
    gimbal.setAngleLimits(145, 55, 179, 179, 30, 30);

    auto sent = mockCan.reconstructLastSdkFrame();
    uint8_t cmd_type; uint16_t seq; uint8_t cmd_set, cmd_id;
    const uint8_t* payload; size_t payload_len;
    parseFrame(sent.data(), sent.size(), cmd_type, seq, cmd_set, cmd_id, payload, payload_len);

    TEST_ASSERT_EQUAL_HEX8(CMD_ID_SET_ANGLE_LIMIT, cmd_id);
    TEST_ASSERT_EQUAL(7, payload_len);
    TEST_ASSERT_EQUAL_HEX8(0x01, payload[0]);
    TEST_ASSERT_EQUAL(145, payload[1]);
    TEST_ASSERT_EQUAL(55, payload[2]);
}

void test_set_motor_stiffness() {
    getToActive(gimbal, mockCan);
    mockCan.tx_log.clear();
    gimbal.setMotorStiffness(80, 70, 60);

    auto sent = mockCan.reconstructLastSdkFrame();
    uint8_t cmd_type; uint16_t seq; uint8_t cmd_set, cmd_id;
    const uint8_t* payload; size_t payload_len;
    parseFrame(sent.data(), sent.size(), cmd_type, seq, cmd_set, cmd_id, payload, payload_len);

    TEST_ASSERT_EQUAL_HEX8(CMD_ID_SET_MOTOR_STIFFNESS, cmd_id);
    TEST_ASSERT_EQUAL_HEX8(0x01, payload[0]);
    TEST_ASSERT_EQUAL(80, payload[1]);
    TEST_ASSERT_EQUAL(60, payload[2]);
    TEST_ASSERT_EQUAL(70, payload[3]);
}

// ============================================================================
// Rate Limiting Tests
// ============================================================================

void test_position_rate_limit_rejects_fast_calls() {
    getToActive(gimbal, mockCan, 100);

    // First call at t=100 succeeds (last_position_cmd_ms=0, 100-0 >= 100)
    TEST_ASSERT_EQUAL(CommandResult::SUCCESS, gimbal.setPosition(10.0f, 0.0f, 0.0f, 1000));

    // Immediate second call at t=100 fails (0ms elapsed < 100ms minimum)
    TEST_ASSERT_EQUAL(CommandResult::EXECUTION_FAILED, gimbal.setPosition(20.0f, 0.0f, 0.0f, 1000));
}

void test_position_rate_limit_allows_after_interval() {
    getToActive(gimbal, mockCan, 100);

    TEST_ASSERT_EQUAL(CommandResult::SUCCESS, gimbal.setPosition(10.0f, 0.0f, 0.0f, 1000));

    // Advance time past the minimum interval
    gimbal.update(200);  // 100ms later
    TEST_ASSERT_EQUAL(CommandResult::SUCCESS, gimbal.setPosition(20.0f, 0.0f, 0.0f, 1000));
}

void test_pan_tilt_shares_position_rate_limit() {
    getToActive(gimbal, mockCan, 100);

    TEST_ASSERT_EQUAL(CommandResult::SUCCESS, gimbal.setPosition(10.0f, 0.0f, 0.0f, 1000));

    // setPanTilt also sends a position command — should be rate-limited
    TEST_ASSERT_EQUAL(CommandResult::EXECUTION_FAILED, gimbal.setPanTilt(20.0f, 0.0f, 1000));

    // After interval, setPanTilt works
    gimbal.update(200);
    TEST_ASSERT_EQUAL(CommandResult::SUCCESS, gimbal.setPanTilt(20.0f, 0.0f, 1000));
}

// ============================================================================
// CAN Segmentation Tests
// ============================================================================

void test_enable_push_segmentation() {
    gimbal.begin(&mockCan);
    // 19 bytes = 3 CAN frames (8+8+3)
    TEST_ASSERT_EQUAL(3, mockCan.tx_log.size());
    TEST_ASSERT_EQUAL(8, mockCan.tx_log[0].length);
    TEST_ASSERT_EQUAL(8, mockCan.tx_log[1].length);
    TEST_ASSERT_EQUAL(3, mockCan.tx_log[2].length);
    TEST_ASSERT_EQUAL(19, mockCan.totalBytesSent());
}

// ============================================================================
// Reassembly Tests
// ============================================================================

void test_partial_frame_no_crash() {
    gimbal.begin(&mockCan);
    uint8_t frame[64];
    uint8_t payload[22];
    memset(payload, 0, sizeof(payload));
    payload[0] = 0x01;
    (void)buildFrame(0x00, CMD_SET_GIMBAL, CMD_ID_PUSH_DATA,
                     payload, 22, 0x0001, frame);

    // Only enqueue 2 of 5 CAN frames
    MockCanBus::CanFrame f1, f2;
    f1.id = CAN_RX_ID; f1.length = 8; memcpy(f1.data, &frame[0], 8);
    f2.id = CAN_RX_ID; f2.length = 8; memcpy(f2.data, &frame[8], 8);
    mockCan.rx_queue.push(f1);
    mockCan.rx_queue.push(f2);

    gimbal.update(100);
    TEST_ASSERT_EQUAL(GimbalState::CONNECTING, gimbal.getState());
}

void test_corrupted_frame_rejected() {
    gimbal.begin(&mockCan);
    uint8_t payload[22];
    memset(payload, 0, sizeof(payload));
    payload[0] = 0x01;
    uint8_t frame[64];
    size_t len = buildFrame(0x00, CMD_SET_GIMBAL, CMD_ID_PUSH_DATA,
                            payload, 22, 0x0001, frame);
    frame[15] ^= 0xFF;

    mockCan.enqueueSdkFrame(frame, len);
    gimbal.update(100);
    TEST_ASSERT_EQUAL(GimbalState::CONNECTING, gimbal.getState());
}

// ============================================================================
// Main
// ============================================================================

int main() {
    UNITY_BEGIN();

    // CRC
    RUN_TEST(test_crc16_sdk_example);
    RUN_TEST(test_crc16_empty);
    RUN_TEST(test_crc32_sdk_example);
    RUN_TEST(test_crc32_empty);

    // Frame building
    RUN_TEST(test_build_position_command);
    RUN_TEST(test_build_enable_push);
    RUN_TEST(test_build_zero_payload);
    RUN_TEST(test_build_rejects_oversized);

    // Frame validation
    RUN_TEST(test_validate_valid_frame);
    RUN_TEST(test_validate_corrupt_crc16);
    RUN_TEST(test_validate_corrupt_crc32);
    RUN_TEST(test_validate_too_short);

    // Frame parsing
    RUN_TEST(test_parse_extracts_fields);

    // extractFrameLength
    RUN_TEST(test_extract_length_valid);
    RUN_TEST(test_extract_length_large);
    RUN_TEST(test_extract_length_invalid_sof);
    RUN_TEST(test_extract_length_too_small);

    // Sequence counter
    RUN_TEST(test_seq_starts_at_min);
    RUN_TEST(test_seq_increments);
    RUN_TEST(test_seq_wraps);
    RUN_TEST(test_seq_reset);

    // State machine
    RUN_TEST(test_initial_state_disconnected);
    RUN_TEST(test_begin_transitions_to_connecting);
    RUN_TEST(test_begin_sends_enable_push);
    RUN_TEST(test_telemetry_transitions_to_active);
    RUN_TEST(test_attitude_values_parsed);
    RUN_TEST(test_telemetry_limits_and_stiffness);
    RUN_TEST(test_connect_timeout_to_error);
    RUN_TEST(test_telemetry_timeout_from_active);
    RUN_TEST(test_retry_after_error);
    RUN_TEST(test_retry_reconnect_success);
    RUN_TEST(test_max_retries_to_disconnected);
    RUN_TEST(test_end_transitions_to_disconnected);
    RUN_TEST(test_time_since_telemetry);

    // Commands
    RUN_TEST(test_commands_not_connected);
    RUN_TEST(test_set_position_payload);
    RUN_TEST(test_set_pan_tilt_roll_ignore);
    RUN_TEST(test_camera_shutter);
    RUN_TEST(test_set_speed_payload);
    RUN_TEST(test_stop_speed_release);
    RUN_TEST(test_set_angle_limits);
    RUN_TEST(test_set_motor_stiffness);

    // Rate limiting
    RUN_TEST(test_position_rate_limit_rejects_fast_calls);
    RUN_TEST(test_position_rate_limit_allows_after_interval);
    RUN_TEST(test_pan_tilt_shares_position_rate_limit);

    // CAN segmentation
    RUN_TEST(test_enable_push_segmentation);

    // Reassembly
    RUN_TEST(test_partial_frame_no_crash);
    RUN_TEST(test_corrupted_frame_rejected);

    return UNITY_END();
}
