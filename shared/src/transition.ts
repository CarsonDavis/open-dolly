/** A point on a progress curve. Both values are in normalized [0, 1] space. */
export interface ProgressCurvePoint {
  /** Normalized time within the transition [0, 1]. */
  t: number;
  /** Normalized progress toward the target position [0, 1]. */
  progress: number;
}

/** Per-axis progress curve definition for a transition. */
export interface AxisCurve {
  /** Interior control points between the implicit (0,0) and (1,1) endpoints.
   *  Empty array = linear. Points must be sorted by t, ascending. */
  points: ProgressCurvePoint[];
}

/** Buffer configuration for a transition. */
export interface BufferConfig {
  enabled: boolean;
  /** Hold time before motion starts, in ms. Default: 1000. */
  pre_ms: number;
  /** Hold time after motion ends, in ms. Default: 1000. */
  post_ms: number;
}

/** A transition between two adjacent keyframes. */
export interface Transition {
  id: string;
  /** Duration of the transition in milliseconds. */
  duration_ms: number;
  /** Per-axis curve definitions. Axes not listed default to linear. */
  curves: Record<string, AxisCurve>;
  /** Optional per-transition buffer override. */
  buffer?: BufferConfig;
}
