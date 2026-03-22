/**
 * Monotone piecewise cubic Hermite interpolation (PCHIP) for progress curves.
 *
 * Maps normalized time [0, 1] to normalized progress [0, 1].
 * Guarantees: passes through all control points, monotone non-decreasing,
 * f(0) = 0, f(1) = 1, output clamped to [0, 1].
 *
 * Uses Fritsch-Carlson method for monotonicity enforcement.
 */

import type { ProgressCurvePoint } from "./types.js";

/**
 * Create a monotone progress curve function from interior control points.
 *
 * @param interiorPoints - Control points between the implicit endpoints (0,0) and (1,1).
 *   Must be sorted by t ascending. Each point must have 0 < t < 1 and 0 <= progress <= 1.
 *   Empty array = linear (identity function).
 * @returns A function mapping t in [0,1] to progress in [0,1], guaranteed monotone non-decreasing.
 */
export function createProgressCurveFunction(
  interiorPoints: ProgressCurvePoint[],
): (t: number) => number {
  // Linear: no interior points
  if (interiorPoints.length === 0) {
    return (t: number) => clamp01(t);
  }

  // Build full knot array with implicit endpoints
  const knots: ProgressCurvePoint[] = [
    { t: 0, progress: 0 },
    ...interiorPoints,
    { t: 1, progress: 1 },
  ];

  const n = knots.length;

  // Compute interval slopes (delta)
  const delta: number[] = [];
  for (let i = 0; i < n - 1; i++) {
    const dt = knots[i + 1].t - knots[i].t;
    if (dt <= 0) {
      throw new Error(
        `Progress curve points must have strictly ascending t values: t[${i}]=${knots[i].t}, t[${i + 1}]=${knots[i + 1].t}`,
      );
    }
    delta.push((knots[i + 1].progress - knots[i].progress) / dt);
  }

  // Compute initial tangents using three-point formula
  const m: number[] = new Array(n);

  // Endpoint tangents: use one-sided differences
  m[0] = delta[0];
  m[n - 1] = delta[n - 2];

  // Interior tangents: average of adjacent slopes
  for (let i = 1; i < n - 1; i++) {
    if (delta[i - 1] * delta[i] <= 0) {
      // Adjacent slopes have different signs or one is zero — set tangent to 0
      m[i] = 0;
    } else {
      m[i] = (delta[i - 1] + delta[i]) / 2;
    }
  }

  // Fritsch-Carlson monotonicity enforcement
  for (let i = 0; i < n - 1; i++) {
    if (Math.abs(delta[i]) < 1e-12) {
      // Flat segment: both endpoint tangents must be zero
      m[i] = 0;
      m[i + 1] = 0;
    } else {
      const alpha = m[i] / delta[i];
      const beta = m[i + 1] / delta[i];

      // Check Fritsch-Carlson condition: alpha^2 + beta^2 <= 9
      const r2 = alpha * alpha + beta * beta;
      if (r2 > 9) {
        const tau = 3 / Math.sqrt(r2);
        m[i] = tau * alpha * delta[i];
        m[i + 1] = tau * beta * delta[i];
      }
    }
  }

  // Return evaluator function
  return (t: number): number => {
    // Boundary handling
    if (t <= 0) return 0;
    if (t >= 1) return 1;

    // Find segment: knots[seg] <= t < knots[seg+1]
    let seg = 0;
    for (let i = 0; i < n - 1; i++) {
      if (t >= knots[i].t && t < knots[i + 1].t) {
        seg = i;
        break;
      }
    }

    // Evaluate cubic Hermite basis
    const h = knots[seg + 1].t - knots[seg].t;
    const s = (t - knots[seg].t) / h;
    const s2 = s * s;
    const s3 = s2 * s;

    // Hermite basis functions
    const h00 = 2 * s3 - 3 * s2 + 1;
    const h10 = s3 - 2 * s2 + s;
    const h01 = -2 * s3 + 3 * s2;
    const h11 = s3 - s2;

    const p0 = knots[seg].progress;
    const p1 = knots[seg + 1].progress;

    const result =
      h00 * p0 + h10 * h * m[seg] + h01 * p1 + h11 * h * m[seg + 1];

    return clamp01(result);
  };
}

function clamp01(v: number): number {
  if (v < 0) return 0;
  if (v > 1) return 1;
  return v;
}
