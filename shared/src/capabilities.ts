/** Unit of measurement for an axis. */
export type AxisUnit = "deg" | "mm";

/** Single axis definition as reported by GET /api/capabilities. */
export interface AxisCapability {
  name: string;
  min: number;
  max: number;
  unit: AxisUnit;
}

/** GET /api/capabilities response. */
export interface Capabilities {
  axes: AxisCapability[];
  max_trajectory_points: number;
  max_trajectory_duration_ms: number;
  telemetry_rate_hz: number;
  firmware_version: string;
  board: string;
}
