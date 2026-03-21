# Firmware Implementation Plan

*Status: implemented*
*Last updated: 2026-03-21*

ESP32-S3 firmware for the camera slider controller board. Implements the [Board API](../board-api.md) — serves the web UI over Wi-Fi, accepts trajectory uploads, drives a stepper motor and DJI gimbal, and reports telemetry.

**Prerequisites:** read [architecture.md](../architecture.md), [board-api.md](../board-api.md), [ADR-001](../adr/adr-001-microcontroller.md), [ADR-002](../adr/adr-002-status-display.md), and [bom.md](../bom.md) first.

---

## Project Structure

```
firmware/
├── platformio.ini
├── src/
│   ├── main.cpp                # Setup, FreeRTOS task creation, pin init
│   ├── config.h                # Pin definitions, axis config, build-time constants
│   ├── web_server.h/.cpp       # REST endpoints + WebSocket handler
│   ├── trajectory.h/.cpp       # Upload parsing, storage, playback walker
│   ├── stepper.h/.cpp          # FastAccelStepper wrapper, homing, position tracking
│   ├── state_machine.h/.cpp    # System state management, transitions, error handling
│   ├── display.h/.cpp          # Abstract display interface + implementations
│   ├── telemetry.h/.cpp        # Position reporting to WebSocket clients
│   └── commands.h              # Shared command/event types for inter-task queues
├── lib/
│   └── dji_can/                # DJI gimbal CAN protocol library (see plan-dji-can.md)
├── data/                       # Web UI static files (LittleFS partition)
│   ├── index.html
│   ├── app.js
│   └── style.css
└── test/
    ├── test_trajectory/        # Trajectory parsing and walker logic
    ├── test_state_machine/     # State transition validation
    └── test_message_parsing/   # WebSocket JSON command parsing
```

---

## PlatformIO Configuration

```ini
; platformio.ini

[env]
framework = arduino
lib_deps =
    gin66/FastAccelStepper@^0.30.0
    me-no-dev/ESPAsyncWebServer@^1.2.6
    me-no-dev/AsyncTCP@^1.1.4
    bblanchon/ArduinoJson@^7.0.0
    olikraus/U8g2@^2.35.0
    ; DNSServer is built-in to ESP32 Arduino core (no version needed)

[env:esp32s3]
platform = espressif32
board = esp32-s3-devkitc-1
board_build.partitions = partitions.csv
board_build.filesystem = littlefs
monitor_speed = 115200
upload_speed = 921600

build_flags =
    -DBOARD_HAS_PSRAM
    -DCORE_DEBUG_LEVEL=3            ; 0=none, 1=error, 2=warn, 3=info, 4=debug, 5=verbose
    -DCONFIG_ARDUHAL_LOG_COLORS=1
    -DMAX_AXES=4
    -DMAX_TRAJECTORY_POINTS=10000
    -DTELEMETRY_RATE_HZ=50

; Upload web UI files to LittleFS
extra_scripts = pre:scripts/build_littlefs.py

[env:native]
platform = native
build_flags =
    -DMAX_AXES=4
    -DMAX_TRAJECTORY_POINTS=10000
    -DNATIVE_TEST                   ; Guard hardware-dependent code
test_filter = test_*
lib_ignore =
    FastAccelStepper
    ESPAsyncWebServer
    AsyncTCP
    U8g2
```

### Flash Partition Table

```
; partitions.csv — 8MB flash layout
# Name,    Type, SubType, Offset,  Size
nvs,       data, nvs,     0x9000,  0x5000      # 20KB  — NVS key-value storage
otadata,   data, ota,     0xE000,  0x2000      # 8KB   — OTA state
app0,      app,  ota_0,   0x10000, 0x300000    # 3MB   — firmware slot A
app1,      app,  ota_1,   0x310000,0x300000    # 3MB   — firmware slot B (OTA)
littlefs,  data, spiffs,  0x610000,0x1E0000    # 1.875MB — web UI files
coredump,  data, coredump,0x7F0000,0x10000     # 64KB  — crash dump
```

**Budget:** 3MB per firmware slot is generous for Arduino firmware (typical is 1-1.5MB). The 1.875MB LittleFS partition holds the web UI with plenty of room — target is <500KB compressed. The second app slot enables OTA updates.

---

## FreeRTOS Task Architecture

```
┌─────────────────────────────────────────────────────────────────┐
│  CORE 0 (network)                  CORE 1 (motion)             │
│                                                                  │
│  ┌──────────────┐                  ┌──────────────────┐         │
│  │ WiFi/mDNS    │                  │ Trajectory Walker│         │
│  │ (Arduino     │                  │ (hw timer, 100Hz)│         │
│  │  default)    │                  │ pri: 5           │         │
│  │ pri: 1       │                  │ stack: 4096      │         │
│  └──────────────┘                  └───────┬──────────┘         │
│                                            │                     │
│  ┌──────────────┐     cmd_queue    ┌───────▼──────────┐         │
│  │ Web Server   │ ───────────────► │ Motion Control   │         │
│  │ (HTTP + WS)  │                  │ (stepper + CAN)  │         │
│  │ pri: 2       │ ◄─────────────── │ pri: 4           │         │
│  │ stack: 8192  │   telemetry_q    │ stack: 8192      │         │
│  └──────────────┘                  └──────────────────┘         │
│                                                                  │
│  ┌──────────────┐                  ┌──────────────────┐         │
│  │ Telemetry    │                  │ DJI CAN          │         │
│  │ Broadcaster  │                  │ (TWAI driver)    │         │
│  │ pri: 1       │                  │ pri: 3           │         │
│  │ stack: 4096  │                  │ stack: 4096      │         │
│  └──────────────┘                  └──────────────────┘         │
│                                                                  │
│  ┌──────────────┐                                               │
│  │ Display      │                                               │
│  │ Update       │                                               │
│  │ pri: 1       │                                               │
│  │ stack: 4096  │                                               │
│  └──────────────┘                                               │
└─────────────────────────────────────────────────────────────────┘

         ─────────► = FreeRTOS queue
         ──mutex──► = shared state with mutex
```

### Task Definitions

| Task | Core | Priority | Stack | Role |
|------|------|----------|-------|------|
| **WiFi/mDNS** | 0 | 1 | (Arduino default) | Managed by Arduino framework. Starts AP, registers `slider.local` via mDNS |
| **Web Server** | 0 | 2 | 8192 bytes | Handles HTTP requests (REST) and WebSocket frames. Dispatches commands to `cmd_queue` |
| **Telemetry Broadcaster** | 0 | 1 | 4096 bytes | Reads from `telemetry_queue`, formats JSON, broadcasts to all connected WebSocket clients |
| **Display Update** | 0 | 1 | 4096 bytes | Reads shared state (mutex-guarded), updates display at 5 Hz. No-op if no display attached |
| **Motion Control** | 1 | 4 | 8192 bytes | Main motion loop. Reads `cmd_queue`, manages stepper and DJI axes. Produces telemetry |
| **Trajectory Walker** | 1 | 5 | 4096 bytes | Hardware timer ISR + deferred task. Steps through trajectory at 100 Hz, feeds positions to motion control |
| **DJI CAN** | 1 | 3 | 4096 bytes | Sends CAN frames to gimbal, reads responses. Called by motion control for position updates |

### Inter-Task Communication

```cpp
// commands.h — shared types

enum class CommandType : uint8_t {
    PLAY,
    PAUSE,
    RESUME,
    STOP,
    JOG,
    MOVE_TO,
    HOME,
    SCRUB,
    CLEAR_ERROR
};

struct Command {
    CommandType type;
    union {
        struct { char trajectory_id[16]; } play;
        struct { float axes[MAX_AXES]; } jog;           // relative deltas
        struct { float axes[MAX_AXES]; uint32_t duration_ms; } move_to;  // absolute targets
        struct { uint8_t axis_mask; } home;              // bitmask of axes to home
        struct { uint32_t t_ms; } scrub;                 // target timestamp
    } data;
};

struct TelemetryPoint {
    uint32_t t_ms;                  // elapsed time since playback start (0 for manual)
    float axes[MAX_AXES];           // current positions in native units
};
```

