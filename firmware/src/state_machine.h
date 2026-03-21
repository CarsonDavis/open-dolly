#pragma once

#include <cstdint>

#ifndef MAX_AXES
#define MAX_AXES 4
#endif

#ifndef NATIVE_TEST
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#endif

// ---------------------------------------------------------------------------
// States and error codes
// ---------------------------------------------------------------------------

enum class State : uint8_t {
    IDLE    = 0,
    PLAYING = 1,
    PAUSED  = 2,
    HOMING  = 3,
    MOVING  = 4,
    ERROR   = 5
};

static constexpr uint8_t STATE_COUNT = 6;

enum class ErrorCode : uint8_t {
    NONE               = 0,
    LIMIT_HIT          = 1,
    MOTOR_STALL        = 2,
    TRAJECTORY_INVALID = 3,
    OUT_OF_MEMORY      = 4,
    COMMUNICATION_LOST = 5,
    CAN_TIMEOUT        = 6,
    INVALID_STATE      = 7
};

// ---------------------------------------------------------------------------
// Shared system state snapshot
// ---------------------------------------------------------------------------

struct SystemState {
    volatile State     current_state;
    volatile ErrorCode error_code;
    volatile float     positions[MAX_AXES];
    volatile bool      trajectory_loaded;
    char               trajectory_id[16];
    volatile uint32_t  trajectory_duration_ms;
    volatile uint32_t  trajectory_elapsed_ms;
    volatile uint16_t  trajectory_point_count;
};

// ---------------------------------------------------------------------------
// Globals
// ---------------------------------------------------------------------------

extern SystemState g_state;

#ifndef NATIVE_TEST
extern SemaphoreHandle_t g_state_mutex;
#endif

// ---------------------------------------------------------------------------
// API
// ---------------------------------------------------------------------------

/// Check whether a transition from `from` to `to` is allowed.
bool tryTransition(State from, State to);

/// Attempt to change the global state. Takes the mutex, validates the
/// transition, updates g_state, and broadcasts a WebSocket event.
/// Returns true if the transition succeeded.
bool changeState(State new_state, ErrorCode error = ErrorCode::NONE);

/// Human-readable name for a state.
const char* stateName(State s);

/// Human-readable name for an error code.
const char* errorCodeName(ErrorCode e);
