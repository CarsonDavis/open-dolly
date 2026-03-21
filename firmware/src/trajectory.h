#pragma once

#include <cstdint>
#include <cstddef>

#ifndef MAX_AXES
#define MAX_AXES 4
#endif

// ---------------------------------------------------------------------------
// Data types
// ---------------------------------------------------------------------------

struct TrajectoryPoint {
    uint32_t t_ms;
    float    axes[MAX_AXES];
};

struct TrajectoryEvent {
    uint32_t t_ms;
    uint8_t  type;   // 0 = shutter, 1 = record_start, 2 = record_stop
};

struct Trajectory {
    char             id[16];
    uint16_t         point_count;
    uint32_t         duration_ms;
    bool             loop;
    TrajectoryPoint* points;
    TrajectoryEvent* events;
    uint16_t         event_count;
};

// ---------------------------------------------------------------------------
// Globals
// ---------------------------------------------------------------------------

extern Trajectory g_trajectory;

// ---------------------------------------------------------------------------
// API
// ---------------------------------------------------------------------------

/// Parse a JSON trajectory payload into g_trajectory.
/// Allocates memory for points and events arrays (frees any previous).
/// Returns true on success.
bool parseTrajectoryJson(const char* json, size_t len);

/// Free the trajectory's dynamically-allocated arrays.
void freeTrajectory();

/// Linearly interpolate axis positions at the given time.
/// `result` must point to an array of at least g_axis_count floats.
/// `hint` is an optional starting index for forward scanning (pass 0 for
/// arbitrary access — the function falls back to binary search).
void interpolateAtTime(uint32_t t_ms, float* result, uint16_t hint = 0);
