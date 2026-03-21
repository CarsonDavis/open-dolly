#include "dji_gimbal.h"
#include "dji_protocol.h"
#include "dji_crc.h"
#include <cstring>

namespace dji {

// ---- Internal implementation ----

struct DjiGimbal::Impl {
    CanBusInterface* can = nullptr;
    GimbalState state = GimbalState::DISCONNECTED;
    SequenceCounter seq;

    // Callbacks
    StateChangeCallback on_state_change = nullptr;
    AttitudeCallback on_attitude = nullptr;
    TelemetryCallback on_telemetry = nullptr;

    // Latest telemetry
    GimbalTelemetry telemetry = {};

    // CAN frame reassembly buffer
    uint8_t reassembly_buf[MAX_FRAME_SIZE];
    size_t reassembly_len = 0;
    size_t reassembly_expected = 0;

    // Timing
    uint32_t now_ms = 0;  // updated every update() call
    uint32_t last_telemetry_ms = 0;
    bool ever_received_telemetry = false;
    uint32_t connect_start_ms = 0;
    bool connect_start_set = false;
    uint32_t last_retry_ms = 0;
    uint32_t retry_count = 0;
    uint32_t last_position_cmd_ms = 0;

    // Speed command auto-refresh
    bool speed_active = false;
    float speed_pan = 0;
    float speed_tilt = 0;
    float speed_roll = 0;
    uint32_t last_speed_cmd_ms = 0;

    // Reply correlation: track the last command we sent
    uint16_t last_sent_seq = 0;
    uint8_t last_sent_cmd_set = 0;
    uint8_t last_sent_cmd_id = 0;

    // Frame send buffer
    uint8_t tx_frame[MAX_FRAME_SIZE];

    void transitionTo(GimbalState new_state) {
        if (new_state == state) return;
        GimbalState old = state;
        state = new_state;
        if (on_state_change) {
            on_state_change(old, new_state);
        }
    }

    // Send an SDK frame segmented into CAN frames
    bool sendSdkFrame(const uint8_t* frame, size_t len) {
        if (!can) return false;
        size_t offset = 0;
        while (offset < len) {
            uint8_t chunk = static_cast<uint8_t>((len - offset > CAN_FRAME_MAX_DATA)
                                                  ? CAN_FRAME_MAX_DATA
                                                  : (len - offset));
            if (!can->send(CAN_TX_ID, &frame[offset], chunk)) {
                return false;
            }
            offset += chunk;
        }
        return true;
    }

    // Build and send a command frame
    CommandResult sendCommand(uint8_t cmd_set, uint8_t cmd_id,
                              const uint8_t* payload, size_t payload_len) {
        uint16_t s = seq.next();
        size_t len = buildFrame(CMD_TYPE_CMD_REPLY_REQUIRED,
                                cmd_set, cmd_id,
                                payload, payload_len,
                                s, tx_frame);
        if (len == 0) return CommandResult::UNDEFINED_ERROR;

        last_sent_seq = s;
        last_sent_cmd_set = cmd_set;
        last_sent_cmd_id = cmd_id;

        return sendSdkFrame(tx_frame, len)
            ? CommandResult::SUCCESS
            : CommandResult::UNDEFINED_ERROR;
    }

    // Send the enable-push command
    bool sendEnablePush() {
        uint8_t payload = 0x01;
        CommandResult r = sendCommand(CMD_SET_GIMBAL, CMD_ID_SET_PUSH,
                                      &payload, 1);
        return r == CommandResult::SUCCESS;
    }

    // Process a complete, validated SDK frame
    void processFrame(const uint8_t* frame, size_t len, uint32_t now_ms) {
        uint8_t cmd_type;
        uint16_t frame_seq;
        uint8_t cmd_set, cmd_id;
        const uint8_t* payload;
        size_t payload_len;

        if (!parseFrame(frame, len, cmd_type, frame_seq,
                        cmd_set, cmd_id, payload, payload_len)) {
            return;
        }

        if (cmd_type == CMD_TYPE_REPLY) {
            // Reply frame — for v2.2, reply frames do NOT contain CmdSet/CmdID,
            // so we correlate by sequence number using the last-sent command.
            // For v2.5, they do contain CmdSet/CmdID, but sequence correlation
            // still works as a universal approach.
            // Currently we don't do anything special with reply data beyond
            // confirming the command was received.
            return;
        }

        // Push data (telemetry)
        if (cmd_set == CMD_SET_GIMBAL && cmd_id == CMD_ID_PUSH_DATA) {
            parseTelemetryPush(payload, payload_len, now_ms);
            return;
        }
    }

