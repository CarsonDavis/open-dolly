# OpenDolly

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

## Requirements

| Requirement | Spec | Notes |
|-------------|------|-------|
| **Slide travel** | 3 ft (900mm) minimum | Rail is modular — longer builds (4-5 ft) use the same design |
| **Payload capacity** | 15 lb (7 kg) in any orientation | Sized for mirrorless + gimbal + heavy glass with 2x safety margin |
| **Orientation** | Horizontal, vertical, inverted, any angle | Electromagnetic brake holds payload when stopped or on power loss |
| **Speed** | 500+ mm/s horizontal, 100+ mm/s vertical | Belt drive, no length-dependent speed limit |
| **Precision** | Sub-0.1mm repeatability (with optional linear encoder) | For focus stacking, macro rails, repeatable moves |
| **Control** | Browser-based, no app install | Wi-Fi AP mode — works anywhere with no internet |
| **Playback** | Autonomous, no network needed | Full trajectory uploaded to board before move starts |

**Reference payload** (what we're designing around):

| Component | Weight |
|-----------|--------|
| DJI RS 5 gimbal (with battery grip & QR plates) | 1,460g (3.2 lb) |
| Sony A7IV (with battery & card) | 658g (1.5 lb) |
| Sigma 28-105mm f/2.8 DG DN Art | 990g (2.2 lb) |
| Carriage hardware | ~400g (0.9 lb) |
| **Total** | **~3,500g (7.7 lb)** |

The 15 lb design load is 2x this reference payload — enough headroom for heavier camera/lens combinations or a cine setup.

## Design principles

**Bring your own hardware.** The board reports what axes it has and their limits. The UI adapts. Use a DJI RS gimbal, a servo gimbal, or no gimbal at all — the software doesn't care.

**Modular at every layer.** The board API (REST + WebSocket, JSON) is a documented contract. Swap the web UI for a Python script. Swap the ESP32 for a Raspberry Pi. Swap the DJI gimbal for hobby servos. Each layer is independent.

**Smart browser, simple board.** Complex motion planning runs in JavaScript where it's easy to iterate and test. The board firmware stays minimal — receive a trajectory, execute it, report position.

**No network needed during playback.** Trajectories are uploaded in full before the move starts. The board executes autonomously. Your phone can disconnect and the move still completes.

**Open and scriptable.** Everything the web UI can do, a curl command can do. The API is the product as much as the UI is.

**Works in any orientation.** Horizontal, vertical, inverted, or any angle in between. A fail-safe electromagnetic brake holds the payload when stopped and engages automatically on power loss — the camera never falls.

## What you can build

| Config | Axes | Est. Cost |
|--------|------|-----------|
| Slider only | 1 (slide) | ~$100-175 |
| Slider + servo gimbal | 4 (slide, pan, tilt, roll) | ~$130-225 |
| Slider + DJI RS 2 (used) | 4 | ~$350-525 |
| Slider + DJI RS 4 | 4 | ~$525-675 |

See [`docs/project/bom.md`](docs/project/bom.md) for the full parts list.

## Project status

**Implementation in progress.** Core software components are built and working. Hardware build not yet started.

| Component | Status | Location |
|-----------|--------|----------|
| Shared types | Done | [`shared/`](shared/) |
| Motion math library | Done | [`motion-math/`](motion-math/) |
| Mock board server | Done | [`mock-server/`](mock-server/) |
| Web UI (V2) | Implemented (untested in browser) | [`web/`](web/) |
| DJI CAN protocol library | Done | [`firmware/lib/dji_can/`](firmware/lib/dji_can/) |
| ESP32 firmware | Done (tested on ESP32-S3-DevKitC-1 v1.1) | [`firmware/`](firmware/) |

- [`docs/project/architecture.md`](docs/project/architecture.md) — system layers, trajectory model, design decisions
- [`docs/project/board-api.md`](docs/project/board-api.md) — REST + WebSocket protocol spec
- [`docs/project/bom.md`](docs/project/bom.md) — bill of materials and cost estimates
- [`docs/project/plans/`](docs/project/plans/) — implementation plans (motion math, firmware, web UI, mock server, DJI CAN)
- [`docs/research/`](docs/research/) — deep dives on DJI SDK protocols and keyframe interpolation math
