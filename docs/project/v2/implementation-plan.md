# V2 Implementation Plan

Status: **active** — All phases implemented. Pending manual browser testing.

Build phases for the V2 web UI. Each phase produces a working app testable against the mock server. Informed by:
- [Functional Requirements](functional-requirements.md) (D1)
- [User Stories](user-stories.md) (D2)
- [Data Model Spec](data-model.md) (D3)
- [Wireframes](wireframes.md) (D4)

---

## Dependency Graph

```
Phase 0 → Phase 1 → Phase 2 → Phase 3 → Phase 4 ─┐
                                       → Phase 5 ─┤→ Phase 6
```

Phases 4 and 5 are parallelizable after Phase 3.

---

## Phase 0: Data Model Foundation — DONE

**Goal:** Define V2 types across packages. No runtime changes. All 49 existing motion-math tests pass.

### Changes

| File | Action | What |
|------|--------|------|
| `shared/src/transition.ts` | Create | `ProgressCurvePoint`, `AxisCurve`, `BufferConfig`, `Transition` types |
| `shared/src/index.ts` | Modify | Export new types |
| `motion-math/src/types.ts` | Modify | Add `ProgressCurve` type; widen `AxisConfig.easing` to `EasingConfig \| ProgressCurve` |

### Verification
- `npm test` in `motion-math/` — all 49 tests pass unchanged
- `npm run build` in `shared/` and `motion-math/` — TypeScript compiles

---

## Phase 1: Monotone Progress Curve Engine — DONE

**Goal:** Build the new curve math that everything depends on.

### Changes

| File | Action | What |
|------|--------|------|
| `motion-math/src/progress-curve.ts` | Create | `createProgressCurveFunction(points): (t) => number` — monotone PCHIP interpolation with Fritsch-Carlson tangent limiting |
| `motion-math/src/progress-curve.test.ts` | Create | Tests: linear, single point, multi-point, pass-through, monotonicity (1000 samples), boundaries, preset comparison |
| `motion-math/src/trajectory.ts` | Modify | Type guard `isProgressCurve()`; route to new function in easing builder (line ~96-99) |
| `motion-math/src/trajectory.test.ts` | Modify | Add tests: trajectory with `ProgressCurve` easing produces valid output |
| `motion-math/src/index.ts` | Modify | Export `createProgressCurveFunction`, `ProgressCurve`, `ProgressCurvePoint` |

### Key Algorithm: Monotone PCHIP

1. Build knot array: `[(0,0), ...interior points, (1,1)]`
2. Compute slopes between adjacent knots: `δk = (yk+1 - yk) / (tk+1 - tk)`
3. Compute initial tangents at knots: arithmetic mean of adjacent slopes (Catmull-Rom style)
4. **Fritsch-Carlson monotonicity fix:** For each interval, if `δk = 0`, set both endpoint tangents to 0. Otherwise, compute `αk = mk/δk` and `βk = mk+1/δk`. If `αk² + βk² > 9`, scale down tangents to ensure monotonicity.
5. Evaluate: standard cubic Hermite basis functions `h00, h10, h01, h11` per segment.

### Verification
- `npm test` in `motion-math/` — all existing + new tests pass
- Manual spot-check: generate a trajectory with a delay curve (point at (0.5, 0.0)) and verify the axis doesn't move for the first half

---

## Phase 2: Sequence Store — DONE

**Goal:** Replace flat keyframe store with unified sequence model. UI still looks like V1 externally.

### Changes

