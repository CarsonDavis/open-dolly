#ifndef NATIVE_TEST

#include "web_server.h"
#include "config.h"
#include "commands.h"
#include "state_machine.h"
#include "trajectory.h"
#include <ArduinoJson.h>
#include <LittleFS.h>

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

extern QueueHandle_t cmd_queue;

// --- Helper: send JSON error to a single WS client ---

void sendError(AsyncWebSocketClient* client, const char* code, const char* detail) {
    JsonDocument doc;
    doc["evt"] = "error";
    doc["code"] = code;
    doc["detail"] = detail;
    char buf[256];
    size_t len = serializeJson(doc, buf, sizeof(buf));
    client->text(buf, len);
}

// --- Helper: send state event to a single client ---

void sendStateEvent(AsyncWebSocketClient* client) {
    static const char* STATE_NAMES[] = {
        "idle", "playing", "paused", "homing", "moving", "error"
    };

    JsonDocument doc;
    doc["evt"] = "state";

    xSemaphoreTake(g_state_mutex, portMAX_DELAY);
    State st = g_state.current_state;
    xSemaphoreGive(g_state_mutex);

    doc["state"] = STATE_NAMES[(uint8_t)st];
    char buf[128];
    size_t len = serializeJson(doc, buf, sizeof(buf));
    client->text(buf, len);
}

// --- Helper: broadcast state event to all WS clients ---

void broadcastStateEvent() {
    static const char* STATE_NAMES[] = {
        "idle", "playing", "paused", "homing", "moving", "error"
    };

    JsonDocument doc;
    doc["evt"] = "state";

    xSemaphoreTake(g_state_mutex, portMAX_DELAY);
    State st = g_state.current_state;
    ErrorCode err = g_state.error_code;
    xSemaphoreGive(g_state_mutex);

    doc["state"] = STATE_NAMES[(uint8_t)st];
    if (st == State::ERROR && err != ErrorCode::NONE) {
        static const char* ERROR_NAMES[] = {
            "NONE", "LIMIT_HIT", "MOTOR_STALL", "TRAJECTORY_INVALID",
            "OUT_OF_MEMORY", "COMMUNICATION_LOST", "CAN_TIMEOUT", "INVALID_STATE"
        };
        doc["error"] = ERROR_NAMES[(uint8_t)err];
    }

    char buf[256];
    size_t len = serializeJson(doc, buf, sizeof(buf));
    ws.textAll(buf, len);
}

// --- REST: GET /api/status ---

static void handleGetStatus(AsyncWebServerRequest* request) {
    JsonDocument doc;

    xSemaphoreTake(g_state_mutex, portMAX_DELAY);
    State st = g_state.current_state;
    float positions[MAX_AXES];
    memcpy(positions, (void*)g_state.positions, sizeof(float) * g_axis_count);
    bool traj_loaded = g_state.trajectory_loaded;
    char traj_id[16];
    if (traj_loaded) {
        strlcpy(traj_id, g_state.trajectory_id, sizeof(traj_id));
    }
    xSemaphoreGive(g_state_mutex);

    static const char* STATE_NAMES[] = {
        "idle", "playing", "paused", "homing", "moving", "error"
    };
    doc["state"] = STATE_NAMES[(uint8_t)st];

    JsonObject pos = doc["position"].to<JsonObject>();
    for (int i = 0; i < g_axis_count; i++) {
        pos[g_axis_config[i].name] = serialized(String(positions[i], 1));
    }

    doc["battery"] = 82; // Mock battery value for now
    doc["trajectory_loaded"] = traj_loaded ? (const char*)traj_id : (const char*)nullptr;

    char buf[256];
    serializeJson(doc, buf, sizeof(buf));
    request->send(200, "application/json", buf);
}

// --- REST: GET /api/capabilities ---

static void handleGetCapabilities(AsyncWebServerRequest* request) {
    JsonDocument doc;

    JsonArray axes = doc["axes"].to<JsonArray>();
    for (int i = 0; i < g_axis_count; i++) {
        JsonObject axis = axes.add<JsonObject>();
        axis["name"] = g_axis_config[i].name;
        axis["min"] = g_axis_config[i].min;
        axis["max"] = g_axis_config[i].max;
        axis["unit"] = g_axis_config[i].unit;
    }

    doc["max_trajectory_points"] = MAX_TRAJECTORY_POINTS;
    doc["max_trajectory_duration_ms"] = 300000;
    doc["telemetry_rate_hz"] = TELEMETRY_RATE_HZ;
    doc["firmware_version"] = FIRMWARE_VERSION;
    doc["board"] = "esp32-s3";

    char buf[512];
    serializeJson(doc, buf, sizeof(buf));
    request->send(200, "application/json", buf);
}