    void parseTelemetryPush(const uint8_t* data, size_t len, uint32_t now_ms) {
        // Minimum telemetry payload: 1 (ctrl) + 12 (attitude+joint) = 13 bytes
        if (len < 13) return;

        uint8_t ctrl = data[0];

        telemetry.angles_valid = (ctrl & 0x01) != 0;
        telemetry.limits_valid = (ctrl & 0x02) != 0;
        telemetry.stiffness_valid = (ctrl & 0x04) != 0;

        if (telemetry.angles_valid) {
            // Attitude angles: int16_t LE, unit 0.1 deg
            int16_t yaw_att  = static_cast<int16_t>(data[1]  | (data[2]  << 8));
            int16_t roll_att = static_cast<int16_t>(data[3]  | (data[4]  << 8));
            int16_t pit_att  = static_cast<int16_t>(data[5]  | (data[6]  << 8));
            int16_t yaw_jnt  = static_cast<int16_t>(data[7]  | (data[8]  << 8));
            int16_t roll_jnt = static_cast<int16_t>(data[9]  | (data[10] << 8));
            int16_t pit_jnt  = static_cast<int16_t>(data[11] | (data[12] << 8));

            telemetry.attitude.yaw   = yaw_att  * 0.1f;
            telemetry.attitude.roll  = roll_att * 0.1f;
            telemetry.attitude.pitch = pit_att  * 0.1f;
            telemetry.joint.yaw      = yaw_jnt  * 0.1f;
            telemetry.joint.roll     = roll_jnt * 0.1f;
            telemetry.joint.pitch    = pit_jnt  * 0.1f;
        }

        // Limits (bytes 13-18) if present
        if (telemetry.limits_valid && len >= 19) {
            telemetry.pitch_max = data[13];
            telemetry.pitch_min = data[14];
            telemetry.yaw_max   = data[15];
            telemetry.yaw_min   = data[16];
            telemetry.roll_max  = data[17];
            telemetry.roll_min  = data[18];
        }

        // Stiffness (bytes 19-21) if present
        if (telemetry.stiffness_valid && len >= 22) {
            telemetry.pitch_stiffness = data[19];
            telemetry.yaw_stiffness   = data[20];
            telemetry.roll_stiffness  = data[21];
        }

        last_telemetry_ms = now_ms;
        ever_received_telemetry = true;

        // State transition: CONNECTING -> ACTIVE on first telemetry
        if (state == GimbalState::CONNECTING) {
            transitionTo(GimbalState::ACTIVE);
        }

        // Invoke callbacks
        if (on_attitude && telemetry.angles_valid) {
            on_attitude(telemetry.attitude);
        }
        if (on_telemetry) {
            on_telemetry(telemetry);
        }
    }

    // Read all available CAN frames and reassemble SDK packets
    void readCanFrames(uint32_t now_ms) {
        uint32_t rx_id;
        uint8_t rx_data[CAN_FRAME_MAX_DATA];
        uint8_t rx_len;

        // Drain all available frames
        while (can->receive(rx_id, rx_data, rx_len, 0)) {
            if (rx_id != CAN_RX_ID) continue;

            // Append to reassembly buffer
            for (uint8_t i = 0; i < rx_len && reassembly_len < MAX_FRAME_SIZE; i++) {
                reassembly_buf[reassembly_len++] = rx_data[i];
            }

            // Try to determine expected length once we have the first 3 bytes
            if (reassembly_expected == 0 && reassembly_len >= 3) {
                reassembly_expected = extractFrameLength(reassembly_buf);
                if (reassembly_expected == 0) {
                    // Invalid header — reset
                    reassembly_len = 0;
                    continue;
                }
            }

            // Check if we have a complete frame
            if (reassembly_expected > 0 && reassembly_len >= reassembly_expected) {
                if (validateFrame(reassembly_buf, reassembly_expected)) {
                    processFrame(reassembly_buf, reassembly_expected, now_ms);
                }
                // Reset for next frame
                // If there are leftover bytes beyond the expected frame, shift them
                size_t leftover = reassembly_len - reassembly_expected;
                if (leftover > 0) {
                    memmove(reassembly_buf, &reassembly_buf[reassembly_expected], leftover);
                }
                reassembly_len = leftover;
                reassembly_expected = 0;
            }
        }
    }

