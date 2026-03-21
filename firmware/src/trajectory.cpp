#include "trajectory.h"
#include "config.h"
#include <cstring>
#include <cstdlib>

#include <ArduinoJson.h>

#ifndef NATIVE_TEST
#include <esp32-hal-psram.h>
#endif

// ---------------------------------------------------------------------------
// Globals
// ---------------------------------------------------------------------------

Trajectory g_trajectory = {};

// ---------------------------------------------------------------------------
// Memory helpers
// ---------------------------------------------------------------------------

static void* allocMemory(size_t bytes) {
#ifndef NATIVE_TEST
    // Prefer PSRAM on ESP32-S3
    void* p = ps_malloc(bytes);
    if (!p) p = malloc(bytes);
    return p;
#else
    return malloc(bytes);
#endif
}

void freeTrajectory() {
    if (g_trajectory.points) {
        free(g_trajectory.points);
        g_trajectory.points = nullptr;
    }
    if (g_trajectory.events) {
        free(g_trajectory.events);
        g_trajectory.events = nullptr;
    }
    g_trajectory.point_count = 0;
    g_trajectory.event_count = 0;
    g_trajectory.duration_ms = 0;
    g_trajectory.loop = false;
    memset(g_trajectory.id, 0, sizeof(g_trajectory.id));
}

// ---------------------------------------------------------------------------
// JSON parsing
// ---------------------------------------------------------------------------

bool parseTrajectoryJson(const char* json, size_t len) {
    // Free any previous trajectory
    freeTrajectory();

    // Use a filter to reduce memory usage — we only need specific keys
    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, json, len);
    if (err) {
        return false;
    }

    // --- ID ---
    const char* id = doc["id"] | "";
    strncpy(g_trajectory.id, id, sizeof(g_trajectory.id) - 1);
    g_trajectory.id[sizeof(g_trajectory.id) - 1] = '\0';

    // --- Loop flag ---
    g_trajectory.loop = doc["loop"] | false;

    // --- Points ---
    JsonArray pts = doc["points"].as<JsonArray>();
    if (pts.isNull() || pts.size() == 0) {
        return false;
    }

    uint16_t count = static_cast<uint16_t>(pts.size());
    if (count > MAX_TRAJECTORY_POINTS) {
        return false;
    }

    g_trajectory.points = static_cast<TrajectoryPoint*>(
        allocMemory(count * sizeof(TrajectoryPoint)));
    if (!g_trajectory.points) {
        return false;
    }

    uint32_t prev_t = 0;
    uint16_t idx = 0;
    for (JsonObject pt : pts) {
        TrajectoryPoint& tp = g_trajectory.points[idx];

        tp.t_ms = pt["t"] | (uint32_t)0;

        // Validate ascending timestamps (first point may be 0)
        if (idx > 0 && tp.t_ms <= prev_t) {
            freeTrajectory();
            return false;
        }
        prev_t = tp.t_ms;

        // Parse axis values by name (matches board-api.md format)
        for (uint8_t a = 0; a < g_axis_count && a < MAX_AXES; a++) {
            tp.axes[a] = pt[g_axis_config[a].name] | 0.0f;

            // Validate axis value within limits
            if (tp.axes[a] < g_axis_config[a].min ||
                tp.axes[a] > g_axis_config[a].max) {
                freeTrajectory();
                return false;
            }
        }
        // Zero-fill unused axes
        for (uint8_t a = g_axis_count; a < MAX_AXES; a++) {
            tp.axes[a] = 0.0f;
        }

        idx++;
    }

    g_trajectory.point_count = count;
    g_trajectory.duration_ms = g_trajectory.points[count - 1].t_ms;

    // --- Events (optional) ---
    JsonArray evts = doc["events"].as<JsonArray>();
    if (!evts.isNull() && evts.size() > 0) {
        uint16_t ecount = static_cast<uint16_t>(evts.size());
        g_trajectory.events = static_cast<TrajectoryEvent*>(
            allocMemory(ecount * sizeof(TrajectoryEvent)));
        if (g_trajectory.events) {
            uint16_t ei = 0;
            for (JsonObject ev : evts) {
                g_trajectory.events[ei].t_ms = ev["t"] | (uint32_t)0;
                // Map event type string to uint8_t
                const char* type_str = ev["type"] | "shutter";
                if (strcmp(type_str, "shutter") == 0)
                    g_trajectory.events[ei].type = 0;
                else if (strcmp(type_str, "record_start") == 0)
                    g_trajectory.events[ei].type = 1;
                else if (strcmp(type_str, "record_stop") == 0)
                    g_trajectory.events[ei].type = 2;
                else
                    g_trajectory.events[ei].type = 0;
                ei++;
            }
            g_trajectory.event_count = ecount;
        }
    }

    return true;
}

// ---------------------------------------------------------------------------
// Interpolation
// ---------------------------------------------------------------------------

/// Binary search: find the largest index i such that points[i].t_ms <= t_ms
static uint16_t findBracketBinary(uint32_t t_ms, const TrajectoryPoint* pts,
                                  uint16_t count) {
    uint16_t lo = 0;
    uint16_t hi = count - 1;
    while (lo < hi) {
        uint16_t mid = lo + (hi - lo + 1) / 2;
        if (pts[mid].t_ms <= t_ms) {
            lo = mid;
        } else {
            hi = mid - 1;
        }
    }
    return lo;
}

void interpolateAtTime(uint32_t t_ms, float* result, uint16_t hint) {
    if (!g_trajectory.points || g_trajectory.point_count == 0) {
        for (uint8_t a = 0; a < g_axis_count; a++) result[a] = 0.0f;
        return;
    }

    const uint16_t count = g_trajectory.point_count;
    const TrajectoryPoint* pts = g_trajectory.points;

    // Clamp to trajectory bounds
    if (t_ms <= pts[0].t_ms) {
        for (uint8_t a = 0; a < g_axis_count; a++) result[a] = pts[0].axes[a];
        return;
    }
    if (t_ms >= pts[count - 1].t_ms) {
        for (uint8_t a = 0; a < g_axis_count; a++)
            result[a] = pts[count - 1].axes[a];
        return;
    }

    // Find bracket: pts[lo].t_ms <= t_ms < pts[lo+1].t_ms
    uint16_t lo;
    if (hint > 0 && hint < count - 1 && pts[hint].t_ms <= t_ms) {
        // Forward scan from hint
        lo = hint;
        while (lo < count - 2 && pts[lo + 1].t_ms <= t_ms) {
            lo++;
        }
    } else {
        lo = findBracketBinary(t_ms, pts, count);
    }

    // Ensure we have a valid bracket
    uint16_t hi_idx = lo + 1;
    if (hi_idx >= count) {
        for (uint8_t a = 0; a < g_axis_count; a++) result[a] = pts[lo].axes[a];
        return;
    }

    // Linear interpolation
    uint32_t dt = pts[hi_idx].t_ms - pts[lo].t_ms;
    if (dt == 0) {
        for (uint8_t a = 0; a < g_axis_count; a++) result[a] = pts[lo].axes[a];
        return;
    }

    float alpha = static_cast<float>(t_ms - pts[lo].t_ms) /
                  static_cast<float>(dt);

    for (uint8_t a = 0; a < g_axis_count; a++) {
        result[a] = pts[lo].axes[a] +
                    alpha * (pts[hi_idx].axes[a] - pts[lo].axes[a]);
    }
}
