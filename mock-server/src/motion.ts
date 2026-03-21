import type { Position, WsEvent } from "@slider/shared";
import type { BoardState } from "./state.js";
import type { StoredTrajectory } from "./trajectory.js";

type BroadcastFn = (event: WsEvent) => void;

/**
 * Simulates all motion types: trajectory playback, jog, move_to, homing.
 * Runs a tick loop at the configured telemetry rate while motion is active.
 */
export class MotionSimulator {
  private readonly _boardState: BoardState;
  private readonly _broadcast: BroadcastFn;
  private readonly _tickIntervalMs: number;

  private _timer: ReturnType<typeof setInterval> | null = null;
  private _activeMotion: ActiveMotion | null = null;

  constructor(
    boardState: BoardState,
    broadcast: BroadcastFn,
    telemetryRateHz: number,
  ) {
    this._boardState = boardState;
    this._broadcast = broadcast;
    this._tickIntervalMs = 1000 / telemetryRateHz;
  }

  // ---------------------------------------------------------------------------
  // Trajectory playback
  // ---------------------------------------------------------------------------

  startPlayback(trajectory: StoredTrajectory): void {
    this._stopMotion();
    this._activeMotion = new TrajectoryPlayback(
      trajectory,
      this._boardState,
      this._broadcast,
    );
    this._startTimer();
  }

  pause(): void {
    if (this._activeMotion instanceof TrajectoryPlayback) {
      this._activeMotion.pause();
    }
  }

  resume(): void {
    if (this._activeMotion instanceof TrajectoryPlayback) {
      this._activeMotion.resume();
    }
  }

  scrub(t: number): void {
    if (this._activeMotion instanceof TrajectoryPlayback) {
      this._activeMotion.scrub(t);
    }
  }

  // ---------------------------------------------------------------------------
  // Jog
  // ---------------------------------------------------------------------------

  startJog(deltas: Record<string, number>): void {
    let jog: JogMotion;

    if (this._activeMotion instanceof JogMotion) {
      jog = this._activeMotion;
    } else {
      this._stopMotion();
      jog = new JogMotion(this._boardState, this._broadcast);
      this._activeMotion = jog;
      this._startTimer();
    }

    jog.setDeltas(deltas);
  }

  // ---------------------------------------------------------------------------
  // Move to
  // ---------------------------------------------------------------------------

  startMoveTo(target: Position, duration_ms: number): void {
    this._stopMotion();
    this._activeMotion = new MoveToMotion(
      this._boardState,
      this._broadcast,
      target,
      duration_ms,
    );
    this._boardState.transition("moving");
    this._startTimer();
  }

  // ---------------------------------------------------------------------------
  // Homing
  // ---------------------------------------------------------------------------

  startHoming(axes: string[]): void {
    this._stopMotion();
    const target: Position = {};
    for (const axis of this._boardState.axes) {
      target[axis.name] = axes.includes(axis.name)
        ? Math.max(axis.min, Math.min(0, axis.max))
        : this._boardState.position[axis.name]!;
    }
    this._activeMotion = new MoveToMotion(
      this._boardState,
      this._broadcast,
      target,
      2000,
    );
    this._boardState.transition("homing");
    this._startTimer();
  }

  // ---------------------------------------------------------------------------
  // Stop
  // ---------------------------------------------------------------------------

  stop(): void {
    this._stopMotion();
  }

  // ---------------------------------------------------------------------------
  // Internal
  // ---------------------------------------------------------------------------

  private _startTimer(): void {
    if (this._timer !== null) return;
    this._timer = setInterval(() => this._tick(), this._tickIntervalMs);
  }

  private _stopTimer(): void {
    if (this._timer !== null) {
      clearInterval(this._timer);
      this._timer = null;
    }
  }

  private _stopMotion(): void {
    this._activeMotion = null;
    this._stopTimer();
  }

  private _tick(): void {
    if (this._activeMotion === null) {
      this._stopTimer();
      return;
    }

    const result = this._activeMotion.tick();

    if (result === TickResult.COMPLETE) {
      this._activeMotion = null;
      this._stopTimer();
    }
  }
}

// ---------------------------------------------------------------------------
// Motion types
// ---------------------------------------------------------------------------

const enum TickResult {
  CONTINUE,
  COMPLETE,
}

interface ActiveMotion {
  tick(): TickResult;
}

// ---------------------------------------------------------------------------
// Trajectory playback
// ---------------------------------------------------------------------------