**Queues:**

| Queue | Direction | Depth | Item Size | Purpose |
|-------|-----------|-------|-----------|---------|
| `cmd_queue` | Core 0 → Core 1 | 16 | `sizeof(Command)` ~72 bytes | Playback control, jog, move_to, home |
| `telemetry_queue` | Core 1 → Core 0 | 4 | `sizeof(TelemetryPoint)` ~20 bytes | Position updates for WebSocket broadcast |

**Shared State (mutex-protected):**

```cpp
struct SystemState {
    volatile State current_state;       // idle, playing, paused, homing, moving, error
    volatile ErrorCode error_code;      // NONE, LIMIT_HIT, MOTOR_STALL, etc.
    volatile float positions[MAX_AXES]; // latest known positions
    volatile bool trajectory_loaded;
    char trajectory_id[16];
    uint32_t trajectory_duration_ms;
    uint32_t trajectory_elapsed_ms;
    uint16_t trajectory_point_count;
};

// Global instance, accessed by both cores
extern SystemState g_state;
extern SemaphoreHandle_t g_state_mutex;
```

Both cores read `g_state`. Only the motion control task on Core 1 writes `current_state`, `positions`, and playback fields. The web server task on Core 0 writes `trajectory_loaded` (after upload) and reads everything else. All access goes through `g_state_mutex` with short critical sections — take mutex, copy needed fields to a local, release mutex, then do work.

---

## Pin Assignments (config.h)

```cpp
// config.h — pin definitions for ESP32-S3-DevKitC-1

// --- Stepper: slider axis ---
#define PIN_STEP        GPIO_NUM_15
#define PIN_DIR         GPIO_NUM_16
#define PIN_EN          GPIO_NUM_17

// --- TMC2209 UART (optional, for StealthChop config) ---
#define PIN_TMC_UART_TX GPIO_NUM_18
#define PIN_TMC_UART_RX GPIO_NUM_18  // Single-wire UART (TX=RX with 1k resistor)

// --- CAN bus (TWAI) ---
#define PIN_CAN_TX      GPIO_NUM_5
#define PIN_CAN_RX      GPIO_NUM_6

// --- Limit switches (active LOW with internal pullup) ---
#define PIN_LIMIT_MIN   GPIO_NUM_7
#define PIN_LIMIT_MAX   GPIO_NUM_8

// --- I2C bus (shared: display + expansion) ---
#define PIN_SDA         GPIO_NUM_1
#define PIN_SCL         GPIO_NUM_2

// --- Battery voltage (ADC) ---
#define PIN_VBAT        GPIO_NUM_4   // Via voltage divider (100k/27k for 24V → 3.3V range)

// --- Status LED ---
#define PIN_STATUS_LED  GPIO_NUM_48  // Onboard RGB LED on DevKitC-1
```

---

## Web Server Module

### Initialization

```cpp
// web_server.cpp

#include <ESPAsyncWebServer.h>
#include <AsyncWebSocket.h>
#include <ArduinoJson.h>
#include <LittleFS.h>

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

void setupWebServer() {
    // Serve static files from LittleFS (the web UI)
    server.serveStatic("/", LittleFS, "/").setDefaultFile("index.html");

    // REST endpoints
    server.on("/api/status",       HTTP_GET,  handleGetStatus);
    server.on("/api/capabilities", HTTP_GET,  handleGetCapabilities);
    server.on("/api/trajectory",   HTTP_POST, handlePostTrajectory, NULL, handleTrajectoryBody);
    server.on("/api/settings",     HTTP_GET,  handleGetSettings);
    // PATCH for /api/settings handled via rewrite
    // DELETE and GET for /api/trajectory/:id handled via rewrite + handler

    // CORS headers for development (UI on localhost:3000, ESP32 on 192.168.4.1)
    DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
    DefaultHeaders::Instance().addHeader("Access-Control-Allow-Methods", "GET, POST, DELETE, OPTIONS");
    DefaultHeaders::Instance().addHeader("Access-Control-Allow-Headers", "Content-Type");

    // WebSocket
    ws.onEvent(onWebSocketEvent);
    server.addHandler(&ws);

    server.begin();
}
```

### REST Endpoint Handlers

Each endpoint maps directly to the [Board API spec](../board-api.md):

| Endpoint | Handler Function | JSON Buffer | Notes |
|----------|-----------------|-------------|-------|
| `GET /api/status` | `handleGetStatus` | 256 bytes | Reads `g_state` under mutex, builds JSON |
| `GET /api/capabilities` | `handleGetCapabilities` | 512 bytes | Reads compile-time axis config |
| `POST /api/trajectory` | `handleTrajectoryBody` | Streaming | Parses body in chunks — see Trajectory Module |
| `DELETE /api/trajectory/:id` | `handleDeleteTrajectory` | 64 bytes | Frees trajectory memory, clears state |
| `GET /api/trajectory/:id/status` | `handleTrajectoryStatus` | 128 bytes | Reads playback progress from `g_state` |
| `GET /api/settings` | `handleGetSettings` | 512 bytes | Returns all NVS settings as JSON |
| `PATCH /api/settings` | `handlePatchSettings` | 512 bytes | Updates settings in NVS, returns updated values |

**ArduinoJson buffer sizing:** All response documents are statically allocated (`JsonDocument` with template size). Request parsing for small messages (WebSocket commands) uses a 256-byte `JsonDocument`. Trajectory upload uses streaming — see below.

### WebSocket Handler

```cpp
void onWebSocketEvent(AsyncWebSocket *server, AsyncWebSocketClient *client,
                      AwsEventType type, void *arg, uint8_t *data, size_t len) {
    switch (type) {
        case WS_EVT_CONNECT:
            Serial.printf("WS client #%u connected from %s\n",
                          client->id(), client->remoteIP().toString().c_str());
            // Send current state immediately
            sendStateEvent(client);
            break;

        case WS_EVT_DISCONNECT:
            Serial.printf("WS client #%u disconnected\n", client->id());
            break;

        case WS_EVT_DATA: {
            AwsFrameInfo *info = (AwsFrameInfo *)arg;
            if (info->opcode == WS_TEXT) {
                parseWebSocketCommand((char *)data, len, client);
            }
            break;
        }

        case WS_EVT_ERROR:
            Serial.printf("WS client #%u error\n", client->id());
            break;
    }
}

void parseWebSocketCommand(const char *data, size_t len, AsyncWebSocketClient *client) {
    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, data, len);
    if (err) {
        sendError(client, "PARSE_ERROR", err.c_str());
        return;
    }

    const char *cmd = doc["cmd"];
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
            command.data.move_to.axes[i] = doc[g_axis_config[i].name] | 0.0f;
        }
    }
    else if (strcmp(cmd, "home") == 0) {
        command.type = CommandType::HOME;
        // Parse axis mask from "axes" array
        JsonArray axes = doc["axes"];
        command.data.home.axis_mask = 0;
        for (const char *name : axes) {
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
    else {
        sendError(client, "UNKNOWN_CMD", cmd);
        return;
    }

    // Non-blocking send to motion control task
    if (xQueueSend(cmd_queue, &command, 0) != pdTRUE) {
        sendError(client, "QUEUE_FULL", "command queue full");
    }
}
```

### Telemetry Broadcasting

The telemetry broadcaster task runs on Core 0. It reads from `telemetry_queue` and sends position updates to all connected WebSocket clients:

