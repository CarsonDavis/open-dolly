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

        // Build JSON message matching Board API spec:
        // {"evt": "position", "t": 12400, "slide": 45.20, "pan": 0.00, ...}
        JsonDocument doc;
        doc["evt"] = "position";
        doc["t"]   = tp.t_ms;

        for (uint8_t a = 0; a < g_axis_count; a++) {
            doc[g_axis_config[a].name] = serialized(String(tp.axes[a], 2));
        }

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
