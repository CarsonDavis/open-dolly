# V2 Web UI Vision

*Compiled from voice notes captured 2026-03-21. This document expands on the V2 items outlined in [`initial-vision.md`](initial-vision.md) with full detail on interaction models, data structures, user stories, and open design questions.*

Status: **draft**

---

## Motivation

The V1 UI gets the job done — you can jog, capture keyframes, pick a global easing preset, and play back. But the keyframe editing experience is shallow. You can't shape how individual axes move between keyframes. You can't see or edit timing relationships. You can't delay one axis relative to another within a transition. These are solved problems in professional animation and video editing software, and we should adopt their interaction models rather than invent our own.

The goal is to make the keyframe workflow feel like working in Premiere or After Effects — direct manipulation of curves, visual timelines, drag-to-retime — while keeping the interface usable on a phone held in one hand on set.

---

## Tasks

### Research

All research tasks are complete. Results in [`docs/research/slider-ui-ux/`](../../research/slider-ui-ux/report.md).

- [x] **R1: Professional slider UI survey.** Study professional-grade camera slider and motion control software UIs. Focus on products where real thought has gone into UX — not hobby projects. Key targets:
  - iFootage / edelkrone slider app (closest to what we're building)
  - Dragonframe (stop motion, but excellent keyframe/timeline UI)
  - edelkrone HTTP SDK documentation
  - Any other commercial slider apps with notable UX
  - Deliverable: summary of each product's UI model, what it does well, screenshots/screen recordings where possible
  - **Result:** edelkrone, iFootage Moco, Rhino Arc II, Dragonframe ARC. See `background-slider-apps.md`.

- [x] **R2: Critical reviews and missing features.** Search for reviews, forum posts, Reddit threads, and YouTube comments where users of slider software (especially iFootage/edelkrone) describe features they wish existed or pain points with the current UI. We want to know what the market wants but doesn't have.
  - Deliverable: categorized list of user complaints and feature requests with sources
  - **Result:** Covered in R1. Bluetooth drops are the #1 complaint; iFootage's per-axis graph editor is "too slow"; no one has shipped reliable curve editing on mobile.

- [x] **R3: Slider workflow tutorials.** Find tutorials (YouTube, blog posts) where experienced cinematographers explain how they actually use camera sliders — how they plan moves, how they set up keyframes, what their workflow looks like from concept to final shot. We need to understand real usage patterns so we design for how people actually work, not how we imagine they work.
  - Deliverable: summary of common workflows, pain points, and tricks users employ
  - **Result:** Real-world workflows, common moves, solo vs crew patterns. See `background-workflows.md`.

- [x] **R4: Animation/curve editor UI patterns.** Study how professional creative software handles curve editing and timeline interaction. Specific references called out in the notes:
  - **Premiere Pro** — volume keyframes on a clip (click a line, drag to create a curve, the curve maps to an equation)
  - **Photoshop** — Curves adjustment dialog (click to add control points, drag to reshape)
  - **After Effects** — keyframe graph editor (value graph and speed graph)
  - CSS `cubic-bezier()` editors (e.g. cubic-bezier.com)
  - Deliverable: interaction model comparison — how each tool handles adding points, reshaping curves, constraining edits, and previewing results
  - **Result:** AE, Premiere, Photoshop Curves, Blender, Theatre.js, web-based editors. See `background-curve-editors.md`.

### Documentation

- [ ] **D1: Functional requirements document.** Define every capability the V2 UI must support, divorced from wireframes and page layout. Pure "the user should be able to..." statements. Organized by feature area, not by screen. This document drives everything else.

- [ ] **D2: User stories.** Write detailed user stories that walk through complete workflows end-to-end. At minimum:
  - First-time setup and first keyframe capture
  - Multi-keyframe product shoot with curve editing
  - Using buffers to get jitter-free stops
  - Editing an existing sequence (retiming, adjusting curves)
  - Working on a phone vs. working on a desktop

- [ ] **D3: Data model spec.** Define the V2 data structures — keyframes, transitions, curves, buffers — and how they relate to the existing `@opendolly/shared` types and the Board API. This is the contract between the UI and the motion-math library.

- [ ] **D4: Wireframes.** Create wireframes for all views and states described below — keyframe view (phone portrait, phone landscape, desktop), timeline view (desktop), curve editor, control panel states. Do these after R1-R4 and D1-D2 are complete so they're informed by research.

- [ ] **D5: V2 implementation plan.** Break the build into phases with clear milestones. Written after D1-D4 are done.

---

## Core Concept: Two Views

The app has two primary ways to work with keyframes. They operate on the same underlying data and can be switched between freely.

### Keyframe View

This is the card-based view — an evolution of the current V1 UI. It works well on phones and is the primary mobile interaction mode.

**Layout:**
- Keyframes are displayed as boxes/cards in a stack
- In portrait orientation: keyframes stack top-to-bottom, control panel at bottom
- In landscape orientation: keyframes on the right, control panel on the left (side-by-side split)
- The control panel lives in a collapsible tray — the user can minimize it to see more keyframes on screen

**Keyframe cards:**
- Each card shows a brief summary of all axis values for that keyframe (e.g., "Pan 45.0° | Tilt -10.0° | Slide 500mm")
- Clicking a keyframe **selects** it — the hardware moves to that keyframe's position (in interactive mode), and the control panel reflects those values
- While a keyframe is selected, **jogging overwrites that keyframe's stored values** — this is how you fine-tune a keyframe after capture
- Tapping elsewhere (or a deselect action) returns to **free jog mode** — jogging moves the hardware without affecting any stored keyframe, and "Generate Keyframe" captures a new one

**Duration brackets:**
- Between each pair of adjacent keyframes, a bracket (thin line with a number in the center) shows the transition duration in seconds
- The bracket joins the two keyframe cards visually
- Clicking the duration number opens the curve editor for that transition (see Curve Editor section below)

**Key difference from V1:** In V1, keyframes are simple position snapshots with a single global easing preset. In V2, every pair of adjacent keyframes has a rich transition definition with per-axis curves, timing, and optional buffers.

### Timeline View

A horizontal, multi-channel timeline showing the full sequence at once.

**Layout:**
- Left-to-right timeline spanning the full duration of the sequence
- One horizontal channel/lane per axis, stacked vertically
- Each channel shows the curve for that axis's motion over time
- Keyframe positions appear as vertical markers across all channels, with small boxes showing the axis values at that point
- Time scale at the bottom with second markers

**Direct manipulation on the timeline:**
- Drag a keyframe marker left or right to change its timing
- Dragging automatically redistributes duration between adjacent transitions
- Example: three keyframes with transitions of 5s, 10s, 5s. Drag the middle keyframe earlier so the first transition becomes 2.5s — the second transition automatically becomes 12.5s, and the third stays 5s
- Edit curves directly on the timeline channels (same interaction as the curve editor, but inline)

**Scrub preview (V2):**
- The timeline has a draggable **playhead** (vertical line showing current time position)
- Dragging the playhead sends `scrub` commands to the board — the physical hardware moves to the corresponding trajectory position in real-time
- This lets the user verify any point in the move without running the full playback
- The firmware already supports scrub via binary-search interpolation on the uploaded trajectory

**Visual animation preview (V3):**
- A live visual preview of the animation at the top of the screen while the timeline is at the bottom
- Open question: what does this look like? Options: vector representation of the gimbal/slider moving, schematic overhead view, or just the physical hardware moving via scrub
- Deferred to V3

**Differences from Keyframe View:**
- Timeline view uses drag-based timing; keyframe view uses typed numeric input
- Timeline view shows all transitions at once; keyframe view focuses on one transition at a time
- Timeline view makes relative timing between axes immediately visible

---

## Control Panel

The control panel is the primary surface for moving the hardware and positioning the camera. It is always present (at the bottom on portrait phone, on the left on landscape/desktop) and can be minimized to a tray.

### Interaction Modes

The control panel has two modes:

**Interactive mode (default):** When you adjust values in the control panel, the physical system moves in real time. The control panel always reflects the actual position of the hardware as reported by the board's sensors. This is the default because the primary workflow is: move the camera to where you want it, then capture a keyframe.

**Static editing mode:** Edit keyframe values theoretically without moving the hardware. Useful when you're not connected to a board or when you want to plan a sequence without executing it. The control panel values are the source of truth for creating keyframes (not sensor readings).

### Movement Speed

When you change a value in the control panel (interactive mode), the system moves the hardware to the new position. The speed at which it moves is configurable:

- **Default:** 25% of the maximum speed of each axis
- **Configurable per-axis** in user preferences/settings — you can set the slider to move at 100% max speed while the gimbal moves at 25%
- The speed setting is for **manual repositioning only** — it has no relationship to the speed used during keyframe playback
- The board reports min and max speeds per axis, so the UI knows what range is valid


### Movement Controls

The user can toggle between two control modes for via a switch in the control panel:

**Position mode:** A line/bar representing the full range of the axis (e.g., 0–900mm for slide, -180° to +180° for pan). Clicking anywhere on the line jumps the hardware to that position. Shows the current position as a marker. Good for quickly jumping to a known position.

**Jog mode (default):** A bidirectional drag control. Sliding left/right adjusts the axis, with speed proportional to how far the user drags from center. 

Both modes share:
- **Fine/coarse increment buttons** at each end of the control — for linear axes: +1 mm (fine) and +10 mm (coarse); for rotational axes: +1° (fine) and +15° (coarse)
- **Typed input field** — tap the current value readout to type an exact position (e.g., 101.356 mm, 1.53°)

### Precision Constraints

- The board reports the actual resolution of each axis (determined by motor steps, microstepping, gear ratios)
- The UI enforces this — you cannot type in precision that the hardware can't achieve
- Input values are rounded/snapped to the nearest achievable position
- This prevents meaningless precision like "5.999998799354 seconds" for a duration

### Firmware Configuration

The board needs to know the hardware specs so it can report capabilities accurately to the UI:

- When flashing firmware, the user provides a configuration that defines: motor specs (min/max speeds, step resolution), rail length, gimbal type and protocol
- The board uses this config to report capabilities via the API
- The UI adapts to whatever the board reports — it doesn't hardcode axis counts, ranges, or resolution
- If a DJI gimbal is detected, the board communicates this to the UI so gimbal-specific features can be enabled

---

## Keyframe Workflow

This section walks through the primary user workflow in detail.

### First Keyframe

1. User opens the app. No keyframes exist yet.
2. The control panel shows the current position of all axes as read from the board's sensors (e.g., slider at the far left, gimbal pointing straight ahead).
3. User adjusts the control panel — presses buttons, drags sliders, or types values. The hardware moves in real time to the new position.
4. When the user is happy with the framing, they press "Generate Keyframe."
5. A keyframe card appears with the exact position values from the controller at the moment of capture.

### Second Keyframe and Duration

1. User continues adjusting the control panel to find the next position.
2. They press "Generate Keyframe" again. Now two keyframe cards are visible.
3. The control panel (bottom tray) automatically appears showing the **transition simple view** for the new transition. This view has:
   - A **duration input** field (e.g., "10" seconds). This is the primary input.
   - Below the duration, a **greyed-out speed indicator** showing the derived speed for each axis that changes between the two keyframes — e.g., "125 mm/s" for slide, "4.5 °/s" for pan. Only axes with a delta are shown; if only rotational axes change, no mm/s appears.
   - The speed indicators are **clickable** — tapping one switches that axis to speed-based input, and the duration auto-adjusts to match. This lets the user think in either duration or speed, whichever is more natural for the move.
   - An **"Advanced"** button in the top-right corner opens the full per-axis curve editor (see Editing Transition Curves below).
4. **Duration entry is blocking.** The user must enter a duration (or set a speed that derives one) before they can navigate away or capture another keyframe. This ensures every transition has a defined duration.
5. By default, the transition is linear on all axes — constant speed from position A to position B.

### Editing Transition Curves

The curve editor is accessed from the transition simple view by pressing the **"Advanced"** button. It only controls the timing/easing of the transition — you cannot modify keyframe positions from here.

1. The control panel area transforms into a **stacked per-axis curve editor** (Premiere-style inline lanes). One lane per axis, all visible simultaneously.
2. Each lane spans the duration of the transition (e.g., 10 seconds, shown at the bottom).
3. Initially each shows a straight line from bottom-left to top-right (linear interpolation).
4. The user can:
   - **Click on the curve** to add a control point
   - **Drag a control point** to reshape the curve
   - **Drag from the left edge** to delay when that axis starts moving (the line stays flat at the bottom, then ramps up)
   - **Drag from the middle** to change the shape of the curve (ease in, ease out, S-curve)
   - Add multiple control points to create complex curve shapes
5. The curve editing model is **Photoshop Curves style** — not preset-based, but direct point manipulation.
6. Each curve is backed by a mathematical equation. The points the user creates define that equation. This is the same concept as Premiere's volume keyframe line — the visual line you drag corresponds to an underlying function.
7. A "Done" / "Back" button exits curve editing and returns to the position control panel.

**Multi-axis linking:** Each axis lane has a **checkbox**. When multiple checkboxes are selected, the system asks "Edit multiple axes at once?" If confirmed, editing the curve on any checked axis applies the same curve shape to all checked axes. This is useful for rotation axes that should move together (e.g., pan and tilt tracking a subject) without having to draw the same curve repeatedly.

**Example — delayed pan:**
- 10-second move, 4 axes
- User wants the pan to not start until 5 seconds in
- Goes to the pan channel in the curve editor
- Clicks a point at the 5-second mark on the line
- Drags this point down to the bottom
- This results in a flat line for 5 seconds and then a straight line from the bottom to the top right
- The line stays flat (no movement) from 0s to 5s, then goes straight up from 5s to 10s
- All other axes move for the full 10 seconds as normal

**Example — ease into a stop:**
- User wants the slide to decelerate smoothly into the final position
- Click on the middle of the line
- Drag the line up and it will curve, making a steep curve at the beginning and a gentle curve at the end
- Result: fast start, gentle coast to a stop

### Subsequent Keyframes

- The workflow repeats: position → capture → set duration → optionally edit curves
- Each pair of adjacent keyframes gets its own independent transition with its own per-axis curves
- In the keyframe view, you're always looking at one transition at a time (the one whose bracket you clicked)
- In the timeline view, you see all transitions at once and can edit any of them inline

---

## Transition Data Model

A transition between two keyframes needs to capture:

- **Duration** in seconds (with precision bounded by system capabilities)
- **Per-axis curve definitions** — for each axis:
  - A set of control points that define the curve shape (could be a spline, bezier, or piecewise function)
  - Default: linear (no control points, straight line)
  - Some axes may be linear while others have custom curves
  - The curve maps normalized time [0, 1] to normalized progress [0, 1]
- **Buffer settings** (see Buffers section) — whether buffers are enabled for this transition and their configuration

This is distinct from a keyframe, which is just a position snapshot. The transition is its own first-class data structure.

**Relationship to existing code:** The current `@opendolly/motion-math` library supports per-axis easing via `AxisConfig` (delay, duration, easing preset or custom bezier). The V2 curve editor needs to map to a more expressive model — likely multi-point bezier or spline curves rather than single cubic-bezier. This is a data model question that needs to be resolved in task D3.

---

## Buffers (Anti-Jitter System)

### The Problem

When the system arrives at a keyframe at speed, physical inertia may causes the rig to flex and rebound. The camera jitters at the stop point. This would be especially noticeable on fast moves or with heavy payloads.

### Current Workaround (No Software Support)

The user manually creates extra keyframes to handle deceleration:
1. Keyframe A: starting position
2. Keyframe B: the actual end position (the framing they want)
3. Keyframe C: a few inches past B, with a slow transition — this is where the system actually decelerates, absorbing the jitter

This works but it's tedious. Every move needs extra keyframes that exist purely for mechanical reasons, not creative ones. The user has to think about physics instead of cinematography.

### Built-in Buffer Feature

An advanced setting that automates the deceleration padding:

**How it works:**
1. User enables "Buffers" in settings
2. When a keyframe transition is generated, the system automatically adds extra travel before or after the keyframe
3. The buffer is calculated based on:
   - The speed of the move (how fast each axis is traveling as it approaches the keyframe)
   - An estimate of the system's inertia / stiffness (could be a user-configured value or derived from hardware specs)
   - The physical room available (how far the axis is from its limits)

**Buffer rules:**
- Buffers default would be exactly **1 second** in duration on each side — this makes it trivial to trim in editing software (just cut the first and last second of the clip)
- The buffer duration should be configurable for users with stiff or flexible setups
- The system must check if there's physical room for the buffer:
  - If the slider is at the far left end of the rail, there's no room for a pre-buffer on that axis
  - If a rotational axis is at its limit, no buffer in that direction
- When a buffer can't be generated, the system **warns the user** with a clear message (e.g., "Buffer cannot be generated for the slide axis at this keyframe — not enough rail travel remaining")
- The user can choose to proceed without the buffer or adjust their keyframe positions

**Buffer as a setting:**
- Global toggle: on/off
- Per-axis buffer duration (default 1 second)
- Stiffness/inertia parameter (affects how much extra travel the buffer needs)
- Could be enabled per-transition rather than globally (design question)

**Relationship to video editing:** The reason for exact 1-second buffers is that the user knows they're going to import this footage into Premiere/Resolve/Final Cut and they need to trim the settling period. A 1-second buffer is a clean, predictable cut point. The system should make the buffer boundaries obvious — no ambiguity about where the "real" move starts and ends.

This idea that we have a 1 second buffer might make it unneccessary to actually do calculations, but there is always the question of what distance is travelled in that 1 second.

---

## Responsive Layout

The UI must work on both phones and desktop browsers, with the same codebase adapting its layout.

### Portrait Phone (Primary Mobile Use Case)

```
┌─────────────────────────┐
│                         │
│      Keyframe cards     │
│      (vertical stack,   │
│       scrollable)       │
│                         │
│       |- [KF 1]         │
│   5s -|                 │
│       |- [KF 2]         │
│  10s -|                 │
│       |- [KF 3]         │
│                         │
├─────────────────────────┤
│  Control panel (tray)   │
│  [minimize ▼]           │
│  Axis controls, capture │
│  button, etc.           │
└─────────────────────────┘
```

- Keyframes are top-to-bottom
- Control panel is a collapsible tray at the bottom
- Minimize the tray to see more keyframes
- Duration brackets appear between keyframe cards
- Clicking a bracket replaces the control panel with the curve editor for that transition

### Landscape Phone / Tablet

```
┌──────────────┬──────────────────┐
│              │                  │
│  Control     │   Keyframe cards │
│  panel       │   (horizontal    │
│              │    or vertical   │
│  Axis        │    scroll)       │
│  controls,   │                  │
│  capture     │   [KF1] [KF2]    │
│  button      │                  │
│              │                  │
└──────────────┴──────────────────┘
```

- Split screen: control panel on the left, keyframes on the right
- Keyframes may flow left-to-right in landscape since there's horizontal space

### Desktop

- Full timeline view is available as an additional option
- Keyframe view also accessible (same as landscape layout but with more space)
- Timeline view: timeline at bottom, live preview at top

### Dev Access from Phone

The dev server (Vite) must be accessible from a phone on the same Wi-Fi network. This is how we test during development — serve on a local port on the MacBook, navigate to that IP:port from the phone. This already works with the current Vite setup.

---

## Speed-Based Duration Input

### Integrated into Transition Simple View (V2)

The transition simple view shows duration as the primary input, with derived per-axis speeds displayed below it. Each speed indicator is clickable to switch to speed-based input for that axis, which auto-adjusts the duration.

**How it works:**
- Duration is the primary input. Below it, each axis that has a delta between the two keyframes shows its derived speed in greyed-out text (e.g., "125 mm/s" for slide, "4.5 °/s" for pan).
- Axes with no change between the keyframes are not shown.
- Clicking a speed indicator makes that axis's speed the primary input — the user edits the speed value, and the duration auto-adjusts. All other axes' speeds update accordingly.
- Only one input mode is active at a time: either duration-primary or speed-primary for a specific axis.

**Why per-axis:** A transition involves multiple axes with different units. "50 mm/s" is meaningful for slide but not for pan. Each axis shows its own speed in its own unit (mm/s for linear, °/s for rotation). The user picks which axis's speed to drive the duration from.

If an advanced curve has been created in the curves mode, then editing a single speed value will give a warning that this will erase the curve. If the user accepts, it puts that axis back to linear.

---

## Open Design Questions

### Resolved

1. **Curve math model:** Use **monotone Catmull-Rom interpolation** for progress curves. Interpolating (curve passes through user-placed control points, matching the Photoshop Curves mental model), monotone (prevents overshoot — critical for physical hardware), and already partially implemented in `@opendolly/motion-math`. The `generateTrajectory()` API needs to be extended to accept multi-point curve definitions per axis instead of single bezier presets. See research: `docs/research/slider-ui-ux/report.md` section 3.

2. **Jog + keyframes interaction:** Selecting a keyframe card moves hardware there; jogging while selected overwrites that keyframe. Deselecting returns to free jog mode for new keyframe capture.

3. **Curve editor scope:** Curve editing only controls timing/easing. Keyframe positions are edited via the control panel, not the curve editor. The curve editor replaces the control panel area while open.

### Still Open

5. **Buffer calculation inputs:** How does the system estimate the inertia/stiffness needed for buffer calculations? Options: (a) user enters a stiffness parameter manually, (b) system derives it from hardware config (payload weight, motor torque), (c) system runs a calibration move and measures settling time. The first is simplest; the last is most accurate. To be determined from testing with actual hardware.

6. **Simultaneous timeline + preview (V3):** What does the animation preview look like? A 3D visualization? A schematic overhead view of the slider? Just live video from the camera? Or just the physical hardware moving? Deferred to V3.

---

## Relationship to Existing V1 Code

The V2 UI builds on the existing V1 codebase in `web/`. Key areas of change:

| V1 Component | V2 Change |
|---|---|
| `keyframeStore` | Needs to store transitions (curves, per-axis timing) between keyframes, not just position snapshots |
| `PlaybackControls` | Easing preset dropdown replaced by curve editor integration; trajectory generation uses new curve data |
| `@opendolly/motion-math` | `generateTrajectory()` needs to accept multi-point curve definitions, not just single bezier presets |
| `KeyframeList` / `KeyframeCard` | Duration brackets between cards; click-to-edit transitions; richer card display |
| New: `CurveEditor` component | Per-axis curve editor with point manipulation (does not exist in V1) |
| New: `TimelineView` component | Multi-channel horizontal timeline with drag-to-retime (does not exist in V1) |
| `JogControl` | Evolves into V2 control panel: two modes (position bar + jog) with toggle, fine/coarse increment buttons, typed input per axis |
| `@opendolly/shared` | Transition type needs to be added to the shared types |
| Board API | May need to communicate axis resolution/precision so the UI can enforce valid inputs |

---

## Source Notes

These notes were captured via voice transcription on 2026-03-21 and are preserved in their original form in `voice-notes/`:

| File | Primary Topics |
|------|---------------|
| `voice-notes/2026-03-21T081716-0500.md` | Motivation, professional UI survey, iFootage/edelkrone reference, critical reviews |
| `voice-notes/2026-03-21T082723-0500.md` | Functional requirements approach, keyframe vs timeline view, transition data model, curve editing (Premiere/Photoshop model), buffer concept, user stories |
| `voice-notes/2026-03-21T083025-0500.md` | Buffer feature detail — auto-generation, 1-second duration, physical room checks, warnings, settings |
| `voice-notes/2026-03-21T083725-0500.md` | Keyframe view UI — duration brackets, interactive vs static mode, control panel position, firmware config |
| `voice-notes/2026-03-21T084311-0500.md` | Movement speed (25% default), increment controls (1mm/10mm, 1°), precision constraints, typed input |
| `voice-notes/2026-03-21T085826-0500.md` | Speed settings as V1 feature, keyframe creation walkthrough, curve editor detail, timeline view detail, dragging keyframes to retime |
| `voice-notes/2026-03-21T090501-0500.md` | Responsive layout (portrait vs landscape split), dev server phone access, speed-based duration (partially deferred) |