```cpp
void telemetryBroadcastTask(void *param) {
    TelemetryPoint point;
    char json_buf[256];

    for (;;) {
        // Block until a telemetry point is available
        if (xQueueReceive(telemetry_queue, &point, pdMS_TO_TICKS(100)) == pdTRUE) {
            // Build JSON: {"evt":"position","t":12400,"pan":34.2,"tilt":-8.1,...}
            JsonDocument doc;
            doc["evt"] = "position";
            doc["t"] = point.t_ms;
            for (int i = 0; i < g_axis_count; i++) {
                doc[g_axis_config[i].name] = serialized(String(point.axes[i], 1));
            }
            size_t len = serializeJson(doc, json_buf, sizeof(json_buf));
            ws.textAll(json_buf, len);
        }
    }
}
```

**Rate control:** The motion control task pushes to `telemetry_queue` at `TELEMETRY_RATE_HZ` (default 50 Hz). The queue depth is 4 — if the broadcaster falls behind (e.g., slow WebSocket send), old points are dropped. This is acceptable because position telemetry is best-effort; the UI cares about latest position, not history.

---

## Trajectory Module

### Data Structure

```cpp
// trajectory.h

struct TrajectoryPoint {
    uint32_t t_ms;                  // timestamp in milliseconds from start
    float axes[MAX_AXES];           // axis positions in native units
};
// Size per point: 4 + (4 * MAX_AXES) = 20 bytes when MAX_AXES=4

struct Trajectory {
    char id[16];                    // short identifier
    uint16_t point_count;
    uint32_t duration_ms;           // t_ms of last point
    bool loop;
    TrajectoryPoint *points;        // allocated in PSRAM
};

extern Trajectory g_trajectory;     // single active trajectory
```

**Memory allocation:** `points` is allocated in PSRAM via `ps_malloc()`. At 20 bytes per point and 10,000 max points, worst case is 200KB — trivial for 8MB PSRAM. The array is allocated once on first upload and reused (reallocated if needed).

### Upload Parsing

Trajectory upload (`POST /api/trajectory`) receives a JSON body that may be large (200KB+). We use a streaming approach:

```cpp
// Streaming body handler — called with chunks as they arrive
void handleTrajectoryBody(AsyncWebServerRequest *request, uint8_t *data,
                          size_t len, size_t index, size_t total) {
    if (index == 0) {
        // First chunk — prepare for parsing
        g_upload_buffer = (char *)ps_malloc(total + 1);
        if (!g_upload_buffer) {
            request->send(507, "application/json", "{\"error\":\"OUT_OF_MEMORY\"}");
            return;
        }
        g_upload_total = total;
    }

    // Accumulate into PSRAM buffer
    memcpy(g_upload_buffer + index, data, len);

    if (index + len == total) {
        // Last chunk — parse the complete JSON
        g_upload_buffer[total] = '\0';
        bool ok = parseTrajectoryJson(g_upload_buffer, total);
        free(g_upload_buffer);
        g_upload_buffer = nullptr;

        if (ok) {
            // Generate trajectory ID
            snprintf(g_trajectory.id, sizeof(g_trajectory.id), "%08lx",
                     (unsigned long)esp_random());

            // Update shared state
            xSemaphoreTake(g_state_mutex, portMAX_DELAY);
            g_state.trajectory_loaded = true;
            strlcpy(g_state.trajectory_id, g_trajectory.id, 16);
            g_state.trajectory_point_count = g_trajectory.point_count;
            g_state.trajectory_duration_ms = g_trajectory.duration_ms;
            xSemaphoreGive(g_state_mutex);

            // Send response
            JsonDocument resp;
            resp["trajectory_id"] = g_trajectory.id;
            resp["point_count"] = g_trajectory.point_count;
            resp["duration_ms"] = g_trajectory.duration_ms;
            char buf[128];
            serializeJson(resp, buf);
            request->send(200, "application/json", buf);
        } else {
            request->send(400, "application/json", "{\"error\":\"TRAJECTORY_INVALID\"}");
        }
    }
}
```

### Parsing and Validation

```cpp
bool parseTrajectoryJson(const char *json, size_t len) {
    // Use ArduinoJson streaming filter to extract only what we need
    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, json, len);
    if (err) return false;

    JsonArray points = doc["points"];
    if (points.isNull()) return false;

    uint16_t count = points.size();
    if (count == 0 || count > MAX_TRAJECTORY_POINTS) return false;

    // Allocate or resize PSRAM array
    if (g_trajectory.points) {
        free(g_trajectory.points);
    }
    g_trajectory.points = (TrajectoryPoint *)ps_malloc(count * sizeof(TrajectoryPoint));
    if (!g_trajectory.points) return false;

    uint32_t prev_t = 0;
    for (uint16_t i = 0; i < count; i++) {
        JsonObject pt = points[i];
        uint32_t t = pt["t"];

        // Validate ascending timestamps
        if (i > 0 && t <= prev_t) {
            free(g_trajectory.points);
            g_trajectory.points = nullptr;
            return false;
        }
        prev_t = t;

        g_trajectory.points[i].t_ms = t;
        for (int a = 0; a < g_axis_count; a++) {
            float val = pt[g_axis_config[a].name] | 0.0f;
            // Validate within axis limits
            if (val < g_axis_config[a].min || val > g_axis_config[a].max) {
                free(g_trajectory.points);
                g_trajectory.points = nullptr;
                return false;
            }
            g_trajectory.points[i].axes[a] = val;
        }
    }

    g_trajectory.point_count = count;
    g_trajectory.duration_ms = g_trajectory.points[count - 1].t_ms;
    g_trajectory.loop = doc["loop"] | false;
    return true;
}
```

### Playback Walker

The walker runs on Core 1, driven by a hardware timer at 100 Hz. At each tick it finds the current position by interpolating between trajectory points.

```cpp
// trajectory.cpp — walker implementation

static hw_timer_t *walker_timer = nullptr;
static volatile bool walker_tick = false;
static uint16_t walker_index = 0;          // current segment index (left point)
static uint32_t playback_start_us = 0;     // micros() at play start

// Timer ISR — just sets a flag, real work happens in the task
void IRAM_ATTR onWalkerTimer() {
    walker_tick = true;
}

void trajectoryWalkerTask(void *param) {
    for (;;) {
        // Wait for timer tick
        if (!walker_tick) {
            vTaskDelay(1);  // yield, check again next tick
            continue;
        }
        walker_tick = false;

        // Only run during playback
        State state;
        xSemaphoreTake(g_state_mutex, portMAX_DELAY);
        state = g_state.current_state;
        xSemaphoreGive(g_state_mutex);

        if (state != State::PLAYING) continue;

        uint32_t elapsed_ms = (micros() - playback_start_us) / 1000;

        // Check for trajectory complete
        if (elapsed_ms >= g_trajectory.duration_ms) {
            if (g_trajectory.loop) {
                playback_start_us = micros();
                walker_index = 0;
                elapsed_ms = 0;
            } else {
                Command stop_cmd = { .type = CommandType::STOP };
                xQueueSend(cmd_queue, &stop_cmd, 0);
                continue;
            }
        }

        // Advance index to bracket current time
        // walker_index tracks the LEFT point of the current segment.
        // Points are in ascending t order, so we only need to scan forward.
        while (walker_index < g_trajectory.point_count - 2 &&
               g_trajectory.points[walker_index + 1].t_ms <= elapsed_ms) {
            walker_index++;
        }

        // Linear interpolation between points[walker_index] and points[walker_index + 1]
        TrajectoryPoint *p0 = &g_trajectory.points[walker_index];
        TrajectoryPoint *p1 = &g_trajectory.points[walker_index + 1];

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

        // Send to motor control
        setStepperTarget(interpolated[AXIS_SLIDE]);

        // Send to DJI gimbal (if axes configured)
        #if defined(AXIS_PAN) || defined(AXIS_TILT) || defined(AXIS_ROLL)
        setDjiTarget(interpolated);
        #endif

        // Push to telemetry queue (non-blocking, drop if full)
        static uint8_t telemetry_divider = 0;
        telemetry_divider++;
        // 100Hz walker / (100/TELEMETRY_RATE_HZ) = desired telemetry rate
        if (telemetry_divider >= (100 / TELEMETRY_RATE_HZ)) {
            telemetry_divider = 0;
            TelemetryPoint telem;
            telem.t_ms = elapsed_ms;
            memcpy(telem.axes, interpolated, sizeof(float) * g_axis_count);
            xQueueSend(telemetry_queue, &telem, 0);  // non-blocking
        }

        // Update shared state
        xSemaphoreTake(g_state_mutex, portMAX_DELAY);
        g_state.trajectory_elapsed_ms = elapsed_ms;
        memcpy((void *)g_state.positions, interpolated, sizeof(float) * g_axis_count);
        xSemaphoreGive(g_state_mutex);
    }
}
```

