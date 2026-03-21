import type {
  SystemState,
  Position,
  Capabilities,
  AxisCapability,
} from "@opendolly/shared";

/**
 * Centralized system state. Owns current position, system state,
 * loaded trajectory, and broadcasts state transitions via a listener.
 */
export class BoardState {
  private _state: SystemState = "idle";
  private _position: Position;
  private _trajectoryLoaded: string | null = null;
  private readonly _axes: AxisCapability[];
  private readonly _listeners: Array<(state: SystemState) => void> = [];

  constructor(capabilities: Capabilities) {
    this._axes = capabilities.axes;
    this._position = {};
    for (const axis of this._axes) {
      // All axes start at 0 (or clamped to min if 0 is out of range)
      this._position[axis.name] = Math.max(axis.min, Math.min(0, axis.max));
    }
  }

  get state(): SystemState {
    return this._state;
  }

  get position(): Position {
    return { ...this._position };
  }

  get trajectoryLoaded(): string | null {
    return this._trajectoryLoaded;
  }

  get axes(): readonly AxisCapability[] {
    return this._axes;
  }

  /** Register a listener that fires on every state transition. */
  onStateChange(listener: (state: SystemState) => void): void {
    this._listeners.push(listener);
  }

  /** Transition to a new state. Notifies all listeners. */
  transition(newState: SystemState): void {
    if (this._state === newState) return;
    this._state = newState;
    for (const listener of this._listeners) {
      listener(newState);
    }
  }

  /** Update position for one or more axes. Clamps to axis limits. */
  updatePosition(updates: Partial<Position>): void {
    for (const axis of this._axes) {
      if (axis.name in updates) {
        const value = updates[axis.name]!;
        this._position[axis.name] = Math.max(
          axis.min,
          Math.min(value, axis.max),
        );
      }
    }
  }

  /** Set all axis positions at once (e.g., from trajectory interpolation). */
  setPosition(pos: Position): void {
    for (const axis of this._axes) {
      if (axis.name in pos) {
        this._position[axis.name] = Math.max(
          axis.min,
          Math.min(pos[axis.name]!, axis.max),
        );
      }
    }
  }

  setTrajectoryLoaded(id: string | null): void {
    this._trajectoryLoaded = id;
  }

  /** Check if a given set of axis names matches the profile. */
  validateAxes(axisNames: string[]): string | null {
    const known = new Set(this._axes.map((a) => a.name));
    for (const name of axisNames) {
      if (!known.has(name)) return `Unknown axis: '${name}'`;
    }
    return null;
  }

  /** Check if axis values are within limits. Returns error string or null. */
  validateAxisValues(values: Record<string, number>): string | null {
    for (const axis of this._axes) {
      if (axis.name in values) {
        const v = values[axis.name]!;
        if (v < axis.min || v > axis.max) {
          return `Axis '${axis.name}' value ${v} out of range [${axis.min}, ${axis.max}]`;
        }
      }
    }
    return null;
  }
}
