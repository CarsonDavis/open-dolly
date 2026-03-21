/**
 * Single row in the trajectory table.
 * Contains `t` (milliseconds) plus one value per axis.
 */
export interface TrajectoryPoint {
  t: number;
  [axis: string]: number;
}

/** Timed event within a trajectory (camera trigger, etc.). */
export interface TrajectoryEvent {
  t: number;
  type: "shutter" | "record_start" | "record_stop";
}

/** POST /api/trajectory request body. */
export interface TrajectoryUpload {
  points: TrajectoryPoint[];
  loop: boolean;
  events?: TrajectoryEvent[];
}

/** POST /api/trajectory response. */
export interface TrajectoryUploadResponse {
  trajectory_id: string;
  point_count: number;
  duration_ms: number;
}

/** GET /api/trajectory/:id/status response. */
export interface TrajectoryStatus {
  state: "idle" | "playing" | "paused";
  elapsed_ms: number;
  progress: number;
}
