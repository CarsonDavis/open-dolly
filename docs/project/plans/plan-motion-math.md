# Motion Math Library — Implementation Plan

*Status: active*
*Last updated: 2026-03-21*

This document specifies a pure TypeScript motion math library that runs in the browser. It takes user-defined keyframes and timing configuration, and produces dense trajectory point tables for upload to the board via `POST /api/trajectory`.

No external dependencies. All math is implemented from scratch.

---

## Module Structure

```
motion-math/
  types.ts          — shared types
  catmull-rom.ts    — centripetal Catmull-Rom spline (linear axes)
  squad.ts          — SQUAD quaternion interpolation (rotation axes)
  easing.ts         — easing function engine (cubic bezier)
  trajectory.ts     — trajectory table generator (composes all of the above)
```

---

## Data Types (`types.ts`)

```typescript
/** Quaternion in w, x, y, z order (Hamilton convention). */
interface Quaternion {
  w: number;
  x: number;
  y: number;
  z: number;
}

/** Euler angles in degrees, ZYX intrinsic convention. */
interface EulerAngles {
  pan: number;   // rotation about Z (yaw)
  tilt: number;  // rotation about Y (pitch)
  roll: number;  // rotation about X (roll)
}

/** A single keyframe as the user defines it. */
interface Keyframe {
  /** Time in milliseconds from the start of the move. */
  t: number;
  /** Axis positions at this keyframe, keyed by axis name.
   *  e.g. { pan: 45, tilt: -10, slide: 500 } */
  positions: Record<string, number>;
}

/** One row of the output trajectory table.
 *  Matches the element format of POST /api/trajectory `points` array. */
interface TrajectoryPoint {
  /** Milliseconds from start. */
  t: number;
  /** Axis values, keyed by axis name. */
  [axis: string]: number;
}

/** Cubic bezier control points (CSS cubic-bezier format).
 *  P0 is implicitly (0,0), P3 is implicitly (1,1).
 *  The two control points are (x1,y1) and (x2,y2). */
interface CubicBezierParams {
  x1: number;
  y1: number;
  x2: number;
  y2: number;
}

/** Easing preset names. */
type EasingPreset =
  | "linear"
  | "easeIn"
  | "easeOut"
  | "easeInOut"
  | "easeInCubic"
  | "easeOutCubic"
  | "easeInOutCubic";

/** Easing configuration: either a preset name or custom bezier parameters. */
type EasingConfig = EasingPreset | CubicBezierParams;

/** Per-axis timing and easing configuration. */
interface AxisConfig {
  /** Delay before this axis starts moving, in ms from move start. Default: 0. */
  delay_ms: number;
  /** Duration of this axis's movement, in ms. Default: total move duration. */
  duration_ms: number;
  /** Easing function for this axis. Default: "linear". */
  easing: EasingConfig;
}

/** Whether an axis represents a rotation or linear motion.
 *  Rotation axes are interpolated together via SQUAD quaternions.
 *  Linear axes get individual Catmull-Rom splines. */
type AxisInterpolation = 'rotation' | 'linear';

/** Top-level input to the trajectory generator. */
interface TrajectoryConfig {
  /** Ordered keyframes. Must have at least 1. Times must be ascending. */
  keyframes: Keyframe[];
  /** Maps each axis name to its interpolation type.
   *  e.g. { pan: 'rotation', tilt: 'rotation', roll: 'rotation', slide: 'linear' } */
  axisTypes: Record<string, AxisInterpolation>;
  /** Per-axis timing/easing overrides. Axes not listed use defaults. */
  axes?: Record<string, Partial<AxisConfig>>;
  /** Sample interval in ms. Default: 10 (100 Hz). */
  sample_interval_ms?: number;
  /** Whether the trajectory should loop. Default: false. */
  loop?: boolean;
}

/** Final output, ready to POST to /api/trajectory. */
interface Trajectory {
  points: TrajectoryPoint[];
  loop: boolean;
}
```

---

## Catmull-Rom Spline (`catmull-rom.ts`)

### Public API

