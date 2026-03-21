import { describe, it, expect } from "vitest";
import {
  eulerToQuaternion,
  quaternionToEuler,
  slerp,
  createSquadInterpolator,
} from "./squad.js";
import type { EulerAngles, Quaternion } from "./types.js";

function qMag(q: Quaternion): number {
  return Math.sqrt(q.w * q.w + q.x * q.x + q.y * q.y + q.z * q.z);
}

describe("eulerToQuaternion / quaternionToEuler", () => {
  it("identity: (0,0,0) -> (1,0,0,0)", () => {
    const q = eulerToQuaternion({ pan: 0, tilt: 0, roll: 0 });
    expect(q.w).toBeCloseTo(1, 10);
    expect(q.x).toBeCloseTo(0, 10);
    expect(q.y).toBeCloseTo(0, 10);
    expect(q.z).toBeCloseTo(0, 10);
  });

  it("round-trips through various angles", () => {
    const cases: EulerAngles[] = [
      { pan: 0, tilt: 0, roll: 0 },
      { pan: 90, tilt: 0, roll: 0 },
      { pan: 0, tilt: 45, roll: 0 },
      { pan: 0, tilt: 0, roll: 30 },
      { pan: 45, tilt: -10, roll: 5 },
      { pan: -120, tilt: 60, roll: -20 },
      { pan: 180, tilt: 0, roll: 0 },
    ];
    for (const euler of cases) {
      const q = eulerToQuaternion(euler);
      const back = quaternionToEuler(q);
      expect(back.pan).toBeCloseTo(euler.pan, 5);
      expect(back.tilt).toBeCloseTo(euler.tilt, 5);
      expect(back.roll).toBeCloseTo(euler.roll, 5);
    }
  });

  it("produces unit quaternions", () => {
    const cases: EulerAngles[] = [
      { pan: 45, tilt: -10, roll: 5 },
      { pan: 170, tilt: 80, roll: -45 },
      { pan: 0, tilt: 89, roll: 0 },
    ];
    for (const euler of cases) {
      const q = eulerToQuaternion(euler);
      expect(qMag(q)).toBeCloseTo(1, 10);
    }
  });
});

describe("slerp", () => {
  it("endpoints are exact", () => {
    const q0 = eulerToQuaternion({ pan: 0, tilt: 0, roll: 0 });
    const q1 = eulerToQuaternion({ pan: 90, tilt: 0, roll: 0 });
    const r0 = slerp(q0, q1, 0);
    const r1 = slerp(q0, q1, 1);
    expect(r0.w).toBeCloseTo(q0.w, 10);
    expect(r0.x).toBeCloseTo(q0.x, 10);
    expect(r1.w).toBeCloseTo(q1.w, 10);
    expect(r1.z).toBeCloseTo(q1.z, 10);
  });

  it("midpoint of 0->90 pan is 45", () => {
    const q0 = eulerToQuaternion({ pan: 0, tilt: 0, roll: 0 });
    const q1 = eulerToQuaternion({ pan: 90, tilt: 0, roll: 0 });
    const mid = slerp(q0, q1, 0.5);
    const euler = quaternionToEuler(mid);
    expect(euler.pan).toBeCloseTo(45, 3);
    expect(euler.tilt).toBeCloseTo(0, 3);
    expect(euler.roll).toBeCloseTo(0, 3);
  });

  it("output is always unit quaternion", () => {
    const q0 = eulerToQuaternion({ pan: 30, tilt: -20, roll: 10 });
    const q1 = eulerToQuaternion({ pan: 150, tilt: 40, roll: -30 });
    for (let i = 0; i <= 20; i++) {
      const t = i / 20;
      const q = slerp(q0, q1, t);
      expect(qMag(q)).toBeCloseTo(1, 10);
    }
  });

  it("takes shortest path (handles antipodal quaternions)", () => {
    const q0 = eulerToQuaternion({ pan: 0, tilt: 0, roll: 0 });
    // 170 degrees: close to 180, should still take short path
    const q1 = eulerToQuaternion({ pan: 170, tilt: 0, roll: 0 });
    const mid = slerp(q0, q1, 0.5);
    const euler = quaternionToEuler(mid);
    expect(euler.pan).toBeCloseTo(85, 1);
  });
});

describe("createSquadInterpolator", () => {
  it("single knot returns constant", () => {
    const interp = createSquadInterpolator([
      { t: 0, angles: { pan: 45, tilt: -10, roll: 5 } },
    ]);
    const r = interp(0.5);
    expect(r.pan).toBeCloseTo(45, 5);
    expect(r.tilt).toBeCloseTo(-10, 5);
    expect(r.roll).toBeCloseTo(5, 5);
  });

  it("two knots: linear pan 0->90 (plan test 1 rotation check)", () => {
    const interp = createSquadInterpolator([
      { t: 0, angles: { pan: 0, tilt: 0, roll: 0 } },
      { t: 1, angles: { pan: 90, tilt: 0, roll: 0 } },
    ]);
    expect(interp(0).pan).toBeCloseTo(0, 3);
    expect(interp(0.25).pan).toBeCloseTo(22.5, 1);
    expect(interp(0.5).pan).toBeCloseTo(45, 1);
    expect(interp(0.75).pan).toBeCloseTo(67.5, 1);
    expect(interp(1).pan).toBeCloseTo(90, 3);
  });

  it("passes through all keyframes exactly (plan test 5)", () => {
    const interp = createSquadInterpolator([
      { t: 0, angles: { pan: 0, tilt: 0, roll: 0 } },
      { t: 0.5, angles: { pan: 90, tilt: 30, roll: 0 } },
      { t: 1, angles: { pan: 180, tilt: 0, roll: 0 } },
    ]);
    const r0 = interp(0);
    expect(r0.pan).toBeCloseTo(0, 2);
    expect(r0.tilt).toBeCloseTo(0, 2);

    const r1 = interp(0.5);
    expect(r1.pan).toBeCloseTo(90, 1);
    expect(r1.tilt).toBeCloseTo(30, 1);

    const r2 = interp(1);
    expect(r2.pan).toBeCloseTo(180, 1);
    expect(r2.tilt).toBeCloseTo(0, 1);
  });

  it("smooth: no large jumps between adjacent samples", () => {
    // Stay within ±90° pan to avoid Euler wrapping at ±180°
    const interp = createSquadInterpolator([
      { t: 0, angles: { pan: 0, tilt: 0, roll: 0 } },
      { t: 0.333, angles: { pan: 30, tilt: 20, roll: 5 } },
      { t: 0.666, angles: { pan: 60, tilt: 0, roll: -5 } },
      { t: 1, angles: { pan: 90, tilt: -10, roll: 0 } },
    ]);

    const steps = 500;
    let prev = interp(0);
    for (let i = 1; i <= steps; i++) {
      const t = i / steps;
      const cur = interp(t);
      expect(Math.abs(cur.pan - prev.pan)).toBeLessThan(3);
      expect(Math.abs(cur.tilt - prev.tilt)).toBeLessThan(3);
      expect(Math.abs(cur.roll - prev.roll)).toBeLessThan(3);
      prev = cur;
    }
  });

  it("clamps outside range", () => {
    const interp = createSquadInterpolator([
      { t: 0, angles: { pan: 10, tilt: 20, roll: 30 } },
      { t: 1, angles: { pan: 50, tilt: 60, roll: 70 } },
    ]);
    const before = interp(-0.5);
    expect(before.pan).toBeCloseTo(10, 3);
    const after = interp(1.5);
    expect(after.pan).toBeCloseTo(50, 3);
  });
});