// --- REST: POST /api/trajectory (body handler) ---

static char* g_upload_buffer = nullptr;
static size_t g_upload_total = 0;

static void handleTrajectoryBody(AsyncWebServerRequest* request, uint8_t* data,
                                  size_t len, size_t index, size_t total) {
    if (index == 0) {
        g_upload_buffer = (char*)ps_malloc(total + 1);
        if (!g_upload_buffer) {
            request->send(507, "application/json", "{\"error\":\"OUT_OF_MEMORY\"}");
            return;
        }
        g_upload_total = total;
    }

    if (!g_upload_buffer) return;

    memcpy(g_upload_buffer + index, data, len);

    if (index + len == total) {
        g_upload_buffer[total] = '\0';
        bool ok = parseTrajectoryJson(g_upload_buffer, total);
        free(g_upload_buffer);
        g_upload_buffer = nullptr;

        if (ok) {
            snprintf(g_trajectory.id, sizeof(g_trajectory.id), "%08lx",
                     (unsigned long)esp_random());

            xSemaphoreTake(g_state_mutex, portMAX_DELAY);
            g_state.trajectory_loaded = true;
            strlcpy(g_state.trajectory_id, g_trajectory.id, 16);
            g_state.trajectory_point_count = g_trajectory.point_count;
            g_state.trajectory_duration_ms = g_trajectory.duration_ms;
            xSemaphoreGive(g_state_mutex);

            JsonDocument resp;
            resp["trajectory_id"] = g_trajectory.id;
            resp["point_count"] = g_trajectory.point_count;
            resp["duration_ms"] = g_trajectory.duration_ms;
            char buf[128];
            serializeJson(resp, buf, sizeof(buf));
            request->send(200, "application/json", buf);
        } else {
            request->send(400, "application/json",
                          "{\"error\":\"TRAJECTORY_INVALID\",\"detail\":\"Failed to parse trajectory\"}");
        }
    }
}

static void handlePostTrajectory(AsyncWebServerRequest* request) {
    // Body is handled by handleTrajectoryBody; this is the final handler
    // If we get here without the body handler completing, it means the
    // body was empty or the handler already sent a response.
    if (!request->_tempObject) {
        // No response sent yet by body handler
    }
}

// --- REST: DELETE /api/trajectory/:id ---

static void handleDeleteTrajectory(AsyncWebServerRequest* request) {
    String id = request->pathArg(0);

    xSemaphoreTake(g_state_mutex, portMAX_DELAY);
    bool loaded = g_state.trajectory_loaded;
    bool id_match = (strcmp(g_state.trajectory_id, id.c_str()) == 0);
    State st = g_state.current_state;
    xSemaphoreGive(g_state_mutex);

    if (!loaded || !id_match) {
        char buf[128];
        snprintf(buf, sizeof(buf),
                 "{\"error\":\"NOT_FOUND\",\"detail\":\"No trajectory with id '%s'\"}",
                 id.c_str());
        request->send(404, "application/json", buf);
        return;
    }

    if (st == State::PLAYING || st == State::PAUSED) {
        request->send(409, "application/json",
                      "{\"error\":\"CONFLICT\",\"detail\":\"Cannot delete trajectory while playback is active\"}");
        return;
    }

    xSemaphoreTake(g_state_mutex, portMAX_DELAY);
    g_state.trajectory_loaded = false;
    g_state.trajectory_id[0] = '\0';
    g_state.trajectory_point_count = 0;
    g_state.trajectory_duration_ms = 0;
    xSemaphoreGive(g_state_mutex);

    if (g_trajectory.points) {
        free(g_trajectory.points);
        g_trajectory.points = nullptr;
    }
    g_trajectory.point_count = 0;

    request->send(204);
}

// --- REST: GET /api/trajectory/:id/status ---

