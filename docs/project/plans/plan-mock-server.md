# Mock Board Server — Implementation Plan

*Status: implemented*
*Last updated: 2026-03-21*

This document specifies a mock implementation of the [Board API](../board-api.md) for developing the web UI without hardware. The mock server implements every REST endpoint and WebSocket message defined in the spec, with simulated motion and configurable hardware profiles.

---

## Tech Choice

### Approach: Standalone Fastify Server

**Recommended: standalone Node.js process** running alongside the SvelteKit dev server.

| Approach | Pros | Cons |
|----------|------|------|
| **Standalone Fastify** (recommended) | Usable with curl/Postman independently; same URL structure as real board; no SvelteKit coupling; WebSocket support via @fastify/websocket is mature | Two processes to manage (solvable with `concurrently`) |
| SvelteKit API routes | Single process; simpler dev start | Couples mock to UI framework; WebSocket support in SvelteKit hooks is awkward; can't test mock independently; mock lifecycle tied to UI dev server restarts; doesn't match real deployment topology |

The standalone approach wins because: (1) the mock server's URL structure mirrors the real board exactly — `http://localhost:3001/api/status` — so switching between mock and real hardware is just changing a base URL, and (2) testing the API with curl or scripts requires no UI at all.

**Stack:**

- **Runtime:** Node.js 20+
- **Language:** TypeScript (strict mode)
- **Framework:** Fastify 5.x
- **WebSocket:** @fastify/websocket (wraps `ws` library)
- **CLI args:** `commander` (lightweight, typed option parsing)
- **No database** — all state in memory

---

## Project Structure

```
mock-server/
├── src/
│   ├── index.ts          # Entry point, CLI args, starts server
│   ├── server.ts         # Fastify instance setup, route registration
│   ├── routes.ts         # REST endpoint handlers
│   ├── websocket.ts      # WebSocket connection handler, command dispatch
│   ├── state.ts          # System state machine (idle/moving/playing/paused/homing/error)
│   ├── motion.ts         # Motion simulation: trajectory playback, jog, move_to, homing
│   ├── trajectory.ts     # Trajectory storage, validation, ID generation
│   ├── profiles.ts       # Capability profiles (slider-only, slider-servo, slider-dji)
│   └── types.ts          # TypeScript types matching board-api.md exactly
├── package.json
└── tsconfig.json
```

---

## Types (`types.ts`)

These types are the source of truth for all mock responses. They must match `board-api.md` exactly.

```typescript
// Axis definition as reported by /api/capabilities
interface AxisCapability {
  name: string;
  min: number;
  max: number;
  unit: "deg" | "mm";
}

// Full capabilities response
interface Capabilities {
  axes: AxisCapability[];
  max_trajectory_points: number;
  max_trajectory_duration_ms: number;
  telemetry_rate_hz: number;
  firmware_version: string;
  board: string;
}

// System state enum
type SystemState = "idle" | "moving" | "playing" | "paused" | "homing" | "error";

// Axis positions — keys are dynamic based on profile
type Position = Record<string, number>;

// GET /api/status response
interface StatusResponse {
  state: SystemState;
  position: Position;
  battery: number;
  trajectory_loaded: string | null;
}

// Trajectory point — t + one value per axis
interface TrajectoryPoint {
  t: number;
  [axis: string]: number;
}

// POST /api/trajectory request
interface TrajectoryUpload {
  points: TrajectoryPoint[];
  loop: boolean;
}

// POST /api/trajectory response
interface TrajectoryUploadResponse {
  trajectory_id: string;
  point_count: number;
  duration_ms: number;
}

// GET /api/trajectory/:id/status response
interface TrajectoryStatus {
  state: "idle" | "playing" | "paused";
  elapsed_ms: number;
  progress: number;
}

// WebSocket client → server commands
type WsCommand =
  | { cmd: "play"; trajectory_id: string }
  | { cmd: "pause" }
  | { cmd: "resume" }
  | { cmd: "stop" }
  | { cmd: "scrub"; t: number }
  | { cmd: "jog"; [axis: string]: number | string }  // cmd + axis deltas
  | { cmd: "move_to"; duration_ms: number; [axis: string]: number | string }
  | { cmd: "home"; axes: string[] }
  | { cmd: "simulate_error"; code: ErrorCode; axis?: string };

// WebSocket server → client events
type WsEvent =
  | { evt: "position"; t: number; [axis: string]: number | string }
  | { evt: "state"; state: SystemState }
  | { evt: "complete"; trajectory_id: string }
  | { evt: "error"; code: ErrorCode; axis?: string; detail: string };

// Device settings
interface Settings {
  device_name: string;
  ap_ssid: string;
  ap_password: string;
  telemetry_rate_hz: number;
  jog_sensitivity: number;
  home_on_boot: boolean;
}

// Error codes from board-api.md
type ErrorCode =
  | "LIMIT_HIT"
  | "MOTOR_STALL"
  | "TRAJECTORY_INVALID"
  | "OUT_OF_MEMORY"
  | "COMMUNICATION_LOST";
```