```typescript
/**
 * Build a centripetal Catmull-Rom spline from a sequence of (t, value) pairs,
 * then evaluate it at arbitrary parameter values.
 *
 * @param knots - Array of {t, value} where t is in [0, 1] (normalized time)
 *                and value is the axis position. Must be sorted by t, length >= 2.
 * @returns A function that takes t in [0, 1] and returns the interpolated value.
 */
function createCatmullRomSpline(
  knots: Array<{ t: number; value: number }>
): (t: number) => number;
```

### Algorithm: Centripetal Catmull-Rom (alpha = 0.5)

Given four control points P0, P1, P2, P3 and their associated parameter values t0, t1, t2, t3, the goal is to interpolate between P1 and P2.

**Step 1 — Parameterize knot intervals using centripetal distance:**

For each consecutive pair of points, compute the knot interval:

```
d_ij = |P_j - P_i|
t_ij = d_ij^alpha    (alpha = 0.5, so this is sqrt(|P_j - P_i|))
```

If `d_ij = 0` (duplicate points), set `t_ij` to a small epsilon (1e-8) to avoid division by zero.

The cumulative knot values are:

```
t0 = 0
t1 = t0 + t_01
t2 = t1 + t_12
t3 = t2 + t_23
```

**Step 2 — Compute intermediate points via linear interpolation (Barry-Goldman formulation):**

For a query parameter `t` in `[t1, t2]`:

```
A1 = P0 * (t1 - t) / (t1 - t0) + P1 * (t - t0) / (t1 - t0)
A2 = P1 * (t2 - t) / (t2 - t1) + P2 * (t - t1) / (t2 - t1)
A3 = P2 * (t3 - t) / (t3 - t2) + P3 * (t - t2) / (t3 - t2)

B1 = A1 * (t2 - t) / (t2 - t0) + A2 * (t - t0) / (t2 - t0)
B2 = A2 * (t3 - t) / (t3 - t1) + A3 * (t - t1) / (t3 - t1)

C  = B1 * (t2 - t) / (t2 - t1) + B2 * (t - t1) / (t2 - t1)
```

`C` is the interpolated value.

**Step 3 — Map the caller's normalized `t` (in [0, 1]) to the spline's internal parameter space:**

The caller provides `t` in `[0, 1]`. Each segment spans between two knots. First, identify which segment `t` falls in (based on the input knot `t` values), then remap the caller's `t` into the internal `[t1, t2]` range of that segment.

### Endpoint Handling (Ghost Points)

Catmull-Rom needs 4 points per segment, but the first and last segments only have 3 real points. We generate ghost points:

- **Before the first knot (P_{-1}):** Reflect P1 across P0: `P_{-1} = 2 * P0 - P1`.
- **After the last knot (P_{n}):** Reflect P_{n-2} across P_{n-1}: `P_n = 2 * P_{n-1} - P_{n-2}`.

This produces zero velocity at the endpoints (the spline arrives at and departs from the first/last knot with a natural deceleration).

### Edge Cases

| Condition | Behavior |
|-----------|----------|
| 1 keyframe | Return constant value (no interpolation needed). |
| 2 keyframes | Degenerate to linear interpolation (LERP) between them. Ghost points are still generated, but the result is equivalent to a straight line. |
| Collinear points | The centripetal parameterization handles this correctly — `t_ij` values are non-zero as long as points are distinct. |
| Duplicate adjacent points | Knot interval forced to epsilon to avoid division by zero. Output holds the value constant through that region. |

---

## SQUAD Quaternion Interpolation (`squad.ts`)

### Public API

```typescript
/**
 * Convert Euler angles (degrees, ZYX convention) to a unit quaternion.
 */
function eulerToQuaternion(euler: EulerAngles): Quaternion;

/**
 * Convert a unit quaternion back to Euler angles (degrees, ZYX convention).
 */
function quaternionToEuler(q: Quaternion): EulerAngles;

/**
 * Spherical linear interpolation between two quaternions.
 * Always takes the shortest path (corrects sign if needed).
 *
 * @param q0 - Start quaternion (unit).
 * @param q1 - End quaternion (unit).
 * @param t  - Parameter in [0, 1].
 * @returns Interpolated unit quaternion.
 */
function slerp(q0: Quaternion, q1: Quaternion, t: number): Quaternion;

/**
 * Build a SQUAD interpolation curve from a sequence of (t, euler) pairs,
 * then evaluate it at arbitrary parameter values.
 *
 * @param knots - Array of {t, angles} where t is in [0, 1] (normalized time)
 *                and angles is an EulerAngles object. Must be sorted by t, length >= 2.
 * @returns A function that takes t in [0, 1] and returns interpolated EulerAngles.
 */
function createSquadInterpolator(
  knots: Array<{ t: number; angles: EulerAngles }>
): (t: number) => EulerAngles;
```

