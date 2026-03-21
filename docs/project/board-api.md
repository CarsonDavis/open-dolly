# Board API Specification

*Status: draft*
*Last updated: 2026-03-20*

The board API is the contract boundary between the web UI (or any client) and the control board. It uses REST for configuration and trajectory upload, and WebSocket for real-time control and telemetry.

All messages are JSON. The board serves both the REST and WebSocket endpoints over Wi-Fi.

---

## Connection & Discovery

- **AP mode:** The board creates its own WiFi network. The SSID comes from the `ap_ssid` setting (default `OpenDolly-XXXX`, where `XXXX` is derived from the board's MAC address).
- **Board IP:** Always `192.168.4.1` in AP mode.
- **Captive portal:** When a device joins the AP, it is automatically redirected to the web UI.
- **mDNS:** The board advertises as `opendolly.local` (configurable via the `device_name` setting, e.g. a board named `Dolly-A3F2` advertises as `dolly-a3f2.local`).

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

`state` is one of: `idle`, `moving`, `playing`, `paused`, `homing`, `error`.

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
  "events": [
    { "t": 5000, "type": "shutter" },
    { "t": 15000, "type": "record_start" },
    { "t": 25000, "type": "record_stop" }
  ],
  "loop": false
}
```

- `t` is milliseconds from start.
- Each point contains a value for every axis reported by `/api/capabilities`.
- Points must be in ascending `t` order.
- The browser pre-computes all interpolation, easing, and per-axis timing before sending — the board does not need to understand any of that.
- `events` (optional) — an array of camera trigger events. Each has a `t` (ms timestamp) and a `type`: `shutter`, `record_start`, or `record_stop`. These trigger DJI camera commands at the specified timestamps during playback.

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

## Settings Endpoints

### `GET /api/settings`

Returns current board settings.

```json
{
  "device_name": "Dolly-A3F2",
  "ap_ssid": "Dolly-A3F2",
  "ap_password": "",
  "telemetry_rate_hz": 50,
  "jog_sensitivity": 1.0,
  "home_on_boot": false
}
```

### `PATCH /api/settings`

Update one or more settings. Body is a partial settings object. Returns the full updated settings.

Settings are persisted to NVS (survive power cycles). The board may need to restart for WiFi changes to take effect — if so, the response includes `"restart_required": true`. To trigger a restart, send `{"restart": true}` in a PATCH request.

**Request:**

```json
{
  "ap_password": "my-secret",
  "telemetry_rate_hz": 25
}
```

**Response:**

```json
{
  "device_name": "Dolly-A3F2",
  "ap_ssid": "Dolly-A3F2",
  "ap_password": "my-secret",
  "telemetry_rate_hz": 25,
  "jog_sensitivity": 1.0,
  "home_on_boot": false,
  "restart_required": true
}
```

---

## WebSocket Messages

Connect to `ws://<board-ip>/ws`. All messages are JSON with a `cmd` (client→board) or `evt` (board→client) field.

### Command Validity by State

| Command | idle | moving | playing | paused | homing | error |
|---------|------|--------|---------|--------|--------|-------|
| `play` | ✓ | | | | | |
| `pause` | | | ✓ | | | |
| `resume` | | | | ✓ | | |
| `stop` | | ✓ | ✓ | ✓ | ✓ | ✓ |
| `scrub` | ✓ | | ✓ | ✓ | | |
| `jog` | ✓ | ✓* | | | | |
| `move_to` | ✓ | ✓* | | | | |
| `home` | ✓ | | | | | |

\* In `moving` state: `jog` cancels the move (→ idle). `move_to` replaces the current move.

Invalid commands receive an error event: `{ "evt": "error", "code": "INVALID_STATE", "detail": "..." }`.

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

- Transitions the board to `moving` state (not `playing`).
- While in `moving`, a new `move_to` replaces the current move (restarts with the new target).
- While in `moving`, a `jog` command cancels the move and returns to `idle`.
- While in `moving`, `stop` cancels the move and returns to `idle`.
- On completion, the board transitions back to `idle`.

#### Homing

```json
{ "cmd": "home", "axes": ["slide"] }
```

Triggers the homing sequence for the specified axes. Only relevant for axes with limit switches or encoders.

#### Heartbeat

```json
{ "cmd": "ping" }
```

Connection health check. Valid from any state — does not affect system state or enter the command queue. The board responds immediately with `{ "evt": "pong" }`. The web UI sends a ping if no message has been received for 5 seconds, and closes the connection (triggering auto-reconnect) if no pong arrives within 3 seconds.

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

Emitted whenever the board transitions between states: `idle`, `moving`, `playing`, `paused`, `homing`, `error`.

#### Playback complete

```json
{ "evt": "complete", "trajectory_id": "abc123" }
```

#### Heartbeat response

```json
{ "evt": "pong" }
```

Immediate response to a `ping` command. See Heartbeat above.

#### Errors

```json
{ "evt": "error", "code": "LIMIT_HIT", "axis": "slide", "detail": "max limit reached" }
```

Error codes: `LIMIT_HIT`, `MOTOR_STALL`, `TRAJECTORY_INVALID`, `OUT_OF_MEMORY`, `COMMUNICATION_LOST`, `INVALID_STATE`.