| File | Action | What |
|------|--------|------|
| `web/src/lib/stores/sequence.ts` | Create | `Sequence`, `SequenceKeyframe`, `SequenceTransition` types; store with add/remove/reorder/update methods; `selectedKeyframeId`; localStorage with V1 migration |
| `web/src/lib/utils/trajectory-builder.ts` | Create | `buildTrajectory(sequence, axisTypes)` — per-transition concatenation via `generateTrajectory()` |
| `web/src/lib/stores/keyframes.ts` | Delete | Replaced by sequence.ts |
| `web/src/components/KeyframeList.svelte` | Modify | Import from sequence store; pass transitions for bracket rendering (Phase 3 will add brackets) |
| `web/src/components/KeyframeCard.svelte` | Modify | Import from sequence store |
| `web/src/components/PlaybackControls.svelte` | Modify | Use trajectory builder; remove global duration/easing inputs |

### Trajectory Builder Design

Per the [data model spec](data-model.md) section 5:

```typescript
function buildTrajectory(sequence: Sequence, axisTypes: Record<string, AxisInterpolation>): TrajectoryPoint[] {
  const allPoints: TrajectoryPoint[] = [];
  let cumulativeTime = 0;

  for (let i = 0; i < sequence.transitions.length; i++) {
    const segment = generateTrajectory({
      keyframes: [
        { t: 0, positions: sequence.keyframes[i].positions },
        { t: sequence.transitions[i].duration_ms, positions: sequence.keyframes[i + 1].positions },
      ],
      axisTypes,
      axes: buildAxisConfigs(sequence.transitions[i]),
    });

    // Offset timestamps, skip first point of subsequent segments (boundary dedup)
    const startIdx = i === 0 ? 0 : 1;
    for (let j = startIdx; j < segment.points.length; j++) {
      allPoints.push({ ...segment.points[j], t: segment.points[j].t + cumulativeTime });
    }
    cumulativeTime += sequence.transitions[i].duration_ms;
  }
  return allPoints;
}
```

### PlaybackControls Changes

The trajectory generation logic moves from `PlaybackControls.svelte` inline code to `trajectory-builder.ts`. PlaybackControls calls `buildTrajectory()` and uploads the result. The duration/easing inputs are removed — these now live per-transition (visible after Phase 3 adds TransitionEditor).

**Temporary state:** Between Phase 2 (store change) and Phase 3 (TransitionEditor), there is no UI for setting transition durations. To keep the app functional during this gap, PlaybackControls should check that all transitions have `duration_ms > 0` before enabling the play button, and show "Set transition durations" as disabled text if any are unset. New keyframes captured during Phase 2 will need their transition durations set via Phase 3.

### Verification
- `npm run dev` — app loads, existing keyframes migrate from V1 localStorage
- Can still capture keyframes, clear sequence
- Build compiles without errors

---

## Phase 3: Control Panel + Transition Editing — DONE

**Goal:** Build the V2 UI surface. The app starts looking like V2.

### Changes

| File | Action | What |
|------|--------|------|
| `web/src/components/ControlPanel.svelte` | Create | Per-axis strips, Interactive/Static toggle, collapsible tray, capture button |
| `web/src/components/AxisStrip.svelte` | Create | Single axis: jog/position toggle, fine/coarse increments, typed input, value readout |
| `web/src/components/TransitionEditor.svelte` | Create | Simple view: duration input, derived speed indicators, speed-primary mode, "Advanced" button stub |
| `web/src/components/KeyframeList.svelte` | Modify | Render duration brackets between cards; clicking bracket opens TransitionEditor |
| `web/src/components/KeyframeCard.svelte` | Modify | Selection behavior (tap to select, highlight, deselect) |
| `web/src/components/PlaybackControls.svelte` | Modify | Strip to transport-only: play/pause/resume/stop/progress |
| `web/src/routes/+page.svelte` | Modify | New layout: portrait (keyframes + tray), landscape (split), desktop (+ timeline slot) |
| `web/src/components/TabBar.svelte` | Delete | Replaced by unified layout |
| `web/src/components/JogControl.svelte` | Keep | Not deleted yet — ControlPanel replaces its role but JogControl stays as fallback until Phase 6 cleanup |

### AxisStrip Component

