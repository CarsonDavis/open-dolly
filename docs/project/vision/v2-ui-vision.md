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

These should be completed before writing functional requirements or wireframes.

- [ ] **R1: Professional slider UI survey.** Study professional-grade camera slider and motion control software UIs. Focus on products where real thought has gone into UX — not hobby projects. Key targets:
  - iFootage / edelkrone slider app (closest to what we're building)
  - Dragonframe (stop motion, but excellent keyframe/timeline UI)
  - edelkrone HTTP SDK documentation
  - Any other commercial slider apps with notable UX
  - Deliverable: summary of each product's UI model, what it does well, screenshots/screen recordings where possible

- [ ] **R2: Critical reviews and missing features.** Search for reviews, forum posts, Reddit threads, and YouTube comments where users of slider software (especially iFootage/edelkrone) describe features they wish existed or pain points with the current UI. We want to know what the market wants but doesn't have.
  - Deliverable: categorized list of user complaints and feature requests with sources

- [ ] **R3: Slider workflow tutorials.** Find tutorials (YouTube, blog posts) where experienced cinematographers explain how they actually use camera sliders — how they plan moves, how they set up keyframes, what their workflow looks like from concept to final shot. We need to understand real usage patterns so we design for how people actually work, not how we imagine they work.
  - Deliverable: summary of common workflows, pain points, and tricks users employ

- [ ] **R4: Animation/curve editor UI patterns.** Study how professional creative software handles curve editing and timeline interaction. Specific references called out in the notes:
  - **Premiere Pro** — volume keyframes on a clip (click a line, drag to create a curve, the curve maps to an equation)
  - **Photoshop** — Curves adjustment dialog (click to add control points, drag to reshape)
  - **After Effects** — keyframe graph editor (value graph and speed graph)
  - CSS `cubic-bezier()` editors (e.g. cubic-bezier.com)
  - Deliverable: interaction model comparison — how each tool handles adding points, reshaping curves, constraining edits, and previewing results

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
- Clicking a keyframe selects it — the control panel updates to show that keyframe's values and the physical system moves to that position (in interactive mode)

**Duration brackets:**
- Between each pair of adjacent keyframes, a bracket (thin line with a number in the center) shows the transition duration in seconds
- The bracket joins the two keyframe cards visually
- Clicking the duration number opens the curve editor for that transition (see Curve Editor section below)

**Key difference from V1:** In V1, keyframes are simple position snapshots with a single global easing preset. In V2, every pair of adjacent keyframes has a rich transition definition with per-axis curves, timing, and optional buffers.

### Timeline View

A horizontal, multi-channel timeline showing the full sequence at once. This is primarily a desktop feature — it may not translate well to phone screens (open question).

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

**Simultaneous preview:**
- This is a v3 feature, not v2
- The timeline is at the bottom of the screen
- A live preview of the animation can play at the top — so you can see the actual motion while looking at the timeline
- this is an open question...what constitutes a preview
- i think that maybe we can create a vector representation of a gimbal with the axises and show it move across the screen

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

**Static editing mode:** Edit keyframe values theoretically without moving the hardware. Useful when you're not connected to a board or when you want to plan a sequence without executing it. The control panel values are the source of truth (not sensor readings).

### Movement Speed

When you change a value in the control panel (interactive mode), the system moves the hardware to the new position. The speed at which it moves is configurable:

- **Default:** 25% of the maximum speed of each axis
- **Configurable per-axis** in user preferences/settings — you can set the slider to move at 100% max speed while the gimbal moves at 25%
- The speed setting is for **manual repositioning only** — it has no relationship to the speed used during keyframe playback
- The board reports min and max speeds per axis, so the UI knows what range is valid
- V1 scope: just the per-axis percentage setting in preferences. V2 could add inline speed controls during positioning (e.g., a clickable speed readout in mm/s or °/s)

### Increment Controls

The UI needs to support both coarse positioning (get roughly in the right area) and fine positioning (nail the exact framing). The approach differs by axis type:

**Linear axes (slider):**
- Two increment buttons: +1 mm (fine) and +10 mm (coarse) in each direction
- Between these two increments you can reach any position efficiently
- For ultra-precise work: type in an exact value (e.g., 1.356 mm)

**Rotational axes (pan, tilt, roll):**
- Increment button: +1° in each direction
- For precision: type in an exact value (e.g., 1.53°)

**Precision constraints:**
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
3. At this point the system doesn't know how long the transition should take. A flashing/highlighted box appears between the two keyframes prompting the user to enter a duration in seconds.
4. The user types in the duration (e.g., "10"). The precision of the input is bounded by system capabilities.
5. By default, the transition is linear on all axes — constant speed from position A to position B.

### Editing Transition Curves

1. The user clicks the duration bracket between two keyframes.
2. The control panel area transforms: instead of position controls, it now shows **per-axis curve editors** — one mini-timeline per axis.
3. Each mini-timeline spans the duration of the transition (e.g., 10 seconds, shown at the bottom).
4. Initially each shows a straight line from bottom-left to top-right (linear interpolation).
5. The user can:
   - **Click on the curve** to add a control point
   - **Drag a control point** to reshape the curve
   - **Drag from the left edge** to delay when that axis starts moving (the line stays flat at the bottom, then ramps up)
   - **Drag from the middle** to change the shape of the curve (ease in, ease out, S-curve)
   - Add multiple control points to create complex curve shapes
6. The curve editing model is **Photoshop Curves style** — not preset-based, but direct point manipulation.
7. Each curve is backed by a mathematical equation. The points the user creates define that equation. This is the same concept as Premiere's volume keyframe line — the visual line you drag corresponds to an underlying function.
8. A "Done" / "Back" button exits curve editing and returns to the position control panel.

**Example — delayed pan:**
- 10-second move, 4 axes
- User wants the pan to not start until 5 seconds in
- Goes to the pan channel in the curve editor
- Clicks a point at the 5-second mark on the flat bottom of the line
- The line stays flat (no movement) from 0s to 5s, then curves up from 5s to 10s
- All other axes move for the full 10 seconds as normal

**Example — ease into a stop:**
- User wants the slide to decelerate smoothly into the final position
- Drags the right end of the slide curve down so it flattens out as it approaches the end
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

When the system arrives at a keyframe at speed, physical inertia causes the rig to flex and rebound. The camera jitters at the stop point. This is especially noticeable on fast moves or with heavy payloads.

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
2. When a keyframe transition is generated, the system automatically adds extra travel before and/or after the keyframe
3. The buffer is calculated based on:
   - The speed of the move (how fast each axis is traveling as it approaches the keyframe)
   - An estimate of the system's inertia / stiffness (could be a user-configured value or derived from hardware specs)
   - The physical room available (how far the axis is from its limits)

**Buffer rules:**
- Buffers are exactly **1 second** in duration on each side — this makes it trivial to trim in editing software (just cut the first and last second of the clip)
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

---

## Responsive Layout

The UI must work on both phones and desktop browsers, with the same codebase adapting its layout.

### Portrait Phone (Primary Mobile Use Case)

```
┌─────────────────────────┐
│                         │
│    Keyframe cards        │
│    (vertical stack,     │
│     scrollable)         │
│                         │
│    [KF 1]               │
│    ── 5.0s ──           │
│    [KF 2]               │
│    ── 10.0s ──          │
│    [KF 3]               │
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
│  capture     │   [KF1] [KF2]   │
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

### Considered and Partially Deferred

The idea: instead of typing "this transition takes 10 seconds," the user could say "move the slider at 50 mm/s" and the system calculates the duration based on the distance.

**Why it's complicated:** A transition involves multiple axes. If you're changing the slider position and the pan angle, what does "50 mm/s" mean for the pan axis? It only makes sense for the axis that's actually measured in mm. If only the pan changes between two keyframes and the slider doesn't move at all, a mm/s input is meaningless.

**Partial approach:** Allow speed-based input only when:
- The axis in question actually has a delta between the two keyframes
- The axis type matches the unit (mm/s for linear, °/s for rotation)

**Current decision:** Write this down as explicitly not supported in V1. Revisit for V2 with the constraint that it must be smart enough to only offer speed-based input on axes where it makes sense.

---

## Open Design Questions

These need to be resolved during the research and requirements phases before implementation.

1. **Curve math model:** The current motion-math library uses single cubic-bezier easing functions. The curve editor envisions multi-point curves (like Photoshop curves). What's the right mathematical model? Multi-segment bezier? Cubic spline with user-defined control points? How does this map to the trajectory generator?

2. **Controls placement — shared vs. inline:** When editing a keyframe's position, should there be a single shared control panel at the bottom that updates all keyframes (click keyframe → controls update), or should each keyframe card have its own inline controls? Both have tradeoffs. The shared approach saves screen space; inline gives context. Could prototype both.

3. **Timeline view on phone:** Does the timeline view make sense on a phone screen at all, or is it strictly desktop? The horizontal space on a phone in portrait mode is very limited. Landscape might work but competes with the split-screen keyframe layout.

4. **Buffer calculation inputs:** How does the system estimate the inertia/stiffness needed for buffer calculations? Options: (a) user enters a stiffness parameter manually, (b) system derives it from hardware config (payload weight, motor torque), (c) system runs a calibration move and measures settling time. The first is simplest; the last is most accurate.

5. **Curve editor point limits:** The notes say "you can't add an infinite number of points but you can add a whole bunch." What's the practical limit? This depends on the math model. A 10-point bezier might be overkill; a 4-point one might be too limiting.

6. **Live preview during curve editing:** When editing transition curves, should the system play a preview of the motion in real time? This would mean the board executes a short trajectory segment repeatedly while you tweak. Very powerful UX but adds complexity.

7. **Simultaneous timeline + preview:** The notes describe "timeline at the bottom, animation at the top." What does the animation preview look like? A 3D visualization? A schematic overhead view of the slider? Just live video from the camera? Or just the physical hardware moving?

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
| `JogControl` | May evolve into the V2 control panel with increment buttons (fine/coarse) and typed input |
| `@opendolly/shared` | Transition type needs to be added to the shared types |
| Board API | May need to communicate axis resolution/precision so the UI can enforce valid inputs |

---

## Source Notes

These notes were captured via voice transcription on 2026-03-21 and are preserved in their original form:

| File | Primary Topics |
|------|---------------|
| `2026-03-21T081716-0500.md` | Motivation, professional UI survey, iFootage/edelkrone reference, critical reviews |
| `2026-03-21T082723-0500.md` | Functional requirements approach, keyframe vs timeline view, transition data model, curve editing (Premiere/Photoshop model), buffer concept, user stories |
| `2026-03-21T083025-0500.md` | Buffer feature detail — auto-generation, 1-second duration, physical room checks, warnings, settings |
| `2026-03-21T083725-0500.md` | Keyframe view UI — duration brackets, interactive vs static mode, control panel position, firmware config |
| `2026-03-21T084311-0500.md` | Movement speed (25% default), increment controls (1mm/10mm, 1°), precision constraints, typed input |
| `2026-03-21T085826-0500.md` | Speed settings as V1 feature, keyframe creation walkthrough, curve editor detail, timeline view detail, dragging keyframes to retime |
| `2026-03-21T090501-0500.md` | Responsive layout (portrait vs landscape split), dev server phone access, speed-based duration (partially deferred) |
