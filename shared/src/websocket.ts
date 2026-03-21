import type { ErrorCode } from "./errors.js";
import type { SystemState } from "./state.js";

// ---------------------------------------------------------------------------
// Client → Board commands
// ---------------------------------------------------------------------------

export interface PlayCommand {
  cmd: "play";
  trajectory_id: string;
}

export interface PauseCommand {
  cmd: "pause";
}

export interface ResumeCommand {
  cmd: "resume";
}

export interface StopCommand {
  cmd: "stop";
}

export interface ScrubCommand {
  cmd: "scrub";
  t: number;
}

/**
 * Jog command — relative axis deltas.
 * Only axes being jogged are present. Values are in native units (deg/mm).
 */
export interface JogCommand {
  cmd: "jog";
  [axis: string]: number | string; // `cmd` is string, axis values are number
}

/**
 * Move to absolute position over a duration.
 * All axis target values plus `duration_ms`.
 */
export interface MoveToCommand {
  cmd: "move_to";
  duration_ms: number;
  [axis: string]: number | string;
}

export interface HomeCommand {
  cmd: "home";
  axes: string[];
}

export interface SimulateErrorCommand {
  cmd: "simulate_error";
  code: ErrorCode;
  axis?: string;
}

/** Union of all WebSocket client → board commands. */
export type WsCommand =
  | PlayCommand
  | PauseCommand
  | ResumeCommand
  | StopCommand
  | ScrubCommand
  | JogCommand
  | MoveToCommand
  | HomeCommand
  | SimulateErrorCommand;

// ---------------------------------------------------------------------------
// Board → Client events
// ---------------------------------------------------------------------------

export interface PositionEvent {
  evt: "position";
  t: number;
  [axis: string]: number | string;
}

export interface StateEvent {
  evt: "state";
  state: SystemState;
}

export interface CompleteEvent {
  evt: "complete";
  trajectory_id: string;
}

export interface ErrorEvent {
  evt: "error";
  code: ErrorCode;
  axis?: string;
  detail: string;
}

/** Union of all WebSocket board → client events. */
export type WsEvent =
  | PositionEvent
  | StateEvent
  | CompleteEvent
  | ErrorEvent;