Receives an `AxisCapability` prop. Contains:
- Mode toggle (Jog / Position)
- **Jog mode:** Bidirectional drag area. `ontouchmove`/`onmousemove` compute delta from center → `ws.sendJog({ [axis]: delta })`. Zero-velocity on release.
- **Position mode:** Range bar from `min` to `max`. Click sends `ws.sendMoveTo({ [axis]: value, duration_ms: computed })`.
- Increment buttons: values from `AxisCapability.unit` — "mm" → +1/+10, "deg" → +1/+15.
- Value readout: shows live position from `positionStore`. Tap to open typed input (number field). Snapping to resolution TBD (requires axis resolution in capabilities — for now, round to 0.1).

### Control Panel Layout

```
[Interactive ● | ○ Static]

[AxisStrip: Slide]
[AxisStrip: Pan]
[AxisStrip: Tilt]
[AxisStrip: Roll]  ← only if axis exists in capabilities

[★ Capture Keyframe]  ← hidden when a keyframe is selected
[Editing Keyframe N]  ← shown when selected

[▼ minimize]
```

### TransitionEditor Integration

When a duration bracket is tapped:
- On phone: TransitionEditor replaces the control panel area (slides up from bottom).
- On desktop: TransitionEditor expands inline between the keyframe cards.

Duration blocking: if `duration_ms === 0`, the TransitionEditor refuses to close. A visual indicator (red border, pulsing) signals that duration is required.

### Page Layout Breakpoints

| Breakpoint | Layout |
|-----------|--------|
| `<640px` | Portrait: keyframe list (scroll) + ControlPanel tray (bottom) |
| `640-1024px` | Landscape: ControlPanel (left 40%) + keyframe list (right 60%) |
| `>1024px` | Same as landscape + timeline slot below (Phase 5) |

### Verification
- Phone portrait: keyframes stack, control panel at bottom, brackets visible
- Phone landscape: split layout
- Capture keyframe works via new ControlPanel
- Duration entry on transitions works, speed indicators calculated correctly
- Keyframe selection: tap card → hardware moves, jog overwrites keyframe
- Playback works with per-transition durations

---

## Phase 4: Curve Editor — DONE

**Goal:** Interactive SVG-based per-axis progress curve editor.

### Changes

| File | Action | What |
|------|--------|------|
| `web/src/components/CurveEditor.svelte` | Create | Container: stacked `CurveLane` components, presets bar, linking checkboxes, Done button |
| `web/src/components/CurveLane.svelte` | Create | SVG lane: grid, curve path, draggable control points |
| `web/src/components/TransitionEditor.svelte` | Modify | "Advanced" button wires to CurveEditor; speed-edit warning for custom curves |

### CurveLane Rendering

1. Read `AxisCurve.points` from the transition in the sequence store.
2. Call `createProgressCurveFunction(points)` to get the math function.
3. Sample at 200 evenly-spaced `t` values → build SVG `<path d="...">`.
4. Render grid lines (0.25 increments on both axes).
5. Render control points as SVG circles with drag handlers.

### Point Interaction

- **Add:** `onclick` on the SVG area finds the nearest `t` on the curve, inserts a point at `(t, currentCurveValue(t))`. Then the user can drag it.
- **Move:** `onpointerdown` on a point starts drag. `onpointermove` updates the point's `(t, progress)` with constraints:
  - `t` clamped between neighboring points' `t` values (or 0/1 for edge points).
  - `progress` clamped to [0, 1].
- **Delete:** If a dragged point exits the SVG bounds, it is removed.
- **Touch:** Same pointer events work for touch. Long-press (300ms) on the curve line to add a point (prevents accidental adds while scrolling).

### Store Updates

Every point manipulation dispatches: `sequenceStore.updateTransition(transitionId, { curves: { [axis]: { points: newPoints } } })`.

### Multi-Axis Linking

State: `let linkedAxes = $state(new Set<string>())`. When a point is added/moved/deleted on a linked axis, the same operation is applied to all other linked axes.

