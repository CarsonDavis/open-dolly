#ifndef NATIVE_TEST

#include <Arduino.h>
#include <WiFi.h>
#include <DNSServer.h>
#include <ESPmDNS.h>
#include <LittleFS.h>
#include <esp_mac.h>
#include <esp_wifi.h>

#include "config.h"
#include "commands.h"
#include "state_machine.h"
#include "trajectory.h"
#include "stepper.h"
#include "display.h"
#include "telemetry.h"
#include "web_server.h"

#ifndef BUILD_SLIDER_ONLY
#include <dji_gimbal.h>
#include <dji_types.h>
#endif

// --- Globals ---

QueueHandle_t cmd_queue = nullptr;
QueueHandle_t telemetry_queue = nullptr;
DNSServer dnsServer;

// --- DJI CAN ESP32 TWAI implementation ---

#ifndef BUILD_SLIDER_ONLY

#include <driver/twai.h>
#include <can_bus_interface.h>

class TwaiCanBus : public dji::CanBusInterface {
public:
    bool send(uint32_t id, const uint8_t* data, uint8_t length) override {
        twai_message_t msg = {};
        msg.identifier = id;
        msg.data_length_code = length;
        memcpy(msg.data, data, length);
        return twai_transmit(&msg, pdMS_TO_TICKS(10)) == ESP_OK;
    }

    bool receive(uint32_t& id, uint8_t* data, uint8_t& length, uint32_t timeout_ms) override {
        twai_message_t msg;
        if (twai_receive(&msg, pdMS_TO_TICKS(timeout_ms)) == ESP_OK) {
            id = msg.identifier;
            length = msg.data_length_code;
            memcpy(data, msg.data, length);
            return true;
        }
        return false;
    }
};

static TwaiCanBus twaiCan;
static dji::DjiGimbal djiGimbal;

static void djiCanInit() {
    twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(
        (gpio_num_t)PIN_CAN_TX, (gpio_num_t)PIN_CAN_RX, TWAI_MODE_NORMAL);
    g_config.rx_queue_len = 32;

    twai_timing_config_t t_config = TWAI_TIMING_CONFIG_1MBITS();

    // Accept only CAN ID 0x222 (gimbal responses)
    twai_filter_config_t f_config = {
        .acceptance_code = (dji::CAN_RX_ID << 21),
        .acceptance_mask = ~(0x7FF << 21),
        .single_filter = true
    };

    esp_err_t err = twai_driver_install(&g_config, &t_config, &f_config);
    if (err != ESP_OK) {
        Serial.printf("TWAI install failed: %d\n", err);
        return;
    }

    err = twai_start();
    if (err != ESP_OK) {
        Serial.printf("TWAI start failed: %d\n", err);
        return;
    }

    Serial.println("TWAI (CAN) initialized at 1 Mbps");
}

// DJI CAN task — processes gimbal communication
static void djiCanTask(void* param) {
    djiGimbal.begin(&twaiCan);

    djiGimbal.onStateChange([](dji::GimbalState old_st, dji::GimbalState new_st) {
        Serial.printf("DJI gimbal: %d -> %d\n", (int)old_st, (int)new_st);
    });

    djiGimbal.onAttitudeUpdate([](const dji::Attitude& att) {
        xSemaphoreTake(g_state_mutex, portMAX_DELAY);
        // Update pan/tilt/roll positions from gimbal telemetry
        for (int i = 0; i < g_axis_count; i++) {
            if (g_axis_config[i].type == AxisType::DJI_CAN) {
                if (strcmp(g_axis_config[i].name, "pan") == 0)
                    g_state.positions[i] = att.yaw;
                else if (strcmp(g_axis_config[i].name, "tilt") == 0)
                    g_state.positions[i] = att.pitch;
                else if (strcmp(g_axis_config[i].name, "roll") == 0)
                    g_state.positions[i] = att.roll;
            }
        }
        xSemaphoreGive(g_state_mutex);
    });

    for (;;) {
        djiGimbal.update(millis());
        vTaskDelay(pdMS_TO_TICKS(5)); // ~200 Hz update rate
    }
}

