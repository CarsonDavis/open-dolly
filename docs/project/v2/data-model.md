# V2 Data Model Spec

Status: **active**

Defines the V2 data structures — keyframes, transitions, progress curves, buffers — and how they relate to existing types across all packages.

---

## 1. Core Data Structures

### Sequence

The top-level data structure for the V2 UI. Replaces the V1 flat `Keyframe[]` array.

```typescript
interface Sequence {
  keyframes: SequenceKeyframe[];
  transitions: SequenceTransition[];
}
```

**Invariant:** `transitions.length === max(0, keyframes.length - 1)`. `transitions[i]` connects `keyframes[i]` to `keyframes[i+1]`.

### SequenceKeyframe

A position snapshot. Stored in the UI only — the board never sees this type.

```typescript
interface SequenceKeyframe {
  id: string;               // crypto.randomUUID()
  label: string;             // user-editable, default "Keyframe N"
  positions: Record<string, number>;  // axis name → value (e.g., { pan: 45, slide: 500 })
  createdAt: number;         // Date.now() at capture
}
```

**Relationship to `motion-math` `Keyframe` type:** The motion-math `Keyframe` has `{ t: number; positions: Record<string, number> }`. The `t` field is computed at trajectory generation time from transition durations — it is not stored in `SequenceKeyframe`.

### SequenceTransition

The timing and curve definition for motion between two adjacent keyframes.

```typescript
interface SequenceTransition {
  id: string;                // crypto.randomUUID()
  duration_ms: number;       // must be > 0 before playback; 0 = unset (blocking)
  curves: Record<string, AxisCurve>;  // axis name → curve; missing axes default to linear
  buffer?: BufferConfig;     // per-transition buffer override (see section 4)
}
```

### AxisCurve

Defines the progress curve for a single axis within a transition.

```typescript
interface AxisCurve {
  points: ProgressCurvePoint[];  // interior control points; empty = linear
}
```

### ProgressCurvePoint

A single control point on a progress curve. Both values are in normalized [0, 1] space.

```typescript
interface ProgressCurvePoint {
  t: number;         // normalized time within the transition [0, 1]
  progress: number;  // normalized progress toward the target position [0, 1]
}
```

**Semantics:** The curve maps time to progress. At `t = 0`, progress is implicitly `0` (at the starting keyframe). At `t = 1`, progress is implicitly `1` (at the ending keyframe). Interior points define the shape of the motion between these endpoints. A linear curve (empty points array) means constant-speed motion.

**Constraints:**
- Points must be sorted by `t`, ascending.
- `0 < t < 1` for all interior points (endpoints are implicit).
- `0 ≤ progress ≤ 1` for all points.
- No two points may share the same `t` value.

---

## 2. Package Changes

### `@opendolly/shared`

**New file:** `shared/src/transition.ts`

Exports: `ProgressCurvePoint`, `AxisCurve`, `Transition`, `BufferConfig`

```typescript
// shared/src/transition.ts

export interface ProgressCurvePoint {
  t: number;
  progress: number;
}

export interface AxisCurve {
  points: ProgressCurvePoint[];
}

export interface BufferConfig {
  enabled: boolean;
  pre_ms: number;    // hold time before motion starts (default 1000)
  post_ms: number;   // hold time after motion ends (default 1000)
}

export interface Transition {
  id: string;
  duration_ms: number;
  curves: Record<string, AxisCurve>;
  buffer?: BufferConfig;
}
```

**Note:** The `Transition` type in `shared` is the wire-compatible/serializable form. The UI store uses `SequenceTransition` which is structurally identical but defined locally in the web package for store ergonomics. This avoids the web package needing to import from shared just for store internals, while keeping the shared type available for future use by other consumers (e.g., a future save/load API).

**Updated file:** `shared/src/index.ts` — add exports for the new types.

**No changes to:** `capabilities.ts`, `state.ts`, `trajectory.ts`, `settings.ts`, `errors.ts`, `websocket.ts`. The Board API is unchanged — the board receives pre-computed trajectory point tables, not curve definitions.

