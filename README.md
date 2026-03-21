# Slider

Open-source motorized camera slider with gimbal control. One web interface for all axes of motion — slide, pan, tilt, roll. Define keyframes, shape the motion between them, hit play.

## What it is

A motorized rail with a camera gimbal mounted on it, controlled entirely from a browser. No app to install, no cloud account, no proprietary software. Connect to the board's Wi-Fi, open the page, and you have full motion control from your phone or laptop.

You move the camera to a position you like and save it as a keyframe. Move to another position, save another keyframe. The software interpolates smooth motion between them — with full control over easing, timing, and per-axis choreography. Then the board executes the move autonomously, no network needed during playback.

## How it works

```
  Your phone/laptop                     On the slider
┌──────────────────┐    Wi-Fi    ┌──────────────────────────┐
│                  │◄──────────-►│                          │
│  Web UI          │  REST +     │  ESP32 control board     │
│  - Keyframes     │  WebSocket  │  - Walks trajectory      │
│  - Timeline      │             │  - Drives motors         │
│  - Path math     │             │  - Reports position      │
│  - Easing curves │             │                          │
│                  │             │         │                │
└──────────────────┘             └─────────┼────────────────┘
                                           │
                                   ┌───────▼───────┐
                                   │ Slider motor  │
                                   │ Gimbal (DJI   │
                                   │  or custom)   │
                                   └───────────────┘
```

The browser does the hard math — spline interpolation, easing curves, quaternion rotation — and computes a dense table of positions over time. That table gets uploaded to the board before playback. The board's job is simple: step through the table and drive motors. A 30-second cinematic move is ~60KB of data.

## Design principles

**Bring your own hardware.** The board reports what axes it has and their limits. The UI adapts. Use a DJI RS gimbal, a servo gimbal, or no gimbal at all — the software doesn't care.

**Modular at every layer.** The board API (REST + WebSocket, JSON) is a documented contract. Swap the web UI for a Python script. Swap the ESP32 for a Raspberry Pi. Swap the DJI gimbal for hobby servos. Each layer is independent.

**Smart browser, simple board.** Complex motion planning runs in JavaScript where it's easy to iterate and test. The board firmware stays minimal — receive a trajectory, execute it, report position.

**No network needed during playback.** Trajectories are uploaded in full before the move starts. The board executes autonomously. Your phone can disconnect and the move still completes.

**Open and scriptable.** Everything the web UI can do, a curl command can do. The API is the product as much as the UI is.

## What you can build

| Config | Axes | Est. Cost |
|--------|------|-----------|
| Slider only | 1 (slide) | ~$100-175 |
| Slider + servo gimbal | 4 (slide, pan, tilt, roll) | ~$130-225 |
| Slider + DJI RS 2 (used) | 4 | ~$350-525 |
| Slider + DJI RS 4 | 4 | ~$525-675 |

See [`docs/project/bom.md`](docs/project/bom.md) for the full parts list.

## Project status

Early design phase. We're defining the architecture and API before writing code.

- [`docs/project/architecture.md`](docs/project/architecture.md) — system layers, trajectory model, design decisions
- [`docs/project/board-api.md`](docs/project/board-api.md) — REST + WebSocket protocol spec
- [`docs/project/bom.md`](docs/project/bom.md) — bill of materials and cost estimates
- [`docs/research/`](docs/research/) — deep dives on DJI SDK protocols and keyframe interpolation math
