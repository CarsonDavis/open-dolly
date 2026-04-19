#pragma once

#include <cstdint>

#ifndef MAX_AXES
#define MAX_AXES 4
#endif

// ---------------------------------------------------------------------------
// Inter-task command types
// ---------------------------------------------------------------------------

enum class CommandType : uint8_t {
    PLAY,
    PAUSE,
    RESUME,
    STOP,
    JOG,
    MOVE_TO,
    HOME,
    SCRUB,
    CLEAR_ERROR,
    DISABLE
};

struct Command {
    CommandType type;

    union {
        // PLAY: trajectory ID to play
        struct {
            char trajectory_id[16];
        } play;

        // JOG: delta per axis
        struct {
            float axes[MAX_AXES];
        } jog;

        // MOVE_TO: absolute target per axis + duration
        struct {
            float    axes[MAX_AXES];
            uint32_t duration_ms;
        } move_to;

        // HOME: bitmask of axes to home (bit 0 = axis 0, etc.)
        struct {
            uint8_t axis_mask;
        } home;

        // SCRUB: jump to a point in the loaded trajectory
        struct {
            uint32_t t_ms;
        } scrub;
    } data;
};

// ---------------------------------------------------------------------------
// Telemetry point (snapshot pushed to WebSocket clients)
// ---------------------------------------------------------------------------

struct TelemetryPoint {
    uint32_t t_ms;
    float    axes[MAX_AXES];
};