**Why linear interpolation and not something fancier:** All non-linearity (easing, splines) is pre-computed by the browser and baked into the point table. The points are dense enough (10-20ms apart) that linear interpolation between adjacent points is indistinguishable from the original curve. The board's job is to faithfully walk the table, not to re-interpret it.

**Walker index tracking vs binary search:** Since the walker always moves forward in time (except on loop reset or scrub), we track the current segment index and scan forward. This is O(1) per tick in normal playback. On `scrub`, we do a binary search to find the new index.

### DJI Command Rate Limiting

The trajectory walker runs at 100 Hz (10ms ticks), but the DJI gimbal SDK specifies a minimum position command interval of 100ms. Sending commands faster than this is unreliable and may be ignored by the gimbal.

**Solution:** A rate divider sends DJI position commands every 10th walker tick (100ms intervals). The stepper motor continues to receive targets at full 100 Hz — only DJI commands are rate-limited.

```cpp
// In trajectoryWalkerTask, after interpolation:

static const uint8_t DJI_CMD_INTERVAL_TICKS = 10;  // 100Hz / 10 = 10Hz DJI updates
static uint8_t dji_divider = 0;

// Stepper runs at full 100Hz
setStepperTarget(interpolated[AXIS_SLIDE]);

// DJI gimbal at 10Hz (every 100ms)
dji_divider++;
if (dji_divider >= DJI_CMD_INTERVAL_TICKS) {
    dji_divider = 0;
    // time_for_action = 1 (100ms) so the gimbal interpolates smoothly
    // between 10Hz position updates
    setDjiTarget(interpolated, /*time_for_action=*/1);
}
```

Each DJI position command includes `time_for_action = 1` (units of 100ms), which tells the gimbal to smoothly interpolate to the target position over 100ms. This bridges the gap between 10 Hz command updates and produces fluid motion.

> **Known constraint:** `DJI_CMD_INTERVAL_TICKS = 10` is derived from the SDK spec but should be validated during hardware testing (Milestone 3). If the gimbal handles faster updates reliably, the divider can be reduced for tighter tracking.

### Camera Trigger Events

During trajectory playback, the walker checks for camera trigger events embedded in the trajectory data. Events have a timestamp `t` (in ms). On each walker tick, if any event's `t` falls between the previous tick time and the current tick time, the corresponding DJI camera command is dispatched via the DJI CAN library.

**Event types:**
- `shutter` — trigger a still photo capture
- `record_start` — begin video recording
- `record_stop` — stop video recording

Events are fire-and-forget: the walker dispatches the CAN command and continues without waiting for a camera response. This avoids blocking the 100 Hz motion loop. If a camera command fails (e.g., CAN timeout), it is logged but does not affect motion playback.

---

## Stepper Module

### Interface

```cpp
// stepper.h

void stepperInit();                            // Configure FastAccelStepper, set pins
void stepperEnable();                          // Enable motor driver (pull EN low)
void stepperDisable();                         // Disable motor driver (release EN high)
void setStepperTarget(float position_mm);      // Set target position in mm
float getStepperPositionMm();                  // Read current position in mm
int32_t mmToSteps(float mm);                   // Convert mm to step count
float stepsToMm(int32_t steps);                // Convert step count to mm
void stepperHome();                            // Run homing sequence (blocking)
bool isStepperMoving();                        // True if motor is in motion
void stepperEmergencyStop();                   // Immediate halt, no deceleration
```

### Implementation

```cpp
// stepper.cpp

#include <FastAccelStepper.h>

FastAccelStepperEngine engine = FastAccelStepperEngine();
FastAccelStepper *stepper = nullptr;

// Drive parameters — GT2 belt, 20-tooth pulley, 200-step motor, TMC2209 at 1/16 microstepping
// GT2 20T pulley: 20 teeth × 2mm pitch = 40mm per revolution
// 200 steps × 16 microsteps = 3200 microsteps per revolution
// Resolution: 40mm / 3200 = 0.0125 mm per microstep (12.5 microns)
static const float STEPS_PER_MM = 3200.0f / 40.0f;  // 80 steps/mm
static const float MM_PER_STEP = 40.0f / 3200.0f;    // 0.0125 mm/step

void stepperInit() {
    engine.init();
    stepper = engine.stepperConnectToPin(PIN_STEP);
    if (!stepper) {
        Serial.println("ERROR: FastAccelStepper failed to connect");
        return;
    }

    stepper->setDirectionPin(PIN_DIR);
    stepper->setEnablePin(PIN_EN);
    stepper->setAutoEnable(false);  // We manage enable/disable explicitly

    // Speed and acceleration defaults for jog/move_to
    // 500 mm/sec max → 500 * 80 = 40,000 steps/sec
    stepper->setSpeedInHz(40000);
    // 2000 mm/sec² acceleration → 2000 * 80 = 160,000 steps/sec²
    stepper->setAcceleration(160000);
}

void setStepperTarget(float position_mm) {
    int32_t target_steps = mmToSteps(position_mm);
    stepper->moveTo(target_steps);
}

int32_t mmToSteps(float mm) {
    return (int32_t)(mm * STEPS_PER_MM);
}

float stepsToMm(int32_t steps) {
    return (float)steps * MM_PER_STEP;
}

float getStepperPositionMm() {
    return stepsToMm(stepper->getCurrentPosition());
}
```

### Homing Sequence

```cpp
void stepperHome() {
    // Phase 1: Move toward MIN limit at slow speed
    stepper->setSpeedInHz(4000);  // 50 mm/sec
    stepper->runBackward();       // Toward limit

    // Wait for limit switch
    while (digitalRead(PIN_LIMIT_MIN) == HIGH) {
        vTaskDelay(pdMS_TO_TICKS(1));
    }
    stepper->forceStop();
    vTaskDelay(pdMS_TO_TICKS(100));  // Settle

    // Phase 2: Back off slowly
    stepper->setSpeedInHz(800);   // 10 mm/sec
    stepper->move(mmToSteps(5));  // Move 5mm away from switch
    while (stepper->isRunning()) {
        vTaskDelay(pdMS_TO_TICKS(1));
    }

    // Phase 3: Approach again very slowly for precision
    stepper->setSpeedInHz(160);   // 2 mm/sec
    stepper->runBackward();
    while (digitalRead(PIN_LIMIT_MIN) == HIGH) {
        vTaskDelay(pdMS_TO_TICKS(1));
    }
    stepper->forceStop();

    // Set position to 0
    stepper->setCurrentPosition(0);

    // Restore normal speed
    stepper->setSpeedInHz(40000);
}
```

### Safety

- **Software limits:** Before any `moveTo`, check target against `g_axis_config[AXIS_SLIDE].min` and `.max` (converted to steps). Clamp if out of range.
- **Hardware limits:** Limit switch ISR calls `stepperEmergencyStop()` and transitions to `State::ERROR` with `ErrorCode::LIMIT_HIT`.
- **Motor stall:** If using TMC2209 UART, StallGuard can detect stalls. Otherwise, detect stall by checking if the motor reached its target position within an expected time.

```cpp
// Limit switch ISR — attached in setup()
void IRAM_ATTR onLimitSwitch() {
    if (stepper) stepper->forceStopAndNewPosition(stepper->getCurrentPosition());
    // Set error flag (atomic, safe from ISR)
    g_limit_hit = true;  // volatile bool, checked by motion control task
}
```

