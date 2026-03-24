# Camera Control

Programmatic interface to Sony Alpha cameras for the OpenDolly system. Captures live video for AI tracking and sends focus/exposure commands back to the camera.

## What it does

1. **Video capture** — get a live feed from the camera (USB-C or HDMI) for the AI detection pipeline
2. **Focus control** — adjust AF point position, trigger AF, or send manual focus commands to keep the tracked subject sharp
3. **Exposure control** — set aperture, ISO, and shutter speed programmatically (future)
4. **Camera discovery** — detect connected Sony cameras and report their capabilities

## Why it's separate

Camera communication is its own problem domain with its own protocols, SDKs, and quirks. It sits between the camera hardware and the AI tracking system — the tracking system consumes the video feed and produces focus commands, but doesn't need to know whether we're talking PTP, BLE, or Sony's HTTP API.

## Target cameras

- **Sony A7IV** (primary — what we own and test against)
- Sony Alpha series generally (A7III has limited support; A7RV, A9III, etc. should work via Camera Remote SDK)

## Protocols

Based on research in `docs/research/sony-camera-control/`:

| Protocol | Used for | Notes |
|----------|----------|-------|
| **Sony Camera Remote SDK** | Liveview, AF control, exposure | Official C/C++ SDK, supports A7IV+, runs on Linux ARM; [download](https://support.d-imaging.sony.co.jp/app/sdk/licenseagreement_d/en.html) |
| **HDMI capture** | Video feed (fallback) | ~150ms latency, works on all cameras, no protocol needed |
| **USB PTP** | Video feed (UVC mode) | Cannot coexist with PTP control on same USB port |
| **BLE** | Manual focus nudges | freemote project documents GATT protocol for focus in/out |

## Architecture

```
Sony A7IV
  ├── USB-C ──► Video capture (UVC or PTP liveview)
  │                └──► AI tracking pipeline
  ├── WiFi ───► Camera Remote SDK (AF, exposure, shutter)
  └── BLE ────► Focus nudge commands (freemote protocol)
```

## Status

**Research complete, implementation not started.** See `docs/research/sony-camera-control/report.md` for the full protocol survey and repo catalog.