---

## Capability Profiles (`profiles.ts`)

Selected via `--profile` CLI flag. The profile determines what axes exist and their limits.

### `slider-only`

Minimal build — slider rail only.

```json
{
  "axes": [
    { "name": "slide", "min": 0.0, "max": 1000.0, "unit": "mm" }
  ],
  "max_trajectory_points": 10000,
  "max_trajectory_duration_ms": 300000,
  "telemetry_rate_hz": 50,
  "firmware_version": "1.0.0-mock",
  "board": "esp32-s3-mock"
}
```

### `slider-servo`

Slider with generic servo-based pan/tilt/roll head.

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
  "firmware_version": "1.0.0-mock",
  "board": "esp32-s3-mock"
}
```

### `slider-dji` (default)

Slider with DJI RS-series gimbal. Reflects realistic DJI axis limits.

```json
{
  "axes": [
    { "name": "pan",   "min": -330.0, "max": 330.0, "unit": "deg" },
    { "name": "tilt",  "min": -135.0, "max": 200.0, "unit": "deg" },
    { "name": "roll",  "min": -90.0,  "max": 90.0,  "unit": "deg" },
    { "name": "slide", "min": 0.0,    "max": 1000.0, "unit": "mm" }
  ],
  "max_trajectory_points": 10000,
  "max_trajectory_duration_ms": 300000,
  "telemetry_rate_hz": 50,
  "firmware_version": "1.0.0-mock",
  "board": "esp32-s3-mock"
}
```

The `telemetry_rate_hz` value in the profile is overridden by the `--telemetry-rate` CLI flag if provided.

---

## State Machine (`state.ts`)

The mock maintains a single `SystemState` that governs what commands are valid.

```
         ┌──────────────────────────────────────┐
         │                                      │
         ▼                                      │
    ┌─────────┐   play   ┌─────────┐   stop   │
    │  idle   │────────▶│ playing │──────────┘
    │         │◀────────│         │
    └────┬────┘  stop   └────┬────┘
         │  │                 │
         │  │  home      pause│  ▲ resume
         │  │                 ▼  │
         │  │            ┌─────────┐
         │  │            │ paused  │
         │  │            └────┬────┘
         │  │                 │ stop
         │  │  move_to        │
         │  ▼                 ▼
         │ ┌─────────┐      idle
         │ │ moving  │──────────┐
         │ │         │  stop/   │
         │ │         │  jog     │
         │ └────┬────┘         │
         │      │ done          │
         │      ▼               │
         │     idle ◀───────────┘
         │
         │  home
         ▼
    ┌─────────┐
    │ homing  │
    └────┬────┘
         │ done
         ▼
        idle

    Any state ──error──▶ error ──stop──▶ idle