class TrajectoryPlayback implements ActiveMotion {
  private readonly _trajectory: StoredTrajectory;
  private readonly _boardState: BoardState;
  private readonly _broadcast: BroadcastFn;
  private readonly _firedEvents = new Set<number>();

  private _startTime: number;
  private _pausedElapsed: number | null = null;

  constructor(
    trajectory: StoredTrajectory,
    boardState: BoardState,
    broadcast: BroadcastFn,
  ) {
    this._trajectory = trajectory;
    this._boardState = boardState;
    this._broadcast = broadcast;
    this._startTime = Date.now();
  }

  pause(): void {
    this._pausedElapsed = Date.now() - this._startTime;
  }

  resume(): void {
    if (this._pausedElapsed !== null) {
      this._startTime = Date.now() - this._pausedElapsed;
      this._pausedElapsed = null;
    }
  }

  scrub(t: number): void {
    const clamped = Math.max(0, Math.min(t, this._trajectory.duration_ms));
    const pos = this._interpolateAt(clamped);
    this._boardState.setPosition(pos);
    this._emitPosition(clamped);

    if (this._pausedElapsed !== null) {
      // Paused: update paused elapsed
      this._pausedElapsed = clamped;
    } else {
      // Playing: adjust start time so playback continues from here
      this._startTime = Date.now() - clamped;
    }
  }

  tick(): TickResult {
    if (this._pausedElapsed !== null) {
      return TickResult.CONTINUE; // Paused, don't advance
    }

    const elapsed = Date.now() - this._startTime;
    const { points, duration_ms, loop } = this._trajectory;

    // Fire events
    this._fireEvents(elapsed);

    if (elapsed >= duration_ms) {
      if (loop) {
        this._startTime = Date.now();
        this._firedEvents.clear();
        return TickResult.CONTINUE;
      }

      // Set final position exactly
      const lastPoint = points[points.length - 1]!;
      this._boardState.setPosition(lastPoint as unknown as Position);
      this._emitPosition(duration_ms);
      this._broadcast({
        evt: "complete",
        trajectory_id: this._trajectory.id,
      });
      this._boardState.transition("idle");
      return TickResult.COMPLETE;
    }

    const pos = this._interpolateAt(elapsed);
    this._boardState.setPosition(pos);
    this._emitPosition(elapsed);
    return TickResult.CONTINUE;
  }

  private _interpolateAt(elapsed: number): Position {
    const { points } = this._trajectory;

    // Binary search for bracketing points
    let lo = 0;
    let hi = points.length - 1;

    if (elapsed <= points[0]!.t) return this._pointToPosition(points[0]!);
    if (elapsed >= points[hi]!.t) return this._pointToPosition(points[hi]!);

    while (hi - lo > 1) {
      const mid = (lo + hi) >>> 1;
      if (points[mid]!.t <= elapsed) {
        lo = mid;
      } else {
        hi = mid;
      }
    }

    const p1 = points[lo]!;
    const p2 = points[hi]!;
    const ratio = (elapsed - p1.t) / (p2.t - p1.t);

    const pos: Position = {};
    for (const axis of this._boardState.axes) {
      const v1 = p1[axis.name] as number;
      const v2 = p2[axis.name] as number;
      pos[axis.name] = v1 + ratio * (v2 - v1);
    }
    return pos;
  }

  private _pointToPosition(point: Record<string, number>): Position {
    const pos: Position = {};
    for (const axis of this._boardState.axes) {
      pos[axis.name] = point[axis.name]!;
    }
    return pos;
  }

  private _fireEvents(elapsed: number): void {
    for (let i = 0; i < this._trajectory.events.length; i++) {
      if (this._firedEvents.has(i)) continue;
      const event = this._trajectory.events[i]!;
      if (elapsed >= event.t) {
        this._firedEvents.add(i);
        console.log(
          `[mock] Event fired at t=${event.t}ms: ${event.type}`,
        );
      }
    }
  }

  private _emitPosition(elapsed: number): void {
    const pos = this._boardState.position;
    const event: Record<string, number | string> = {
      evt: "position",
      t: Math.round(elapsed),
    };
    for (const axis of this._boardState.axes) {
      event[axis.name] = pos[axis.name]!;
    }
    this._broadcast(event as unknown as WsEvent);
  }
}

// ---------------------------------------------------------------------------
// Jog
// ---------------------------------------------------------------------------

const JOG_MAX_VELOCITY = 100; // units/sec (scaled per axis)
const JOG_ACCELERATION = 500; // units/sec^2
const JOG_DECELERATION = 800; // units/sec^2
const JOG_IDLE_TIMEOUT_MS = 100;