### Homing Behavior

On power-on, the slide position is "unknown" — the stepper position counter starts at 0 but this does not correspond to a physical location. The board accepts `jog` commands before homing; positions are relative until a home sequence establishes the origin.

**Homing sequence** (triggered by `home` WebSocket command):
1. Move toward the MIN limit switch at slow speed (50 mm/sec)
2. On limit switch trigger, stop and back off 5mm
3. Approach again at very slow speed (2 mm/sec) for precision
4. On second trigger, set position to 0

**`home_on_boot` setting** (default: `false`): When enabled, the homing sequence runs automatically during startup before the board accepts trajectory playback commands. Stored in NVS — see Settings Persistence below.

**Axes without limit switches** (DJI gimbal, servos) don't need homing. The gimbal reports its own absolute position via CAN, and servo positions are inherently known from the PWM signal. Only stepper axes with `can_home = true` in the axis config participate in homing.

Position is tracked continuously after homing until power-off. There is no need to re-home unless the stepper motor loses steps (e.g., due to a stall or manual movement while powered off).

---

## State Machine

### States and Transitions

```
                    ┌─────────────────────────────────────────────────────────┐
                    │                                                         │
                    ▼                                                         │
              ┌──────────┐  play (valid traj)  ┌──────────┐                  │
         ┌───►│   IDLE   │───────────────────►│ PLAYING  │                  │
         │    └──────────┘                     └──────────┘                  │
         │     │    │    │                      │    │    │                  │
         │     │    │    │move_to         pause │    │    │ stop /           │
         │     │    │home│                      ▼    │    │ complete         │
         │     │    │    ▼                ┌──────────┐│    │                  │
         │     │    │ ┌──────────┐        │  PAUSED  ││    │                  │
         │     │    │ │  MOVING  │◄──┐    └──────────┘│    │                  │
         │     │    │ └──────────┘   │     │    │     │    │                  │
         │     │    │  │  │  │  │    │     │    │     │    │                  │
         │     │    │  │  │  │  └────┘    resume│    │stop  │    │                  │
         │     │    │  │  │  │ new move_to│    │     │    │                  │
         │     │    │  │  │  │            │    ▼     │    ▼                  │
         │     │    ▼  │  │  │            ┌┘   ┌─────┘    │                  │
         │    ┌──────────┐ │  │            │    │          │                  │
         │    │  HOMING  │ │  │jog/stop/   │    │          │                  │
         │    └──────────┘ │  │complete    │    │          │                  │
         │         │       │  │            │    │          │                  │
         │         │ complete  │            │    │          │                  │
         │         │       │  │            │    │          │                  │
         │         ▼       ▼  ▼            │    │          │                  │
         └────────────── IDLE ◄────────────┘    │          │                  │
                    ▲                           │          │                  │
                    │  clear_error              ▼          ▼                  │
              ┌──────────┐◄──────── any state: limit hit, stall ─────────────┘
              │  ERROR   │
              └──────────┘
```

### Implementation

```cpp
// state_machine.h

enum class State : uint8_t {
    IDLE,
    PLAYING,
    PAUSED,
    HOMING,
    MOVING,
    ERROR
};

enum class ErrorCode : uint8_t {
    NONE,
    LIMIT_HIT,
    MOTOR_STALL,
    TRAJECTORY_INVALID,
    OUT_OF_MEMORY,
    COMMUNICATION_LOST,
    CAN_TIMEOUT
};

// Returns true if the transition is valid, false otherwise
bool tryTransition(State from, State to);

// Attempt a state change. Sends "state" event to WebSocket clients on success.
bool changeState(State new_state, ErrorCode error = ErrorCode::NONE);
```

```cpp
// state_machine.cpp

// Transition table: [from][to] = allowed
static const bool TRANSITIONS[6][6] = {
    //                IDLE   PLAYING PAUSED HOMING MOVING ERROR
    /* IDLE    */ {  false, true,   false, true,  true,  true  },
    /* PLAYING */ {  true,  false,  true,  false, false, true  },
    /* PAUSED  */ {  true,  true,   false, false, false, true  },
    /* HOMING  */ {  true,  false,  false, false, false, true  },
    /* MOVING  */ {  true,  false,  false, false, true,  true  },
    /* ERROR   */ {  true,  false,  false, false, false, false },
};

bool tryTransition(State from, State to) {
    return TRANSITIONS[(uint8_t)from][(uint8_t)to];
}

bool changeState(State new_state, ErrorCode error) {
    xSemaphoreTake(g_state_mutex, portMAX_DELAY);
    State current = g_state.current_state;

    if (!tryTransition(current, new_state)) {
        xSemaphoreGive(g_state_mutex);
        Serial.printf("Invalid transition: %d -> %d\n", (int)current, (int)new_state);
        return false;
    }

    g_state.current_state = new_state;
    g_state.error_code = error;
    xSemaphoreGive(g_state_mutex);

    // Notify WebSocket clients
    broadcastStateEvent(new_state, error);

    Serial.printf("State: %d -> %d\n", (int)current, (int)new_state);
    return true;
}
```

### Motion Control Task (command dispatcher)

The motion control task is the main loop on Core 1. It reads commands from `cmd_queue` and orchestrates stepper, DJI, and state transitions:

```cpp
void motionControlTask(void *param) {
    Command cmd;

    for (;;) {
        // Check for limit switch hit (set by ISR)
        if (g_limit_hit) {
            g_limit_hit = false;
            changeState(State::ERROR, ErrorCode::LIMIT_HIT);
            stopAllMotion();
        }

        // Process commands (non-blocking, 10ms timeout)
        if (xQueueReceive(cmd_queue, &cmd, pdMS_TO_TICKS(10)) == pdTRUE) {
            switch (cmd.type) {
                case CommandType::PLAY:
                    if (g_state.trajectory_loaded &&
                        strcmp(cmd.data.play.trajectory_id, g_trajectory.id) == 0) {
                        if (changeState(State::PLAYING)) {
                            startPlayback();
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
                    if (changeState(State::IDLE)) {
                        stopPlayback();
                        stopAllMotion();
                    }
                    break;

                case CommandType::JOG:
                    // Jog cancels an in-progress move_to
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
                        g_state.current_state == State::PLAYING) {
                        scrubToTime(cmd.data.scrub.t_ms);
                    }
                    break;

                case CommandType::CLEAR_ERROR:
                    if (g_state.current_state == State::ERROR) {
                        changeState(State::IDLE);
                    }
                    break;
            }
        }
    }
}
```

---

## Capabilities / Config System

Axis configuration is defined at compile time in `config.h`. Different build environments produce firmware for different hardware setups.

```cpp
// config.h — axis configuration

enum class AxisType : uint8_t {
    STEPPER,    // driven by FastAccelStepper (STEP/DIR/EN)
    DJI_CAN,    // driven by DJI gimbal over CAN bus
    SERVO       // driven by PWM signal
};

struct AxisConfig {
    const char *name;       // matches API axis names: "slide", "pan", "tilt", "roll"
    AxisType type;
    float min;              // minimum position in native units
    float max;              // maximum position in native units
    const char *unit;       // "mm", "deg", etc.
    bool can_home;          // has limit switches or reference position
};

// --- Build profile: slider + DJI gimbal (default) ---
#if !defined(BUILD_SLIDER_ONLY) && !defined(BUILD_SLIDER_SERVO)

static const AxisConfig g_axis_config[] = {
    { "slide", AxisType::STEPPER, 0.0f,    1000.0f, "mm",  true  },
    { "pan",   AxisType::DJI_CAN, -180.0f, 180.0f,  "deg", false },
    { "tilt",  AxisType::DJI_CAN, -90.0f,  90.0f,   "deg", false },
    { "roll",  AxisType::DJI_CAN, -45.0f,  45.0f,   "deg", false },
};
static const int g_axis_count = 4;

// --- Build profile: slider only ---
#elif defined(BUILD_SLIDER_ONLY)

static const AxisConfig g_axis_config[] = {
    { "slide", AxisType::STEPPER, 0.0f, 1000.0f, "mm", true },
};
static const int g_axis_count = 1;

// --- Build profile: slider + servo gimbal ---
#elif defined(BUILD_SLIDER_SERVO)

static const AxisConfig g_axis_config[] = {
    { "slide", AxisType::STEPPER, 0.0f,    1000.0f, "mm",  true  },
    { "pan",   AxisType::SERVO,   -180.0f, 180.0f,  "deg", false },
    { "tilt",  AxisType::SERVO,   -90.0f,  90.0f,   "deg", false },
    { "roll",  AxisType::SERVO,   -45.0f,  45.0f,   "deg", false },
};
static const int g_axis_count = 4;

#endif
```

