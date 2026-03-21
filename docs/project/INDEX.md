# Project Docs Index

## Specs & Design

| Path | Purpose | Key Details |
|------|---------|-------------|
| `initial-vision.md` | Original product concept and UX goals | Slider + gimbal, keyframe system, web app control; status: active |
| `architecture.md` | System architecture and design principles | Hybrid trajectory model, layer diagram, modularity boundaries; status: draft |
| `board-api.md` | Board API specification (REST + WebSocket) | Hardware-agnostic JSON protocol; capabilities discovery, trajectory upload, real-time jog/telemetry; 6 states (idle/moving/playing/paused/homing/error); settings, camera trigger events; status: draft |
| `bom.md` | Bill of materials with cost estimates | 4 build tiers from $105 (slider-only) to $680 (slider + DJI RS 4); GPIO pin budget; power architecture; optional add-ons; status: draft |

## Architecture Decision Records (`adr/`)

| Path | Purpose | Key Details |
|------|---------|-------------|
| `adr/adr-001-microcontroller.md` | ADR: why ESP32-S3 over all alternatives | Compared 9 options; ESP32-S3 wins on Wi-Fi + CAN + USB-OTG + stepper HW + GPIO count in one chip; specific board: Espressif DevKitC-1 N8R8 or N32R16V; status: accepted |
| `adr/adr-002-status-display.md` | ADR: optional on-board status display | SH1107 1.5" I2C OLED (default) or ST7789 SPI TFT (alternative); display is optional; status: accepted |
| `adr/adr-003-linear-drive.md` | ADR: linear drive system, motor, and vertical operation | Payload spec (3.5kg real / 7kg design), torque equations for vertical, 4 drive options (belt/ball screw/lead screw/hybrid), motor analysis (NEMA 17 + EM brake recommended), self-locking analysis, thermal analysis, brake control hardware; status: exploring |
| `adr/adr-004-linear-guide.md` | ADR: linear guide and rail system | MGN12H-CC on 2040 extrusion (tall orientation); rods ruled out (deflection at 1200mm); V-wheels ruled out (no inverted operation); all-orientation capable; 0.76mm deflection; 4040 upgrade path; status: accepted |

## Implementation Plans (`plans/`)

| Path | Purpose | Key Details |
|------|---------|-------------|
| `plans/plan-motion-math.md` | Motion math library (pure TypeScript) | Catmull-Rom, SQUAD, easing, trajectory generator; dynamic axis model; full API signatures, algorithm details, per-axis timing model, test strategy; zero dependencies; status: draft |
| `plans/plan-mock-server.md` | Mock board server | Fastify + TypeScript standalone process; implements all board-api.md endpoints; simulated motion model; 3 capability profiles; WebSocket telemetry; error simulation; status: implemented |
| `plans/plan-dji-can.md` | DJI gimbal CAN protocol library (C++) | Portable library (no ESP32 deps); frame format, CRC16/CRC32, CanBusInterface abstraction; state machine; 3-phase command set (camera control in Phase 1); risk register; status: implemented |
| `plans/plan-web-ui.md` | SvelteKit web UI | Component architecture, store design, API client, V1/V2 scope, wireframes, <500KB bundle budget; captive portal, camera trigger V1, shared types, multi-client; status: draft |
| `plans/plan-firmware.md` | ESP32-S3 firmware | FreeRTOS tasks (Core 0=network, Core 1=motion), 6-state machine, DJI 10Hz rate limiting, camera trigger events, homing, WiFi AP + captive portal, NVS settings, multi-client WebSocket; status: implemented |
| `plans/shared-types.md` | Shared TypeScript types strategy | `shared/` directory at project root; web/ and mock-server/ import from it; firmware C++ types in commands.h; status: implemented |