static void handleTrajectoryStatus(AsyncWebServerRequest* request) {
    String id = request->pathArg(0);

    xSemaphoreTake(g_state_mutex, portMAX_DELAY);
    bool loaded = g_state.trajectory_loaded;
    bool id_match = loaded && (strcmp(g_state.trajectory_id, id.c_str()) == 0);
    State st = g_state.current_state;
    uint32_t elapsed = g_state.trajectory_elapsed_ms;
    uint32_t duration = g_state.trajectory_duration_ms;
    xSemaphoreGive(g_state_mutex);

    if (!id_match) {
        char buf[128];
        snprintf(buf, sizeof(buf),
                 "{\"error\":\"NOT_FOUND\",\"detail\":\"No trajectory with id '%s'\"}",
                 id.c_str());
        request->send(404, "application/json", buf);
        return;
    }

    JsonDocument doc;
    if (st == State::PLAYING) {
        doc["state"] = "playing";
    } else if (st == State::PAUSED) {
        doc["state"] = "paused";
    } else {
        doc["state"] = "idle";
    }
    doc["elapsed_ms"] = (st == State::PLAYING || st == State::PAUSED) ? elapsed : 0;
    doc["progress"] = (duration > 0 && (st == State::PLAYING || st == State::PAUSED))
                      ? (float)elapsed / (float)duration : 0.0f;

    char buf[128];
    serializeJson(doc, buf, sizeof(buf));
    request->send(200, "application/json", buf);
}

// --- REST: GET /api/settings ---

// Settings stored in NVS — for now, use defaults in memory
struct DeviceSettings {
    char device_name[32] = "opendolly";
    char ap_ssid[32] = "OpenDolly-AP";
    char ap_password[32] = "opendolly";
    uint16_t telemetry_rate_hz = TELEMETRY_RATE_HZ;
    float jog_sensitivity = 1.0f;
    bool home_on_boot = false;
};

static DeviceSettings g_settings;

static void handleGetSettings(AsyncWebServerRequest* request) {
    JsonDocument doc;
    doc["device_name"] = g_settings.device_name;
    doc["ap_ssid"] = g_settings.ap_ssid;
    doc["ap_password"] = g_settings.ap_password;
    doc["telemetry_rate_hz"] = g_settings.telemetry_rate_hz;
    doc["jog_sensitivity"] = g_settings.jog_sensitivity;
    doc["home_on_boot"] = g_settings.home_on_boot;

    char buf[512];
    serializeJson(doc, buf, sizeof(buf));
    request->send(200, "application/json", buf);
}

// --- REST: PATCH /api/settings ---

static void handlePatchSettingsBody(AsyncWebServerRequest* request, uint8_t* data,
                                     size_t len, size_t index, size_t total) {
    // Accumulate body (settings are small, <512 bytes)
    static char settings_buf[512];
    if (index == 0) memset(settings_buf, 0, sizeof(settings_buf));
    if (index + len < sizeof(settings_buf)) {
        memcpy(settings_buf + index, data, len);
    }

    if (index + len == total) {
        settings_buf[total < sizeof(settings_buf) ? total : sizeof(settings_buf) - 1] = '\0';

        JsonDocument doc;
        DeserializationError err = deserializeJson(doc, settings_buf, total);
        if (err) {
            request->send(400, "application/json",
                          "{\"error\":\"PARSE_ERROR\",\"detail\":\"Invalid JSON\"}");
            return;
        }

        bool wifi_changed = false;

        if (doc["device_name"].is<const char*>()) {
            strlcpy(g_settings.device_name, doc["device_name"], sizeof(g_settings.device_name));
        }
        if (doc["ap_ssid"].is<const char*>()) {
            strlcpy(g_settings.ap_ssid, doc["ap_ssid"], sizeof(g_settings.ap_ssid));
            wifi_changed = true;
        }
        if (doc["ap_password"].is<const char*>()) {
            strlcpy(g_settings.ap_password, doc["ap_password"], sizeof(g_settings.ap_password));
            wifi_changed = true;
        }
        if (doc["telemetry_rate_hz"].is<int>()) {
            g_settings.telemetry_rate_hz = doc["telemetry_rate_hz"];
        }
        if (doc["jog_sensitivity"].is<float>()) {
            g_settings.jog_sensitivity = doc["jog_sensitivity"];
        }
        if (doc["home_on_boot"].is<bool>()) {
            g_settings.home_on_boot = doc["home_on_boot"];
        }

        // TODO: persist to NVS with Preferences library

        // Build response
        JsonDocument resp;
        resp["device_name"] = g_settings.device_name;
        resp["ap_ssid"] = g_settings.ap_ssid;
        resp["ap_password"] = g_settings.ap_password;
        resp["telemetry_rate_hz"] = g_settings.telemetry_rate_hz;
        resp["jog_sensitivity"] = g_settings.jog_sensitivity;
        resp["home_on_boot"] = g_settings.home_on_boot;
        if (wifi_changed) {
            resp["restart_required"] = true;
        }

        char buf[512];
        serializeJson(resp, buf, sizeof(buf));
        request->send(200, "application/json", buf);
    }
}

