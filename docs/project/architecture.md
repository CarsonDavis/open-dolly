# Architecture

*Status: draft*
*Last updated: 2026-03-20*

## Design Principles

1. **Modular and swappable.** Every layer can be replaced independently. Someone can use our web UI with their own board, or skip the UI entirely and script against the API.
2. **Bring your own hardware.** The board API is hardware-agnostic. Axis names, limits, and units are reported by the board at runtime via a capabilities endpoint — the UI adapts.
3. **Smart browser, simple board.** All complex math (spline interpolation, easing, per-axis timing) runs in the browser. The board's job is to walk a pre-computed trajectory table and drive motors.
4. **Wi-Fi-independent playback.** Pre-programmed moves are uploaded to the board as complete trajectories before playback starts. The board executes autonomously — no network dependency during motion.
5. **Open, scriptable interface.** The board API uses REST + WebSocket with JSON. Any language or tool that can make HTTP requests can control the system.

## System Layers

```
┌─────────────────────────────────────────────┐
│  Web UI (browser)                           │
│  - Manual control (joystick/drag)           │
│  - Keyframe editor + timeline               │
│  - Path computation (SQUAD, Catmull-Rom)    │
│  - Easing & per-axis timing                 │
│  - Trajectory preview                       │
│  - Generates dense trajectory point tables  │
└──────────────┬──────────────────────────────┘
               │  REST + WebSocket (Wi-Fi)
               │  JSON messages
┌──────────────▼──────────────────────────────┐
│  Board API (contract boundary)              │
│  - REST: config, capabilities, trajectory   │
│  - WS: playback control, jog, telemetry     │
│  - Hardware-agnostic protocol spec          │
└──────────────┬──────────────────────────────┘
               │
┌──────────────▼──────────────────────────────┐
│  Board Firmware (ESP32, Pi, etc.)            │
│  - Stores trajectory point table in memory  │
│  - Walks table with linear interpolation    │
│  - Drives motors via step/dir, CAN, PWM     │
│  - Reports position telemetry               │
└──────────────┬──────────────────────────────┘
               │
┌──────────────▼──────────────────────────────┐
│  Hardware                                   │
│  - Motor drivers (stepper, servo, CAN)      │
│  - Gimbal (DJI RS via CAN, custom, etc.)    │
│  - Slider rail + motor                      │
│  - Limit switches, encoders                 │
└─────────────────────────────────────────────┘
```

## Hybrid Trajectory Model

The system uses a hybrid approach to motion execution:

### Pre-programmed moves (keyframe playback)

1. User defines keyframes in the web UI — each keyframe is a full system state (position of all axes) at a point in time.
2. User configures easing curves and per-axis timing offsets in the timeline editor.
3. The browser computes the full interpolated path:
   - Rotational axes (pan/tilt/roll): SQUAD interpolation (quaternion-based, avoids gimbal lock)
   - Linear axes (slide, focus): Centripetal Catmull-Rom splines
   - Timing: easing functions (cubic bezier, polynomial, etc.) applied per-axis
   - Per-axis timing offsets allow axes to start/finish at different times
4. The result is a dense point table — one row per ~10-20ms, each row containing all axis positions at that instant.
5. The table is uploaded to the board via `POST /api/trajectory`.
6. Playback is triggered via WebSocket. The board walks the table autonomously.

A 30-second move at 100Hz = 3,000 points x ~20 bytes = ~60KB. Fits easily on an ESP32.

### Manual control (real-time jogging)

Direct joystick/drag input is sent as real-time WebSocket messages. The board executes these immediately. Small Wi-Fi latency hiccups are acceptable here since manual control is inherently imprecise.

### Non-linear motion

All non-linearity is computed browser-side before the trajectory is sent:

- **Easing curves** (ease-in, ease-out, cubic bezier, custom) control acceleration/deceleration profiles
- **Per-axis timing** allows axes to move independently — e.g., pan completes in the first 8 seconds while slide doesn't start until second 5
- **Curve shape** comes from the spline interpolation (Catmull-Rom for position, SQUAD for rotation)

The board never needs to understand easing or splines. By the time the trajectory reaches the board, non-linear motion is just a sequence of positions that happen to be non-uniformly spaced in value.
