# Mock Board Server

A development mock of the [Board API](../docs/project/board-api.md). Implements every REST endpoint and WebSocket command with simulated motion, so the web UI can be developed and tested without hardware.

## Quick Start

From the project root:

```bash
npm install
npm run mock
```

The server starts on port 3001 with the `slider-dji` profile (4 axes).

## CLI Options

```
Usage: mock-server [options]

Options:
  --port <number>          Server port (default: 3001)
  --profile <name>         Capability profile (default: slider-dji)
  --telemetry-rate <hz>    Position telemetry rate in Hz (default: 50)
  --latency <ms>           Artificial delay on all responses (default: 0)
```

### Profiles

| Profile | Axes | Use Case |
|---------|------|----------|
| `slider-only` | slide | Minimal build — slider rail only |
| `slider-servo` | pan, tilt, roll, slide | Generic servo-based pan/tilt/roll head |
| `slider-dji` | pan, tilt, roll, slide | DJI RS-series gimbal (realistic axis limits) |

### Examples

```bash
# Default: full DJI profile on port 3001
npm run mock

# Slider-only with 100ms simulated Wi-Fi latency
npm run mock -- --profile slider-only --latency 100

# Custom port and telemetry rate
npm run mock -- --port 8080 --telemetry-rate 30
```

## REST Endpoints

| Method | Path | Description |
|--------|------|-------------|
| GET | `/api/status` | System state, axis positions, battery, loaded trajectory |
| GET | `/api/capabilities` | Axes, limits, firmware version (from active profile) |
| POST | `/api/trajectory` | Upload a trajectory (validated against profile) |
| DELETE | `/api/trajectory/:id` | Remove a stored trajectory |
| GET | `/api/trajectory/:id/status` | Playback progress for a trajectory |
| GET | `/api/settings` | Device settings |
| PATCH | `/api/settings` | Update settings (partial) |

## WebSocket

Connect to `ws://localhost:3001/ws`. All messages are JSON.

### Commands (client → server)

| Command | Valid States | Description |
|---------|-------------|-------------|
| `play` | idle | Start trajectory playback |
| `pause` | playing | Pause playback |
| `resume` | paused | Resume playback |
| `stop` | moving, playing, paused, homing, error | Stop and return to idle |
| `scrub` | idle, playing, paused | Jump to timestamp in loaded trajectory |
| `jog` | idle, moving | Relative axis movement (velocity model) |
| `move_to` | idle, moving | Absolute move over duration |
| `home` | idle | Home specified axes to zero |
| `simulate_error` | any | Trigger a simulated error state |

### Events (server → client)

| Event | Description |
|-------|-------------|
| `position` | Axis positions + elapsed time (emitted at telemetry rate during motion) |
| `state` | State transition notification |
| `complete` | Trajectory playback finished |
| `error` | Error with code and detail |

## Quick Test with curl

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
      { "t": 0,     "pan": 0, "tilt": 0, "roll": 0, "slide": 0 },
      { "t": 5000,  "pan": 45, "tilt": -10, "roll": 0, "slide": 250 },
      { "t": 10000, "pan": 90, "tilt": 0, "roll": 5, "slide": 500 }
    ],
    "loop": false
  }'
```

## Simulated Motion

The mock server simulates realistic motion for all command types:

- **Trajectory playback**: Linear interpolation between trajectory points at the telemetry rate. Supports pause/resume, scrub, and loop.
- **Jog**: Velocity model with acceleration (500 units/s²) and deceleration (800 units/s²). Axes decelerate to a stop when no jog commands are received for 100ms.
- **Move to**: Linear interpolation from current position to target over the specified duration.
- **Homing**: Same as move_to but targets zero for specified axes over a fixed 2-second duration.

All motion clamps axis values to profile limits.

## Architecture

```
src/
  index.ts        CLI entry point (commander)
  server.ts       Fastify setup, CORS, WebSocket, dependency wiring
  profiles.ts     Capability profiles (slider-only, slider-servo, slider-dji)
  state.ts        BoardState: position, system state, axis validation
  trajectory.ts   TrajectoryStore: upload validation, in-memory storage
  motion.ts       MotionSimulator: playback, jog, move_to, homing
  routes.ts       REST endpoint handlers
  websocket.ts    WebSocket command dispatch and state machine enforcement
```

Types are imported from `@opendolly/shared` — the same types the web UI uses.