**Note on axis resolution:** The vision doc and FR-CP-7/8 reference per-axis resolution for input snapping, but `AxisCapability` currently only has `min`, `max`, `unit` — no resolution field. For V2, all axis values round to 0.1 (via `trajectory.ts` rounding and `AxisStrip` input `step="0.1"`). Adding a true `resolution` field to `AxisCapability` is deferred to a Board API update.

**Note on type duplication:** `ProgressCurvePoint` is defined in three places: `shared/src/transition.ts`, `motion-math/src/types.ts`, and `web/src/lib/stores/sequence.ts`. They are structurally identical. The `shared` package defines the canonical version; `motion-math` defines its own for package independence (no dependency on `shared`); the web store re-defines locally for ergonomics. If the shape ever changes, all three must be updated.

### `@opendolly/motion-math`

**Updated file:** `motion-math/src/types.ts`

```typescript
// New type
export interface ProgressCurvePoint {
  t: number;
  progress: number;
}

export interface ProgressCurve {
  points: ProgressCurvePoint[];
}

// Updated type (widened easing field)
export interface AxisConfig {
  delay_ms: number;
  duration_ms: number;
  easing: EasingConfig | ProgressCurve;  // was: EasingConfig
}
```

**Type guard for detection:**
```typescript
function isProgressCurve(easing: EasingConfig | ProgressCurve): easing is ProgressCurve {
  return typeof easing === 'object' && 'points' in easing;
}
```

This distinguishes `ProgressCurve` (has `points` array) from `CubicBezierParams` (has `x1`, `y1`, `x2`, `y2`) and `EasingPreset` (string).

**New file:** `motion-math/src/progress-curve.ts`
- `createProgressCurveFunction(points: ProgressCurvePoint[]): (t: number) => number`
- Implements monotone piecewise cubic Hermite interpolation (PCHIP)
- See implementation plan Phase 1 for details

**Updated file:** `motion-math/src/trajectory.ts`
- Easing function builder detects `ProgressCurve` via type guard, routes to `createProgressCurveFunction`
- Backward-compatible: existing `EasingPreset` and `CubicBezierParams` paths untouched

**Backward compatibility:** The widened `easing` type is additive. All existing code that passes `EasingPreset` or `CubicBezierParams` compiles without changes. The type guard routes new `ProgressCurve` values to the new code path.

---

## 3. localStorage Schema

### V2 Format

Key: `opendolly-sequence`

```json
{
  "version": 2,
  "keyframes": [
    {
      "id": "uuid-1",
      "label": "Start",
      "positions": { "pan": 0, "tilt": 0, "slide": 0 },
      "createdAt": 1711000000000
    },
    {
      "id": "uuid-2",
      "label": "End",
      "positions": { "pan": 45, "tilt": -10, "slide": 500 },
      "createdAt": 1711000001000
    }
  ],
  "transitions": [
    {
      "id": "uuid-t1",
      "duration_ms": 10000,
      "curves": {
        "pan": { "points": [{ "t": 0.5, "progress": 0.2 }] },
        "slide": { "points": [] }
      }
    }
  ]
}
```

### V1 Format (legacy, migrate on load)

Key: `opendolly-keyframes`

```json
[
  {
    "id": "uuid-1",
    "label": "Keyframe 1",
    "positions": { "pan": 0, "tilt": 0, "slide": 0 },
    "createdAt": 1711000000000
  }
]
```

### Migration Logic

On store initialization:
1. Check for `opendolly-sequence` (V2 format). If found and valid, use it.
2. If not found, check for `opendolly-keyframes` (V1 format). If found:
   - Convert to V2 `Sequence`: keyframes carry over as-is, transitions are created with `duration_ms: 0` (unset) and empty curves (linear).
   - Write the V2 format to `opendolly-sequence`.
   - Remove `opendolly-keyframes`.
3. If neither found, start with an empty sequence.

---

## 4. Buffer Data Model

```typescript
interface BufferConfig {
  enabled: boolean;
  pre_ms: number;   // default 1000
  post_ms: number;  // default 1000
}
```

Buffers are stored per-transition in `SequenceTransition.buffer`. A global default is stored in settings (not in the sequence) as:

