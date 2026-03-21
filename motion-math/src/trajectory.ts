/**
 * Trajectory table generator.
 *
 * Takes user-defined keyframes and timing configuration, produces
 * a dense point table ready for POST /api/trajectory.
 */

import type {
  TrajectoryConfig,
  TrajectoryPoint,
  Trajectory,
  AxisConfig,
  EasingConfig,
  EulerAngles,
} from "./types.js";
import { createCatmullRomSpline } from "./catmull-rom.js";
import { createSquadInterpolator } from "./squad.js";
import { createEasingFunction } from "./easing.js";

const VALID_ROTATION_AXES = new Set(["pan", "tilt", "roll"]);

/**
 * Generate a complete trajectory point table from keyframes and axis configs.
 *
 * @throws Error if keyframes are empty, times are not ascending, or config is invalid.
 */
export function generateTrajectory(config: TrajectoryConfig): Trajectory {
  const {
    keyframes,
    axisTypes,
    axes: axisOverrides,
    sample_interval_ms = 10,
    loop = false,
  } = config;

  // ── Validation ──────────────────────────────────────────────────
  const allAxes = Object.keys(axisTypes);

  if (keyframes.length === 0) {
    throw new Error("At least 1 keyframe is required");
  }
  for (let i = 1; i < keyframes.length; i++) {
    if (keyframes[i].t <= keyframes[i - 1].t) {
      throw new Error(
        `Keyframe times must be strictly ascending: t[${i}]=${keyframes[i].t} <= t[${i - 1}]=${keyframes[i - 1].t}`,
      );
    }
  }

  // Validate rotation axis names — SQUAD requires Euler angles (pan/tilt/roll)
  const rotationAxes = allAxes.filter((a) => axisTypes[a] === "rotation");
  for (const axis of rotationAxes) {
    if (!VALID_ROTATION_AXES.has(axis)) {
      throw new Error(
        `Rotation axis "${axis}" is not supported. Rotation axes must be named "pan", "tilt", or "roll".`,
      );
    }
  }

  // ── Single keyframe special case ────────────────────────────────
  if (keyframes.length === 1) {
    const point: TrajectoryPoint = { t: 0 };
    for (const axis of allAxes) {
      point[axis] = keyframes[0].positions[axis] ?? 0;
    }
    return { points: [point], loop };
  }

  // ── Resolve per-axis configs ────────────────────────────────────
  const keyframeDuration = keyframes[keyframes.length - 1].t - keyframes[0].t;

  const resolvedAxes: Record<string, AxisConfig> = {};
  for (const axis of allAxes) {
    const override = axisOverrides?.[axis];
    resolvedAxes[axis] = {
      delay_ms: override?.delay_ms ?? 0,
      duration_ms: override?.duration_ms ?? keyframeDuration,
      easing: override?.easing ?? "linear",
    };
    if (resolvedAxes[axis].delay_ms < 0) {
      throw new Error(`Axis "${axis}": delay_ms must be >= 0`);
    }
    if (resolvedAxes[axis].duration_ms <= 0) {
      throw new Error(`Axis "${axis}": duration_ms must be > 0`);
    }
  }

  // ── Compute total duration ──────────────────────────────────────
  let totalDuration = 0;
  for (const axis of allAxes) {
    const end = resolvedAxes[axis].delay_ms + resolvedAxes[axis].duration_ms;
    if (end > totalDuration) totalDuration = end;
  }

  // ── Build easing functions ──────────────────────────────────────
  const easingFns: Record<string, (t: number) => number> = {};
  for (const axis of allAxes) {
    easingFns[axis] = createEasingFunction(resolvedAxes[axis].easing);
  }

  // ── Normalize keyframe times to [0, 1] ─────────────────────────
  const tStart = keyframes[0].t;
  const tRange = keyframeDuration;
  const normalizedT = keyframes.map((kf) =>
    tRange > 0 ? (kf.t - tStart) / tRange : 0,
  );

  // ── Build interpolators ─────────────────────────────────────────
  const linearAxes = allAxes.filter((a) => axisTypes[a] === "linear");

  // Linear: one Catmull-Rom spline per axis
  const linearSplines: Record<string, (t: number) => number> = {};
  for (const axis of linearAxes) {
    const knots = keyframes.map((kf, i) => ({
      t: normalizedT[i],
      value: kf.positions[axis] ?? 0,
    }));
    linearSplines[axis] = createCatmullRomSpline(knots);
  }

  // Rotation: SQUAD interpolator (all rotation axes together)
  let squadInterp: ((t: number) => EulerAngles) | null = null;
  if (rotationAxes.length > 0) {
    const squadKnots = keyframes.map((kf, i) => ({
      t: normalizedT[i],
      angles: {
        pan: kf.positions["pan"] ?? 0,
        tilt: kf.positions["tilt"] ?? 0,
        roll: kf.positions["roll"] ?? 0,
      } as EulerAngles,
    }));
    squadInterp = createSquadInterpolator(squadKnots);
  }

  // ── Check if all rotation axes share the same timing ────────────
  let sharedRotationTiming = true;
  if (rotationAxes.length > 1) {
    const ref = resolvedAxes[rotationAxes[0]];
    for (let i = 1; i < rotationAxes.length; i++) {
      const ax = resolvedAxes[rotationAxes[i]];
      if (
        ax.delay_ms !== ref.delay_ms ||
        ax.duration_ms !== ref.duration_ms ||
        !easingEqual(ax.easing, ref.easing)
      ) {
        sharedRotationTiming = false;
        break;
      }
    }
  }

  // ── Helper: compute per-axis normalized+eased t ─────────────────
  function axisEasedT(axis: string, globalT_ms: number): number {
    const cfg = resolvedAxes[axis];
    let axisT = (globalT_ms - cfg.delay_ms) / cfg.duration_ms;
    if (axisT < 0) axisT = 0;
    if (axisT > 1) axisT = 1;
    return easingFns[axis](axisT);
  }

  // ── Generate sample points ──────────────────────────────────────
  const numPoints = Math.floor(totalDuration / sample_interval_ms) + 1;
  const points: TrajectoryPoint[] = [];

  for (let i = 0; i < numPoints; i++) {
    let T = i * sample_interval_ms;
    if (T > totalDuration) T = totalDuration;

    const point: TrajectoryPoint = { t: T };

    // Linear axes
    for (const axis of linearAxes) {
      const et = axisEasedT(axis, T);
      point[axis] = round1(linearSplines[axis](et));
    }

    // Rotation axes
    if (squadInterp) {
      if (sharedRotationTiming && rotationAxes.length > 0) {
        // Common case: evaluate SQUAD once
        const et = axisEasedT(rotationAxes[0], T);
        const euler = squadInterp(et);
        for (const axis of rotationAxes) {
          point[axis] = round1(euler[axis as keyof EulerAngles]);
        }
      } else {
        // Independent timing: evaluate SQUAD per rotation axis
        for (const axis of rotationAxes) {
          const et = axisEasedT(axis, T);
          const euler = squadInterp(et);
          point[axis] = round1(euler[axis as keyof EulerAngles]);
        }
      }
    }

    points.push(point);
  }

  // ── Ensure last point is exactly at totalDuration ───────────────
  const lastPoint = points[points.length - 1];
  if (lastPoint.t !== totalDuration) {
    // Add a final point at totalDuration
    const finalPoint: TrajectoryPoint = { t: totalDuration };
    for (const axis of linearAxes) {
      const et = axisEasedT(axis, totalDuration);
      finalPoint[axis] = round1(linearSplines[axis](et));
    }
    if (squadInterp) {
      if (sharedRotationTiming && rotationAxes.length > 0) {
        const et = axisEasedT(rotationAxes[0], totalDuration);
        const euler = squadInterp(et);
        for (const axis of rotationAxes) {
          finalPoint[axis] = round1(euler[axis as keyof EulerAngles]);
        }
      } else {
        for (const axis of rotationAxes) {
          const et = axisEasedT(axis, totalDuration);
          const euler = squadInterp(et);
          finalPoint[axis] = round1(euler[axis as keyof EulerAngles]);
        }
      }
    }
    points.push(finalPoint);
  }

  // ── Force exact boundary values ─────────────────────────────────
  // First point must match first keyframe exactly
  const first = points[0];
  for (const axis of allAxes) {
    first[axis] = round1(keyframes[0].positions[axis] ?? 0);
  }

  // Last point must match last keyframe exactly (after per-axis clamping)
  const last = points[points.length - 1];
  for (const axis of allAxes) {
    // Only set to last keyframe value if the axis has completed its motion
    const cfg = resolvedAxes[axis];
    if (totalDuration >= cfg.delay_ms + cfg.duration_ms) {
      last[axis] = round1(keyframes[keyframes.length - 1].positions[axis] ?? 0);
    }
  }

  return { points, loop };
}

/** Round to 1 decimal place. */
function round1(v: number): number {
  return Math.round(v * 10) / 10;
}

/** Compare two EasingConfig values for equality. */
function easingEqual(a: EasingConfig, b: EasingConfig): boolean {
  if (typeof a === "string" || typeof b === "string") return a === b;
  return a.x1 === b.x1 && a.y1 === b.y1 && a.x2 === b.x2 && a.y2 === b.y2;
}
