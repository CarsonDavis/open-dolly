import type { EasingConfig, CubicBezierParams } from "./types.js";

/** Preset bezier control points (CSS cubic-bezier convention). */
const PRESETS: Record<string, CubicBezierParams> = {
  linear:         { x1: 0.0,  y1: 0.0, x2: 1.0,  y2: 1.0 },
  easeIn:         { x1: 0.42, y1: 0.0, x2: 1.0,  y2: 1.0 },
  easeOut:        { x1: 0.0,  y1: 0.0, x2: 0.58, y2: 1.0 },
  easeInOut:      { x1: 0.42, y1: 0.0, x2: 0.58, y2: 1.0 },
  easeInCubic:    { x1: 0.32, y1: 0.0, x2: 0.67, y2: 0.0 },
  easeOutCubic:   { x1: 0.33, y1: 1.0, x2: 0.68, y2: 1.0 },
  easeInOutCubic: { x1: 0.65, y1: 0.0, x2: 0.35, y2: 1.0 },
};

/**
 * Evaluate a cubic bezier easing curve.
 *
 * Given the CSS cubic-bezier control points and an input time t in [0,1],
 * returns the eased value. Uses Newton-Raphson to solve for the bezier
 * parameter, with binary search fallback.
 */
export function cubicBezier(
  x1: number,
  y1: number,
  x2: number,
  y2: number,
  t: number,
): number {
  // Boundary: exact endpoints
  if (t <= 0) return 0;
  if (t >= 1) return 1;

  // Special case: linear
  if (x1 === 0 && y1 === 0 && x2 === 1 && y2 === 1) return t;

  // Bezier x(s) = 3(1-s)^2 * s * x1 + 3(1-s) * s^2 * x2 + s^3
  const bx = (s: number) => {
    const inv = 1 - s;
    return 3 * inv * inv * s * x1 + 3 * inv * s * s * x2 + s * s * s;
  };

  // Derivative dx/ds
  const dbx = (s: number) => {
    const inv = 1 - s;
    return 3 * inv * inv * x1 + 6 * inv * s * (x2 - x1) + 3 * s * s * (1 - x2);
  };

  // Bezier y(s) = 3(1-s)^2 * s * y1 + 3(1-s) * s^2 * y2 + s^3
  const by = (s: number) => {
    const inv = 1 - s;
    return 3 * inv * inv * s * y1 + 3 * inv * s * s * y2 + s * s * s;
  };

  // Step 1: Solve for s such that bx(s) = t using Newton-Raphson
  let s = t; // initial guess
  for (let i = 0; i < 8; i++) {
    const xErr = bx(s) - t;
    if (Math.abs(xErr) < 1e-7) break;
    const dx = dbx(s);
    if (Math.abs(dx) < 1e-12) break;
    s = s - xErr / dx;
    if (s < 0) s = 0;
    if (s > 1) s = 1;
  }

  // Fallback: binary search if Newton didn't converge
  if (Math.abs(bx(s) - t) > 1e-5) {
    let lo = 0;
    let hi = 1;
    s = t;
    for (let i = 0; i < 20; i++) {
      const mid = (lo + hi) / 2;
      const xMid = bx(mid);
      if (Math.abs(xMid - t) < 1e-7) {
        s = mid;
        break;
      }
      if (xMid < t) {
        lo = mid;
      } else {
        hi = mid;
      }
      s = mid;
    }
  }

  // Step 2: Evaluate y at the solved s
  return by(s);
}

/**
 * Create an easing function from a config (preset name or custom bezier).
 *
 * @returns A function mapping t in [0,1] to eased_t in [0,1].
 *          Guaranteed: f(0) = 0, f(1) = 1.
 */
export function createEasingFunction(config: EasingConfig): (t: number) => number {
  if (typeof config === "string") {
    const preset = PRESETS[config];
    if (!preset) {
      throw new Error(`Unknown easing preset: ${config}`);
    }
    // Fast path for linear
    if (config === "linear") return (t: number) => t;
    const { x1, y1, x2, y2 } = preset;
    return (t: number) => cubicBezier(x1, y1, x2, y2, t);
  }

  const { x1, y1, x2, y2 } = config;
  return (t: number) => cubicBezier(x1, y1, x2, y2, t);
}
