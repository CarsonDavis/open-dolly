/**
 * SQUAD quaternion interpolation for smooth rotation through keyframes.
 *
 * Converts Euler angles (degrees, ZYX intrinsic) to quaternions,
 * interpolates via SQUAD (Spherical and Quadrangle), and converts back.
 */

import type { Quaternion, EulerAngles } from "./types.js";

const DEG2RAD = Math.PI / 180;
const RAD2DEG = 180 / Math.PI;
const SLERP_THRESHOLD = 0.9995;
const EPSILON = 1e-8;

// ── Euler ↔ Quaternion conversion (ZYX intrinsic) ──────────────────

/**
 * Convert Euler angles (degrees, ZYX convention) to a unit quaternion.
 */
export function eulerToQuaternion(euler: EulerAngles): Quaternion {
  const psi = euler.pan * DEG2RAD;   // Z / yaw
  const theta = euler.tilt * DEG2RAD; // Y / pitch
  const phi = euler.roll * DEG2RAD;   // X / roll

  const cPsi = Math.cos(psi / 2);
  const sPsi = Math.sin(psi / 2);
  const cTheta = Math.cos(theta / 2);
  const sTheta = Math.sin(theta / 2);
  const cPhi = Math.cos(phi / 2);
  const sPhi = Math.sin(phi / 2);

  return {
    w: cPsi * cTheta * cPhi + sPsi * sTheta * sPhi,
    x: cPsi * cTheta * sPhi - sPsi * sTheta * cPhi,
    y: cPsi * sTheta * cPhi + sPsi * cTheta * sPhi,
    z: sPsi * cTheta * cPhi - cPsi * sTheta * sPhi,
  };
}

/**
 * Convert a unit quaternion back to Euler angles (degrees, ZYX convention).
 */
export function quaternionToEuler(q: Quaternion): EulerAngles {
  // Roll (X)
  const sinrCosp = 2 * (q.w * q.x + q.y * q.z);
  const cosrCosp = 1 - 2 * (q.x * q.x + q.y * q.y);
  const roll = Math.atan2(sinrCosp, cosrCosp);

  // Pitch (Y) — clamp to avoid NaN
  const sinp = 2 * (q.w * q.y - q.z * q.x);
  let pitch: number;
  if (Math.abs(sinp) >= 1) {
    pitch = Math.sign(sinp) * (Math.PI / 2); // gimbal lock
  } else {
    pitch = Math.asin(sinp);
  }

  // Yaw (Z)
  const sinyCosp = 2 * (q.w * q.z + q.x * q.y);
  const cosyCosp = 1 - 2 * (q.y * q.y + q.z * q.z);
  const yaw = Math.atan2(sinyCosp, cosyCosp);

  return {
    pan: yaw * RAD2DEG,
    tilt: pitch * RAD2DEG,
    roll: roll * RAD2DEG,
  };
}

// ── Quaternion math ────────────────────────────────────────────────

function qDot(a: Quaternion, b: Quaternion): number {
  return a.w * b.w + a.x * b.x + a.y * b.y + a.z * b.z;
}

function qNormalize(q: Quaternion): Quaternion {
  const mag = Math.sqrt(q.w * q.w + q.x * q.x + q.y * q.y + q.z * q.z);
  if (mag < EPSILON) return { w: 1, x: 0, y: 0, z: 0 };
  return { w: q.w / mag, x: q.x / mag, y: q.y / mag, z: q.z / mag };
}

function qNegate(q: Quaternion): Quaternion {
  return { w: -q.w, x: -q.x, y: -q.y, z: -q.z };
}

function qConjugate(q: Quaternion): Quaternion {
  return { w: q.w, x: -q.x, y: -q.y, z: -q.z };
}

function qMultiply(a: Quaternion, b: Quaternion): Quaternion {
  return {
    w: a.w * b.w - a.x * b.x - a.y * b.y - a.z * b.z,
    x: a.w * b.x + a.x * b.w + a.y * b.z - a.z * b.y,
    y: a.w * b.y - a.x * b.z + a.y * b.w + a.z * b.x,
    z: a.w * b.z + a.x * b.y - a.y * b.x + a.z * b.w,
  };
}

/**
 * Quaternion logarithm: for unit q = cos(θ) + sin(θ)·û, returns θ·û as [x,y,z].
 */
function qLog(q: Quaternion): [number, number, number] {
  // Clamp w to [-1, 1] for numerical safety
  const w = Math.max(-1, Math.min(1, q.w));
  const theta = Math.acos(w);
  const sinTheta = Math.sin(theta);
  if (Math.abs(sinTheta) < EPSILON) {
    return [0, 0, 0];
  }
  const s = theta / sinTheta;
  return [q.x * s, q.y * s, q.z * s];
}

/**
 * Quaternion exponential: for a 3-vector v with magnitude θ,
 * returns cos(θ) + sin(θ)·(v/θ).
 */