class JogMotion implements ActiveMotion {
  private readonly _boardState: BoardState;
  private readonly _broadcast: BroadcastFn;
  private readonly _velocities: Record<string, number> = {};
  private readonly _targetVelocities: Record<string, number> = {};
  private _lastTickTime = Date.now();
  private _lastCommandTime = Date.now();

  constructor(boardState: BoardState, broadcast: BroadcastFn) {
    this._boardState = boardState;
    this._broadcast = broadcast;
    for (const axis of boardState.axes) {
      this._velocities[axis.name] = 0;
      this._targetVelocities[axis.name] = 0;
    }
  }

  setDeltas(deltas: Record<string, number>): void {
    this._lastCommandTime = Date.now();
    for (const axis of this._boardState.axes) {
      if (axis.name in deltas) {
        const range = axis.max - axis.min;
        const delta = deltas[axis.name]!;
        this._targetVelocities[axis.name] =
          (delta / range) * JOG_MAX_VELOCITY;
      }
    }
  }

  tick(): TickResult {
    const now = Date.now();
    const dt = (now - this._lastTickTime) / 1000;
    this._lastTickTime = now;

    // If no jog commands received recently, decelerate all axes
    if (now - this._lastCommandTime > JOG_IDLE_TIMEOUT_MS) {
      for (const axis of this._boardState.axes) {
        this._targetVelocities[axis.name] = 0;
      }
    }

    let anyMoving = false;
    const pos = this._boardState.position;

    for (const axis of this._boardState.axes) {
      const name = axis.name;
      let vel = this._velocities[name]!;
      const target = this._targetVelocities[name]!;

      // Accelerate or decelerate toward target velocity
      if (vel < target) {
        vel = Math.min(vel + JOG_ACCELERATION * dt, target);
      } else if (vel > target) {
        vel = Math.max(vel - JOG_DECELERATION * dt, target);
      }

      // Update position
      const range = axis.max - axis.min;
      const scaledVel = vel * (range / JOG_MAX_VELOCITY);
      let newPos = pos[name]! + scaledVel * dt;

      // Clamp to limits
      if (newPos <= axis.min) {
        newPos = axis.min;
        vel = 0;
      } else if (newPos >= axis.max) {
        newPos = axis.max;
        vel = 0;
      }

      this._velocities[name] = vel;
      pos[name] = newPos;

      if (Math.abs(vel) > 0.001 || Math.abs(target) > 0.001) {
        anyMoving = true;
      }
    }

    this._boardState.setPosition(pos);

    // Emit position event
    const event: Record<string, number | string> = { evt: "position", t: 0 };
    for (const axis of this._boardState.axes) {
      event[axis.name] = pos[axis.name]!;
    }
    this._broadcast(event as unknown as WsEvent);

    if (!anyMoving) {
      this._boardState.transition("idle");
      return TickResult.COMPLETE;
    }

    return TickResult.CONTINUE;
  }
}

// ---------------------------------------------------------------------------
// Move to (also used for homing)
// ---------------------------------------------------------------------------

class MoveToMotion implements ActiveMotion {
  private readonly _boardState: BoardState;
  private readonly _broadcast: BroadcastFn;
  private readonly _startPosition: Position;
  private readonly _target: Position;
  private readonly _duration_ms: number;
  private readonly _startTime: number;

  constructor(
    boardState: BoardState,
    broadcast: BroadcastFn,
    target: Position,
    duration_ms: number,
  ) {
    this._boardState = boardState;
    this._broadcast = broadcast;
    this._startPosition = { ...boardState.position };
    this._target = target;
    this._duration_ms = duration_ms;
    this._startTime = Date.now();
  }

  tick(): TickResult {
    const elapsed = Date.now() - this._startTime;
    const ratio = Math.min(elapsed / this._duration_ms, 1);

    const pos: Position = {};
    for (const axis of this._boardState.axes) {
      const name = axis.name;
      const start = this._startPosition[name]!;
      const end = this._target[name]!;
      pos[name] = ratio >= 1 ? end : start + ratio * (end - start);
    }

    this._boardState.setPosition(pos);

    // Emit position
    const event: Record<string, number | string> = { evt: "position", t: 0 };
    const currentPos = this._boardState.position;
    for (const axis of this._boardState.axes) {
      event[axis.name] = currentPos[axis.name]!;
    }
    this._broadcast(event as unknown as WsEvent);

    if (ratio >= 1) {
      this._boardState.transition("idle");
      return TickResult.COMPLETE;
    }

    return TickResult.CONTINUE;
  }
}