### PlatformIO Build Environments

```ini
; Additional environments in platformio.ini

[env:slider_only]
extends = env:esp32s3
build_flags =
    ${env:esp32s3.build_flags}
    -DBUILD_SLIDER_ONLY

[env:slider_servo]
extends = env:esp32s3
build_flags =
    ${env:esp32s3.build_flags}
    -DBUILD_SLIDER_SERVO
```

### Capabilities Endpoint

```cpp
void handleGetCapabilities(AsyncWebServerRequest *request) {
    JsonDocument doc;

    JsonArray axes = doc["axes"].to<JsonArray>();
    for (int i = 0; i < g_axis_count; i++) {
        JsonObject axis = axes.add<JsonObject>();
        axis["name"] = g_axis_config[i].name;
        axis["min"]  = g_axis_config[i].min;
        axis["max"]  = g_axis_config[i].max;
        axis["unit"] = g_axis_config[i].unit;
    }

    doc["max_trajectory_points"] = MAX_TRAJECTORY_POINTS;
    doc["max_trajectory_duration_ms"] = 300000;  // 5 minutes
    doc["telemetry_rate_hz"] = TELEMETRY_RATE_HZ;
    doc["firmware_version"] = FIRMWARE_VERSION;
    doc["board"] = "esp32-s3";

    char buf[512];
    serializeJson(doc, buf);
    request->send(200, "application/json", buf);
}
```

---

## Display Module

### Abstract Interface

```cpp
// display.h

class DisplayInterface {
public:
    virtual ~DisplayInterface() = default;
    virtual void init() = 0;
    virtual void showBoot(const char *version) = 0;
    virtual void showStatus(const char *ssid, const char *ip, float battery_v) = 0;
    virtual void showPlayback(float progress, uint32_t elapsed_ms, uint32_t total_ms) = 0;
    virtual void showPositions(const float *positions, int axis_count) = 0;
    virtual void showError(const char *code, const char *detail) = 0;
    virtual void showHoming(const char *axis_name) = 0;
    virtual void clear() = 0;
};
```

### Implementations

```cpp
// display.cpp

// --- I2C OLED (SH1107 128x128) using U8g2 ---
class OledDisplay : public DisplayInterface {
    U8G2_SH1107_128X128_F_HW_I2C u8g2;
public:
    OledDisplay() : u8g2(U8G2_R0, U8X8_PIN_NONE) {}

    void init() override {
        u8g2.begin();
        u8g2.setFont(u8g2_font_6x10_tr);
    }

    void showStatus(const char *ssid, const char *ip, float battery_v) override {
        u8g2.clearBuffer();
        u8g2.setCursor(0, 12);
        u8g2.printf("SSID: %s", ssid);
        u8g2.setCursor(0, 26);
        u8g2.printf("IP: %s", ip);
        u8g2.setCursor(0, 40);
        u8g2.printf("Bat: %.1fV", battery_v);
        u8g2.sendBuffer();
    }

    void showPlayback(float progress, uint32_t elapsed_ms, uint32_t total_ms) override {
        u8g2.clearBuffer();
        u8g2.setCursor(0, 12);
        u8g2.print("PLAYING");
        // Progress bar
        u8g2.drawFrame(0, 20, 128, 12);
        u8g2.drawBox(2, 22, (int)(124 * progress), 8);
        u8g2.setCursor(0, 48);
        u8g2.printf("%lu / %lu s", elapsed_ms / 1000, total_ms / 1000);
        u8g2.sendBuffer();
    }

    // ... other methods follow the same pattern
};

// --- No-op (no display connected) ---
class NullDisplay : public DisplayInterface {
public:
    void init() override {}
    void showBoot(const char *) override {}
    void showStatus(const char *, const char *, float) override {}
    void showPlayback(float, uint32_t, uint32_t) override {}
    void showPositions(const float *, int) override {}
    void showError(const char *, const char *) override {}
    void showHoming(const char *) override {}
    void clear() override {}
};
```

### Display Task

```cpp
// Detect display at startup via I2C scan
DisplayInterface *createDisplay() {
    Wire.begin(PIN_SDA, PIN_SCL);
    Wire.beginTransmission(0x3C);  // SH1107 default address
    if (Wire.endTransmission() == 0) {
        Serial.println("OLED display detected at 0x3C");
        return new OledDisplay();
    }
    Serial.println("No display detected, using NullDisplay");
    return new NullDisplay();
}

void displayUpdateTask(void *param) {
    DisplayInterface *display = createDisplay();
    display->init();
    display->showBoot(FIRMWARE_VERSION);
    vTaskDelay(pdMS_TO_TICKS(2000));

    for (;;) {
        xSemaphoreTake(g_state_mutex, portMAX_DELAY);
        State state = g_state.current_state;
        float positions[MAX_AXES];
        memcpy(positions, (void *)g_state.positions, sizeof(float) * g_axis_count);
        uint32_t elapsed = g_state.trajectory_elapsed_ms;
        uint32_t total = g_state.trajectory_duration_ms;
        xSemaphoreGive(g_state_mutex);

        switch (state) {
            case State::IDLE:
                display->showStatus("Slider-AP", WiFi.softAPIP().toString().c_str(),
                                    readBatteryVoltage());
                break;
            case State::PLAYING:
                display->showPlayback((float)elapsed / total, elapsed, total);
                break;
            case State::PAUSED:
                display->showPlayback((float)elapsed / total, elapsed, total);
                break;
            case State::HOMING:
                display->showHoming("slide");
                break;
            case State::MOVING:
                display->showPositions(positions, g_axis_count);
                break;
            case State::ERROR:
                display->showError("ERROR", "");
                break;
        }

        vTaskDelay(pdMS_TO_TICKS(200));  // 5 Hz update rate
    }
}
```

---

## main.cpp — Startup Sequence