### Euler-to-Quaternion Conversion (ZYX Intrinsic)

Given pan (yaw, Z), tilt (pitch, Y), roll (X) in degrees, first convert to radians:

```
ψ = pan  * π / 180    (Z / yaw)
θ = tilt * π / 180    (Y / pitch)
φ = roll * π / 180    (X / roll)
```

ZYX intrinsic rotation means: rotate around Z first, then Y, then X.

The quaternion is:

```
cψ = cos(ψ/2),  sψ = sin(ψ/2)
cθ = cos(θ/2),  sθ = sin(θ/2)
cφ = cos(φ/2),  sφ = sin(φ/2)

w = cψ * cθ * cφ + sψ * sθ * sφ
x = cψ * cθ * sφ - sψ * sθ * cφ
y = cψ * sθ * cφ + sψ * cθ * sφ
z = sψ * cθ * cφ - cψ * sθ * sφ
```

### Quaternion-to-Euler Conversion (ZYX)

From a unit quaternion `{w, x, y, z}`:

```
// Roll (X)
sinr_cosp = 2 * (w * x + y * z)
cosr_cosp = 1 - 2 * (x * x + y * y)
roll = atan2(sinr_cosp, cosr_cosp)

// Pitch (Y) — clamp to avoid NaN from asin
sinp = 2 * (w * y - z * x)
if |sinp| >= 1:
  pitch = copysign(π/2, sinp)   // gimbal lock
else:
  pitch = asin(sinp)

// Yaw (Z)
siny_cosp = 2 * (w * z + x * y)
cosy_cosp = 1 - 2 * (y * y + z * z)
yaw = atan2(siny_cosp, cosy_cosp)
```

Convert radians back to degrees for the output.

### SLERP Formula

For two unit quaternions `q0` and `q1`, with parameter `t` in `[0, 1]`:

```
cosΩ = q0 · q1    (4D dot product: w0*w1 + x0*x1 + y0*y1 + z0*z1)
```

**Shortest path correction:** If `cosΩ < 0`, negate `q1` and set `cosΩ = -cosΩ`. This ensures interpolation takes the short way around the 4-sphere.

**Near-identity shortcut:** If `cosΩ > 0.9995`, the quaternions are nearly identical — use normalized linear interpolation (NLERP) to avoid numerical instability:

```
result = normalize((1 - t) * q0 + t * q1)
```

**Standard SLERP:**

```
Ω = acos(cosΩ)
sinΩ = sin(Ω)
s0 = sin((1 - t) * Ω) / sinΩ
s1 = sin(t * Ω) / sinΩ
result = s0 * q0 + s1 * q1
```

### SQUAD Formula

SQUAD produces smooth C¹-continuous interpolation through a sequence of quaternion keyframes, analogous to what Catmull-Rom does for scalar values.

Given a sequence of quaternions `q_0, q_1, ..., q_n` at times `t_0, t_1, ..., t_n`:

**Step 1 — Compute auxiliary quaternions (inner control points) `s_i` for each interior keyframe:**

For keyframe `i` (where `1 <= i <= n-1`):

```
s_i = q_i * exp( -(log(q_i⁻¹ * q_{i+1}) + log(q_i⁻¹ * q_{i-1})) / 4 )
```

