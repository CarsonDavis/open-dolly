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
| Mock board server implementation plan | `docs/project/plans/plan-mock-server.md` |
| Web UI implementation plan | `docs/project/plans/plan-web-ui.md` |
| DJI CAN protocol library implementation plan | `docs/project/plans/plan-dji-can.md` |
| ESP32-S3 firmware implementation plan | `docs/project/plans/plan-firmware.md` |
| Shared TypeScript types strategy | `docs/project/plans/shared-types.md` |

## Root

| Path | Purpose | Key Details |
|------|---------|-------------|
| `README.md` | Top-level project overview and vision | Design principles, how-it-works diagram, build configs with costs, links to all docs |
| `CLAUDE.md` | Indexing protocol and repo conventions | Defines INDEX.md format, update rules, sub-index structure |
| `docs/` | All documentation | Has its own `INDEX.md`; subdivided into `research/`, `project/`, `external/` |