// Set DJI gimbal target position
// duration_ms: how long the gimbal should take to reach the position
void setDjiTarget(const float* interpolated, uint32_t duration_ms) {
    if (djiGimbal.getState() != dji::GimbalState::ACTIVE) return;

    float pan = 0, tilt = 0, roll = 0;
    for (int i = 0; i < g_axis_count; i++) {
        if (g_axis_config[i].type == AxisType::DJI_CAN) {
            if (strcmp(g_axis_config[i].name, "pan") == 0) pan = interpolated[i];
            else if (strcmp(g_axis_config[i].name, "tilt") == 0) tilt = interpolated[i];
            else if (strcmp(g_axis_config[i].name, "roll") == 0) roll = interpolated[i];
        }
    }
    djiGimbal.setPosition(pan, tilt, roll, (uint16_t)duration_ms);
}

// Trigger DJI camera action
void triggerCameraEvent(uint8_t event_type) {
    if (djiGimbal.getState() != dji::GimbalState::ACTIVE) return;
    switch (event_type) {
        case 0: djiGimbal.cameraControl(dji::CameraAction::SHUTTER); break;
        case 1: djiGimbal.cameraControl(dji::CameraAction::START_RECORD); break;
        case 2: djiGimbal.cameraControl(dji::CameraAction::STOP_RECORD); break;
    }
}

#else
// Slider-only build: no DJI
void setDjiTarget(const float*, uint8_t) {}
void triggerCameraEvent(uint8_t) {}
#endif // BUILD_SLIDER_ONLY

// --- Forward declarations for jog/move_to helpers ---

void applyJogDelta(int axis_index, float delta);
void executeMoveToCmd(const float* targets, uint32_t duration_ms);
void executeHomeCmd(uint8_t axis_mask);
void stopAllMotion();
void startPlayback();
void pausePlayback();
void resumePlayback();
void stopPlayback();
void scrubToTime(uint32_t t_ms);

// --- Trajectory Walker ---

static hw_timer_t* walker_timer = nullptr;
static volatile bool walker_tick = false;
static uint16_t walker_index = 0;
static uint32_t playback_start_us = 0;
static uint32_t paused_elapsed_us = 0;
static uint16_t event_index = 0;

void IRAM_ATTR onWalkerTimer() {
    walker_tick = true;
}

void startPlayback() {
    playback_start_us = micros();
    walker_index = 0;
    event_index = 0;
    timerAlarmEnable(walker_timer);
}

void pausePlayback() {
    paused_elapsed_us = micros() - playback_start_us;
    timerAlarmDisable(walker_timer);
}

void resumePlayback() {
    playback_start_us = micros() - paused_elapsed_us;
    timerAlarmEnable(walker_timer);
}

void stopPlayback() {
    timerAlarmDisable(walker_timer);
    walker_index = 0;
    event_index = 0;
}

void scrubToTime(uint32_t t_ms) {
    if (g_trajectory.point_count < 2) return;

    // Binary search for the segment containing t_ms
    uint16_t lo = 0, hi = g_trajectory.point_count - 1;
    while (lo < hi - 1) {
        uint16_t mid = (lo + hi) / 2;
        if (g_trajectory.points[mid].t_ms <= t_ms) lo = mid;
        else hi = mid;
    }
    walker_index = lo;

    // Interpolate position at t_ms
    float interpolated[MAX_AXES];
    interpolateAtTime(t_ms, interpolated);

    // Update position
    setStepperTarget(interpolated[AXIS_SLIDE]);
    #ifndef BUILD_SLIDER_ONLY
    setDjiTarget(interpolated, 500);
    #endif

    // Update shared state
    xSemaphoreTake(g_state_mutex, portMAX_DELAY);
    memcpy((void*)g_state.positions, interpolated, sizeof(float) * g_axis_count);
    g_state.trajectory_elapsed_ms = t_ms;
    xSemaphoreGive(g_state_mutex);

    // If playing, adjust start time so playback continues from here
    if (g_state.current_state == State::PLAYING) {
        playback_start_us = micros() - (uint32_t)(t_ms * 1000UL);
    } else if (g_state.current_state == State::PAUSED) {
        paused_elapsed_us = t_ms * 1000UL;
    }

    // Push telemetry
    TelemetryPoint telem;
    telem.t_ms = t_ms;
    memcpy(telem.axes, interpolated, sizeof(float) * g_axis_count);
    xQueueSend(telemetry_queue, &telem, 0);
}

