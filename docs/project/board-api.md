# Board API Specification

*Status: draft*
*Last updated: 2026-03-20*

The board API is the contract boundary between the web UI (or any client) and the control board. It uses REST for configuration and trajectory upload, and WebSocket for real-time control and telemetry.

All messages are JSON. The board serves both the REST and WebSocket endpoints over Wi-Fi.

---

## REST Endpoints

### `GET /api/status`

Returns current system state and axis positions.

```json
{
  "state": "idle",
  "position": {
    "pan": 0.0,
    "tilt": 0.0,
    "roll": 0.0,
    "slide": 0.0
  },
  "battery": 82,
  "trajectory_loaded": null
}
```

`state` is one of: `idle`, `playing`, `paused`, `homing`, `error`.

---

### `GET /api/capabilities`

Returns hardware configuration. The web UI uses this to adapt to whatever axes exist.

```json
{
  "axes": [
    { "name": "pan",   "min": -180.0, "max": 180.0, "unit": "deg" },
    { "name": "tilt",  "min": -90.0,  "max": 90.0,  "unit": "deg" },
    { "name": "roll",  "min": -45.0,  "max": 45.0,  "unit": "deg" },
    { "name": "slide", "min": 0.0,    "max": 1000.0, "unit": "mm" }
  ],
  "max_trajectory_points": 10000,
  "max_trajectory_duration_ms": 300000,
  "telemetry_rate_hz": 50,
  "firmware_version": "1.0.0",
  "board": "esp32-s3"
}
```

This is the main coupling point between the UI and hardware. A board with only a slider and pan motor would report 2 axes. The UI renders controls for whatever axes are present.

---

### `POST /api/trajectory`

Uploads a pre-computed trajectory for playback.

**Request:**

```json
{
  "points": [
    { "t": 0,    "pan": 0.0,  "tilt": 0.0, "roll": 0.0, "slide": 0.0   },
    { "t": 10,   "pan": 0.1,  "tilt": 0.0, "roll": 0.0, "slide": 1.7   },
    { "t": 20,   "pan": 0.4,  "tilt": 0.0, "roll": 0.0, "slide": 6.5   },
    { "t": 30,   "pan": 0.9,  "tilt": 0.0, "roll": 0.0, "slide": 14.2  }
  ],
  "loop": false
}
```

- `t` is milliseconds from start.
- Each point contains a value for every axis reported by `/api/capabilities`.
- Points must be in ascending `t` order.
- The browser pre-computes all interpolation, easing, and per-axis timing before sending — the board does not need to understand any of that.

**Response:**

```json
{
  "trajectory_id": "abc123",
  "point_count": 3000,
  "duration_ms": 30000
}
```

---

### `DELETE /api/trajectory/:id`

Removes a stored trajectory from board memory.

---

### `GET /api/trajectory/:id/status`

Returns playback progress for a trajectory.

```json
{
  "state": "playing",
  "elapsed_ms": 12400,
  "progress": 0.41
}
```

---

## WebSocket Messages

Connect to `ws://<board-ip>/ws`. All messages are JSON with a `cmd` (client→board) or `evt` (board→client) field.

### Client → Board

#### Playback control

```json
{ "cmd": "play", "trajectory_id": "abc123" }
{ "cmd": "pause" }
{ "cmd": "resume" }
{ "cmd": "stop" }
{ "cmd": "scrub", "t": 15000 }
```

`scrub` jumps to a timestamp within the loaded trajectory — useful for previewing a specific point without playing the full sequence.

#### Manual control

```json
{ "cmd": "jog", "pan": 2.5, "tilt": -1.0 }
```

Relative movement. Values are in the axis's native unit (degrees, mm). Sent continuously while the user drags/joysticks. Board applies at whatever rate it receives.

```json
{ "cmd": "move_to", "pan": 45.0, "tilt": -10.0, "roll": 0.0, "slide": 500.0, "duration_ms": 1000 }
```

Absolute move to a position over a specified duration. Used for "go to keyframe" type navigation. The board generates a simple linear ramp internally.

#### Homing

```json
{ "cmd": "home", "axes": ["slide"] }
```

Triggers the homing sequence for the specified axes. Only relevant for axes with limit switches or encoders.

---

### Board → Client

#### Position telemetry

Sent at a regular rate (configurable, default 50Hz) during any motion.

```json
{ "evt": "position", "t": 12400, "pan": 34.2, "tilt": -8.1, "roll": 0.0, "slide": 423.5 }
```

`t` is milliseconds since playback started (0 during manual control).

#### State changes

```json
{ "evt": "state", "state": "playing" }
```

Emitted whenever the board transitions between states: `idle`, `playing`, `paused`, `homing`, `error`.

#### Playback complete

```json
{ "evt": "complete", "trajectory_id": "abc123" }
```

#### Errors

```json
{ "evt": "error", "code": "LIMIT_HIT", "axis": "slide", "detail": "max limit reached" }
```

Error codes: `LIMIT_HIT`, `MOTOR_STALL`, `TRAJECTORY_INVALID`, `OUT_OF_MEMORY`, `COMMUNICATION_LOST`.