    void checkTimers(uint32_t now_ms) {
        switch (state) {
            case GimbalState::CONNECTING:
                if (connect_start_set &&
                    now_ms - connect_start_ms >= CONNECT_TIMEOUT_MS) {
                    transitionTo(GimbalState::ERROR);
                    last_retry_ms = now_ms;
                    // Don't reset retry_count — preserve it across retry cycles
                }
                break;

            case GimbalState::ACTIVE:
                if (ever_received_telemetry &&
                    now_ms - last_telemetry_ms >= TELEMETRY_TIMEOUT_MS) {
                    transitionTo(GimbalState::ERROR);
                    last_retry_ms = now_ms;
                    retry_count = 0;  // fresh error from ACTIVE, start retry count
                }
                // Auto-refresh speed commands
                if (speed_active &&
                    now_ms - last_speed_cmd_ms >= SPEED_CMD_REFRESH_MS) {
                    sendSpeedCommand(speed_pan, speed_tilt, speed_roll, now_ms);
                }
                break;

            case GimbalState::ERROR:
                if (now_ms - last_retry_ms >= RETRY_INTERVAL_MS) {
                    retry_count++;
                    if (retry_count > MAX_RETRIES) {
                        transitionTo(GimbalState::DISCONNECTED);
                    } else {
                        // Retry: re-send enable-push and go back to CONNECTING
                        if (sendEnablePush()) {
                            connect_start_ms = now_ms;
                            connect_start_set = true;
                            transitionTo(GimbalState::CONNECTING);
                        } else {
                            last_retry_ms = now_ms;
                        }
                    }
                }
                break;

            default:
                break;
        }
    }