```

**Transition rules:**

| Current State | Valid Commands | Invalid Commands (return error event) |
|---------------|---------------|---------------------------------------|
| `idle` | `play`, `home`, `jog`, `move_to`, `scrub` | `pause`, `resume` |
| `moving` | `move_to` (restart), `jog` (cancel → idle), `stop` (→ idle) | `play`, `pause`, `resume`, `home` |
| `playing` | `pause`, `stop`, `scrub` | `play`, `resume`, `jog`, `move_to`, `home` |
| `paused` | `resume`, `stop`, `scrub` | `play`, `pause`, `jog`, `move_to`, `home` |
| `homing` | `stop` | everything else |
| `error` | `stop` | everything else |

The state machine:
- Stores current `SystemState`.
- Stores current `Position` (all axis values).
- Stores currently loaded `trajectory_id` (or null).
- Broadcasts `{ evt: "state", state }` to all connected WebSocket clients on every transition.

---

## REST Endpoints (`routes.ts`)

### `GET /api/status`

Returns a snapshot of the state machine.

**Mock behavior:** Read current state and position from the state machine. Battery is a constant mock value (82) or optionally decremented slowly for realism.

**Response (200):**

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

The `position` keys match the active profile's axes. For `slider-only`, only `slide` appears.

---

### `GET /api/capabilities`

Returns the active profile.

**Mock behavior:** Return the profile object selected by `--profile`. No computation needed.

**Response (200):**

```json
{
  "axes": [
    { "name": "pan",   "min": -330.0, "max": 330.0, "unit": "deg" },
    { "name": "tilt",  "min": -135.0, "max": 200.0, "unit": "deg" },
    { "name": "roll",  "min": -90.0,  "max": 90.0,  "unit": "deg" },
    { "name": "slide", "min": 0.0,    "max": 1000.0, "unit": "mm" }
  ],
  "max_trajectory_points": 10000,
  "max_trajectory_duration_ms": 300000,
  "telemetry_rate_hz": 50,
  "firmware_version": "1.0.0-mock",
  "board": "esp32-s3-mock"
}
```

---

### `POST /api/trajectory`

Accepts and validates a trajectory upload.

**Mock behavior:**

1. Parse the JSON body as `TrajectoryUpload`.
2. Validate:
   - `points` is a non-empty array.
   - `t` values are in strictly ascending order.
   - Every point contains a value for every axis in the active profile (no extra, no missing).
   - All axis values are within the profile's `[min, max]` range.
   - `points.length <= max_trajectory_points`.
   - Final `t` value `<= max_trajectory_duration_ms`.
3. If validation fails, return 400 with error detail.
4. Generate a trajectory ID (nanoid or `crypto.randomUUID()`).
5. Store the trajectory in an in-memory `Map<string, StoredTrajectory>`.
6. Update state machine's `trajectory_loaded` to the new ID.
7. Return the upload response.

**Response (200):**

```json
{
  "trajectory_id": "traj_a1b2c3d4",
  "point_count": 3000,
  "duration_ms": 30000
}
```

**Response (400) — validation failure:**

```json
{
  "error": "TRAJECTORY_INVALID",
  "detail": "Point 42: t values not in ascending order (t=500 after t=600)"
}
```

**Response (400) — missing axis:**

```json
{
  "error": "TRAJECTORY_INVALID",
  "detail": "Point 0: missing axis 'roll'"
}
```

---

### `DELETE /api/trajectory/:id`

**Mock behavior:**

1. Look up the trajectory by ID in the in-memory map.
2. If not found, return 404.
3. If the trajectory is currently being played (state is `playing` or `paused`), return 409 (conflict).
4. Remove from the map. If `trajectory_loaded` matches this ID, set it to null.
5. Return 204 (no content).

**Response (204):** empty body.

**Response (404):**

```json
{
  "error": "NOT_FOUND",
  "detail": "No trajectory with id 'traj_xyz'"
}
```

**Response (409):**

```json
{
  "error": "CONFLICT",
  "detail": "Cannot delete trajectory while playback is active"
}
```

---

### `GET /api/trajectory/:id/status`

**Mock behavior:**

1. Look up the trajectory by ID.
2. If not found, return 404.
3. If the trajectory is currently playing or paused, return elapsed time and progress from the motion simulator.
4. If idle (trajectory exists but not playing), return `elapsed_ms: 0, progress: 0`.

**Response (200):**

```json
{
  "state": "playing",
  "elapsed_ms": 12400,
  "progress": 0.41
}
```

---

### Artificial Latency

If `--latency MS` is set, all REST responses are delayed by that amount using a simple `await sleep(latencyMs)` before sending. This simulates Wi-Fi round-trip time.

---

### `GET /api/settings`

Returns the current device settings.

**Mock behavior:** Return the in-memory settings object.

**Response (200):**

```json
{
  "device_name": "slider-01",
  "ap_ssid": "Slider-AP",
  "ap_password": "slider1234",
  "telemetry_rate_hz": 50,
  "jog_sensitivity": 1.0,
  "home_on_boot": false
}
```

---

### `PATCH /api/settings`

Accepts partial updates to device settings, returns the full settings object.

**Mock behavior:**

1. Parse the JSON body as a partial settings object.
2. Merge with the current in-memory settings (only provided keys are updated).
3. Return the full updated settings object.

**Request:**

```json
{
  "device_name": "my-slider",
  "telemetry_rate_hz": 30
}
```

**Response (200):**

```json
{
  "device_name": "my-slider",
  "ap_ssid": "Slider-AP",
  "ap_password": "slider1234",
  "telemetry_rate_hz": 30,
  "jog_sensitivity": 1.0,
  "home_on_boot": false
}
```

---

## WebSocket Handler (`websocket.ts`)

### Connection Management

- Accept connections at `ws://localhost:PORT/ws`.
- Maintain a `Set<WebSocket>` of all connected clients.
- All server events are broadcast to every connected client (mimics the real board which has one WebSocket and one client).
- On disconnect, remove from set and clean up.

