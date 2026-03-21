# @opendolly/motion-math

Pure TypeScript motion math library for the camera slider. Takes user-defined keyframes and produces dense trajectory point tables ready for upload to the board via `POST /api/trajectory`.

No external dependencies. All math is implemented from scratch using standard `Math.*` functions.

## Quick start

```typescript
import { generateTrajectory } from "@opendolly/motion-math";

const trajectory = generateTrajectory({
  keyframes: [
    { t: 0,    positions: { pan: 0,  tilt: 0, roll: 0, slide: 0   } },
    { t: 5000, positions: { pan: 45, tilt: -10, roll: 0, slide: 500 } },
  ],
  axisTypes: { pan: "rotation", tilt: "rotation", roll: "rotation", slide: "linear" },
  sample_interval_ms: 10,  // 100 Hz
});

// trajectory.points is an array of { t, pan, tilt, roll, slide }
// ready to POST to /api/trajectory
```

## What it does

Given a set of keyframes (position snapshots at specific times), the library interpolates smooth motion between them:

- **Linear axes** (e.g. slide) use **centripetal Catmull-Rom splines** — smooth curves that pass exactly through every keyframe without cusps or overshoots.
- **Rotation axes** (pan, tilt, roll) use **SQUAD quaternion interpolation** — smooth rotation through quaternion space that avoids gimbal lock and produces C1-continuous motion.
- **Easing** remaps the time parameter via cubic bezier curves (same model as CSS `cubic-bezier()`), so the camera accelerates and decelerates naturally.
- **Per-axis timing** lets each axis have its own delay, duration, and easing — so you can start the pan before the slide, or ease the tilt differently from the roll.

The output is a dense table of timestamped positions (default 100 Hz) that the board walks through linearly during playback. All the complexity is baked into the table; the board just steps through it.

## API

### `generateTrajectory(config: TrajectoryConfig): Trajectory`

The main entry point. Takes keyframes + configuration, returns a point table.

```typescript
interface TrajectoryConfig {
  keyframes: Keyframe[];                          // at least 1, times ascending
  axisTypes: Record<string, "rotation" | "linear">;
  axes?: Record<string, Partial<AxisConfig>>;     // per-axis timing/easing overrides
  sample_interval_ms?: number;                    // default: 10 (100 Hz)
  loop?: boolean;                                 // default: false
}

interface AxisConfig {
  delay_ms: number;       // default: 0
  duration_ms: number;    // default: total keyframe duration
  easing: EasingConfig;   // default: "linear"
}
```

Rotation axes must be named `pan`, `tilt`, and/or `roll` (they're interpolated together as quaternions via SQUAD).

### `createEasingFunction(config: EasingConfig): (t: number) => number`

Returns an easing function mapping `[0,1] -> [0,1]`.

Built-in presets: `"linear"`, `"easeIn"`, `"easeOut"`, `"easeInOut"`, `"easeInCubic"`, `"easeOutCubic"`, `"easeInOutCubic"`.

Custom bezier: `{ x1: 0.42, y1: 0, x2: 0.58, y2: 1 }` (same format as CSS `cubic-bezier()`).

### `createCatmullRomSpline(knots): (t: number) => number`

Builds a centripetal Catmull-Rom spline from `{ t, value }` pairs. Returns a function that evaluates the spline at any `t` in `[0, 1]`.

### `createSquadInterpolator(knots): (t: number) => EulerAngles`

Builds a SQUAD interpolation curve from `{ t, angles }` pairs. Returns a function that evaluates smooth rotation at any `t` in `[0, 1]`.

### `slerp(q0, q1, t): Quaternion`

Spherical linear interpolation between two quaternions. Always takes the shortest path.

### `eulerToQuaternion(euler): Quaternion` / `quaternionToEuler(q): EulerAngles`

Convert between Euler angles (degrees, ZYX intrinsic convention) and unit quaternions.

## Per-axis timing example

Stagger pan and slide so pan finishes first, then slide starts:

```typescript
const trajectory = generateTrajectory({
  keyframes: [
    { t: 0,    positions: { pan: 0, tilt: 0, roll: 0, slide: 0   } },
    { t: 2000, positions: { pan: 45, tilt: 0, roll: 0, slide: 200 } },
  ],
  axisTypes: { pan: "rotation", tilt: "rotation", roll: "rotation", slide: "linear" },
  axes: {
    pan:   { delay_ms: 0,    duration_ms: 1000, easing: "easeInOut" },
    slide: { delay_ms: 1000, duration_ms: 1000, easing: "easeOut"   },
  },
});
// Pan moves 0-1s with easeInOut, then holds.
// Slide waits 0-1s, then moves 1-2s with easeOut.
```

## Tests

```bash
npm test          # run once
npm run test:watch  # watch mode
```

49 tests covering all plan test vectors, property-based checks (boundary matching, monotonicity, ascending timestamps, unit quaternion output), and edge cases (single keyframe, constant values, very short/long moves, slider-only configs).