Where:
- `q_i⁻¹` is the conjugate of `q_i` (since it's unit: `{w, -x, -y, -z}`)
- `log(q)` for a unit quaternion `q = cos(θ) + sin(θ) * û` returns `θ * û` (a 3-vector)
- `exp(v)` for a 3-vector `v` with magnitude `θ = |v|` returns `cos(θ) + sin(θ) * (v/θ)`
- If `θ ≈ 0`, `log` returns `(0,0,0)` and `exp` returns the identity quaternion

For the endpoints:
```
s_0 = q_0
s_n = q_n
```

**Step 2 — Interpolate within segment `[q_i, q_{i+1}]`:**

For local parameter `u` in `[0, 1]` within this segment:

```
SQUAD(q_i, q_{i+1}, s_i, s_{i+1}, u) = SLERP(SLERP(q_i, q_{i+1}, u), SLERP(s_i, s_{i+1}, u), 2u(1-u))
```

The outer SLERP parameter `2u(1-u)` is a parabola that peaks at 0.5 when `u = 0.5`, producing smooth blending of the "correction" from the inner control points while ensuring exact pass-through of the endpoint quaternions.

**Step 3 — Map the caller's `t` to the correct segment and local `u`:**

Find the segment where `t_i <= t < t_{i+1}`, then compute:

```
u = (t - t_i) / (t_{i+1} - t_i)
```

Convert the resulting quaternion back to Euler angles for output.

### Edge Cases

| Condition | Behavior |
|-----------|----------|
| 2 keyframes | Use SLERP directly (no auxiliary quaternions needed). |
| Near-180° rotation | `cosΩ ≈ -1`. After shortest-path correction, `cosΩ ≈ 1` and NLERP fallback applies. If truly 180° (both paths are equal length), SLERP axis is ambiguous — pick an arbitrary perpendicular axis. In practice this is rare; log a warning if `|cosΩ| < 1e-6` after correction. |
| Near-identity rotation | `cosΩ > 0.9995`. Use NLERP fallback as described above. |
| Quaternion sign ambiguity | Before computing `s_i`, ensure all quaternions in the sequence are in the same hemisphere: for each consecutive pair, if `q_i · q_{i+1} < 0`, negate `q_{i+1}`. This is a pre-processing pass done once before building the SQUAD curve. |
| Gimbal lock (pitch ≈ ±90°) | The quaternion representation handles this internally — there is no singularity. The Euler output may show discontinuities in pan/roll, but the actual rotation is smooth. This is acceptable because the board only needs smooth motion, and the Euler output is just for display/debugging. |

---

## Easing Functions (`easing.ts`)

### Public API

```typescript
/**
 * Create an easing function from a config (preset name or custom bezier).
 *
 * @param config - Preset name or CubicBezierParams.
 * @returns A function mapping t in [0,1] to eased_t in [0,1].
 *          Guaranteed: f(0) = 0, f(1) = 1.
 */
function createEasingFunction(config: EasingConfig): (t: number) => number;

/**
 * Evaluate a cubic bezier easing curve.
 *
 * @param x1, y1, x2, y2 - Control points (CSS cubic-bezier convention).
 * @param t - Input parameter in [0, 1].
 * @returns Eased value in [0, 1] (may slightly overshoot for bounce/elastic curves).
 */
function cubicBezier(x1: number, y1: number, x2: number, y2: number, t: number): number;
```

### Built-in Presets

Each preset is defined as a cubic bezier `(x1, y1, x2, y2)`:

| Preset | x1 | y1 | x2 | y2 | Description |
|--------|----|----|----|----|-------------|
| `linear` | 0.0 | 0.0 | 1.0 | 1.0 | No easing, constant velocity. |
| `easeIn` | 0.42 | 0.0 | 1.0 | 1.0 | Gradual acceleration (CSS `ease-in`). |
| `easeOut` | 0.0 | 0.0 | 0.58 | 1.0 | Gradual deceleration (CSS `ease-out`). |
| `easeInOut` | 0.42 | 0.0 | 0.58 | 1.0 | Accelerate then decelerate (CSS `ease-in-out`). |
| `easeInCubic` | 0.32 | 0.0 | 0.67 | 0.0 | Sharper acceleration. |
| `easeOutCubic` | 0.33 | 1.0 | 0.68 | 1.0 | Sharper deceleration. |
| `easeInOutCubic` | 0.65 | 0.0 | 0.35 | 1.0 | Sharper S-curve. |

### Cubic Bezier Engine

The CSS `cubic-bezier(x1, y1, x2, y2)` defines a parametric curve from `(0, 0)` to `(1, 1)` with two control points. The four points are:

```
P0 = (0, 0)
P1 = (x1, y1)
P2 = (x2, y2)
P3 = (1, 1)
```

Given an input time `t`, we need to find the corresponding `y` value. This is a two-step process:

**Step 1 — Solve for parameter `s` such that `Bx(s) = t`:**

The x-component of the bezier curve as a function of `s` in `[0, 1]`:

```
Bx(s) = 3(1-s)²s·x1 + 3(1-s)s²·x2 + s³
```

We need to find `s` such that `Bx(s) = t`. This is a cubic equation. Solve using Newton-Raphson iteration:

```
Initial guess: s = t
Repeat (max 8 iterations):
  x_err = Bx(s) - t
  dx = 3(1-s)²·x1 + 6(1-s)s·(x2-x1) + 3s²·(1-x2)   // derivative of Bx
  if |dx| < 1e-12: break   // flat region, stop
  s = s - x_err / dx
  clamp s to [0, 1]
  if |x_err| < 1e-7: break  // converged
```

If Newton-Raphson fails to converge (should not happen for valid easing curves where `x1, x2 ∈ [0, 1]`), fall back to binary search on `[0, 1]`.

**Step 2 — Evaluate `By(s)` at the solved `s`:**

```
By(s) = 3(1-s)²s·y1 + 3(1-s)s²·y2 + s³
```

This is the eased output value.

### How Easing Composes with Interpolation

Easing is a remapping of the time parameter, not of the output value. The composition is:

```
eased_t = easingFunction(t)        // remap time
position = interpolate(eased_t)    // evaluate spline at remapped time
```

This means `easeIn` causes the spline to be traversed slowly at first, then quickly — regardless of the spatial shape of the curve. The spline shape is unchanged; only the speed of traversal varies.

---

## Per-Axis Timing Model

Each axis can have an independent `delay_ms` and `duration_ms` within the overall move. This enables choreography where different axes move at different times.

### Default behavior

When no per-axis config is provided, all axes share the same timing:

```
delay_ms = 0
duration_ms = total move duration (last keyframe t - first keyframe t)
easing = "linear"
```

### How per-axis timing works

For a given global timestamp `T` (milliseconds from move start), each axis computes its own normalized parameter:

```
axis_t = (T - delay_ms) / duration_ms
axis_t = clamp(axis_t, 0, 1)
eased_t = easingFunction(axis_t)
```

When `T < delay_ms`, `axis_t = 0` — the axis holds its first keyframe position.
When `T > delay_ms + duration_ms`, `axis_t = 1` — the axis holds its last keyframe position.

### Overall move duration

The total trajectory duration is:

```
total_duration_ms = max over all axes of (delay_ms + duration_ms)
```

This may differ from the last keyframe's `t` value if per-axis timing stretches beyond it. The trajectory generator uses this computed total duration to determine how many sample points to produce.

### Example: staggered pan and slide

```
Keyframes: [{t: 0, positions: {pan: 0, slide: 0}}, {t: 10000, positions: {pan: 90, slide: 500}}]
Axes config:
  pan:   { delay_ms: 0,    duration_ms: 5000,  easing: "easeInOut" }
  slide: { delay_ms: 3000, duration_ms: 7000,  easing: "easeOut" }
```

Result: pan completes its 0→90° move in the first 5 seconds with easeInOut. Slide holds at 0mm for 3 seconds, then moves 0→500mm over the next 7 seconds with easeOut. Total trajectory: 10 seconds.

---

## Trajectory Table Generator (`trajectory.ts`)

### Public API

```typescript
/**
 * Generate a complete trajectory point table from keyframes and axis configs.
 *
 * @param config - Keyframes, per-axis timing/easing, sample rate.
 * @returns Trajectory object with points array, ready for POST /api/trajectory.
 * @throws Error if keyframes are empty, times are not ascending, or config is invalid.
 */
function generateTrajectory(config: TrajectoryConfig): Trajectory;
```

### Algorithm

**Input validation:**

1. `keyframes` must have at least 1 entry.
2. Keyframe `t` values must be strictly ascending.
3. All per-axis `delay_ms` must be >= 0.
4. All per-axis `duration_ms` must be > 0.

**Single keyframe special case:**

If there is exactly 1 keyframe, produce a single-point trajectory with `t = 0` and that keyframe's positions. No interpolation needed.

**Build interpolators (once, up front):**

1. Separate axes into rotation and linear groups based on `config.axisTypes`.
2. For each linear axis, build a `createCatmullRomSpline` using the keyframe `(normalized_t, value)` pairs, where `normalized_t` maps the keyframe times to `[0, 1]` based on the full keyframe time range. Values are read from `keyframe.positions[axisName]`.
3. For all rotation axes together, build a single `createSquadInterpolator` using the keyframe `(normalized_t, euler_angles)` pairs. The `EulerAngles` are constructed from the rotation axis values in `keyframe.positions`. All rotation axes are interpolated together via quaternions to avoid gimbal lock.

**Generate sample points:**

```
sample_interval = config.sample_interval_ms ?? 10
total_duration = compute from per-axis configs (see above)
num_points = floor(total_duration / sample_interval) + 1

for i = 0 to num_points - 1:
  T = i * sample_interval
  if T > total_duration: T = total_duration

  point = { t: T }

  // Linear axes (from axisTypes where value is 'linear')
  for each linear axis in config.axisTypes:
    axis_t = computeAxisT(T, axisConfig)
    eased_t = axisEasingFunction(axis_t)
    point[axisName] = linearSplines[axisName](eased_t)

  // Rotation axes (from axisTypes where value is 'rotation')
  // SQUAD interpolates all rotation axes together as a quaternion.
  // Each rotation axis may have independent timing, so we must evaluate
  // SQUAD separately for each axis's eased_t, extracting only the
  // relevant component.
  //
  // Default (common case): if all rotation axes share the same timing,
  // call SQUAD once and extract all components.

  // For independent rotation timing:
  for each rotation axis:
    axis_t = computeAxisT(T, axisConfig)
    eased_t = axisEasingFunction(axis_t)
    euler = squadInterpolator(eased_t)
    point[axisName] = euler[axisName]

  // For shared rotation timing (the common case):
  euler = squadInterpolator(shared_eased_t)
  for each rotation axis:
    point[axisName] = euler[axisName]

  points.push(point)
```

**Boundary enforcement:**

- The first output point must have `t = 0` and match the first keyframe's axis positions exactly.
- The last output point must have `t = total_duration` and match the last keyframe's axis positions exactly (after per-axis timing clamping).
- Do not rely on floating-point interpolation to hit these values — set them explicitly.

**Output:**

```typescript
return {
  points: points,
  loop: config.loop ?? false
};
```

### Per-Axis Rotation Timing: Design Decision

When rotation axes have independent timing, SQUAD cannot be called with a single parameter since the three Euler components would be driven at different speeds through the quaternion space. Two approaches:

| Approach | Pros | Cons |
|----------|------|------|
| **A: Evaluate SQUAD once per rotation axis at that axis's eased_t, extract only the matching component** | Simple. Each axis is truly independent. | Ignores quaternion coupling between axes. Pan at t=0.3 and tilt at t=0.7 may produce a rotation that was never on the SQUAD curve. |
| **B: Require all rotation axes share the same timing** | Physically correct quaternion interpolation. No gimbal-lock risk. | Less flexible. Users can't stagger pan vs tilt. |

**Decision: Use approach B as the default.** If all rotation axes have the same `delay_ms`, `duration_ms`, and `easing`, evaluate SQUAD once per timestep. If any rotation axis has different timing, fall back to approach A with a console warning that independent rotation timing may produce artifacts. This gives correctness in the common case while still allowing the advanced case.

### Output Format

The generated `points` array matches the `POST /api/trajectory` request format from the board API:

```json
{
  "points": [
    { "t": 0,    "pan": 0.0,  "tilt": 0.0, "roll": 0.0, "slide": 0.0   },
    { "t": 10,   "pan": 0.1,  "tilt": 0.0, "roll": 0.0, "slide": 1.7   },
    { "t": 20,   "pan": 0.4,  "tilt": 0.0, "roll": 0.0, "slide": 6.5   }
  ],
  "loop": false
}
```

Numeric values are rounded to 1 decimal place to keep payload size reasonable. Axis values are in native units (degrees for rotation, mm for linear).

---

## Test Strategy

### Known-Answer Tests

#### Test 1: Linear interpolation (2 keyframes, no easing)

```
Input:
  keyframes: [
    { t: 0,    positions: { pan: 0,  tilt: 0, roll: 0, slide: 0   } },
    { t: 1000, positions: { pan: 90, tilt: 0, roll: 0, slide: 500 } }
  ]
  axisTypes: { pan: 'rotation', tilt: 'rotation', roll: 'rotation', slide: 'linear' }
  sample_interval_ms: 250

Expected output (5 points):
  { t: 0,    pan: 0.0,  tilt: 0.0, roll: 0.0, slide: 0.0   }
  { t: 250,  pan: 22.5, tilt: 0.0, roll: 0.0, slide: 125.0 }
  { t: 500,  pan: 45.0, tilt: 0.0, roll: 0.0, slide: 250.0 }
  { t: 750,  pan: 67.5, tilt: 0.0, roll: 0.0, slide: 375.0 }
  { t: 1000, pan: 90.0, tilt: 0.0, roll: 0.0, slide: 500.0 }
```

Note: with only 2 keyframes, Catmull-Rom degenerates to LERP, and SLERP produces the quaternion equivalent of LERP for small rotations. The pan values should be exactly 22.5° increments because a 0→90° SLERP is linear in angle for single-axis rotation.

#### Test 2: Easing (2 keyframes, easeInOut on slide)

```
Input:
  keyframes: [
    { t: 0,    positions: { pan: 0, tilt: 0, roll: 0, slide: 0    } },
    { t: 1000, positions: { pan: 0, tilt: 0, roll: 0, slide: 1000 } }
  ]
  axisTypes: { pan: 'rotation', tilt: 'rotation', roll: 'rotation', slide: 'linear' }
  axes: { slide: { delay_ms: 0, duration_ms: 1000, easing: "easeInOut" } }
  sample_interval_ms: 250

Expected slide values (approximate, using CSS ease-in-out = cubic-bezier(0.42, 0, 0.58, 1)):
  t=0:     slide = 0.0
  t=250:   slide ≈ 80.5     (easeInOut at 0.25 ≈ 0.0805)
  t=500:   slide = 500.0    (easeInOut at 0.50 = 0.5, by symmetry)
  t=750:   slide ≈ 919.5    (easeInOut at 0.75 ≈ 0.9195)
  t=1000:  slide = 1000.0
```

#### Test 3: Per-axis timing (staggered start)

```
Input:
  keyframes: [
    { t: 0,    positions: { pan: 0,  tilt: 0, roll: 0, slide: 0   } },
    { t: 2000, positions: { pan: 45, tilt: 0, roll: 0, slide: 200 } }
  ]
  axisTypes: { pan: 'rotation', tilt: 'rotation', roll: 'rotation', slide: 'linear' }
  axes: {
    pan:   { delay_ms: 0,    duration_ms: 1000 },
    slide: { delay_ms: 1000, duration_ms: 1000 }
  }
  sample_interval_ms: 500

Expected (5 points):
  { t: 0,    pan: 0.0,  slide: 0.0   }  // both at start
  { t: 500,  pan: 22.5, slide: 0.0   }  // pan moving, slide waiting
  { t: 1000, pan: 45.0, slide: 0.0   }  // pan done, slide starting
  { t: 1500, pan: 45.0, slide: 100.0 }  // pan holding, slide moving
  { t: 2000, pan: 45.0, slide: 200.0 }  // both done
```

#### Test 4: 3+ keyframes with Catmull-Rom curve

```
Input:
  keyframes: [
    { t: 0,    positions: { pan: 0, tilt: 0, roll: 0, slide: 0    } },
    { t: 1000, positions: { pan: 0, tilt: 0, roll: 0, slide: 300  } },
    { t: 2000, positions: { pan: 0, tilt: 0, roll: 0, slide: 400  } },
    { t: 3000, positions: { pan: 0, tilt: 0, roll: 0, slide: 1000 } }
  ]
  axisTypes: { pan: 'rotation', tilt: 'rotation', roll: 'rotation', slide: 'linear' }
  sample_interval_ms: 1000

Expected slide values:
  t=0:     slide = 0.0      (exact, first keyframe)
  t=1000:  slide = 300.0    (exact, passes through keyframe)
  t=2000:  slide = 400.0    (exact, passes through keyframe)
  t=3000:  slide = 1000.0   (exact, last keyframe)
```

The Catmull-Rom spline must pass through all keyframe values exactly. Mid-segment values (e.g., `t=500`) will differ from linear interpolation — they will be smoother.

#### Test 5: SQUAD rotation through 3 keyframes

```
Input:
  keyframes: [
    { t: 0,    positions: { pan: 0,   tilt: 0,  roll: 0, slide: 0 } },
    { t: 1000, positions: { pan: 90,  tilt: 30, roll: 0, slide: 0 } },
    { t: 2000, positions: { pan: 180, tilt: 0,  roll: 0, slide: 0 } }
  ]
  axisTypes: { pan: 'rotation', tilt: 'rotation', roll: 'rotation', slide: 'linear' }
  sample_interval_ms: 1000

Expected rotation values:
  t=0:     pan = 0.0,   tilt = 0.0    (exact, first keyframe)
  t=1000:  pan = 90.0,  tilt = 30.0   (exact, passes through keyframe)
  t=2000:  pan = 180.0, tilt = 0.0    (exact, last keyframe)
```

At mid-segment times (e.g., `t=500`), SQUAD will produce a smooth rotation that is generally close to but not identical to naive Euler LERP.

### Property-Based Tests

| Property | Test |
|----------|------|
| **Boundary match** | `points[0]` axis values must exactly equal first keyframe values. `points[last]` axis values must exactly equal last keyframe values (accounting for per-axis timing clamping). |
| **Time ordering** | `points[i].t < points[i+1].t` for all consecutive points. |
| **Continuity** | For any two adjacent points, the change in each axis value must be within a reasonable epsilon. For a 100Hz trajectory with maximum axis velocity, define a per-axis max-delta threshold and verify no adjacent pair exceeds it. |
| **Easing boundaries** | `easingFunction(0) === 0` and `easingFunction(1) === 1` for all easing configs. |
| **Easing monotonicity** | For standard presets (not custom beziers), `easingFunction(t)` must be monotonically non-decreasing. Test by sampling 1000 evenly-spaced `t` values and verifying `f(t_i) <= f(t_{i+1})`. |
| **SLERP unit quaternion** | Output of every SLERP call must have magnitude within `1.0 ± 1e-10`. |
| **Keyframe pass-through** | When a sample point lands exactly on a keyframe time, the output must match the keyframe values exactly (within floating-point epsilon). |
| **Single keyframe** | Produces exactly 1 point with `t = 0`. |
| **Idle axes** | An axis whose keyframe values are all identical must produce that constant value at every sample point. |
| **Per-axis clamping** | Before `delay_ms`, axis holds first keyframe value. After `delay_ms + duration_ms`, axis holds last keyframe value. |

### Edge Case Tests

| Case | Expected |
|------|----------|
| 1 keyframe | 1-point trajectory, `t = 0`, all values from that keyframe. |
| 2 keyframes, all axes identical | All output points have the same axis values. |
| Very short move (10ms, 2 keyframes) | Produces 2 points: `t = 0` and `t = 10`. |
| Very long move (300s at 100Hz) | Produces 30,001 points. Verify it completes in < 1 second. |
| 180° pan rotation | SLERP handles without singularity. |
| 360° pan rotation across 3+ keyframes | Each segment is < 180°, so SQUAD handles it correctly. |
| Zero-duration axis (`duration_ms` very small) | Axis snaps from start to end value. No division by zero. |
| Keyframes with duplicate times | Rejected by input validation. |

---

## Dependencies

None. Pure TypeScript math. No external packages.

Standard `Math` functions used: `sin`, `cos`, `asin`, `acos`, `atan2`, `sqrt`, `abs`, `floor`, `min`, `max`, `PI`.

---

## Open Questions

1. **Rounding precision.** The spec says 1 decimal place for output values. Should this be configurable, or is 0.1° / 0.1mm always sufficient?
2. **Maximum trajectory size.** The board reports `max_trajectory_points: 10000`. Should the library enforce this limit, or leave it to the caller? If enforced, should it automatically adjust `sample_interval_ms` to fit?

**Note on focus axis:** Focus is a V2 feature. The dynamic axis model (`positions: Record<string, number>` + `axisTypes`) means adding focus later is just adding a new key to `positions` and a `'linear'` entry in `axisTypes`. No structural changes needed.
