#include "state_machine.h"
#include <cstring>

// ---------------------------------------------------------------------------
// Globals
// ---------------------------------------------------------------------------

SystemState g_state = {};

#ifndef NATIVE_TEST
SemaphoreHandle_t g_state_mutex = nullptr;
#endif

// ---------------------------------------------------------------------------
// Broadcast (implemented in web_server.cpp; stub under NATIVE_TEST)
// ---------------------------------------------------------------------------

#include "web_server.h"

// ---------------------------------------------------------------------------
// Transition table
// ---------------------------------------------------------------------------
// Rows = from-state, Columns = to-state
// Order: IDLE, PLAYING, PAUSED, HOMING, MOVING, ERROR

static const bool TRANSITIONS[STATE_COUNT][STATE_COUNT] = {
    //          IDLE   PLAY   PAUS   HOME   MOVE   ERR
    /* IDLE */  { false, true,  false, true,  true,  true  },
    /* PLAY */  { true,  false, true,  false, false, true  },
    /* PAUS */  { true,  true,  false, false, false, true  },
    /* HOME */  { true,  false, false, false, false, true  },
    /* MOVE */  { true,  false, false, false, true,  true  },
    /* ERR  */  { true,  false, false, false, false, false },
};

bool tryTransition(State from, State to) {
    uint8_t f = static_cast<uint8_t>(from);
    uint8_t t = static_cast<uint8_t>(to);
    if (f >= STATE_COUNT || t >= STATE_COUNT) return false;
    return TRANSITIONS[f][t];
}

// ---------------------------------------------------------------------------
// Mutex helpers
// ---------------------------------------------------------------------------

#ifndef NATIVE_TEST

static inline bool takeMutex() {
    if (!g_state_mutex) return false;
    return xSemaphoreTake(g_state_mutex, pdMS_TO_TICKS(100)) == pdTRUE;
}

static inline void giveMutex() {
    xSemaphoreGive(g_state_mutex);
}

#else

// Native-test: no real mutex
static inline bool takeMutex() { return true; }
static inline void giveMutex() {}

#endif

// ---------------------------------------------------------------------------
// changeState
// ---------------------------------------------------------------------------

bool changeState(State new_state, ErrorCode error) {
    if (!takeMutex()) return false;

    bool ok = tryTransition(g_state.current_state, new_state);
    if (ok) {
        g_state.current_state = new_state;
        g_state.error_code    = error;
    }

    giveMutex();

    if (ok) {
        broadcastStateEvent();
    }

    return ok;
}

// ---------------------------------------------------------------------------
// Name helpers
// ---------------------------------------------------------------------------

const char* stateName(State s) {
    switch (s) {
        case State::IDLE:    return "idle";
        case State::PLAYING: return "playing";
        case State::PAUSED:  return "paused";
        case State::HOMING:  return "homing";
        case State::MOVING:  return "moving";
        case State::ERROR:   return "error";
        default:             return "unknown";
    }
}

const char* errorCodeName(ErrorCode e) {
    switch (e) {
        case ErrorCode::NONE:               return "none";
        case ErrorCode::LIMIT_HIT:          return "limit_hit";
        case ErrorCode::MOTOR_STALL:        return "motor_stall";
        case ErrorCode::TRAJECTORY_INVALID: return "trajectory_invalid";
        case ErrorCode::OUT_OF_MEMORY:      return "out_of_memory";
        case ErrorCode::COMMUNICATION_LOST: return "communication_lost";
        case ErrorCode::CAN_TIMEOUT:        return "can_timeout";
        case ErrorCode::INVALID_STATE:      return "invalid_state";
        default:                            return "unknown";
    }
}