function qExp(v: [number, number, number]): Quaternion {
  const theta = Math.sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
  if (theta < EPSILON) {
    return { w: 1, x: 0, y: 0, z: 0 };
  }
  const s = Math.sin(theta) / theta;
  return {
    w: Math.cos(theta),
    x: v[0] * s,
    y: v[1] * s,
    z: v[2] * s,
  };
}

// ── SLERP ──────────────────────────────────────────────────────────

/**
 * Spherical linear interpolation between two quaternions.
 * Always takes the shortest path (corrects sign if needed).
 */
export function slerp(q0: Quaternion, q1: Quaternion, t: number): Quaternion {
  if (t <= 0) return q0;
  if (t >= 1) return q1;

  let cosOmega = qDot(q0, q1);

  // Shortest path correction
  let q1c = q1;
  if (cosOmega < 0) {
    q1c = qNegate(q1);
    cosOmega = -cosOmega;
  }

  // Near-identity: use NLERP
  if (cosOmega > SLERP_THRESHOLD) {
    return qNormalize({
      w: q0.w + t * (q1c.w - q0.w),
      x: q0.x + t * (q1c.x - q0.x),
      y: q0.y + t * (q1c.y - q0.y),
      z: q0.z + t * (q1c.z - q0.z),
    });
  }

  const omega = Math.acos(cosOmega);
  const sinOmega = Math.sin(omega);
  const s0 = Math.sin((1 - t) * omega) / sinOmega;
  const s1 = Math.sin(t * omega) / sinOmega;

  return {
    w: s0 * q0.w + s1 * q1c.w,
    x: s0 * q0.x + s1 * q1c.x,
    y: s0 * q0.y + s1 * q1c.y,
    z: s0 * q0.z + s1 * q1c.z,
  };
}

// ── SQUAD ──────────────────────────────────────────────────────────

interface SquadKnot {
  t: number;
  angles: EulerAngles;
}

/**
 * Build a SQUAD interpolation curve from a sequence of (t, euler) pairs.
 *
 * @param knots - Array of {t, angles} where t is in [0, 1] (normalized time)
 *                and angles is an EulerAngles object. Must be sorted by t, length >= 2.
 * @returns A function that takes t in [0, 1] and returns interpolated EulerAngles.
 */
export function createSquadInterpolator(
  knots: SquadKnot[],
): (t: number) => EulerAngles {
  if (knots.length < 2) {
    if (knots.length === 1) {
      const a = knots[0].angles;
      return () => ({ ...a });
    }
    throw new Error("SQUAD interpolator requires at least 1 knot");
  }

  // Convert all Euler angles to quaternions
  const quats = knots.map((k) => eulerToQuaternion(k.angles));

  // Ensure all quaternions are in the same hemisphere
  for (let i = 1; i < quats.length; i++) {
    if (qDot(quats[i - 1], quats[i]) < 0) {
      quats[i] = qNegate(quats[i]);
    }
  }

  // Two knots: use plain SLERP
  if (knots.length === 2) {
    const q0 = quats[0];
    const q1 = quats[1];
    const t0 = knots[0].t;
    const t1 = knots[1].t;
    return (t: number) => {
      if (t <= t0) return quaternionToEuler(q0);
      if (t >= t1) return quaternionToEuler(q1);
      const u = (t - t0) / (t1 - t0);
      return quaternionToEuler(slerp(q0, q1, u));
    };
  }

  // 3+ knots: compute SQUAD auxiliary quaternions (inner control points)
  const s: Quaternion[] = new Array(quats.length);
  s[0] = quats[0];
  s[quats.length - 1] = quats[quats.length - 1];

  for (let i = 1; i < quats.length - 1; i++) {
    const qi = quats[i];
    const qiInv = qConjugate(qi);

    const logNext = qLog(qMultiply(qiInv, quats[i + 1]));
    const logPrev = qLog(qMultiply(qiInv, quats[i - 1]));

    const sum: [number, number, number] = [
      -(logNext[0] + logPrev[0]) / 4,
      -(logNext[1] + logPrev[1]) / 4,
      -(logNext[2] + logPrev[2]) / 4,
    ];

    s[i] = qMultiply(qi, qExp(sum));
  }

  return (t: number) => {
    // Clamp
    if (t <= knots[0].t) return quaternionToEuler(quats[0]);
    if (t >= knots[knots.length - 1].t) return quaternionToEuler(quats[quats.length - 1]);

    // Find segment
    let segIdx = 0;
    for (let i = 0; i < knots.length - 1; i++) {
      if (t >= knots[i].t && t < knots[i + 1].t) {
        segIdx = i;
        break;
      }
    }

    const t0 = knots[segIdx].t;
    const t1 = knots[segIdx + 1].t;
    const u = (t - t0) / (t1 - t0);

    // SQUAD(qi, qi+1, si, si+1, u) =
    //   SLERP(SLERP(qi, qi+1, u), SLERP(si, si+1, u), 2u(1-u))
    const slerpMain = slerp(quats[segIdx], quats[segIdx + 1], u);
    const slerpAux = slerp(s[segIdx], s[segIdx + 1], u);
    const result = slerp(slerpMain, slerpAux, 2 * u * (1 - u));

    return quaternionToEuler(result);
  };
}