### Command Dispatch

On receiving a message:

1. Parse JSON. If parse fails, send `{ evt: "error", code: "TRAJECTORY_INVALID", detail: "Malformed JSON" }`.
2. Extract `cmd` field.
3. If `--latency` is set, delay processing by that amount.
4. Dispatch to handler based on `cmd`:

#### `play`

```json
{ "cmd": "play", "trajectory_id": "traj_a1b2c3d4" }
```

1. Validate trajectory exists in storage.
2. Validate state machine allows `play` (must be `idle`).
3. Transition state to `playing`. Broadcast `{ evt: "state", state: "playing" }`.
4. Start the trajectory playback timer in the motion simulator.

#### `pause`

```json
{ "cmd": "pause" }
```

1. Validate state is `playing`.
2. Transition to `paused`. Broadcast state event.
3. Pause the motion timer (record elapsed time).

#### `resume`

```json
{ "cmd": "resume" }
```

1. Validate state is `paused`.
2. Transition to `playing`. Broadcast state event.
3. Resume the motion timer from where it left off.

#### `stop`

```json
{ "cmd": "stop" }
```

1. Valid from any state except `idle`.
2. Stop the motion timer. Transition to `idle`. Broadcast state event.
3. Position remains wherever it was when stopped (does NOT reset to start).

#### `scrub`

```json
{ "cmd": "scrub", "t": 15000 }
```

1. Validate a trajectory is loaded.
2. Valid from `idle`, `playing`, or `paused`.
3. Jump the motion simulator to timestamp `t`. Interpolate the position at that `t` from the trajectory points.
4. Emit a single `position` event with the interpolated position.
5. If state was `playing`, continue playback from the new `t`.
6. If state was `idle` or `paused`, remain in that state.

#### `jog`

```json
{ "cmd": "jog", "pan": 2.5, "tilt": -1.0 }
```

1. Validate state is `idle` or `moving` (jog is manual control, not available during playback). If `moving`, cancel the active move first and transition to `idle`.
2. Only axis keys present in the message are affected (partial update).
3. Add delta values to current position. Clamp to axis limits.
4. Start the jog simulation in the motion simulator (velocity model, see below).
5. Position telemetry begins emitting at the configured rate.