static void handlePatchSettings(AsyncWebServerRequest* request) {
    // Body handled by handlePatchSettingsBody
}

// --- WebSocket handler ---

static void parseWebSocketCommand(const char* data, size_t len, AsyncWebSocketClient* client) {
    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, data, len);
    if (err) {
        sendError(client, "PARSE_ERROR", err.c_str());
        return;
    }

    const char* cmd = doc["cmd"];
    if (!cmd) {
        sendError(client, "MISSING_CMD", "no cmd field");
        return;
    }

    Command command = {};

    if (strcmp(cmd, "play") == 0) {
        command.type = CommandType::PLAY;
        strlcpy(command.data.play.trajectory_id, doc["trajectory_id"] | "", 16);
    }
    else if (strcmp(cmd, "pause") == 0)  { command.type = CommandType::PAUSE; }
    else if (strcmp(cmd, "resume") == 0) { command.type = CommandType::RESUME; }
    else if (strcmp(cmd, "stop") == 0)   { command.type = CommandType::STOP; }
    else if (strcmp(cmd, "jog") == 0) {
        command.type = CommandType::JOG;
        for (int i = 0; i < g_axis_count; i++) {
            command.data.jog.axes[i] = doc[g_axis_config[i].name] | 0.0f;
        }
    }
    else if (strcmp(cmd, "move_to") == 0) {
        command.type = CommandType::MOVE_TO;
        command.data.move_to.duration_ms = doc["duration_ms"] | 1000;
        for (int i = 0; i < g_axis_count; i++) {
            // Default to current position for unspecified axes (not 0)
            command.data.move_to.axes[i] = doc[g_axis_config[i].name] | g_state.positions[i];
        }
    }
    else if (strcmp(cmd, "home") == 0) {
        command.type = CommandType::HOME;
        JsonArray axes = doc["axes"];
        command.data.home.axis_mask = 0;
        for (JsonVariant v : axes) {
            const char* name = v.as<const char*>();
            if (!name) continue;
            for (int i = 0; i < g_axis_count; i++) {
                if (strcmp(name, g_axis_config[i].name) == 0) {
                    command.data.home.axis_mask |= (1 << i);
                }
            }
        }
    }
    else if (strcmp(cmd, "scrub") == 0) {
        command.type = CommandType::SCRUB;
        command.data.scrub.t_ms = doc["t"] | 0;
    }
    else if (strcmp(cmd, "ping") == 0) {
        // Respond immediately — no state change, no command queue
        JsonDocument pong;
        pong["evt"] = "pong";
        char buf[32];
        serializeJson(pong, buf, sizeof(buf));
        client->text(buf);
        return;
    }
    else if (strcmp(cmd, "clear_error") == 0) {
        command.type = CommandType::CLEAR_ERROR;
    }
    else if (strcmp(cmd, "disable") == 0) {
        command.type = CommandType::DISABLE;
    }
    else if (strcmp(cmd, "restart") == 0) {
        // Nuclear option — reboot the ESP32. WS client will drop.
        // No queue, no state change: just ack and reset.
        Serial.println("Restart requested via WS — rebooting");
        delay(50);
        ESP.restart();
        return; // unreached
    }
    else {
        sendError(client, "UNKNOWN_CMD", cmd);
        return;
    }

    if (xQueueSend(cmd_queue, &command, 0) != pdTRUE) {
        sendError(client, "QUEUE_FULL", "command queue full");
    }
}