    // Low-level speed command send
    void sendSpeedCommand(float pan, float tilt, float roll, uint32_t now_ms) {
        int16_t yaw_raw   = static_cast<int16_t>(pan  * 10.0f);
        int16_t roll_raw  = static_cast<int16_t>(roll * 10.0f);
        int16_t pitch_raw = static_cast<int16_t>(tilt * 10.0f);
        uint8_t ctrl = 0x80;  // take over speed control

        uint8_t payload[7];
        payload[0] = static_cast<uint8_t>(yaw_raw & 0xFF);
        payload[1] = static_cast<uint8_t>((yaw_raw >> 8) & 0xFF);
        payload[2] = static_cast<uint8_t>(roll_raw & 0xFF);
        payload[3] = static_cast<uint8_t>((roll_raw >> 8) & 0xFF);
        payload[4] = static_cast<uint8_t>(pitch_raw & 0xFF);
        payload[5] = static_cast<uint8_t>((pitch_raw >> 8) & 0xFF);
        payload[6] = ctrl;

        sendCommand(CMD_SET_GIMBAL, CMD_ID_SPEED_CONTROL, payload, sizeof(payload));
        last_speed_cmd_ms = now_ms;
    }
};

// ---- Public API ----

DjiGimbal::DjiGimbal() : _impl(new Impl()) {}

DjiGimbal::~DjiGimbal() {
    delete _impl;
}

bool DjiGimbal::begin(CanBusInterface* can) {
    if (!can) return false;

    _impl->can = can;
    _impl->seq.reset();
    _impl->reassembly_len = 0;
    _impl->reassembly_expected = 0;
    _impl->now_ms = 0;
    _impl->last_telemetry_ms = 0;
    _impl->ever_received_telemetry = false;
    _impl->last_position_cmd_ms = 0;
    _impl->speed_active = false;
    _impl->speed_pan = 0;
    _impl->speed_tilt = 0;
    _impl->speed_roll = 0;
    _impl->last_speed_cmd_ms = 0;
    _impl->retry_count = 0;
    memset(&_impl->telemetry, 0, sizeof(GimbalTelemetry));

    // Send enable-push to start the handshake.
    // We don't know now_ms yet (it comes via update()), so we defer
    // the connect timeout start to the first update() call after CONNECTING.
    if (!_impl->sendEnablePush()) {
        return false;
    }

    _impl->connect_start_set = false;  // will be set on first update()
    _impl->transitionTo(GimbalState::CONNECTING);
    return true;
}

void DjiGimbal::update(uint32_t now_ms) {
    if (_impl->state == GimbalState::DISCONNECTED) return;

    _impl->now_ms = now_ms;

    // On the very first update() after begin(), record the connect start time
    if (_impl->state == GimbalState::CONNECTING && !_impl->connect_start_set) {
        _impl->connect_start_ms = now_ms;
        _impl->connect_start_set = true;
    }

    _impl->readCanFrames(now_ms);
    _impl->checkTimers(now_ms);
}

void DjiGimbal::end() {
    _impl->speed_active = false;
    _impl->transitionTo(GimbalState::DISCONNECTED);
    _impl->can = nullptr;
}

GimbalState DjiGimbal::getState() const {
    return _impl->state;
}

Attitude DjiGimbal::getAttitude() const {
    return _impl->telemetry.attitude;
}

Attitude DjiGimbal::getJointAngles() const {
    return _impl->telemetry.joint;
}

GimbalTelemetry DjiGimbal::getTelemetry() const {
    return _impl->telemetry;
}

uint32_t DjiGimbal::getTimeSinceLastTelemetry(uint32_t now_ms) const {
    if (!_impl->ever_received_telemetry) return UINT32_MAX;
    return now_ms - _impl->last_telemetry_ms;
}

// --- Phase 1 commands ---

CommandResult DjiGimbal::setPosition(float pan_deg, float tilt_deg, float roll_deg,
                                     uint16_t duration_ms) {
    if (_impl->state != GimbalState::ACTIVE) {
        return CommandResult::NOT_CONNECTED;
    }

    // Enforce minimum interval between position commands (DJI SDK spec: 100ms)
    if (_impl->now_ms - _impl->last_position_cmd_ms < MIN_POSITION_CMD_INTERVAL_MS) {
        return CommandResult::EXECUTION_FAILED;
    }

    // Angles in 0.1 degree units
    int16_t yaw_raw   = static_cast<int16_t>(pan_deg  * 10.0f);
    int16_t roll_raw  = static_cast<int16_t>(roll_deg * 10.0f);
    int16_t pitch_raw = static_cast<int16_t>(tilt_deg * 10.0f);

    // ctrl_byte: bit 0 = 1 (absolute), bits 1-3 = 0 (all axes valid)
    uint8_t ctrl = 0x01;

    // time_for_action in 0.1 second units, clamped to uint8_t (max 25.5s)
    uint8_t time_action = 0;
    if (duration_ms >= 100) {
        uint32_t tenths = duration_ms / 100;
        time_action = static_cast<uint8_t>(tenths > 255 ? 255 : tenths);
    } else {
        time_action = 1;  // minimum
    }

    uint8_t payload[8];
    payload[0] = static_cast<uint8_t>(yaw_raw & 0xFF);
    payload[1] = static_cast<uint8_t>((yaw_raw >> 8) & 0xFF);
    payload[2] = static_cast<uint8_t>(roll_raw & 0xFF);
    payload[3] = static_cast<uint8_t>((roll_raw >> 8) & 0xFF);
    payload[4] = static_cast<uint8_t>(pitch_raw & 0xFF);
    payload[5] = static_cast<uint8_t>((pitch_raw >> 8) & 0xFF);
    payload[6] = ctrl;
    payload[7] = time_action;

    CommandResult r = _impl->sendCommand(CMD_SET_GIMBAL, CMD_ID_POSITION_CONTROL,
                                          payload, sizeof(payload));
    if (r == CommandResult::SUCCESS) {
        _impl->last_position_cmd_ms = _impl->now_ms;
    }
    return r;
}

CommandResult DjiGimbal::setPanTilt(float pan_deg, float tilt_deg, uint16_t duration_ms) {
    if (_impl->state != GimbalState::ACTIVE) {
        return CommandResult::NOT_CONNECTED;
    }

    if (_impl->now_ms - _impl->last_position_cmd_ms < MIN_POSITION_CMD_INTERVAL_MS) {
        return CommandResult::EXECUTION_FAILED;
    }

    int16_t yaw_raw   = static_cast<int16_t>(pan_deg  * 10.0f);
    int16_t pitch_raw = static_cast<int16_t>(tilt_deg * 10.0f);

    // ctrl_byte: bit 0 = 1 (absolute), bit 2 = 1 (roll invalid/ignore)
    uint8_t ctrl = 0x01 | 0x04;

    uint8_t time_action = 0;
    if (duration_ms >= 100) {
        uint32_t tenths = duration_ms / 100;
        time_action = static_cast<uint8_t>(tenths > 255 ? 255 : tenths);
    } else {
        time_action = 1;
    }

    uint8_t payload[8];
    payload[0] = static_cast<uint8_t>(yaw_raw & 0xFF);
    payload[1] = static_cast<uint8_t>((yaw_raw >> 8) & 0xFF);
    payload[2] = 0x00;  // roll ignored
    payload[3] = 0x00;
    payload[4] = static_cast<uint8_t>(pitch_raw & 0xFF);
    payload[5] = static_cast<uint8_t>((pitch_raw >> 8) & 0xFF);
    payload[6] = ctrl;
    payload[7] = time_action;

    CommandResult r = _impl->sendCommand(CMD_SET_GIMBAL, CMD_ID_POSITION_CONTROL,
                                          payload, sizeof(payload));
    if (r == CommandResult::SUCCESS) {
        _impl->last_position_cmd_ms = _impl->now_ms;
    }
    return r;
}

CommandResult DjiGimbal::cameraControl(CameraAction action) {
    if (_impl->state != GimbalState::ACTIVE) {
        return CommandResult::NOT_CONNECTED;
    }

    uint16_t action_val = static_cast<uint16_t>(action);
    uint8_t payload[2] = {
        static_cast<uint8_t>(action_val & 0xFF),
        static_cast<uint8_t>((action_val >> 8) & 0xFF)
    };
    return _impl->sendCommand(CMD_SET_CAMERA, CMD_ID_CAMERA_CONTROL,
                              payload, 2);
}

// --- Phase 2 commands ---

CommandResult DjiGimbal::setSpeed(float pan_speed, float tilt_speed, float roll_speed) {
    if (_impl->state != GimbalState::ACTIVE) {
        return CommandResult::NOT_CONNECTED;
    }

    _impl->speed_active = true;
    _impl->speed_pan = pan_speed;
    _impl->speed_tilt = tilt_speed;
    _impl->speed_roll = roll_speed;

    // Send immediately — auto-refresh handled in checkTimers()
    _impl->sendSpeedCommand(pan_speed, tilt_speed, roll_speed, _impl->now_ms);
    return CommandResult::SUCCESS;
}

CommandResult DjiGimbal::stopSpeed() {
    if (_impl->state != GimbalState::ACTIVE) {
        return CommandResult::NOT_CONNECTED;
    }

    _impl->speed_active = false;

    // Send zero speed with release bit (ctrl_byte bit 7 = 0)
    uint8_t payload[7] = {0, 0, 0, 0, 0, 0, 0x00};
    return _impl->sendCommand(CMD_SET_GIMBAL, CMD_ID_SPEED_CONTROL,
                              payload, sizeof(payload));
}

CommandResult DjiGimbal::requestInfo() {
    if (_impl->state != GimbalState::ACTIVE) {
        return CommandResult::NOT_CONNECTED;
    }

    uint8_t payload = 0x01;  // get attitude angles
    return _impl->sendCommand(CMD_SET_GIMBAL, CMD_ID_GET_INFO,
                              &payload, 1);
}

// --- Phase 3 commands ---

CommandResult DjiGimbal::setAngleLimits(uint8_t pitch_max, uint8_t pitch_min,
                                        uint8_t yaw_max, uint8_t yaw_min,
                                        uint8_t roll_max, uint8_t roll_min) {
    if (_impl->state != GimbalState::ACTIVE) {
        return CommandResult::NOT_CONNECTED;
    }

    uint8_t payload[7];
    payload[0] = 0x01;       // ctrl_byte: set limits
    payload[1] = pitch_max;
    payload[2] = pitch_min;
    payload[3] = yaw_max;
    payload[4] = yaw_min;
    payload[5] = roll_max;
    payload[6] = roll_min;

    return _impl->sendCommand(CMD_SET_GIMBAL, CMD_ID_SET_ANGLE_LIMIT,
                              payload, sizeof(payload));
}

CommandResult DjiGimbal::setMotorStiffness(uint8_t pitch, uint8_t yaw, uint8_t roll) {
    if (_impl->state != GimbalState::ACTIVE) {
        return CommandResult::NOT_CONNECTED;
    }

    uint8_t payload[4];
    payload[0] = 0x01;    // ctrl_byte: set stiffness
    payload[1] = pitch;
    payload[2] = roll;
    payload[3] = yaw;

    return _impl->sendCommand(CMD_SET_GIMBAL, CMD_ID_SET_MOTOR_STIFFNESS,
                              payload, sizeof(payload));
}

// --- Callbacks ---

void DjiGimbal::onStateChange(StateChangeCallback cb) {
    _impl->on_state_change = cb;
}

void DjiGimbal::onAttitudeUpdate(AttitudeCallback cb) {
    _impl->on_attitude = cb;
}

void DjiGimbal::onTelemetryUpdate(TelemetryCallback cb) {
    _impl->on_telemetry = cb;
}

}  // namespace dji