#### `move_to`

```json
{ "cmd": "move_to", "pan": 45.0, "tilt": -10.0, "roll": 0.0, "slide": 500.0, "duration_ms": 1000 }
```

1. Validate state is `idle` or `moving`.
2. Validate all target values are within axis limits.
3. Start linear interpolation from current position to target over `duration_ms`.
4. Transition state to `moving`. Broadcast state event.
5. Position telemetry emits during the move.
6. On completion, transition to `idle`. Broadcast state event.

While in `moving`, a new `move_to` restarts the animation from the current position to the new target. While in `moving`, a `jog` command cancels the move and returns to `idle`.

#### `home`

```json
{ "cmd": "home", "axes": ["slide"] }
```

1. Validate state is `idle`.
2. Transition to `homing`. Broadcast state event.
3. Animate specified axes to 0 over 2000ms. Unspecified axes hold position.
4. Position telemetry emits during homing.
5. On completion, transition to `idle`. Broadcast state event.

#### `simulate_error` (debug-only)

```json
{ "cmd": "simulate_error", "code": "LIMIT_HIT", "axis": "slide" }
```

1. Valid from any state.
2. If currently in motion, stop the motion timer.
3. Transition to `error`. Broadcast state event.
4. Broadcast error event:

```json
{ "evt": "error", "code": "LIMIT_HIT", "axis": "slide", "detail": "Simulated: max limit reached" }
```

Error detail messages by code:

| Code | Detail |
|------|--------|
| `LIMIT_HIT` | `"Simulated: {axis} max limit reached"` |
| `MOTOR_STALL` | `"Simulated: {axis} motor stall detected"` |
| `TRAJECTORY_INVALID` | `"Simulated: trajectory data corrupted"` |
| `OUT_OF_MEMORY` | `"Simulated: insufficient memory for trajectory"` |
| `COMMUNICATION_LOST` | `"Simulated: CAN bus communication timeout"` |

---

## Simulated Motion Model (`motion.ts`)

All motion types share a single tick loop running at `setInterval(tick, 1000 / telemetryRateHz)`. Each tick:

1. Compute new position based on active motion type.
2. Clamp all axis values to profile limits.
3. Update the state machine's current position.
4. Broadcast a `position` event to all WebSocket clients.

When no motion is active, the timer does not run (no telemetry emitted while idle).

### Trajectory Playback

When `play` is received:

1. Record `startTime = Date.now()`.
2. On each tick, compute `elapsed = Date.now() - startTime`.
3. Find the two trajectory points that bracket `elapsed`: the last point where `point.t <= elapsed` and the first point where `point.t > elapsed`.
4. Linear-interpolate between them: `ratio = (elapsed - p1.t) / (p2.t - p1.t)`, then for each axis: `value = p1[axis] + ratio * (p2[axis] - p1[axis])`.
5. When `elapsed >= lastPoint.t`:
   - If `loop` is true, reset `startTime = Date.now()` and continue.
   - Otherwise, emit final position at the last point, broadcast `{ evt: "complete", trajectory_id }`, transition to `idle`.

When `pause` is received, record `pausedAt = Date.now() - startTime` (elapsed time so far). When `resume` is received, set `startTime = Date.now() - pausedAt` so elapsed continues from where it left off.

### Jog Simulation

Jog uses a simple velocity model for realistic feel:

```
State per axis:
  velocity: number = 0      // current velocity (units/sec)
  targetVelocity: number = 0

Constants:
  MAX_VELOCITY = 100         // units/sec (scaled per axis range)
  ACCELERATION = 500         // units/sec^2
  DECELERATION = 800         // units/sec^2
```

When a `jog` command arrives:
1. Set `targetVelocity` for each specified axis based on the delta sign and magnitude. The delta value maps proportionally to a fraction of `MAX_VELOCITY` (e.g., `jog.pan = 2.5` with axis range 660 -> `targetVelocity = (2.5 / 660) * MAX_VELOCITY`).

