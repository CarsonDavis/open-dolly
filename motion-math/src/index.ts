// Public API
export { generateTrajectory } from "./trajectory.js";
export { createCatmullRomSpline } from "./catmull-rom.js";
export { createSquadInterpolator, eulerToQuaternion, quaternionToEuler, slerp } from "./squad.js";
export { createEasingFunction, cubicBezier } from "./easing.js";
export { createProgressCurveFunction } from "./progress-curve.js";

// Types
export type {
  Quaternion,
  EulerAngles,
  Keyframe,
  TrajectoryPoint,
  TrajectoryConfig,
  Trajectory,
  CubicBezierParams,
  EasingPreset,
  EasingConfig,
  ProgressCurvePoint,
  ProgressCurve,
  AxisConfig,
  AxisInterpolation,
} from "./types.js";