```cpp
#include <WiFi.h>
#include <DNSServer.h>
#include <ESPmDNS.h>
#include <LittleFS.h>

DNSServer dnsServer;

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

    // Pin modes
    pinMode(PIN_LIMIT_MIN, INPUT_PULLUP);
    pinMode(PIN_LIMIT_MAX, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(PIN_LIMIT_MIN), onLimitSwitch, FALLING);
    attachInterrupt(digitalPinToInterrupt(PIN_LIMIT_MAX), onLimitSwitch, FALLING);

    // Initialize subsystems
    stepperInit();

    // Create shared resources
    g_state_mutex = xSemaphoreCreateMutex();
    cmd_queue = xQueueCreate(16, sizeof(Command));
    telemetry_queue = xQueueCreate(4, sizeof(TelemetryPoint));

    // Start WiFi AP
    // AP-only mode for v1 — no station mode
    WiFi.mode(WIFI_AP);
    // Default SSID: "Slider-XXXX" where XXXX = last 4 hex digits of MAC
    // Overridable via NVS setting "ap_ssid"
    uint8_t mac[6];
    esp_read_mac(mac, ESP_MAC_WIFI_SOFTAP);
    char default_ssid[16];
    snprintf(default_ssid, sizeof(default_ssid), "Slider-%02X%02X", mac[4], mac[5]);
    const char *ssid = nvs_get_string("ap_ssid", default_ssid);
    const char *password = nvs_get_string("ap_password", "slider1234");
    WiFi.softAP(ssid, password);
    // Fixed IP: 192.168.4.1 (ESP32 AP default)
    Serial.printf("AP IP: %s\n", WiFi.softAPIP().toString().c_str());

    // Captive portal: redirect all DNS queries to our IP so connecting
    // devices automatically open the web UI
    dnsServer.start(53, "*", WiFi.softAPIP());

    // mDNS: advertise as slider.local (or {device_name}.local)
    const char *device_name = nvs_get_string("device_name", "slider");
    if (MDNS.begin(device_name)) {
        MDNS.addService("http", "tcp", 80);
        Serial.printf("mDNS: %s.local\n", device_name);
    }

    // Start web server
    setupWebServer();

    // Create tasks — Core 0 (network)
    xTaskCreatePinnedToCore(telemetryBroadcastTask, "telemetry", 4096, NULL, 1, NULL, 0);
    xTaskCreatePinnedToCore(displayUpdateTask,      "display",   4096, NULL, 1, NULL, 0);

    // Create tasks — Core 1 (motion)
    xTaskCreatePinnedToCore(motionControlTask,       "motion",    8192, NULL, 4, NULL, 1);
    xTaskCreatePinnedToCore(trajectoryWalkerTask,    "walker",    4096, NULL, 5, NULL, 1);

    // DJI CAN (if configured)
    #ifndef BUILD_SLIDER_ONLY
    djiCanInit(PIN_CAN_TX, PIN_CAN_RX);
    xTaskCreatePinnedToCore(djiCanTask,              "dji_can",   4096, NULL, 3, NULL, 1);
    #endif

    // Setup walker timer (100 Hz, but don't start it until playback)
    walker_timer = timerBegin(0, 80, true);  // prescaler 80 → 1 MHz tick
    timerAttachInterrupt(walker_timer, &onWalkerTimer, true);
    timerAlarmWrite(walker_timer, 10000, true);  // 10,000 us = 100 Hz
    // timerAlarmEnable called when playback starts

    Serial.println("Setup complete");
}

void loop() {
    // Arduino loop() runs on Core 0 at priority 1.
    // Process captive portal DNS requests.
    dnsServer.processNextRequest();
    vTaskDelay(pdMS_TO_TICKS(10));
}
```

---

## WiFi and Captive Portal

The board operates as a WiFi Access Point only (no station mode for v1).

| Setting | Default | Notes |
|---------|---------|-------|
| SSID | `Slider-XXXX` (last 4 hex of MAC) | Configurable via NVS `ap_ssid` |
| Password | `slider1234` | Configurable via NVS `ap_password` |
| IP address | `192.168.4.1` | Fixed (ESP32 AP default) |
| mDNS hostname | `slider.local` | Or `{device_name}.local` if `device_name` is set |

**Captive portal:** A `DNSServer` instance intercepts all DNS queries and responds with `192.168.4.1`. When a phone or laptop connects to the AP, the OS detects the captive portal and automatically opens the web UI. The DNS server runs in `loop()` on Core 0 and processes requests every 10ms.

**mDNS:** The board advertises itself via mDNS so devices that support it can reach the UI at `slider.local` (or `{device_name}.local`) without knowing the IP address. The `_http._tcp` service is also advertised for discovery.

---

## Settings Persistence (NVS)

Runtime-configurable settings are stored in the NVS (Non-Volatile Storage) partition and survive power cycles.

**Persisted settings:**

| Key | Type | Default | Notes |
|-----|------|---------|-------|
| `device_name` | string | `"slider"` | Used for mDNS hostname |
| `ap_ssid` | string | `"Slider-XXXX"` | WiFi AP name |
| `ap_password` | string | `"slider1234"` | WiFi AP password |
| `telemetry_rate_hz` | uint16 | `50` | WebSocket position update rate |
| `jog_sensitivity` | float | `1.0` | Multiplier for jog deltas |
| `home_on_boot` | bool | `false` | Auto-home stepper on startup |

**API:**
- `GET /api/settings` — returns all settings as JSON
- `PATCH /api/settings` — updates one or more settings, writes to NVS

**WiFi settings changes** (`ap_ssid`, `ap_password`) require a board restart to take effect. The `PATCH` response includes `"restart_required": true` when WiFi settings are modified. Other settings take effect immediately.

---

## WebSocket Client Handling

Multiple WebSocket clients can connect simultaneously. All connected clients receive telemetry broadcasts (position events, state change events).

**Command processing:** Commands from any client are placed into `cmd_queue` and processed in order of receipt. There is no client locking, session ownership, or command arbitration — last-writer-wins semantics apply. If two clients send conflicting commands (e.g., one sends `play` while another sends `stop`), whichever arrives at the queue first is executed first.

> **Known limitation (v1):** No multi-client arbitration. This is acceptable for the expected use case (single operator with one browser tab), but could cause confusion if multiple users connect simultaneously. A future version could add client locking (e.g., first client to send a motion command holds a lock, others receive a "locked" error).

---

## Memory Budget

### Flash (8MB)

| Partition | Size | Contents |
|-----------|------|----------|
| app0 (firmware A) | 3 MB | Compiled firmware (~1-1.5 MB typical) |
| app1 (firmware B) | 3 MB | OTA update slot |
| LittleFS | 1.875 MB | Web UI files (target: <500KB gzipped) |
| NVS | 20 KB | Device settings (see Settings Persistence), calibration data |
| OTA data | 8 KB | Boot partition selector |
| Core dump | 64 KB | Crash diagnostics |
| **Total** | **8 MB** | |

### SRAM (~512 KB total, ~320 KB free after WiFi stack)

| Consumer | Estimated Size | Notes |
|----------|---------------|-------|
| WiFi + TCP/IP stack | ~160 KB | Managed by ESP-IDF, not configurable |
| FreeRTOS task stacks | ~33 KB | Sum of all task stack allocations |
| Web server buffers | ~16 KB | ESPAsyncWebServer internal buffers |
| ArduinoJson documents | ~2 KB | Largest response document ~512 bytes |
| State machine + queues | ~2 KB | Queues, mutex, SystemState struct |
| FastAccelStepper | ~1 KB | Internal state |
| TWAI driver buffers | ~2 KB | TX/RX ring buffers |
| Display framebuffer | ~2 KB | U8g2 128x128 mono = 2048 bytes |
| **Total SRAM used** | **~218 KB** | |
| **Free SRAM** | **~102 KB** | Comfortable headroom |

### PSRAM (8 MB)

| Consumer | Estimated Size | Notes |
|----------|---------------|-------|
| Trajectory points (max) | 200 KB | 10,000 points × 20 bytes |
| Upload parse buffer (transient) | 200 KB | Freed after parsing |
| **Peak PSRAM used** | **~400 KB** | During upload |
| **Steady-state PSRAM used** | **~200 KB** | After parsing |
| **Free PSRAM** | **~7.6 MB** | Vast headroom |

---

## Native Test Strategy

Unit tests run on the host machine (macOS/Linux) using PlatformIO's `native` environment. No ESP32 hardware needed. Hardware-dependent code is guarded with `#ifdef NATIVE_TEST` and replaced with stubs/mocks.

### What to test

| Test Suite | What It Covers | Key Cases |
|------------|----------------|-----------|
| `test_trajectory` | JSON parsing → `TrajectoryPoint` array | Valid input; missing axis; non-ascending timestamps; out-of-range values; empty points array; max point count |
| `test_trajectory` | Walker interpolation logic | Midpoint between two points; exact timestamp match; before first point; after last point; single-point trajectory |
| `test_state_machine` | State transition validation | All valid transitions succeed; all invalid transitions fail; error from any state; error clear; double-transition |
| `test_message_parsing` | WebSocket JSON → `Command` struct | Each command type; missing `cmd` field; unknown command; missing required fields; malformed JSON |

