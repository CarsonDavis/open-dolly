# Shared Types Strategy

*Status: draft*
*Last updated: 2026-03-20*

## Overview

A `shared/` directory at the project root contains TypeScript type definitions that match the [Board API spec](../board-api.md). These types are the single source of truth for all TypeScript code in the project.

## Structure

```
shared/
  types.ts    — all shared type definitions
```

Both `web/` and `mock-server/` import from `shared/` rather than defining types locally. This ensures the web UI and mock server always agree on message shapes, and any drift from the API spec is caught in one place.

## Firmware (C++)

The firmware defines equivalent types in `commands.h` — maintained separately in C++ but documented to match the shared TypeScript types. When the API spec changes, update `shared/types.ts` first, then update `commands.h` to match.

## Update workflow

1. Update `board-api.md` with the spec change.
2. Update `shared/types.ts` to match.
3. Update `commands.h` (firmware) to match.
4. Fix any type errors in `web/` and `mock-server/`.

## Key Types

| Type | Purpose |
|------|---------|
| `SystemState` | Union of valid system states: `"idle" \| "moving" \| "playing" \| "paused" \| "homing" \| "error"` |
| `Capabilities` | Response shape for `GET /api/capabilities` (axes, limits, firmware version) |
| `Axis` | Single axis definition (name, min, max, unit) |
| `StatusResponse` | Response shape for `GET /api/status` (state, position, battery, trajectory) |
| `TrajectoryUpload` | Request body for `POST /api/trajectory` (points array, loop flag, optional events) |
| `TrajectoryPoint` | Single row in the trajectory table (`t` + dynamic axis values) |
| `TrajectoryEvent` | Timed event within a trajectory (timestamp + action string) |
| `WsCommand` | Union of all WebSocket client-to-server command shapes |
| `WsEvent` | Union of all WebSocket server-to-client event shapes |
| `ErrorCode` | Union of error code strings |
| `Settings` | Device settings object (device name, AP config, telemetry rate, jog sensitivity, home-on-boot) |