static void onWebSocketEvent(AsyncWebSocket* server, AsyncWebSocketClient* client,
                              AwsEventType type, void* arg, uint8_t* data, size_t len) {
    switch (type) {
        case WS_EVT_CONNECT:
            Serial.printf("WS client #%u connected from %s\n",
                          client->id(), client->remoteIP().toString().c_str());
            sendStateEvent(client);
            break;

        case WS_EVT_DISCONNECT:
            Serial.printf("WS client #%u disconnected\n", client->id());
            break;

        case WS_EVT_DATA: {
            AwsFrameInfo* info = (AwsFrameInfo*)arg;
            if (info->opcode == WS_TEXT) {
                parseWebSocketCommand((char*)data, len, client);
            }
            break;
        }

        case WS_EVT_ERROR:
            Serial.printf("WS client #%u error\n", client->id());
            break;

        default:
            break;
    }
}

// --- Server setup ---

void setupWebServer() {
    // CORS headers (must be set before server.begin())
    DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
    DefaultHeaders::Instance().addHeader("Access-Control-Allow-Methods", "GET, POST, PATCH, DELETE, OPTIONS");
    DefaultHeaders::Instance().addHeader("Access-Control-Allow-Headers", "Content-Type");

    // REST endpoints — registered BEFORE serveStatic so they take priority
    server.on("/api/status", HTTP_GET, handleGetStatus);
    server.on("/api/capabilities", HTTP_GET, handleGetCapabilities);

    // Trajectory endpoints
    server.on("/api/trajectory", HTTP_POST, handlePostTrajectory, NULL, handleTrajectoryBody);

    // Parameterized routes for trajectory/:id
    server.on("^\\/api\\/trajectory\\/([a-zA-Z0-9_]+)$", HTTP_DELETE, handleDeleteTrajectory);
    server.on("^\\/api\\/trajectory\\/([a-zA-Z0-9_]+)\\/status$", HTTP_GET, handleTrajectoryStatus);

    // Settings endpoints
    server.on("/api/settings", HTTP_GET, handleGetSettings);
    server.on("/api/settings", HTTP_PATCH, handlePatchSettings, NULL, handlePatchSettingsBody);

    // Handle CORS preflight
    server.on("/api/*", HTTP_OPTIONS, [](AsyncWebServerRequest* request) {
        request->send(204);
    });

    // Connectivity probes — tell the OS "internet works" so it stays connected
    // Android
    server.on("/generate_204", HTTP_GET, [](AsyncWebServerRequest* request) {
        request->send(204);
    });
    server.on("/gen_204", HTTP_GET, [](AsyncWebServerRequest* request) {
        request->send(204);
    });
    // Apple
    server.on("/hotspot-detect.html", HTTP_GET, [](AsyncWebServerRequest* request) {
        request->send(200, "text/html", "<HTML><HEAD><TITLE>Success</TITLE></HEAD><BODY>Success</BODY></HTML>");
    });
    server.on("/library/test/success.html", HTTP_GET, [](AsyncWebServerRequest* request) {
        request->send(200, "text/html", "<HTML><HEAD><TITLE>Success</TITLE></HEAD><BODY>Success</BODY></HTML>");
    });
    // Windows
    server.on("/connecttest.txt", HTTP_GET, [](AsyncWebServerRequest* request) {
        request->send(200, "text/plain", "Microsoft Connect Test");
    });
    server.on("/ncsi.txt", HTTP_GET, [](AsyncWebServerRequest* request) {
        request->send(200, "text/plain", "Microsoft NCSI");
    });

    // Static files from LittleFS — LAST so API routes take priority
    server.serveStatic("/", LittleFS, "/").setDefaultFile("index.html");

    // Log all unhandled requests (debug)
    server.onNotFound([](AsyncWebServerRequest* request) {
        Serial.printf("[HTTP] 404: %s %s (Host: %s)\n",
            request->methodToString(), request->url().c_str(),
            request->host().c_str());
        request->send(404, "text/plain", "Not found");
    });

    // WebSocket
    ws.onEvent(onWebSocketEvent);
    server.addHandler(&ws);

    server.begin();
    Serial.println("Web server started on port 80");
}

#endif // NATIVE_TEST
