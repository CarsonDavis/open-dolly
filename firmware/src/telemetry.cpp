#include "telemetry.h"

#ifndef NATIVE_TEST

#include "commands.h"
#include "config.h"
#include "state_machine.h"

#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

// ---------------------------------------------------------------------------
// External references (owned by the main WebSocket server module)
// ---------------------------------------------------------------------------

extern AsyncWebSocket  ws;
extern QueueHandle_t   telemetry_queue;

// ---------------------------------------------------------------------------
// Telemetry broadcast task
// ---------------------------------------------------------------------------

void telemetryBroadcastTask(void* param) {
    (void)param;

    TelemetryPoint tp;
    const TickType_t timeout = pdMS_TO_TICKS(100);

    for (;;) {
        // Block until a telemetry point is available
        if (xQueueReceive(telemetry_queue, &tp, timeout) != pdTRUE) {
            continue;
        }

        // Skip if no WebSocket clients are connected
        if (ws.count() == 0) {
            continue;
        }

        // Build JSON message
        JsonDocument doc;
        doc["type"] = "telemetry";
        doc["t"]    = tp.t_ms;

        JsonObject pos = doc["position"].to<JsonObject>();
        for (uint8_t a = 0; a < g_axis_count; a++) {
            pos[g_axis_config[a].name] = serialized(String(tp.axes[a], 2));
        }

        doc["state"] = stateName(g_state.current_state);

        // Serialize and broadcast
        size_t len = measureJson(doc);
        char* buf = static_cast<char*>(malloc(len + 1));
        if (buf) {
            serializeJson(doc, buf, len + 1);
            ws.textAll(buf, len);
            free(buf);
        }
    }
}

#endif // NATIVE_TEST
