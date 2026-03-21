import type {
  TrajectoryUpload,
  TrajectoryUploadResponse,
  TrajectoryEvent,
  TrajectoryPoint,
  Capabilities,
} from "@opendolly/shared";

export interface StoredTrajectory {
  id: string;
  points: TrajectoryPoint[];
  events: TrajectoryEvent[];
  loop: boolean;
  duration_ms: number;
}

/**
 * In-memory trajectory storage.
 * Validates uploads against the active profile and stores them by ID.
 */
export class TrajectoryStore {
  private readonly _trajectories = new Map<string, StoredTrajectory>();
  private readonly _capabilities: Capabilities;

  constructor(capabilities: Capabilities) {
    this._capabilities = capabilities;
  }

  get(id: string): StoredTrajectory | undefined {
    return this._trajectories.get(id);
  }

  has(id: string): boolean {
    return this._trajectories.has(id);
  }

  delete(id: string): boolean {
    return this._trajectories.delete(id);
  }

  /**
   * Validate and store a trajectory upload.
   * Returns the response on success, or throws with a detail message.
   */
  upload(body: TrajectoryUpload): TrajectoryUploadResponse {
    const { points, loop, events } = body;
    const axisNames = this._capabilities.axes.map((a) => a.name);

    // Validate points array
    if (!Array.isArray(points) || points.length === 0) {
      throw new TrajectoryValidationError("points must be a non-empty array");
    }

    if (points.length > this._capabilities.max_trajectory_points) {
      throw new TrajectoryValidationError(
        `Too many points: ${points.length} exceeds max ${this._capabilities.max_trajectory_points}`,
      );
    }

    // Validate each point
    let prevT = -1;
    for (let i = 0; i < points.length; i++) {
      const point = points[i]!;

      // Check t is a number
      if (typeof point.t !== "number" || !Number.isFinite(point.t)) {
        throw new TrajectoryValidationError(
          `Point ${i}: t must be a finite number`,
        );
      }

      // Check ascending order
      if (point.t <= prevT) {
        throw new TrajectoryValidationError(
          `Point ${i}: t values not in ascending order (t=${point.t} after t=${prevT})`,
        );
      }
      prevT = point.t;

      // Check all required axes present
      for (const axisName of axisNames) {
        if (!(axisName in point)) {
          throw new TrajectoryValidationError(
            `Point ${i}: missing axis '${axisName}'`,
          );
        }
        const value = point[axisName]!;
        if (typeof value !== "number" || !Number.isFinite(value)) {
          throw new TrajectoryValidationError(
            `Point ${i}: axis '${axisName}' must be a finite number`,
          );
        }
      }

      // Check no extra axes
      const pointKeys = Object.keys(point).filter((k) => k !== "t");
      for (const key of pointKeys) {
        if (!axisNames.includes(key)) {
          throw new TrajectoryValidationError(
            `Point ${i}: unknown axis '${key}'`,
          );
        }
      }

      // Check axis values within range
      for (const axis of this._capabilities.axes) {
        const value = point[axis.name]!;
        if (value < axis.min || value > axis.max) {
          throw new TrajectoryValidationError(
            `Point ${i}: axis '${axis.name}' value ${value} out of range [${axis.min}, ${axis.max}]`,
          );
        }
      }
    }

    // Check duration limit
    const duration_ms = points[points.length - 1]!.t;
    if (duration_ms > this._capabilities.max_trajectory_duration_ms) {
      throw new TrajectoryValidationError(
        `Duration ${duration_ms}ms exceeds max ${this._capabilities.max_trajectory_duration_ms}ms`,
      );
    }

    // Generate ID
    const id = `traj_${crypto.randomUUID().slice(0, 8)}`;

    const stored: StoredTrajectory = {
      id,
      points,
      events: events ?? [],
      loop,
      duration_ms,
    };

    this._trajectories.set(id, stored);

    return {
      trajectory_id: id,
      point_count: points.length,
      duration_ms,
    };
  }
}

export class TrajectoryValidationError extends Error {
  constructor(detail: string) {
    super(detail);
    this.name = "TrajectoryValidationError";
  }
}
