# OpenDolly Project Index

## Reading Guide

| Task | Read First |
|------|------------|
| Understanding the DJI R SDK protocol | `docs/external/dji-r-sdk/INDEX.md` |
| Research on gimbal software control | `docs/research/dji-gimbal-software-control/report.md` |
| All DJI RS control methods (CAN, SBUS, commercial, dead ends) | `docs/research/dji-rs-control-methods/report.md` |
| Research on keyframe animation | `docs/research/gimbal-keyframe-research/report.md` |
| Microcontroller/board selection | `docs/research/microcontroller-selection/report.md` |
| Motor and drive system selection | `docs/research/slider-motor-selection/report.md` |
| Vertical operation (motor sizing, brakes) | `docs/research/vertical-drive-motor-sizing/report.md` |
| Linear motion system design | `docs/research/long-travel-linear-motion/report.md` |
| Linear guide rail and extrusion selection | `docs/research/linear-guide-system/report.md` |
| Slider electrical bring-up (wiring + firmware tweaks) | `docs/project/slider-wiring.md` |
| KiCad schematic + breadboard-reference walkthrough | `hardware/kicad/slider/README.md` |
| AI object tracking system | `ai-tracking/README.md`, `docs/research/ai-object-tracking/report.md` |
| Sony camera control integration | `sony-control/VERIFIED-REFERENCE.md`, `docs/research/sony-camera-control/report.md` |
| AI tracking + camera control vision | `docs/project/vision/ai-tracking-vision.md` |
| System architecture and design decisions | `docs/project/architecture.md` |
| Board API protocol spec | `docs/project/board-api.md` |
| Motion math library implementation plan | `docs/project/plans/plan-motion-math.md` |
| Motion math library code | `motion-math/` |
| Mock board server implementation plan | `docs/project/plans/plan-mock-server.md` |
| Mock board server code | `mock-server/` |
| Project roadmap (V1/V2/V3) | `docs/project/roadmap.md` |
| Product vision and V2 UI design | `docs/project/vision/` |
| V2 UI/UX research | `docs/research/slider-ui-ux/report.md` |
| Web UI implementation plan | `docs/project/plans/plan-web-ui.md` |
| Web UI code | `web/` |
| DJI CAN protocol library implementation plan | `docs/project/plans/plan-dji-can.md` |
| DJI CAN protocol library code | `firmware/lib/dji_can/` |
| ESP32-S3 firmware implementation plan | `docs/project/plans/plan-firmware.md` |
| Shared TypeScript types strategy | `docs/project/plans/shared-types.md` |
| Shared TypeScript types code | `shared/` |

## Root

| Path | Purpose | Key Details |
|------|---------|-------------|
| `README.md` | Top-level project overview and vision | Design principles, how-it-works diagram, build configs with costs, links to all docs |
| `CLAUDE.md` | Indexing protocol and repo conventions | Defines INDEX.md format, update rules, sub-index structure |
| `package.json` | Root workspace config | npm workspaces: `shared`, `motion-math`, `mock-server`, `web`; `npm run dev` starts mock + UI; `npm run deploy` builds + flashes firmware + UI to board |
| `tsconfig.base.json` | Shared TypeScript config | ES2022, Node16 modules, strict mode; extended by all workspace packages |
| `shared/` | Shared TypeScript types (`@opendolly/shared`) | Board API types matching `board-api.md`; 7 modules: capabilities, state, trajectory, settings, errors, websocket, transition (V2: ProgressCurvePoint, AxisCurve, BufferConfig, Transition) |
| `mock-server/` | Mock board server for UI development | Fastify 5 + WebSocket; implements full Board API; profiles: slider-only, slider-servo, slider-dji; CLI with --port, --profile, --latency flags |
| `motion-math/` | Pure TypeScript motion math library | Has its own `INDEX.md`; Catmull-Rom splines, SQUAD quaternion interpolation, cubic bezier easing, monotone PCHIP progress curves (V2), trajectory table generator; no external deps; 69 tests via Vitest |
| `docs/` | All documentation | Has its own `INDEX.md`; subdivided into `research/`, `project/`, `external/` |
| `web/` | SvelteKit web UI for browser-based slider control | Has its own `INDEX.md`; V2: per-axis control panel (jog/position modes, increments, typed input), transition editing (duration/speed, per-axis curve editor), timeline with scrub preview, buffer support; SPA with adapter-static (204KB build); dark theme; Svelte 5 runes |
| `ai-tracking/` | AI object detection and tracking for automated gimbal control | Python package; plugin architecture: DetectorBase/TrackerBase/DepthEstimatorBase ABCs with registry; ultralytics YOLO11/v8/RT-DETR detectors, BoT-SORT/ByteTrack trackers, Depth Anything V2; 10 YAML profiles (fast/balanced/accurate + depth variants); CLI: `ot-live` (webcam), `ot-benchmark` (comparison table + CSV); not yet tested |
| `sony-control/` | Sony Camera Remote SDK integration and custom focus control tools | SDK v2.01.00 (C/C++, macOS); verified: USB connection, absolute focus position, NearFar stepping, AF trigger via S1; custom tools: focusControl, focusPull, focusPullSmooth; see `VERIFIED-REFERENCE.md` for tested patterns |
| `firmware/` | ESP32-S3 firmware (PlatformIO) | Has its own `INDEX.md`; OPI flash mode required; DJI CAN lib in `lib/dji_can/`; all Board API endpoints implemented; tested on DevKitC-1 v1.1; 7 native test suites |
| `hardware/` | KiCad projects for custom electronics | `hardware/kicad/slider/` — Phase 1 schematic in progress (KiCad 10) and beginner walkthrough doubling as breadboard wiring reference; targets slider bring-up per `docs/project/slider-wiring.md`. |