### Test structure

```cpp
// test/test_trajectory/test_trajectory.cpp

#include <unity.h>
#include "trajectory.h"

void test_parse_valid_trajectory() {
    const char *json = R"({
        "points": [
            {"t": 0,    "slide": 0.0},
            {"t": 1000, "slide": 100.0},
            {"t": 2000, "slide": 200.0}
        ],
        "loop": false
    })";

    bool ok = parseTrajectoryJson(json, strlen(json));
    TEST_ASSERT_TRUE(ok);
    TEST_ASSERT_EQUAL(3, g_trajectory.point_count);
    TEST_ASSERT_EQUAL(2000, g_trajectory.duration_ms);
    TEST_ASSERT_FLOAT_WITHIN(0.01, 100.0, g_trajectory.points[1].axes[0]);
}

void test_parse_non_ascending_timestamps() {
    const char *json = R"({
        "points": [
            {"t": 1000, "slide": 100.0},
            {"t": 500,  "slide": 50.0}
        ]
    })";
    TEST_ASSERT_FALSE(parseTrajectoryJson(json, strlen(json)));
}

void test_walker_interpolation_midpoint() {
    // Set up a 2-point trajectory
    g_trajectory.point_count = 2;
    g_trajectory.points[0] = { .t_ms = 0,    .axes = {0.0f} };
    g_trajectory.points[1] = { .t_ms = 1000, .axes = {100.0f} };

    float result[MAX_AXES];
    interpolateAtTime(500, result);  // midpoint
    TEST_ASSERT_FLOAT_WITHIN(0.01, 50.0, result[0]);
}

void test_state_idle_to_playing() {
    g_state.current_state = State::IDLE;
    TEST_ASSERT_TRUE(tryTransition(State::IDLE, State::PLAYING));
}

void test_state_error_to_playing_invalid() {
    g_state.current_state = State::ERROR;
    TEST_ASSERT_FALSE(tryTransition(State::ERROR, State::PLAYING));
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_parse_valid_trajectory);
    RUN_TEST(test_parse_non_ascending_timestamps);
    RUN_TEST(test_walker_interpolation_midpoint);
    RUN_TEST(test_state_idle_to_playing);
    RUN_TEST(test_state_error_to_playing_invalid);
    return UNITY_END();
}
```

### Mocking hardware dependencies

For `native` builds, mock out:

- `FastAccelStepper` — not included (via `lib_ignore`). Stepper functions are stubbed to record calls.
- `WiFi`, `ESPAsyncWebServer` — not included. Web server tests parse JSON directly, not via HTTP.
- `ps_malloc` — mapped to regular `malloc` on host.
- `xQueueSend`, `xSemaphoreTake`, etc. — thin stubs that pass data through without actual FreeRTOS.

```cpp
// test/mocks/psram_mock.h
#ifdef NATIVE_TEST
#define ps_malloc(size) malloc(size)
#define ps_calloc(n, size) calloc(n, size)
#endif
```

---

## Hardware Validation Milestones

Sequential milestones, each building on the previous. Each milestone has a concrete pass/fail test.

### Milestone 1: ESP32-S3 Only (no motors, no gimbal)

**Hardware:** ESP32-S3 DevKitC-1 + USB cable.

| Test | Pass Criteria |
|------|---------------|
| WiFi AP starts | Phone sees "Slider-AP" network, can connect |
| mDNS works | `slider.local` resolves from connected device |
| Static file serving | Browser at `http://slider.local` loads test page from LittleFS |
| `GET /api/status` | Returns valid JSON with `state: "idle"` |
| `GET /api/capabilities` | Returns axis config matching build profile |
| `POST /api/trajectory` | Upload a 100-point trajectory, get back ID and point count |
| WebSocket connects | Browser connects to `ws://slider.local/ws`, receives `state` event |
| WebSocket commands | Send `play`/`pause`/`stop` commands, observe state transitions in Serial output |
| Telemetry broadcast | During simulated playback, position events arrive at ~50 Hz |
| OTA update | Upload new firmware via ArduinoOTA, board reboots with new version |

### Milestone 2: ESP32-S3 + Stepper Motor

**Hardware:** add TMC2209 driver + NEMA 17 motor + 2 limit switches + 24V supply.

| Test | Pass Criteria |
|------|---------------|
| Motor responds to jog | Send `jog` command, motor turns proportionally |
| `move_to` works | Send `move_to` with position and duration, motor moves smoothly |
| Homing sequence | Send `home` command, motor finds limit switch and zeroes position |
| Trajectory playback | Upload a 30-second trajectory, play it, motor follows expected path |
| Telemetry accuracy | Position telemetry matches expected trajectory within 1mm |
| Limit switch safety | Manually trigger limit switch during motion, motor stops, state goes to ERROR |
| Software limits | Send `move_to` beyond max, position is clamped |
| StealthChop quiet mode | Motor is inaudibly quiet at slow speeds (<50mm/sec) |

### Milestone 3: ESP32-S3 + Stepper + DJI Gimbal

**Hardware:** add TJA1051 CAN transceiver + DJI RS gimbal + CAN cable.

| Test | Pass Criteria |
|------|---------------|
| CAN bus active | TWAI driver initialized, no bus errors in Serial output |
| DJI handshake | Gimbal responds to activation sequence (see plan-dji-can.md) |
| Position command | Send gimbal to pan=45, tilt=-10 — gimbal moves to position |
| Telemetry from gimbal | Gimbal reports actual pan/tilt/roll, matches commanded position |
| Multi-axis trajectory | Upload trajectory with slide + pan + tilt, play it, both motor and gimbal follow |
| Concurrent stress | Run trajectory playback while continuously refreshing web UI — no missed steps, no CAN timeouts |
| Error recovery | Disconnect CAN cable during motion — state goes to ERROR with CAN_TIMEOUT, reconnect and clear error |

---

## Open Questions

1. **TMC2209 UART configuration.** Should the firmware configure the driver at startup (set microstepping, current, StealthChop mode) or assume the user sets hardware jumpers? UART config is more flexible but adds a dependency.

2. **Trajectory upload size limits.** ESPAsyncWebServer's default body size limit may be too small for large trajectories. Need to test with 200KB+ payloads and increase if needed.

3. ~~**WiFi AP credentials.**~~ Resolved: stored in NVS, configurable via `PATCH /api/settings`. See Settings Persistence section.

4. **`move_to` implementation.** The board must generate a smooth linear ramp internally for `move_to` commands. The system transitions to `MOVING` state (see State Machine). Options: (a) generate a mini-trajectory and feed it to the walker, or (b) use FastAccelStepper's built-in speed/acceleration profile and handle DJI axes separately. Option (b) is simpler for stepper-only but splits the motion path for mixed axes. DJI commands during `move_to` are subject to the same 100ms rate limiting as trajectory playback.

5. **Walker timer precision.** The hardware timer runs at 100 Hz. If a trajectory has points spaced at 10ms and the timer fires at 10ms intervals, alignment jitter could skip points. May need to increase walker rate to 200 Hz or use the point spacing to set the timer frequency dynamically.

---

## References

- [Architecture](../architecture.md) — system layer diagram, hybrid trajectory model
- [Board API Spec](../board-api.md) — REST + WebSocket protocol this firmware implements
- [ADR-001: Microcontroller](../adr/adr-001-microcontroller.md) — ESP32-S3, PlatformIO, dual-core, FastAccelStepper, TWAI
- [ADR-002: Status Display](../adr/adr-002-status-display.md) — SH1107 I2C OLED default, ST7789 SPI TFT alternative
- [ADR-003: Linear Drive](../adr/adr-003-linear-drive.md) — belt vs ball screw, drive ratio constants
- [BOM](../bom.md) — GPIO pin budget, power architecture, component costs