static void trajectoryWalkerTask(void* param) {
    static const uint8_t DJI_CMD_INTERVAL_TICKS = 10;
    static uint8_t dji_divider = 0;
    static uint8_t telemetry_divider = 0;

    for (;;) {
        if (!walker_tick) {
            vTaskDelay(1);
            continue;
        }
        walker_tick = false;

        State state;
        xSemaphoreTake(g_state_mutex, portMAX_DELAY);
        state = g_state.current_state;
        xSemaphoreGive(g_state_mutex);

        if (state != State::PLAYING) continue;

        uint32_t elapsed_us = micros() - playback_start_us;
        uint32_t elapsed_ms = elapsed_us / 1000;

        // Check for trajectory complete
        if (elapsed_ms >= g_trajectory.duration_ms) {
            if (g_trajectory.loop) {
                playback_start_us = micros();
                walker_index = 0;
                event_index = 0;
                elapsed_ms = 0;
            } else {
                // Broadcast complete event
                char buf[64];
                snprintf(buf, sizeof(buf), "{\"evt\":\"complete\",\"trajectory_id\":\"%s\"}",
                         g_trajectory.id);
                ws.textAll(buf, strlen(buf));

                Command stop_cmd = {};
                stop_cmd.type = CommandType::STOP;
                xQueueSend(cmd_queue, &stop_cmd, 0);
                continue;
            }
        }

        // Advance walker index
        while (walker_index < g_trajectory.point_count - 2 &&
               g_trajectory.points[walker_index + 1].t_ms <= elapsed_ms) {
            walker_index++;
        }

        // Linear interpolation
        TrajectoryPoint* p0 = &g_trajectory.points[walker_index];
        TrajectoryPoint* p1 = &g_trajectory.points[walker_index + 1];

        float t_frac = 0.0f;
        uint32_t dt = p1->t_ms - p0->t_ms;
        if (dt > 0) {
            t_frac = (float)(elapsed_ms - p0->t_ms) / (float)dt;
            if (t_frac > 1.0f) t_frac = 1.0f;
        }

        float interpolated[MAX_AXES];
        for (int a = 0; a < g_axis_count; a++) {
            interpolated[a] = p0->axes[a] + t_frac * (p1->axes[a] - p0->axes[a]);
        }

        // Send to stepper
        setStepperTarget(interpolated[AXIS_SLIDE]);

        // DJI gimbal at rate-limited interval
        #ifndef BUILD_SLIDER_ONLY
        dji_divider++;
        if (dji_divider >= DJI_CMD_INTERVAL_TICKS) {
            dji_divider = 0;
            // Give the gimbal enough time to reach the next position (~500ms between updates)
            setDjiTarget(interpolated, 500);
        }
        #endif

        // Check for camera trigger events
        if (g_trajectory.events && event_index < g_trajectory.event_count) {
            while (event_index < g_trajectory.event_count &&
                   g_trajectory.events[event_index].t_ms <= elapsed_ms) {
                triggerCameraEvent(g_trajectory.events[event_index].type);
                Serial.printf("Event fired at t=%lums: type=%d\n",
                              (unsigned long)elapsed_ms, g_trajectory.events[event_index].type);
                event_index++;
            }
        }

        // Telemetry at divided rate
        telemetry_divider++;
        if (telemetry_divider >= (100 / TELEMETRY_RATE_HZ)) {
            telemetry_divider = 0;
            TelemetryPoint telem;
            telem.t_ms = elapsed_ms;
            memcpy(telem.axes, interpolated, sizeof(float) * g_axis_count);
            xQueueSend(telemetry_queue, &telem, 0);
        }

        // Update shared state
        xSemaphoreTake(g_state_mutex, portMAX_DELAY);
        g_state.trajectory_elapsed_ms = elapsed_ms;
        memcpy((void*)g_state.positions, interpolated, sizeof(float) * g_axis_count);
        xSemaphoreGive(g_state_mutex);
    }
}

