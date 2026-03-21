/** All valid system states. */
export type SystemState =
  | "idle"
  | "moving"
  | "playing"
  | "paused"
  | "homing"
  | "error";

/** Axis positions keyed by axis name. */
export type Position = Record<string, number>;

/** GET /api/status response. */
export interface StatusResponse {
  state: SystemState;
  position: Position;
  battery: number;
  trajectory_loaded: string | null;
}