On each tick (`dt = 1 / telemetryRateHz`):
1. For each axis, accelerate or decelerate velocity toward `targetVelocity`.
2. Update position: `position += velocity * dt`.
3. Clamp position to axis limits. If clamped, set velocity to 0.

When no `jog` commands are received for 100ms, set all `targetVelocity` to 0 — the axes decelerate to a stop. Once all velocities reach 0, stop the tick timer.

### `move_to` Simulation

Linear interpolation from current position to target:

```
startPosition = snapshot of current position
startTime = Date.now()

On each tick:
  elapsed = Date.now() - startTime
  ratio = clamp(elapsed / duration_ms, 0, 1)
  for each axis:
    position[axis] = startPosition[axis] + ratio * (target[axis] - startPosition[axis])
  if ratio >= 1:
    position = target (exact)
    transition to idle
    broadcast state event
```

State is `moving` during the interpolation (not `playing`). If a new `move_to` arrives while in `moving`, snapshot the current position as the new start and restart the timer. If `jog` arrives while in `moving`, cancel the interpolation and transition to `idle`.

### Homing Simulation

Same as `move_to` but:
- Target is 0 for all specified axes, current position for unspecified axes.
- Fixed duration of 2000ms.
- State is `homing` instead of `playing`.
- On completion, transition to `idle` (no `complete` event, just state change).

---

## Configuration (`index.ts`)

CLI interface using `commander`:

```
Usage: mock-server [options]

Options:
  --port <number>       Server port (default: 3001)
  --profile <name>      Capability profile: slider-only, slider-servo, slider-dji (default: slider-dji)
  --telemetry-rate <hz> Telemetry broadcast rate in Hz (default: 50)
  --latency <ms>        Artificial latency on all responses in ms (default: 0)
  -h, --help            Display help
```

Example:

```bash
# Default: full DJI profile on port 3001
npx tsx src/index.ts

# Slider-only profile with 100ms simulated Wi-Fi latency
npx tsx src/index.ts --profile slider-only --latency 100

# Custom port and telemetry rate
npx tsx src/index.ts --port 8080 --telemetry-rate 30
```

On startup, log:

```
Mock board server started
  Port:           3001
  Profile:        slider-dji (4 axes)
  Telemetry rate: 50 Hz
  Latency:        0 ms
  WebSocket:      ws://localhost:3001/ws
```

---

## Integration with SvelteKit Dev Workflow

### Connection Configuration

The SvelteKit web UI needs a `BOARD_URL` configuration. During development this points at the mock server; in production it points at the real ESP32's IP.

In the SvelteKit app, use an environment variable:

```
# .env.development
PUBLIC_BOARD_URL=http://localhost:3001
PUBLIC_BOARD_WS_URL=ws://localhost:3001/ws
```

The UI code reads `PUBLIC_BOARD_URL` for REST calls and `PUBLIC_BOARD_WS_URL` for WebSocket connections. No proxy needed — the mock server sets CORS headers to allow requests from the SvelteKit dev server origin (`http://localhost:5173`).

### CORS Configuration

The Fastify server registers `@fastify/cors`:

```typescript
server.register(cors, {
  origin: true,  // reflect request origin (permissive for dev)
});
```

### Starting Both Servers

Add scripts to the root `package.json`:

```json
{
  "scripts": {
    "dev": "concurrently --names mock,ui --prefix-colors cyan,magenta \"npm run mock\" \"npm run ui\"",
    "mock": "npx tsx mock-server/src/index.ts",
    "ui": "cd web && npm run dev"
  }
}
```

Running `npm run dev` from the project root starts both the mock server and the SvelteKit dev server. The `concurrently` package prefixes log lines with `[mock]` and `[ui]` for clarity.

For mock-only development (API testing without the UI):

```bash
npm run mock
# or with options:
npm run mock -- --profile slider-only --latency 50
```

---

## Test Strategy

