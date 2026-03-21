# Motion Math Library Index

Pure TypeScript library for computing trajectory point tables from user-defined keyframes. No external dependencies. Runs in the browser.

**Plan:** [`docs/project/plans/plan-motion-math.md`](../docs/project/plans/plan-motion-math.md)

| Path | Purpose | Key Details |
|------|---------|-------------|
| `README.md` | Usage guide and API reference | Quick start, all public functions, per-axis timing example |

## Source Files

| Path | Purpose | Key Details |
|------|---------|-------------|
| `src/types.ts` | All shared type definitions | Keyframe, TrajectoryConfig, TrajectoryPoint, Trajectory, EulerAngles, Quaternion, EasingConfig, AxisConfig, AxisInterpolation |
| `src/easing.ts` | Cubic bezier easing engine | 7 presets (linear, easeIn, easeOut, easeInOut, easeInCubic, easeOutCubic, easeInOutCubic) + custom bezier params; Newton-Raphson solver with binary search fallback |
| `src/catmull-rom.ts` | Centripetal Catmull-Rom spline for linear axes | Alpha=0.5; Barry-Goldman formulation; ghost points at endpoints for zero-velocity boundaries; handles 1 knot (constant), 2 knots (LERP), 3+ knots (full spline) |
| `src/squad.ts` | SQUAD quaternion interpolation for rotation axes | Euler↔quaternion conversion (ZYX intrinsic); SLERP with shortest-path correction and NLERP fallback; SQUAD with auxiliary control points for C1-continuous rotation; hemisphere alignment preprocessing |
| `src/trajectory.ts` | Trajectory table generator (composes all modules) | Per-axis timing/delay/easing; shared vs independent rotation timing detection; validates rotation axes must be pan/tilt/roll; boundary enforcement (first/last points exact); 0.1-degree rounding |
| `src/index.ts` | Public API barrel export | Exports generateTrajectory, createCatmullRomSpline, createSquadInterpolator, createEasingFunction, slerp, eulerToQuaternion, quaternionToEuler, cubicBezier, and all types |

## Tests

| Path | Coverage | Key Details |
|------|----------|-------------|
| `src/easing.test.ts` | 11 tests | Boundary values, monotonicity, symmetry, presets, custom bezier, unknown preset error |
| `src/catmull-rom.test.ts` | 9 tests | Constant, LERP, 3-knot pass-through, 4-knot pass-through, mid-segment curvature, clamping, continuity |
| `src/squad.test.ts` | 12 tests | Euler↔quaternion round-trip, unit quaternion output, SLERP midpoint/endpoints/shortest-path, SQUAD keyframe pass-through, smoothness |
| `src/trajectory.test.ts` | 17 tests | All 5 plan test vectors, boundary matching, ascending timestamps, loop flag, single keyframe, short/long moves, constant values, slider-only, invalid rotation axis, validation errors |