// --- Jog helpers ---

void applyJogDelta(int axis_index, float delta) {
    xSemaphoreTake(g_state_mutex, portMAX_DELAY);
    float current = g_state.positions[axis_index];
    float new_pos = current + delta;
    // Clamp to axis limits
    if (new_pos < g_axis_config[axis_index].min)
        new_pos = g_axis_config[axis_index].min;
    if (new_pos > g_axis_config[axis_index].max)
        new_pos = g_axis_config[axis_index].max;
    g_state.positions[axis_index] = new_pos;
    xSemaphoreGive(g_state_mutex);

    // Drive the appropriate motor
    if (g_axis_config[axis_index].type == AxisType::STEPPER) {
        setStepperTarget(new_pos);
    }
    #ifndef BUILD_SLIDER_ONLY
    else if (g_axis_config[axis_index].type == AxisType::DJI_CAN) {
        // Build current target from all DJI axes and send with 500ms duration
        float positions[MAX_AXES];
        xSemaphoreTake(g_state_mutex, portMAX_DELAY);
        memcpy(positions, (void*)g_state.positions, sizeof(float) * g_axis_count);
        xSemaphoreGive(g_state_mutex);
        setDjiTarget(positions, 500);
    }
    #endif

    // Push telemetry
    TelemetryPoint telem;
    telem.t_ms = 0;
    xSemaphoreTake(g_state_mutex, portMAX_DELAY);
    memcpy(telem.axes, (void*)g_state.positions, sizeof(float) * g_axis_count);
    xSemaphoreGive(g_state_mutex);
    xQueueSend(telemetry_queue, &telem, 0);
}

// --- Move-to implementation ---

static float move_to_start[MAX_AXES];
static float move_to_target[MAX_AXES];
static uint32_t move_to_duration_ms = 0;
static uint32_t move_to_start_ms = 0;

// Pre-roll: move to trajectory start before playing
static bool preroll_pending = false;
static uint32_t preroll_pause_start_ms = 0;
static const uint32_t PREROLL_PAUSE_MS = 1000;
static float preroll_tolerance_deg = 1.5f;  // close enough to start playing

void executeMoveToCmd(const float* targets, uint32_t duration_ms) {
    xSemaphoreTake(g_state_mutex, portMAX_DELAY);
    memcpy(move_to_start, (void*)g_state.positions, sizeof(float) * g_axis_count);
    xSemaphoreGive(g_state_mutex);

    for (int i = 0; i < g_axis_count; i++) {
        move_to_target[i] = targets[i];
        // Clamp to limits
        if (move_to_target[i] < g_axis_config[i].min)
            move_to_target[i] = g_axis_config[i].min;
        if (move_to_target[i] > g_axis_config[i].max)
            move_to_target[i] = g_axis_config[i].max;
    }
    move_to_duration_ms = duration_ms;
    move_to_start_ms = millis();

    // Send a single DJI position command with the full duration — let the
    // gimbal handle smooth motion internally instead of micro-updating.
    #ifndef BUILD_SLIDER_ONLY
    setDjiTarget(move_to_target, duration_ms);
    #endif
}

void executeHomeCmd(uint8_t axis_mask) {
    for (int i = 0; i < g_axis_count; i++) {
        if ((axis_mask & (1 << i)) && g_axis_config[i].can_home) {
            if (g_axis_config[i].type == AxisType::STEPPER) {
                stepperHome();
                xSemaphoreTake(g_state_mutex, portMAX_DELAY);
                g_state.positions[i] = 0.0f;
                xSemaphoreGive(g_state_mutex);
            }
        }
    }
}

void stopAllMotion() {
    stepperEmergencyStop();
    #ifndef BUILD_SLIDER_ONLY
    if (djiGimbal.getState() == dji::GimbalState::ACTIVE) {
        djiGimbal.stopSpeed();
    }
    #endif
}