### Easing Presets

A preset bar at the top offers: Linear, Ease In, Ease Out, Ease In-Out. Clicking a preset populates the selected lane(s) with 1-3 control points that approximate the shape:
- Linear: clear all points (empty array)
- Ease In: `[{ t: 0.4, progress: 0.1 }]`
- Ease Out: `[{ t: 0.6, progress: 0.9 }]`
- Ease In-Out: `[{ t: 0.3, progress: 0.05 }, { t: 0.7, progress: 0.95 }]`

These are starting approximations — the user can adjust them.

### Verification
- Add/move/delete points on curve lanes
- Curves affect trajectory (play back, verify motion shape)
- Multi-axis linking works
- Presets populate correct shapes
- Touch works on phone
- "Done" returns to control panel

---

## Phase 5: Timeline View — DONE

**Goal:** Horizontal multi-channel timeline for desktop/tablet with scrub.

### Changes

| File | Action | What |
|------|--------|------|
| `web/src/components/TimelineView.svelte` | Create | Container: keyframe track, axis lanes, playhead, time scale |
| `web/src/components/Playhead.svelte` | Create | Draggable vertical line with scrub integration |
| `web/src/routes/+page.svelte` | Modify | Add timeline slot for desktop; view toggle |

### Keyframe Track

Horizontal bar spanning `totalDuration` (sum of all transition durations). Keyframe markers positioned at cumulative time offsets. Each marker:
- Shows keyframe label
- Is draggable left/right
- On drag: compute new time → redistribute duration between `transitions[i-1]` and `transitions[i]` (markers at the ends don't drag)
- Snap to prevent any transition from going below a minimum duration (e.g., 100ms)

### Axis Lanes

One lane per axis. Each lane is divided into segments matching transitions. Each segment renders a mini `CurveLane` in compact/read-only mode (no interaction). Tapping a segment expands it to an editable `CurveLane`.

### Scrub

The `Playhead` component:
1. Is draggable along the time axis.
2. On drag, computes the time in ms.
3. Sends `ws.sendScrub(timeMs)` via the WebSocket, throttled to 20Hz.
4. Requires a trajectory to be uploaded. The timeline maintains a reactive auto-upload:
   - Watches the sequence store for changes (Svelte `$effect`)
   - Debounces by 500ms
   - Calls `buildTrajectory()` → `boardClient.uploadTrajectory()`
   - Stores the trajectory ID for scrub reference
   - Shows "Syncing..." indicator during debounce/upload

### Layout

- Desktop (>1024px): timeline appears below the keyframe view + control panel, always visible.
- Tablet (640-1024px): toggle between keyframe view and timeline view.
- Phone (<640px): simplified timeline accessible via a view switch — keyframe track + playhead + collapsed lanes.

### Verification
- Keyframe markers at correct positions
- Drag-to-retime works, durations redistribute correctly
- Scrub playhead moves hardware in real-time
- Auto-upload keeps trajectory in sync
- Inline curve editing from timeline lanes

---

## Phase 6: Buffers + Polish — DONE

**Goal:** Buffer system, movement speed config, responsive polish, V1 component cleanup.

### Changes

| File | Action | What |
|------|--------|------|
| `web/src/lib/utils/trajectory-builder.ts` | Modify | Insert hold periods (pre/post buffers) around motion segments |
| `web/src/components/TransitionEditor.svelte` | Modify | Per-transition buffer toggle + duration config |
| `web/src/routes/settings/+page.svelte` | Modify | Global buffer settings, per-axis movement speed |
| `web/src/components/JogControl.svelte` | Delete | Replaced by ControlPanel |
| `web/src/components/AxisControl.svelte` | Delete | Replaced by AxisStrip |

### Buffer Implementation in Trajectory Builder

```typescript
for (let i = 0; i < sequence.transitions.length; i++) {
  const transition = sequence.transitions[i];
  const bufferConfig = resolveBufferConfig(transition, globalBufferSettings);

  if (bufferConfig?.enabled) {
    // Pre-buffer: hold at start position
    const prePoints = generateHold(sequence.keyframes[i].positions, bufferConfig.pre_ms, cumulativeTime);
    allPoints.push(...prePoints);
    cumulativeTime += bufferConfig.pre_ms;
  }

  // Motion segment (same as Phase 2)
  const segment = generateMotionSegment(sequence, i, axisTypes);
  allPoints.push(...offsetAndMerge(segment, cumulativeTime));
  cumulativeTime += transition.duration_ms;

  if (bufferConfig?.enabled) {
    // Post-buffer: hold at end position
    const postPoints = generateHold(sequence.keyframes[i + 1].positions, bufferConfig.post_ms, cumulativeTime);
    allPoints.push(...postPoints);
    cumulativeTime += bufferConfig.post_ms;
  }
}
```

**Clearance check:** Before buffer generation, for each axis: `remainingRoom = axisMax - keyframePosition` (or `keyframePosition - axisMin` depending on direction). If room < threshold, emit a warning. Threshold TBD — for now, just check that the axis isn't at its limit.

### Movement Speed Settings

New UI in settings page: per-axis slider (0-100% of max speed). Stored in localStorage as `opendolly-settings-v2`:

```json
{
  "movementSpeed": { "slide": 25, "pan": 25, "tilt": 25, "roll": 25 },
  "buffers": { "enabled": false, "default_pre_ms": 1000, "default_post_ms": 1000, "stiffness": 1.0 }
}
```

ControlPanel reads these settings to compute `duration_ms` for `move_to` commands: `duration = distance / (maxSpeed * speedPercent / 100)`.

### Cleanup

Remove deprecated V1 components: `TabBar.svelte`, `JogControl.svelte`, `AxisControl.svelte`. Grep for imports and remove.

### Verification
- Buffers produce correct hold periods in trajectory
- Clearance warnings appear when axis is near limit
- Movement speed setting affects jog/move_to responsiveness
- Build size check: `npm run build` in `web/`, target <500KB
- Full end-to-end: capture keyframes → set durations → edit curves → enable buffers → play → scrub on timeline
- Phone test: portrait + landscape layouts, touch interactions, one-handed operation

---

## Test Strategy Summary

| Phase | Tests |
|-------|-------|
| 0 | Existing 49 motion-math tests pass; TypeScript compiles |
| 1 | New progress-curve unit tests (~15); trajectory integration tests (~5); existing tests pass |
| 2 | Sequence store unit tests (add/remove/reorder/migrate); trajectory builder boundary tests |
| 3 | Manual: layout breakpoints, duration entry, speed indicators, keyframe selection |
| 4 | Manual: point add/move/delete, linking, presets, touch on phone |
| 5 | Manual: timeline layout, drag-to-retime, scrub, auto-upload |
| 6 | Manual: buffers in trajectory, clearance warnings, build size |

Automated tests live in `motion-math/` (Vitest). UI testing is manual against the mock server.

---

## Critical Files Reference

| Purpose | File |
|---------|------|
| Existing spline reference | `motion-math/src/catmull-rom.ts` |
| Trajectory generator to extend | `motion-math/src/trajectory.ts` |
| Type definitions to widen | `motion-math/src/types.ts` |
| Keyframe store to replace | `web/src/lib/stores/keyframes.ts` |
| Main page layout to restructure | `web/src/routes/+page.svelte` |
| Playback logic to refactor | `web/src/components/PlaybackControls.svelte` |
| WebSocket (scrub already implemented) | `web/src/lib/api/websocket.ts` |
| Shared types barrel export | `shared/src/index.ts` |
| Vision doc (feature source) | `docs/project/vision/v2-ui-vision.md` |
| UX research (design rationale) | `docs/research/slider-ui-ux/report.md` |
