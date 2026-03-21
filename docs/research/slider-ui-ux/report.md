# Slider UI/UX Research Report

*2026-03-21*

Research into professional animation software, competing slider apps, and real-world cinematographer workflows to inform the OpenDolly V2 web UI design. Background sources in this directory: `background-slider-apps.md`, `background-curve-editors.md`, `background-workflows.md`.

## Executive Summary

No one has shipped a reliable, mobile-friendly motion control app with per-axis curve editing and timeline management. edelkrone nails simplicity but has no curves. iFootage attempted curves and timelines but shipped a 1.6/5-star app. Dragonframe has the best curve editor but is desktop-only and stop-motion-focused. The gap is clear and OpenDolly V2 targets it directly.

The research yields three core principles for V2:

1. **Simple by default, powerful on demand.** The basic 2-keyframe workflow must remain as fast as edelkrone's. Curve editing layers on top.
2. **Compositions, not axes.** Users think in framings ("camera here, then camera there"), not per-axis values. Per-axis editing is a power-user drill-down.
3. **The curve editor is a time remapper, not a value graph.** Users shape *how* motion progresses through time, not raw position values.

---

## 1. How People Actually Use Sliders

### The Universal Workflow

Every product (edelkrone, Rhino, Syrp, Cinetics, ZEAPON) converges on the same sequence:

1. Physically position camera at start framing (all axes)
2. Save as keyframe
3. Physically position camera at end framing
4. Save as keyframe
5. Set duration or speed
6. Preview, tweak, shoot

Users **never** think per-axis. They think: "I want the camera here at the start and here at the end." The system resolves that into per-axis motion. This matches OpenDolly V1's existing capture model perfectly.

### Most Moves Are Simple

The standard vocabulary is about 5 core move types: lateral slide, push-in/pull-out, parallax (slide + counter-pan), boom/vertical, and low mode. Nearly all are **2-keyframe sequences**. The creative variation comes from easing, speed, and multi-axis coordination -- not complex multi-keyframe paths.

