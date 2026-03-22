/** Quaternion in w, x, y, z order (Hamilton convention). */
export interface Quaternion {
  w: number;
  x: number;
  y: number;
  z: number;
}

/** Euler angles in degrees, ZYX intrinsic convention. */
export interface EulerAngles {
  pan: number;   // rotation about Z (yaw)
  tilt: number;  // rotation about Y (pitch)
  roll: number;  // rotation about X (roll)
}

/** A single keyframe as the user defines it. */
export interface Keyframe {
  /** Time in milliseconds from the start of the move. */
  t: number;
  /** Axis positions at this keyframe, keyed by axis name.
   *  e.g. { pan: 45, tilt: -10, slide: 500 } */
  positions: Record<string, number>;
}

/** One row of the output trajectory table.
 *  Matches the element format of POST /api/trajectory `points` array. */
export interface TrajectoryPoint {
  /** Milliseconds from start. */
  t: number;
  /** Axis values, keyed by axis name. */
  [axis: string]: number;
}

/** Cubic bezier control points (CSS cubic-bezier format).
 *  P0 is implicitly (0,0), P3 is implicitly (1,1).
 *  The two control points are (x1,y1) and (x2,y2). */
export interface CubicBezierParams {
  x1: number;
  y1: number;
  x2: number;
  y2: number;
}

/** Easing preset names. */
export type EasingPreset =
  | "linear"
  | "easeIn"
  | "easeOut"
  | "easeInOut"
  | "easeInCubic"
  | "easeOutCubic"
  | "easeInOutCubic";

/** Easing configuration: either a preset name or custom bezier parameters. */
export type EasingConfig = EasingPreset | CubicBezierParams;

/** A point on a progress curve (normalized [0, 1] space). */
export interface ProgressCurvePoint {
  t: number;
  progress: number;
}

/** Multi-point progress curve for V2 per-axis curve editing.
 *  Points are interior control points; (0,0) and (1,1) are implicit endpoints. */
export interface ProgressCurve {
  points: ProgressCurvePoint[];
}

/** Per-axis timing and easing configuration. */
export interface AxisConfig {
  /** Delay before this axis starts moving, in ms from move start. Default: 0. */
  delay_ms: number;
  /** Duration of this axis's movement, in ms. Default: total move duration. */
  duration_ms: number;
  /** Easing function for this axis. Default: "linear".
   *  Accepts V1 presets/bezier or V2 multi-point progress curves. */
  easing: EasingConfig | ProgressCurve;
}

/** Whether an axis represents a rotation or linear motion.
 *  Rotation axes are interpolated together via SQUAD quaternions.
 *  Linear axes get individual Catmull-Rom splines. */
export type AxisInterpolation = "rotation" | "linear";

/** Top-level input to the trajectory generator. */
export interface TrajectoryConfig {
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
export interface Trajectory {
  points: TrajectoryPoint[];
  loop: boolean;
}