The mock server is itself a test tool, so it does not need extensive unit tests. However, two categories of validation are important:

### 1. Schema Conformance Tests

A small test suite (Vitest) that hits every endpoint and validates the response shape matches `board-api.md`. This catches drift between the mock and the spec.

```typescript
// Example test
test("GET /api/status matches spec shape", async () => {
  const res = await fetch("http://localhost:3001/api/status");
  const body = await res.json();

  expect(body).toHaveProperty("state");
  expect(["idle", "moving", "playing", "paused", "homing", "error"]).toContain(body.state);
  expect(body).toHaveProperty("position");
  expect(body).toHaveProperty("battery");
  expect(body).toHaveProperty("trajectory_loaded");
});
```

### 2. Curl Command Reference

Quick manual validation for each endpoint:

```bash
# Status
curl http://localhost:3001/api/status

# Capabilities
curl http://localhost:3001/api/capabilities

# Upload trajectory
curl -X POST http://localhost:3001/api/trajectory \
  -H "Content-Type: application/json" \
  -d '{
    "points": [
      { "t": 0,     "pan": 0.0, "tilt": 0.0, "roll": 0.0, "slide": 0.0 },
      { "t": 5000,  "pan": 45.0, "tilt": -10.0, "roll": 0.0, "slide": 250.0 },
      { "t": 10000, "pan": 90.0, "tilt": 0.0, "roll": 5.0, "slide": 500.0 }
    ],
    "loop": false
  }'

# Check trajectory status (replace ID with actual)
curl http://localhost:3001/api/trajectory/traj_a1b2c3d4/status

# Delete trajectory
curl -X DELETE http://localhost:3001/api/trajectory/traj_a1b2c3d4

# WebSocket (using websocat)
websocat ws://localhost:3001/ws

# Then type commands:
{"cmd": "play", "trajectory_id": "traj_a1b2c3d4"}
{"cmd": "pause"}
{"cmd": "resume"}
{"cmd": "stop"}
{"cmd": "jog", "pan": 5.0}
{"cmd": "move_to", "pan": 45.0, "tilt": -10.0, "roll": 0.0, "slide": 500.0, "duration_ms": 2000}
{"cmd": "home", "axes": ["slide", "pan", "tilt", "roll"]}
{"cmd": "simulate_error", "code": "LIMIT_HIT", "axis": "slide"}
{"cmd": "scrub", "t": 5000}
```

---

## Shared Types

The `types.ts` file in this plan is for illustration. In implementation, the mock server should import types from the shared types package at the project root (`shared/`) instead of defining types locally. See [`shared-types.md`](shared-types.md) for the shared types strategy.

---

## Trajectory Events (Camera Trigger)

The `TrajectoryUpload` type supports an optional `events` array for camera trigger and other timed actions:

```typescript
interface TrajectoryEvent {
  t: number;          // milliseconds from trajectory start
  type: string;       // "shutter", "record_start", "record_stop"
}

interface TrajectoryUpload {
  points: TrajectoryPoint[];
  loop: boolean;
  events?: TrajectoryEvent[];
}
```

During simulated playback, the motion simulator checks the `events` array on each tick. When `elapsed >= event.t` for an event that hasn't fired yet, log the event to the console:

```
[mock] Event fired at t=5000ms: shutter
[mock] Event fired at t=15000ms: record_start
```

Events are not re-fired on loop iterations unless the trajectory restarts from the beginning.

---

## Dependencies

```json
{
  "dependencies": {
    "fastify": "^5.0.0",
    "@fastify/cors": "^10.0.0",
    "@fastify/websocket": "^11.0.0",
    "commander": "^12.0.0"
  },
  "devDependencies": {
    "typescript": "^5.5.0",
    "tsx": "^4.0.0",
    "vitest": "^2.0.0",
    "@types/node": "^20.0.0",
    "@types/ws": "^8.0.0"
  }
}
```

No other runtime dependencies. The mock server has zero dependency on the web UI or any hardware libraries.
