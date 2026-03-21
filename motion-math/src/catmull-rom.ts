/**
 * Centripetal Catmull-Rom spline interpolation for scalar (linear) axes.
 *
 * Uses alpha = 0.5 (centripetal parameterization) to avoid cusps and
 * self-intersections. Ghost points at endpoints ensure zero-velocity
 * boundary conditions.
 */

interface Knot {
  t: number;     // normalized time in [0, 1]
  value: number;
}

const EPSILON = 1e-8;

/**
 * Build a centripetal Catmull-Rom spline from a sequence of (t, value) pairs,
 * then evaluate it at arbitrary parameter values.
 *
 * @param knots - Array of {t, value} where t is in [0, 1] (normalized time)
 *                and value is the axis position. Must be sorted by t, length >= 1.
 * @returns A function that takes t in [0, 1] and returns the interpolated value.
 */
export function createCatmullRomSpline(
  knots: Knot[],
): (t: number) => number {
  if (knots.length === 0) {
    throw new Error("Catmull-Rom spline requires at least 1 knot");
  }

  // Single knot: constant value
  if (knots.length === 1) {
    const v = knots[0].value;
    return () => v;
  }

  // Two knots: linear interpolation
  if (knots.length === 2) {
    const [a, b] = knots;
    return (t: number) => {
      if (t <= a.t) return a.value;
      if (t >= b.t) return b.value;
      const frac = (t - a.t) / (b.t - a.t);
      return a.value + frac * (b.value - a.value);
    };
  }

  // 3+ knots: full Catmull-Rom
  // Add ghost points by reflecting across endpoints
  const ghost0: Knot = {
    t: knots[0].t - (knots[1].t - knots[0].t),
    value: 2 * knots[0].value - knots[1].value,
  };
  const ghostN: Knot = {
    t: knots[knots.length - 1].t + (knots[knots.length - 1].t - knots[knots.length - 2].t),
    value: 2 * knots[knots.length - 1].value - knots[knots.length - 2].value,
  };

  const extended = [ghost0, ...knots, ghostN];

  return (t: number) => {
    // Clamp
    if (t <= knots[0].t) return knots[0].value;
    if (t >= knots[knots.length - 1].t) return knots[knots.length - 1].value;

    // Find the segment: knots[segIdx] <= t < knots[segIdx+1]
    let segIdx = 0;
    for (let i = 0; i < knots.length - 1; i++) {
      if (t >= knots[i].t && t < knots[i + 1].t) {
        segIdx = i;
        break;
      }
    }

    // The four control points in extended array:
    // P0 = extended[segIdx], P1 = extended[segIdx+1],
    // P2 = extended[segIdx+2], P3 = extended[segIdx+3]
    const P0 = extended[segIdx];
    const P1 = extended[segIdx + 1];
    const P2 = extended[segIdx + 2];
    const P3 = extended[segIdx + 3];

    return evalCentripetalSegment(P0, P1, P2, P3, t);
  };
}

/**
 * Evaluate a single centripetal Catmull-Rom segment using the
 * Barry-Goldman formulation.
 *
 * Interpolates between P1 and P2 using P0 and P3 as context.
 * The query parameter `t` should be in [P1.t, P2.t].
 */
function evalCentripetalSegment(
  P0: Knot, P1: Knot, P2: Knot, P3: Knot,
  t: number,
): number {
  // Centripetal parameterization (alpha = 0.5)
  const knotInterval = (a: Knot, b: Knot): number => {
    const d = Math.abs(b.value - a.value);
    return d < EPSILON ? EPSILON : Math.sqrt(d);
  };

  const t01 = knotInterval(P0, P1);
  const t12 = knotInterval(P1, P2);
  const t23 = knotInterval(P2, P3);

  // Cumulative knot values
  const k0 = 0;
  const k1 = k0 + t01;
  const k2 = k1 + t12;
  const k3 = k2 + t23;

  // Map the query t from [P1.t, P2.t] to [k1, k2]
  const segFrac = P2.t - P1.t;
  const u = segFrac < EPSILON
    ? k1
    : k1 + ((t - P1.t) / segFrac) * (k2 - k1);

  // Barry-Goldman: three levels of linear interpolation
  const A1 = P0.value * (k1 - u) / (k1 - k0) + P1.value * (u - k0) / (k1 - k0);
  const A2 = P1.value * (k2 - u) / (k2 - k1) + P2.value * (u - k1) / (k2 - k1);
  const A3 = P2.value * (k3 - u) / (k3 - k2) + P3.value * (u - k2) / (k3 - k2);

  const B1 = A1 * (k2 - u) / (k2 - k0) + A2 * (u - k0) / (k2 - k0);
  const B2 = A2 * (k3 - u) / (k3 - k1) + A3 * (u - k1) / (k3 - k1);

  const C = B1 * (k2 - u) / (k2 - k1) + B2 * (u - k1) / (k2 - k1);

  return C;
}
