# Slider Project Index

## Reading Guide

| Task | Read First |
|------|------------|
| Understanding the DJI R SDK protocol | `docs/external/dji-r-sdk/INDEX.md` |
| Research on gimbal software control | `docs/research/dji-gimbal-software-control/report.md` |
| Research on keyframe animation | `docs/research/gimbal-keyframe-research/report.md` |
| Microcontroller/board selection | `docs/research/microcontroller-selection/report.md` |
| Motor and drive system selection | `docs/research/slider-motor-selection/report.md` |
| Vertical operation (motor sizing, brakes) | `docs/research/vertical-drive-motor-sizing/report.md` |
| Linear motion system design | `docs/research/long-travel-linear-motion/report.md` |
| Linear guide rail and extrusion selection | `docs/research/linear-guide-system/report.md` |
| System architecture and design decisions | `docs/project/architecture.md` |
| Board API protocol spec | `docs/project/board-api.md` |
| Motion math library implementation plan | `docs/project/plans/plan-motion-math.md` |
| Motion math library code | `motion-math/` |
| Mock board server implementation plan | `docs/project/plans/plan-mock-server.md` |
| Mock board server code | `mock-server/` |
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
| `package.json` | Root workspace config | npm workspaces: `shared`, `motion-math`, `mock-server`, `web`; `npm run dev` starts mock + UI via concurrently |
| `tsconfig.base.json` | Shared TypeScript config | ES2022, Node16 modules, strict mode; extended by all workspace packages |
| `shared/` | Shared TypeScript types (`@slider/shared`) | Board API types matching `board-api.md`; 6 modules: capabilities, state, trajectory, settings, errors, websocket |
| `mock-server/` | Mock board server for UI development | Fastify 5 + WebSocket; implements full Board API; profiles: slider-only, slider-servo, slider-dji; CLI with --port, --profile, --latency flags |
| `motion-math/` | Pure TypeScript motion math library | Has its own `INDEX.md`; Catmull-Rom splines, SQUAD quaternion interpolation, cubic bezier easing, trajectory table generator; no external deps; 49 tests via Vitest |
| `docs/` | All documentation | Has its own `INDEX.md`; subdivided into `research/`, `project/`, `external/` |
| `web/` | SvelteKit web UI for browser-based slider control | Has its own `INDEX.md`; V1: jog pad, keyframe capture, playback with easing via `@slider/motion-math`, settings page; SPA with adapter-static (184KB build); dark theme; imports `@slider/shared` and `@slider/motion-math`; Svelte 5 runes |
| `firmware/` | ESP32-S3 firmware (PlatformIO) | Has its own `INDEX.md`; DJI CAN lib in `lib/dji_can/`; all Board API endpoints implemented; 7 native test suites |