3+ keyframe sequences exist (edelkrone sequencer, Syrp's 10-keyframe mode) but are used less frequently. When they are used, they're typically for timelapse or looping interview B-roll.

### Solo vs. Crew

- **Solo operators** use the slider as an unmanned "second camera" on loop while they work a handheld. They value quick setup, app-only control, and loop mode.
- **Crew workflows** involve someone actively iterating between takes, potentially using physical controllers for faster adjustment.

Both workflows demand that basic setup be fast. The timeline/curve editor is a crew or pre-planned workflow feature.

### Pain Points Across All Products

| Pain Point | Frequency | OpenDolly Response |
|------------|-----------|-------------------|
| Bluetooth connection drops | #1 complaint everywhere | Wi-Fi + WebSocket (already solved) |
| Motor noise kills audio | Very common | Hardware concern, not UI |
| Start/end jitter from inertia | Common | Buffer system (V2 feature) |
| Setup time for multi-axis | Common | Web UI auto-adapts to capabilities (already solved) |
| Battery ecosystem complexity | Common | Hardware concern |
| Per-axis graph editing too slow | iFootage-specific | All-axes-at-once default (see below) |

---

## 2. Competitive Landscape

### Feature Comparison

| Feature | edelkrone | iFootage Moco | Rhino Arc II | Dragonframe ARC | **OpenDolly V1** | **OpenDolly V2** |
|---------|-----------|---------------|--------------|-----------------|-----------------|-----------------|
| Max keyframes | 4-6 | 8 | 5 | Unlimited | Unlimited | Unlimited |
| Transition control | Speed + accel | Speed + time | Speed | Bezier curves | Preset easing | Per-axis curves |
| Per-axis curves | No | Yes (buggy) | No | Yes | No | Yes |
| Timeline view | No | Multi-track (buggy) | No | Yes | No | Yes |
| Duration control | No (speed only) | Yes | Yes | Yes | Yes | Yes |
| Loop mode | Yes | Yes | Unknown | N/A | No | Planned |
| Platform | iOS + Android | iOS only | iOS only | Desktop | Any browser | Any browser |
| DJI gimbal | No (own head) | Yes (RS2-4) | No | Via hardware | Yes (CAN) | Yes (CAN) |
| App reliability | ~3.5/5 | 1.6/5 | ~2.5/5 | N/A (desktop) | N/A | N/A |

### What to Borrow

1. **edelkrone's "absurdly easy" A-to-B flow.** Position, save, position, save, play. OpenDolly V1 has this; V2 must not lose it.

2. **edelkrone's auto-calculation pattern.** Change one parameter (FPS, interval, duration), the rest auto-adjust. Apply this anywhere interrelated parameters exist.

3. **edelkrone's Stable Start.** Wait for vibrations to settle before starting a move. Maps directly to the buffer system.

4. **Dragonframe's axis grouping.** Organize axes into logical groups for complex setups.

5. **Dragonframe's move test mode.** Run a preview at reduced speed to verify a move before committing to a real take.

6. **Syrp's templates.** Preset shot types (Clouds, People, Night Traffic) as starting points that users can customize.

### What to Avoid

1. **Per-axis graph editing as the default.** iFootage's biggest UX complaint: "Having to program each axis separately just takes too much time." Curve editing must show all axes simultaneously, with single-axis focus as a drill-down.

2. **Bluetooth connectivity.** The #1 complaint across edelkrone, iFootage, and Rhino. OpenDolly's Wi-Fi/WebSocket architecture sidesteps this entirely.

3. **Requiring manual positioning to capture keyframes.** iFootage's touchscreen requires physically pushing the slider carriage. Motor-assisted jog-then-capture is mandatory (OpenDolly V1 already does this).

4. **Features without reliability.** iFootage Moco has the right feature set and a 1.6/5 rating. Ship less, ship solid.

5. **Promised features that never arrive.** Rhino Arc II's timelapse mode: 3+ years of "coming very soon."

---

## 3. Curve Editor Design

### The Core Decision: What Kind of Curve?

Professional tools offer two mental models for curves:

| Model | Examples | What the user edits | Best for |
|-------|----------|-------------------|----------|
| **Value graph** | After Effects, Blender F-curves | Actual property values over time (e.g., pan = 45deg at t=3s) | Animation where precise value control matters |
| **Progress curve / transfer function** | Photoshop Curves, CSS cubic-bezier, easing presets | How progress maps through time (0% to 100%) | Timing/feel without touching position values |

**Recommendation: Progress curve (transfer function) model.**

Reasons:
- Users set position values via keyframe capture (jog to framing, save). They don't want to type "pan = 47.3deg at t=2.8s" into a graph.
- The vision doc describes "click on the curve to create control points, drag points to reshape" -- this is the Photoshop Curves interaction, which is a transfer function.
- Per-axis timing (delay, duration) is already implemented in motion-math and maps naturally to progress curves.
- The progress curve answers the question users actually ask: "How should the motion feel between these two keyframes?" Not "What exact value should the axis have at time T?"

Each transition between two keyframes gets one progress curve per axis. The curve maps normalized time [0,1] to normalized progress [0,1]. A diagonal line = linear. An S-curve = ease in/out. A flat-then-steep line = delay then fast move. The position values at 0% and 100% are always the keyframe values -- the curve only shapes *how* you get there.

### Interaction Model

Combine three proven patterns:

1. **Photoshop Curves interaction** -- click on curve to add control points, drag to reshape, drag off to delete, arrow keys to nudge.
2. **After Effects per-axis channels** -- each axis gets its own curve, color-coded, all visible simultaneously.
3. **Premiere inline display** -- curves displayed inline in stacked lanes within the timeline, not in a separate panel.

This hybrid is novel. Most tools force a choice between inline simplicity (Premiere) and separate-panel power (AE). OpenDolly can offer inline lanes that expand into full curve editors on tap/click.

### Math Model

**Use monotone Catmull-Rom interpolation for the progress curves.**

- **Interpolating** -- the curve passes through user-placed control points (matching the Photoshop Curves mental model where points are *on* the curve, not off to the side as bezier handles).
- **Monotone** -- guarantees the curve never overshoots between control points. Critical for physical hardware: a progress curve that temporarily goes backward would reverse motor direction unexpectedly.
- **Already implemented** -- motion-math has Catmull-Rom splines. Adding a monotonicity constraint is a small extension.
- **Convertible** -- Catmull-Rom and cubic bezier are mathematically interchangeable if needed for rendering.

For the V2 default, offer easing presets (the existing library of 7) with a "Custom" option that opens the point-based curve editor. Most users will pick a preset and never touch the curve editor directly.

### Interaction Conventions to Adopt

These are consistent across AE, Blender, C4D, Photoshop, and Resolve:

| Action | Desktop | Touch (phone) |
|--------|---------|---------------|
| Add point | Click on curve | Long-press on curve |
| Move point | Drag | Drag |
| Delete point | Drag off graph, or Delete key | Swipe off graph |
| Nudge point | Arrow keys (Shift+arrow for large) | N/A |
| Zoom time axis | Scroll wheel | Pinch horizontal |
| Zoom value axis | Shift+scroll | Pinch vertical |
| Pan view | Space+drag or middle-click drag | Two-finger drag |
| Undo/redo | Cmd+Z / Cmd+Shift+Z | Shake or UI button |
| Select multiple | Shift+click or marquee drag | N/A (mobile scope TBD) |

### Visual Language

- Each axis gets a distinct color (consistent across timeline, curve editor, and jog controls)
- Selected control points highlighted (white or yellow on dark background)
- Grid with auto-scaling density based on zoom level
- Subtle grid lines, prominent curve lines
- Vertical playhead line showing current time position, draggable for scrubbing

---

## 4. Timeline View Design

### Layout

The vision doc describes: "Full left-to-right timeline showing all keyframes and all axis channels stacked."

Based on the research, the recommended layout is:

```
┌──────────────────────────────────────────────────┐
│ Transport: [|◄] [►/❚❚] [►|] [⟲]    00:05.2/10.0 │
├──────────────────────────────────────────────────┤
│ ▼ Keyframes  [1]────────[2]────────[3]           │  ← keyframe markers, draggable
│              ├── 4.0s ──┤├── 6.0s ──┤            │  ← transition durations
├──────────────────────────────────────────────────┤
│ ▼ Slide  ╱‾‾‾‾‾‾‾‾‾╲_______________             │  ← progress curve inline
│ ▼ Pan    ________╱‾‾‾‾‾‾‾‾‾╲_______             │  ← delayed start
│ ▼ Tilt   ╱‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾             │  ← linear
│ ▼ Roll   ──────────────────────────              │  ← no change (flat)
├──────────────────────────────────────────────────┤
│ [▲ Playhead scrubber]                            │
└──────────────────────────────────────────────────┘
```

Key features:
- **Keyframe markers** are draggable left/right to retime transitions (redistributes duration between neighbors, as the vision doc specifies)
- **Axis lanes** show the progress curve inline. Tap/click a lane to expand it into a full curve editor
- **Collapsed lanes** for axes with no change (constant value) -- saves space
- **Transition duration** labels between keyframes, click to edit

### Desktop vs. Phone

- **Desktop (>= 640px):** Full timeline visible below the main control area. All axis lanes visible. Curve editor expands inline.
- **Phone (< 640px):** Timeline is a dedicated tab (replacing the current "Play" tab). Horizontal scroll for time. Only 2-3 axis lanes visible at once, vertically scrollable. Tap a lane to expand for curve editing.

The vision doc's open question -- "Does the timeline view make sense on a phone?" -- is answered: **yes, but simplified.** Show the keyframe track and transition durations. Per-axis curves are accessible but collapsed by default. The phone timeline is for retiming and preset easing; deep curve editing is a desktop activity.

### Keyframe Retiming

The vision doc describes: "Drag keyframes left/right to change timing. Dragging redistributes time between adjacent transitions."

This is the right interaction. Implementation detail: when a keyframe is dragged, the two adjacent transition durations adjust proportionally. Total sequence duration stays constant. If the user wants to change total duration, they edit the last transition's duration directly (or a global duration field).

---

## 5. Buffer System Design

### The Problem

Physical inertia causes jitter/rebound when the system arrives at or departs from a keyframe at speed. This is the #1 workflow pain point that users can't solve with better easing alone. The SuperReel guide warns: "Watch the start and ends carefully, as this is where the biggest judder occurs." edelkrone added "Stable Start" (vibration settling) in late 2024 specifically to address this.

### Recommended Approach

The vision doc proposes automatic buffer zones. Refined recommendation:

1. **Default: off.** Buffers add complexity. Most 2-keyframe moves with good easing don't need them.
2. **When enabled:** Add a configurable settle time (default 1 second) before and after each transition.
3. **Implementation:** During the settle period, the trajectory holds position (zero velocity). The easing curve handles deceleration into the hold and acceleration out of it.
4. **Clearance check:** Verify physical room exists for deceleration at rail endpoints. Warn if a buffer can't be generated.
5. **Video editing alignment:** Buffer duration should produce clean cut points. The 1-second default gives editors a clear in/out mark.

This is simpler than the vision doc's "estimate moment of inertia" approach. Real-world testing on hardware will determine whether velocity-based buffer sizing is needed or if a fixed settle time suffices.

---

## 6. Recommended V2 Feature Priorities

Based on the research, ordered by impact:

### Must Have (V2 launch)

1. **Per-transition easing with presets + custom curve editor.** The core V2 feature. Presets cover 90% of use. Custom curve editor for the 10% who need more.
2. **Timeline view with draggable keyframe retiming.** Visual overview of the full sequence with direct manipulation of timing.
3. **Per-axis timing (delay + duration).** Already implemented in motion-math. Expose it in the UI with the progress curve model.
4. **Loop mode.** Every competitor has it. Essential for interview B-roll and unmanned "second camera" workflows.
5. **Scrub preview.** Drag the playhead, board moves in real-time. Critical for verifying a move before committing to a take.

### Should Have (V2 or fast follow)

6. **Move test mode.** Run the trajectory at reduced speed for verification (borrowed from Dragonframe).
7. **Buffer system.** Settle time at keyframe boundaries. Important for multi-keyframe sequences at speed.
8. **Save/load sequences.** Persist named sequences for reuse across shoots.
9. **Undo/redo.** Expected in any editor.

### Nice to Have (V2+)

10. **Shot templates.** Preset sequences for common moves (parallax, push-in, tracking). Borrowed from Syrp.
11. **Target tracking.** Aim at subject from two positions, system calculates pan/tilt path. Borrowed from edelkrone. Requires inverse kinematics math.
12. **Speed graph overlay.** Verify that acceleration stays within hardware limits. Borrowed from AE/Blender.

---

## 7. Open Design Questions

These need answers before or during implementation:

1. **Curve editor on phone: how much is enough?** Research says deep curve editing is a desktop activity, but the phone needs *some* curve interaction beyond preset selection. Where's the line?

2. **Mixed interaction model.** What happens when the user jogs the system while keyframes exist? Options: (a) jog freely, keyframes are independent; (b) jog updates the nearest keyframe; (c) jog is disabled during sequence editing. The research suggests (a) -- edelkrone allows free movement between keypose recalls.

3. **Progress curve vs. value graph for power users.** The progress curve model is simpler and recommended as the default. But should a value graph mode exist for users who want to see/edit actual position values over time? This could be a V2+ feature.

4. **Transition-scoped vs. sequence-scoped curves.** Should each transition (keyframe A to B, B to C) have its own independent curve? Or should there be one continuous curve per axis across the entire sequence? The vision doc implies per-transition. Per-transition is simpler and matches the motion-math model. Sequence-scoped curves are more powerful but harder to edit.

5. **How to handle axes with no change.** If pan doesn't change between two keyframes, should the pan lane show anything? Options: (a) show a flat line; (b) collapse/hide the lane; (c) show it grayed out. Recommendation: (c) grayed out with option to collapse.