// --- Motion Control Task ---

static void motionControlTask(void* param) {
    Command cmd;
    static uint8_t move_to_dji_divider = 0;

    for (;;) {
        // Check for limit switch hit (set by ISR)
        if (g_limit_hit) {
            g_limit_hit = false;
            changeState(State::ERROR, ErrorCode::LIMIT_HIT);
            stopAllMotion();
        }

        // Pre-roll settle pause — wait 1s after arriving at start, then play
        if (g_state.current_state == State::MOVING && preroll_pending && preroll_pause_start_ms > 0) {
            if (millis() - preroll_pause_start_ms >= PREROLL_PAUSE_MS) {
                preroll_pending = false;
                preroll_pause_start_ms = 0;
                if (changeState(State::PLAYING)) {
                    Serial.println("Pre-roll done, starting playback");
                    startPlayback();
                } else {
                    changeState(State::IDLE);
                }
            }
        }

        // Process move_to interpolation if in MOVING state
        if (g_state.current_state == State::MOVING && move_to_duration_ms > 0) {
            uint32_t elapsed = millis() - move_to_start_ms;
            float ratio = (float)elapsed / (float)move_to_duration_ms;
            if (ratio >= 1.0f) ratio = 1.0f;

            float interpolated[MAX_AXES];
            for (int i = 0; i < g_axis_count; i++) {
                interpolated[i] = move_to_start[i] + ratio * (move_to_target[i] - move_to_start[i]);
            }

            setStepperTarget(interpolated[AXIS_SLIDE]);

            // DJI axes: don't micro-update — we sent a single command with
            // the full duration in executeMoveToCmd. Just update state positions
            // from telemetry feedback (handled by onAttitudeUpdate callback).

            xSemaphoreTake(g_state_mutex, portMAX_DELAY);
            // For stepper axis, update from interpolation; DJI axes get updated by telemetry
            g_state.positions[AXIS_SLIDE] = interpolated[AXIS_SLIDE];
            xSemaphoreGive(g_state_mutex);

            // Push telemetry
            TelemetryPoint telem;
            telem.t_ms = 0;
            xSemaphoreTake(g_state_mutex, portMAX_DELAY);
            memcpy(telem.axes, (void*)g_state.positions, sizeof(float) * g_axis_count);
            xSemaphoreGive(g_state_mutex);
            xQueueSend(telemetry_queue, &telem, 0);

            if (ratio >= 1.0f) {
                move_to_duration_ms = 0;
                if (preroll_pending) {
                    // Pre-roll move complete — start the settle pause
                    preroll_pause_start_ms = millis();
                    Serial.println("Pre-roll arrived, settling 1s...");
                } else {
                    changeState(State::IDLE);
                }
            }
        }

        // Process commands
        if (xQueueReceive(cmd_queue, &cmd, pdMS_TO_TICKS(10)) == pdTRUE) {
            switch (cmd.type) {
                case CommandType::PLAY:
                    if (g_state.trajectory_loaded &&
                        strcmp(cmd.data.play.trajectory_id, g_state.trajectory_id) == 0) {
                        // Check if we're already at the trajectory start position
                        bool at_start = true;
                        if (g_trajectory.point_count > 0) {
                            xSemaphoreTake(g_state_mutex, portMAX_DELAY);
                            for (int i = 0; i < g_axis_count; i++) {
                                float diff = g_state.positions[i] - g_trajectory.points[0].axes[i];
                                if (diff < 0) diff = -diff;
                                // Use appropriate tolerance per axis type
                                float tol = (g_axis_config[i].type == AxisType::STEPPER) ? 5.0f : preroll_tolerance_deg;
                                if (diff > tol) {
                                    at_start = false;
                                    break;
                                }
                            }
                            xSemaphoreGive(g_state_mutex);
                        }

                        if (at_start) {
                            // Already at start — play immediately
                            if (changeState(State::PLAYING)) {
                                startPlayback();
                            }
                        } else {
                            // Move to start position first, then play
                            if (changeState(State::MOVING)) {
                                preroll_pending = true;
                                executeMoveToCmd(g_trajectory.points[0].axes, 2000);
                                Serial.println("Pre-roll: moving to trajectory start");
                            }
                        }
                    }
                    break;

                case CommandType::PAUSE:
                    if (changeState(State::PAUSED)) {
                        pausePlayback();
                    }
                    break;

                case CommandType::RESUME:
                    if (changeState(State::PLAYING)) {
                        resumePlayback();
                    }
                    break;

                case CommandType::STOP:
                    preroll_pending = false;
                    if (changeState(State::IDLE)) {
                        stopPlayback();
                        stopAllMotion();
                    }
                    break;

                case CommandType::JOG:
                    if (g_state.current_state == State::MOVING) {
                        changeState(State::IDLE);
                        stopAllMotion();
                    }
                    if (g_state.current_state == State::IDLE) {
                        for (int i = 0; i < g_axis_count; i++) {
                            if (cmd.data.jog.axes[i] != 0.0f) {
                                applyJogDelta(i, cmd.data.jog.axes[i]);
                            }
                        }
                    }
                    break;

                case CommandType::MOVE_TO:
                    if (g_state.current_state == State::IDLE ||
                        g_state.current_state == State::MOVING) {
                        if (changeState(State::MOVING)) {
                            executeMoveToCmd(cmd.data.move_to.axes,
                                             cmd.data.move_to.duration_ms);
                        }
                    }
                    break;

                case CommandType::HOME:
                    if (changeState(State::HOMING)) {
                        executeHomeCmd(cmd.data.home.axis_mask);
                        changeState(State::IDLE);
                    }
                    break;

                case CommandType::SCRUB:
                    if (g_state.current_state == State::PAUSED ||
                        g_state.current_state == State::PLAYING ||
                        g_state.current_state == State::IDLE) {
                        scrubToTime(cmd.data.scrub.t_ms);
                    }
                    break;

                case CommandType::CLEAR_ERROR:
                    if (g_state.current_state == State::ERROR) {
                        changeState(State::IDLE);
                    }
                    break;

                case CommandType::DISABLE:
                    // De-energize motor coils (useful between bring-up tests
                    // to cool the driver). State unchanged — user's call whether
                    // to follow up with a re-home.
                    stepperDisable();
                    break;
            }
        }
    }
}

