# OpenDolly Roadmap

Status: **active**

---

## V1 (current — complete)

All software components built and working. Hardware build not yet started.

| Component | Status |
|-----------|--------|
| Shared types (`shared/`) | Done |
| Motion math library (`motion-math/`) | Done |
| Mock board server (`mock-server/`) | Done |
| Web UI (`web/`) | Done |
| DJI CAN protocol library (`firmware/lib/dji_can/`) | Done |
| ESP32 firmware (`firmware/`) | Done (untested on hardware) |

**Web UI V1 features:**
- Connect to board (WebSocket with auto-reconnect)
- Jog control (2D touch pad, per-axis sliders)
- Live axis readouts from telemetry
- Save/manage keyframes (capture, reorder, rename, delete, go-to)
- Trajectory generation with global easing presets (via `@opendolly/motion-math`)
- Playback control (play, pause, resume, stop) with progress bar
- Status display (connection, system state, battery, errors)
- Settings page (device name, WiFi AP, telemetry rate, jog sensitivity)

---

## V2 (next — in design)

Full vision document: [`vision/v2-ui-vision.md`](vision/v2-ui-vision.md)
UX research: [`docs/research/slider-ui-ux/report.md`](../research/slider-ui-ux/report.md)

### Transition editing
- Transition simple view: duration input with derived per-axis speed indicators (mm/s, °/s), clickable to switch to speed-primary input
- Duration entry is blocking — must be set before navigating away or capturing another keyframe
- Per-axis curve editor: Photoshop Curves interaction (click to add points, drag to reshape), Premiere-style stacked lanes (all axes visible), multi-axis linking via checkboxes
- Curve math: monotone Catmull-Rom interpolation (no overshoot, passes through user-placed points)
- Editing speed in simple view warns if it will erase an advanced curve on that axis

### Timeline view
- Horizontal multi-channel timeline showing full sequence
- Draggable keyframe markers to retime transitions (redistributes duration between neighbors)
- Inline curve editing on timeline lanes
- Scrub preview: draggable playhead sends `scrub` commands to board, hardware follows in real-time (firmware support already exists)

### Control panel
- Two movement modes with toggle: position bar (click to jump) and jog (speed proportional to drag)
- Fine/coarse increment buttons: +1mm/+10mm (linear), +1°/+15° (rotation)
- Typed input for exact positioning
- Interactive mode (default, moves hardware) and static editing mode (offline planning)
- 25% default movement speed, configurable per-axis

### Keyframe interaction
- Selecting a keyframe card moves hardware there; jogging while selected overwrites that keyframe
- Deselecting returns to free jog mode for new keyframe capture
- Keyframe cards show axis value summaries with duration brackets between them

### Buffers (anti-jitter)
- Automatic settle zones at keyframe boundaries
- Default 1-second duration (clean cut points for video editing)
- Physical clearance checks with user warnings
- Configurable per-axis buffer duration and stiffness/inertia parameter

### Responsive layout
- Portrait phone: keyframe cards top-to-bottom, control panel tray at bottom
- Landscape phone/tablet: split screen (control panel left, keyframes right)
- Desktop: full timeline view available alongside keyframe view

---

## V3 (future)

### Loop mode
- Continuous back-and-forth playback between first and last keyframe
- Essential for interview B-roll and unmanned "second camera" workflows (every competitor has this)
- Motion-math library `loop` flag already implemented

### Save/load named sequences
- Persist named sequences for reuse across shoots
- Important for cinematographers returning to the same setup on multiple shoot days
- Likely localStorage or file export/import

### Visual animation preview
- Live visual preview at the top of the screen while the timeline is at the bottom
- Open question: what does this look like?
  - Vector representation of gimbal axes moving across the screen
  - Schematic overhead view of the slider
  - Live video from the camera
  - Just the physical hardware moving via scrub
- Distinct from scrub preview (V2) — scrub moves the hardware; this is a software-only visualization

### Move test mode
- Run the full trajectory at reduced speed to verify the move before committing to a real take
- Borrowed from Dragonframe ARC, which captures only video assist frames during test runs
- Useful for expensive or one-shot scenarios where you need confidence before rolling

### Shot templates
- Preset sequences for common cinematic moves: parallax (slide + counter-pan), push-in with ease, tracking, boom reveal
- Users can load a template and customize positions/timing
- Borrowed from Syrp Genie II, which provides templates for Clouds, People, Night Traffic, etc.

### Speed-based duration (full)
- Allow defining transitions by speed (e.g., "50 mm/s") instead of duration
- Only offered on axes where it makes sense (axis has a delta, unit matches)
- V2 has the partial version (derived speed display, clickable to edit); V3 could make speed a first-class input mode

### Target tracking
- Aim at the same subject from two different slider positions; system calculates pan/tilt tracking path automatically
- Borrowed from edelkrone HeadPLUS "teach by showing" feature
- Requires inverse kinematics math to derive gimbal angles from target position + slider position
