// Public API
export { generateTrajectory } from "./trajectory.js";
export { createCatmullRomSpline } from "./catmull-rom.js";
export { createSquadInterpolator, eulerToQuaternion, quaternionToEuler, slerp } from "./squad.js";
export { createEasingFunction, cubicBezier } from "./easing.js";

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
  AxisConfig,
  AxisInterpolation,
} from "./types.js";
