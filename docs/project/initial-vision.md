# Slider + Gimbal System — Initial Vision

*Captured: 2026-03-20*

## Concept

A motorized linear rail (slider) with a 3-axis gimbal mounted on it. A single software interface controls all axes — the slider's linear position plus the gimbal's pan/tilt/roll — giving full camera motion control.

## Hardware

- **Slider:** Two-rod linear rail with a sliding platform, driven by a motor (left/right movement)
- **Gimbal:** Either custom-built or a DJI gimbal mounted on the platform
- **Control board:** Sits on the slider, talks to both the gimbal and the slider motors
  - Should be relatively "dumb" — receives commands, executes motion
  - Communicates with the app via Wi-Fi or Bluetooth

## Software

- **Web app** (phone or computer) is the brain — where you set up motion control
- The control board executes commands received from the web app
- Need to support using the phone as a camera monitor while also running the web app — connectivity/routing TBD

## Interaction Model

### Manual Control
- Direct control of slider position and gimbal orientation through the web app
- Navigate to a position + orientation, then save it as a **keyframe**

### Keyframe System
- Define keyframes (position + gimbal orientation), software interpolates the path between them
- **Version 1:** 2 keyframes only, linear interpolation, adjustable speed
- **Version 2:**
  - N keyframes per sequence (10+)
  - Non-linear speed curves (drag to create ease-in/ease-out, exponential, etc.)
  - Per-axis timing control — up to 4 tracks:
    - X (slider position)
    - Pan, Tilt, Roll (gimbal rotation)
  - Default: all axes move together linearly
  - Advanced: offset axes so e.g. pan completes first, then slider starts moving
  - **Timeline view** with sliders/knobs to edit per-track timing

## UX Goals

- Intuitive for first-time users
- Familiar to anyone who's used animation/keyframe software (After Effects, etc.)
- Need to research how existing keyframe/animation software handles this — look at After Effects, motion control software, etc. for inspiration

## Open Questions

- How to run the web app on the phone while also using the phone as a camera monitor (both need gimbal connectivity)?
- Wi-Fi vs Bluetooth for control board communication — latency/range tradeoffs?
- DJI SDK access for gimbal control?
- What does the timeline/curve editor UI look like in practice?
