# Slider + Gimbal System — Initial Vision

*Captured: 2026-03-20 through 2026-03-21*

## Concept

A motorized linear rail (slider) with a 3-axis gimbal mounted on it. A single software interface controls all axes — the slider's linear position plus the gimbal's pan/tilt/roll — giving full camera motion control.

## Hardware

- **Slider:** Two-rod linear rail with a sliding platform, driven by a motor (left/right movement)
- **Gimbal:** Either custom-built or a DJI gimbal mounted on the platform
- **Control board:** Sits on the slider, talks to both the gimbal and the slider motors
  - Should be relatively "dumb" — receives commands, executes motion
  - Communicates with the app via Wi-Fi or Bluetooth

### Firmware & Configuration

- Firmware should be generic — works for any hardware setup
- User provides a **configuration file** when flashing that defines:
  - Motor specs (min/max speeds, step resolution)
  - Slider rail length
  - Gimbal type and how to communicate with it (e.g. DJI protocol vs custom)
- The board reports its capabilities (resolution, speed limits) to the web app so the UI can enforce valid inputs
- If a DJI gimbal is attached, the board should detect it and communicate that to the web app

## Software

- **Web app** (phone or computer) is the brain — where you set up motion control
- The control board executes commands received from the web app
- Need to support using the phone as a camera monitor while also running the web app — connectivity/routing TBD
- During development: serve on a local port, accessible from phone on same Wi-Fi network

## UX Research

- Study professional-grade keyframe/animation software — not hobby side projects, but tools where real thought has gone into UX
- **edelkrone (iFootage)** slider UI is the closest existing product — identify what it does well
- Look at critical reviews of edelkrone/iFootage to find features users wish existed
- Study After Effects, Premiere, Photoshop curves — these are the interaction models to draw from
- Research tutorials on how people actually use camera sliders to understand real workflows

## Control Panel

The control panel is the primary interaction surface — always visible at the bottom (phone vertical) or left side (phone horizontal / desktop).

### Manual Movement

- Direct control of all axes via the control panel
- Default movement speed: 25% of max system speed (configurable in settings per-axis)
- Two increment levels per axis:
  - **Slider:** +1 mm (fine) and +10 mm (coarse) buttons
  - **Rotational axes:** +1° (fine) buttons
  - For ultra-precise positioning: type in exact values (e.g. 1.356 mm)
- The UI enforces the actual resolution of the hardware — you can't type in precision the motors can't achieve
- When you move via the control panel, the physical system moves in real time (controller is always linked to the board by default)

### Speed Settings (v1)

- User preferences section with a control speed setting
- Adjustable per-axis as a percentage of max speed (default 25%)
- This speed is for **manual repositioning only** — not the speed used during keyframe playback

## Keyframe System

### Data Model

- A **keyframe** stores the exact position of all axes (slider position + gimbal pan/tilt/roll — up to 6 axes)
- A **transition** between two keyframes is its own data structure containing:
  - Duration in seconds
  - Per-axis curve definitions (default: linear)
  - Some axes may have curves, others stay linear

### Creating Keyframes

1. Use the control panel to move to desired position/framing
2. Click "Generate Keyframe" — captures current controller state
3. Move to next position, generate another keyframe
4. After two keyframes exist: define the transition duration (seconds)
  - Duration input respects system precision — can't type in meaningless precision
  - Flashing box prompts you to enter seconds for the transition

### Transition Curves

- After setting duration, the UI switches from the position control panel to **curve editors**
- One timeline per axis (up to 6) showing movement from keyframe A to keyframe B
- **Photoshop curves model:** click on the curve to create control points, drag points to reshape
  - Drag from the left edge → delay the start (flat line then ramp)
  - Drag from the middle → change the curve shape (ease in/out, exponential)
  - Multiple control points allowed for complex curves
- Example: to delay pan until halfway through a 10-second move, click a point on the pan timeline at 5s, keep it flat until that point, then curve up
- A button to exit curve editing and return to position control

### Buffers (Anti-Jitter)

- When the system arrives at a keyframe at speed, physical inertia causes jitter/rebound
- **Workaround (no software support):** User manually creates extra keyframes for deceleration zones
- **Built-in buffer feature:** Enable in settings, system automatically:
  - Estimates moment of inertia based on speeds and hardware config
  - Adds exactly 1-second buffer on each side of a keyframe for clean cuts in editing
  - Smart enough to check if there's physical room for the buffer (e.g. at the end of the rail)
  - Warns the user if a buffer can't be generated for a specific keyframe
  - Buffer size should be configurable for stiff/flexible setups

## Views

### Keyframe View (Phone + Desktop)

- Keyframes displayed as boxes in a vertical stack (portrait) or horizontal row (landscape)
- Each box shows a brief summary of all axis values
- Square brackets between keyframes show transition duration — click to edit
- Click a keyframe to select it → control panel updates to show/edit that keyframe's values
- Two interaction modes:
  - **Interactive (default):** Editing a keyframe physically moves the system to that position
  - **Static editing:** Edit values theoretically without moving hardware, then execute later
- Control panel lives in a collapsible tray — minimize it to see more keyframes

### Timeline View (Desktop Primary, Phone TBD)

- Full left-to-right timeline showing all keyframes and all axis channels stacked
- Each axis has its own channel with visible curves
- Keyframes appear as markers on the timeline with small boxes showing values
- Can drag keyframes left/right to change timing (like Premiere)
  - Dragging redistributes time between adjacent transitions
  - Example: 3 keyframes at 5s, 10s, 5s — drag middle keyframe left to 2.5s → becomes 2.5s, 12.5s, 5s
- Can edit curves directly on the timeline
- Simultaneous preview: timeline at bottom, live animation preview at top

### Responsive Layout

- **Portrait (phone):** Keyframes top-to-bottom, control panel at bottom
- **Landscape (phone/tablet):** Keyframes on right, control panel on left (split screen)
- **Desktop:** Full timeline view available; keyframe view also accessible
- App should be served locally and accessible from any device on the same Wi-Fi

## Versioning Roadmap

### Version 1
- 2 keyframes, linear interpolation
- Adjustable transition duration (seconds)
- Manual control panel with fine/coarse increments
- Default 25% speed for manual repositioning (configurable in settings per-axis)
- Firmware config file for hardware specs

### Version 2
- N keyframes per sequence
- Per-axis curve editing (Photoshop curves model)
- Timeline view with drag-to-reorder/retime
- Buffer system for smooth stops
- Non-linear speed curves between keyframes
- Per-axis timing offsets (delay pan, start slider first, etc.)

## Explicitly Not Supporting (For Now)

- Defining transitions by speed (e.g. "move at X mm/s") instead of duration — gets complicated when only some axes change between keyframes. Revisit later.

## Open Questions

- How to run the web app on the phone while also using the phone as a camera monitor (both need gimbal connectivity)?
- Wi-Fi vs Bluetooth for control board communication — latency/range tradeoffs?
- DJI SDK access for gimbal control?
- Does the timeline view make sense on a phone, or is it desktop-only?
- How should the system handle mixed interaction: user manually controlling while keyframes exist?
- What's the right model for detecting gimbal type (DJI vs custom) on the control board?