// --- NVS helpers (simplified) ---

static const char* nvs_get_string(const char* key, const char* default_val) {
    // TODO: implement with Preferences library
    // For now, return default
    return default_val;
}

// --- Battery voltage reading ---

static float readBatteryVoltage() {
    int raw = analogRead(PIN_VBAT);
    // Voltage divider: 100k/27k for 24V -> 3.3V range
    // ADC: 12-bit (0-4095), 3.3V reference
    float vAdc = (float)raw / 4095.0f * 3.3f;
    float vBat = vAdc * (100.0f + 27.0f) / 27.0f;
    return vBat;
}

// ============================
// Setup and Loop
// ============================

void setup() {
    Serial.begin(115200);
    Serial.printf("Slider firmware %s\n", FIRMWARE_VERSION);

    // Initialize PSRAM
    if (psramFound()) {
        Serial.printf("PSRAM: %d bytes\n", ESP.getPsramSize());
    } else {
        Serial.println("WARNING: no PSRAM detected");
    }

    // Initialize filesystem
    if (!LittleFS.begin(true)) {
        Serial.println("ERROR: LittleFS mount failed");
    }

    // Initialize stepper (also sets up limit switch pins and interrupts)
    stepperInit();

    // Create shared resources
    g_state_mutex = xSemaphoreCreateMutex();
    cmd_queue = xQueueCreate(16, sizeof(Command));
    telemetry_queue = xQueueCreate(4, sizeof(TelemetryPoint));

    // Initialize system state
    g_state.current_state = State::IDLE;
    g_state.error_code = ErrorCode::NONE;
    g_state.trajectory_loaded = false;
    memset((void*)g_state.positions, 0, sizeof(g_state.positions));

    // WiFi event logging
    WiFi.onEvent([](WiFiEvent_t event, WiFiEventInfo_t info) {
        switch (event) {
            case ARDUINO_EVENT_WIFI_AP_STACONNECTED:
                Serial.printf("[WiFi] Client connected: %02X:%02X:%02X:%02X:%02X:%02X\n",
                    info.wifi_ap_staconnected.mac[0], info.wifi_ap_staconnected.mac[1],
                    info.wifi_ap_staconnected.mac[2], info.wifi_ap_staconnected.mac[3],
                    info.wifi_ap_staconnected.mac[4], info.wifi_ap_staconnected.mac[5]);
                break;
            case ARDUINO_EVENT_WIFI_AP_STADISCONNECTED:
                Serial.printf("[WiFi] Client disconnected: %02X:%02X:%02X:%02X:%02X:%02X\n",
                    info.wifi_ap_stadisconnected.mac[0], info.wifi_ap_stadisconnected.mac[1],
                    info.wifi_ap_stadisconnected.mac[2], info.wifi_ap_stadisconnected.mac[3],
                    info.wifi_ap_stadisconnected.mac[4], info.wifi_ap_stadisconnected.mac[5]);
                break;
            case ARDUINO_EVENT_WIFI_AP_STAIPASSIGNED:
                Serial.printf("[WiFi] IP assigned to client\n");
                break;
            default:
                Serial.printf("[WiFi] Event: %d\n", event);
                break;
        }
    });

    // Start WiFi AP
    WiFi.mode(WIFI_AP);
    uint8_t mac[6];
    esp_read_mac(mac, ESP_MAC_WIFI_SOFTAP);
    char default_ssid[32];
    snprintf(default_ssid, sizeof(default_ssid), "OpenDolly-%02X%02X", mac[4], mac[5]);
    const char* ssid = nvs_get_string("ap_ssid", default_ssid);
    const char* password = nvs_get_string("ap_password", "opendolly");
    WiFi.softAP(ssid, password);
    esp_wifi_set_ps(WIFI_PS_NONE);  // Disable modem sleep — prevents client disconnects
    Serial.printf("AP SSID: %s\n", ssid);
    Serial.printf("AP IP: %s\n", WiFi.softAPIP().toString().c_str());

    // Captive portal
    dnsServer.start(53, "*", WiFi.softAPIP());

    // mDNS
    const char* device_name = nvs_get_string("device_name", "opendolly");
    if (MDNS.begin(device_name)) {
        MDNS.addService("http", "tcp", 80);
        Serial.printf("mDNS: %s.local\n", device_name);
    }

    // Start web server
    setupWebServer();

    // Create tasks — Core 0 (network)
    xTaskCreatePinnedToCore(telemetryBroadcastTask, "telemetry", 4096, NULL, 1, NULL, 0);
    DisplayInterface* display = createDisplay();
    display->init();
    display->showBoot(FIRMWARE_VERSION);
    xTaskCreatePinnedToCore(displayUpdateTask, "display", 4096, display, 1, NULL, 0);

    // Create tasks — Core 1 (motion)
    xTaskCreatePinnedToCore(motionControlTask, "motion", 8192, NULL, 4, NULL, 1);
    xTaskCreatePinnedToCore(trajectoryWalkerTask, "walker", 4096, NULL, 5, NULL, 1);

    // DJI CAN
    #ifndef BUILD_SLIDER_ONLY
    djiCanInit();
    xTaskCreatePinnedToCore(djiCanTask, "dji_can", 4096, NULL, 3, NULL, 1);
    #endif

    // Setup walker timer (100 Hz)
    walker_timer = timerBegin(0, 80, true);  // prescaler 80 -> 1 MHz tick
    timerAttachInterrupt(walker_timer, &onWalkerTimer, true);
    timerAlarmWrite(walker_timer, 10000, true);  // 10,000 us = 100 Hz
    // timerAlarmEnable called when playback starts

    Serial.println("Setup complete");
}

void loop() {
    dnsServer.processNextRequest();
    vTaskDelay(pdMS_TO_TICKS(10));
}

#endif // NATIVE_TEST