```typescript
// In the settings store (UI-side, not Board API settings)
interface BufferSettings {
  enabled: boolean;          // global toggle, default false
  default_pre_ms: number;    // default 1000
  default_post_ms: number;   // default 1000
  stiffness: number;         // TBD pending hardware testing
}
```

**Resolution order:** Per-transition `buffer` overrides global `BufferSettings`. If a transition has no `buffer` field, the global setting applies.

---

## 5. Contracts Between Components

### Sequence Store → Trajectory Builder

The sequence store provides the complete `Sequence` (keyframes + transitions). The trajectory builder reads this and produces a `TrajectoryPoint[]` ready for upload.

```
sequenceStore.get() → Sequence
  ↓
trajectoryBuilder.build(sequence, axisTypes) → TrajectoryPoint[]
  ↓
boardClient.uploadTrajectory({ points, loop: false }) → { trajectory_id }
```

### Trajectory Builder → Motion-Math

The trajectory builder calls `generateTrajectory()` once per transition:

```typescript
for (let i = 0; i < sequence.transitions.length; i++) {
  const kfA = sequence.keyframes[i];
  const kfB = sequence.keyframes[i + 1];
  const transition = sequence.transitions[i];

  const segment = generateTrajectory({
    keyframes: [
      { t: 0, positions: kfA.positions },
      { t: transition.duration_ms, positions: kfB.positions },
    ],
    axisTypes,
    axes: buildAxisConfigs(transition),  // converts AxisCurve → ProgressCurve easing
    sample_interval_ms: 10,
    loop: false,
  });

  // Concatenate: offset timestamps, deduplicate boundary point
  allPoints.push(...offsetAndMerge(segment.points, cumulativeTime));
  cumulativeTime += transition.duration_ms;
}
```

**`buildAxisConfigs` mapping:**

```typescript
function buildAxisConfigs(transition: SequenceTransition): Record<string, Partial<AxisConfig>> {
  const configs: Record<string, Partial<AxisConfig>> = {};
  for (const [axis, curve] of Object.entries(transition.curves)) {
    if (curve.points.length > 0) {
      configs[axis] = { easing: { points: curve.points } };  // ProgressCurve
    }
    // else: omit → defaults to linear in generateTrajectory
  }
  return configs;
}
```

### Buffer Integration in Trajectory Builder

When buffers are enabled for a transition:

1. **Pre-buffer:** Before the motion segment, insert `pre_ms` milliseconds of points holding the starting keyframe position.
2. **Motion segment:** The transition's trajectory as computed above.
3. **Post-buffer:** After the motion segment, insert `post_ms` milliseconds of points holding the ending keyframe position.

The hold periods are simple — just repeated trajectory points at the same position sampled at 10ms intervals.

**Boundary handling:** If the first transition has a pre-buffer, the sequence starts with a hold. If the last transition has a post-buffer, the sequence ends with a hold. Buffers between consecutive transitions overlap: transition N's post-buffer and transition N+1's pre-buffer both hold at the shared keyframe position, so they merge into a single hold of `max(post_ms, pre_ms)` duration.

### Curve Editor → Sequence Store

The curve editor reads and writes `AxisCurve.points` for a specific transition via:

```
sequenceStore.updateTransition(transitionId, {
  curves: { [axisName]: { points: newPoints } }
})
```

### Timeline View → Sequence Store

Keyframe retiming updates transition durations:

```
// Dragging keyframe marker between transitions i and i+1
sequenceStore.updateTransition(transitions[i].id, { duration_ms: newLeftDuration });
sequenceStore.updateTransition(transitions[i+1].id, { duration_ms: newRightDuration });
```

### Timeline Scrub → Board

```
trajectoryBuilder.build(sequence, axisTypes)
  → boardClient.uploadTrajectory(points)
  → ws.sendScrub(timeMs)   // throttled to 20Hz
```

---

## 6. Relationship to Board API

The Board API is **unchanged** for V2. The board never sees curves, transitions, or buffer definitions. It receives:

- `POST /api/trajectory` — a dense `TrajectoryPoint[]` table (same format as V1)
- `ws: play` — start playback
- `ws: scrub` — jump to timestamp (already implemented)

All V2 complexity lives in the browser. The trajectory builder pre-computes everything into the same flat point table the board already understands.
