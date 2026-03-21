#pragma once
#include <cstdint>
#include <cstddef>

namespace dji {

// --- Enums ---

enum class GimbalState : uint8_t {
    DISCONNECTED,
    CONNECTING,
    ACTIVE,
    ERROR
};

enum class CommandResult : uint8_t {
    SUCCESS          = 0x00,
    PARSE_ERROR      = 0x01,
    EXECUTION_FAILED = 0x02,
    UNDEFINED_ERROR  = 0xFF,
    TIMEOUT          = 0xFE,
    NOT_CONNECTED    = 0xFD
};

enum class CameraAction : uint8_t {
    SHUTTER        = 0x01,
    STOP_SHUTTER   = 0x02,
    START_RECORD   = 0x03,
    STOP_RECORD    = 0x04,
    CENTER_FOCUS   = 0x05,
    END_FOCUS      = 0x0B
};

// --- Data Structs ---

struct Attitude {
    float yaw;
    float roll;
    float pitch;
};

struct GimbalTelemetry {
    bool     angles_valid;
    Attitude attitude;
    Attitude joint;

    bool     limits_valid;
    uint8_t  pitch_max;
    uint8_t  pitch_min;
    uint8_t  yaw_max;
    uint8_t  yaw_min;
    uint8_t  roll_max;
    uint8_t  roll_min;

    bool     stiffness_valid;
    uint8_t  pitch_stiffness;
    uint8_t  yaw_stiffness;
    uint8_t  roll_stiffness;
};

// --- Callback Types ---

using StateChangeCallback = void (*)(GimbalState old_state, GimbalState new_state);
using AttitudeCallback    = void (*)(const Attitude& attitude);
using TelemetryCallback   = void (*)(const GimbalTelemetry& telemetry);

// --- Constants ---

static constexpr uint32_t CAN_TX_ID = 0x223;
static constexpr uint32_t CAN_RX_ID = 0x222;
static constexpr uint32_t CAN_BAUD_RATE = 1000000;

static constexpr size_t MAX_FRAME_SIZE = 256;
static constexpr size_t CAN_FRAME_MAX_DATA = 8;

// Timing constants
static constexpr uint32_t CONNECT_TIMEOUT_MS           = 3000;
static constexpr uint32_t TELEMETRY_TIMEOUT_MS          = 500;
static constexpr uint32_t RETRY_INTERVAL_MS             = 2000;
static constexpr uint32_t MAX_RETRIES                   = 5;
static constexpr uint32_t MIN_POSITION_CMD_INTERVAL_MS  = 100;
static constexpr uint32_t SPEED_CMD_REFRESH_MS          = 400;

// Protocol constants
static constexpr uint8_t  SOF_BYTE    = 0xAA;
static constexpr uint8_t  CMD_TYPE_CMD_REPLY_REQUIRED = 0x03;
static constexpr uint8_t  CMD_TYPE_REPLY              = 0x20;
static constexpr uint8_t  CMD_SET_GIMBAL  = 0x0E;
static constexpr uint8_t  CMD_SET_CAMERA  = 0x0D;
static constexpr uint8_t  CMD_ID_POSITION_CONTROL  = 0x00;
static constexpr uint8_t  CMD_ID_SPEED_CONTROL     = 0x01;
static constexpr uint8_t  CMD_ID_GET_INFO          = 0x02;
static constexpr uint8_t  CMD_ID_SET_ANGLE_LIMIT   = 0x03;
static constexpr uint8_t  CMD_ID_GET_ANGLE_LIMIT   = 0x04;
static constexpr uint8_t  CMD_ID_SET_MOTOR_STIFFNESS = 0x05;
static constexpr uint8_t  CMD_ID_GET_MOTOR_STIFFNESS = 0x06;
static constexpr uint8_t  CMD_ID_SET_PUSH          = 0x07;
static constexpr uint8_t  CMD_ID_PUSH_DATA         = 0x08;
static constexpr uint8_t  CMD_ID_CAMERA_CONTROL    = 0x00;
static constexpr uint8_t  CMD_ID_FOCUS_CONTROL     = 0x12;

// Frame overhead: 12 (header) + 2 (CmdSet+CmdID) + 4 (CRC32) = 18
static constexpr size_t FRAME_OVERHEAD = 18;

// Sequence number range
static constexpr uint16_t SEQ_MIN = 0x0002;
static constexpr uint16_t SEQ_MAX = 0xFFFD;

}  // namespace dji
