import { describe, it, expect } from "vitest";
import { createProgressCurveFunction } from "./progress-curve.js";
import { createEasingFunction } from "./easing.js";
import { generateTrajectory } from "./trajectory.js";
import type { ProgressCurvePoint } from "./types.js";

describe("createProgressCurveFunction", () => {
  // ── Linear (empty points) ──────────────────────────────────────

  it("returns identity function for empty points array", () => {
    const fn = createProgressCurveFunction([]);
    expect(fn(0)).toBe(0);
    expect(fn(0.25)).toBeCloseTo(0.25, 10);
    expect(fn(0.5)).toBeCloseTo(0.5, 10);
    expect(fn(0.75)).toBeCloseTo(0.75, 10);
    expect(fn(1)).toBe(1);
  });

  // ── Boundary enforcement ───────────────────────────────────────

  it("returns 0 at t=0 and 1 at t=1", () => {
    const points: ProgressCurvePoint[] = [
      { t: 0.3, progress: 0.1 },
      { t: 0.7, progress: 0.9 },
    ];
    const fn = createProgressCurveFunction(points);
    expect(fn(0)).toBe(0);
    expect(fn(1)).toBe(1);
  });

  it("clamps for out-of-range inputs", () => {
    const fn = createProgressCurveFunction([{ t: 0.5, progress: 0.5 }]);
    expect(fn(-0.5)).toBe(0);
    expect(fn(1.5)).toBe(1);
  });

  // ── Pass-through ───────────────────────────────────────────────

  it("passes through a single control point", () => {
    const fn = createProgressCurveFunction([{ t: 0.5, progress: 0.8 }]);
    expect(fn(0.5)).toBeCloseTo(0.8, 5);
  });

  it("passes through multiple control points", () => {
    const points: ProgressCurvePoint[] = [
      { t: 0.2, progress: 0.1 },
      { t: 0.5, progress: 0.5 },
      { t: 0.8, progress: 0.9 },
    ];
    const fn = createProgressCurveFunction(points);
    for (const p of points) {
      expect(fn(p.t)).toBeCloseTo(p.progress, 5);
    }
  });

  it("passes through midpoint at (0.5, 0.5) — near-linear", () => {
    const fn = createProgressCurveFunction([{ t: 0.5, progress: 0.5 }]);
    expect(fn(0.5)).toBeCloseTo(0.5, 5);
    // Should be close to linear
    expect(fn(0.25)).toBeCloseTo(0.25, 1);
    expect(fn(0.75)).toBeCloseTo(0.75, 1);
  });

  // ── Monotonicity ───────────────────────────────────────────────

  function assertMonotone(fn: (t: number) => number, label: string) {
    const samples = 1000;
    let prev = fn(0);
    for (let i = 1; i <= samples; i++) {
      const t = i / samples;
      const val = fn(t);
      expect(val).toBeGreaterThanOrEqual(
        prev - 1e-10,
      );
      prev = val;
    }
  }

  it("is monotone for a single point at (0.5, 0.8) — ease-out shape", () => {
    const fn = createProgressCurveFunction([{ t: 0.5, progress: 0.8 }]);
    assertMonotone(fn, "ease-out");
  });

  it("is monotone for a steep early rise at (0.1, 0.9)", () => {
    const fn = createProgressCurveFunction([{ t: 0.1, progress: 0.9 }]);
    assertMonotone(fn, "steep-early");
  });

  it("is monotone for a delayed start at (0.5, 0.0)", () => {
    const fn = createProgressCurveFunction([{ t: 0.5, progress: 0.0 }]);
    assertMonotone(fn, "delayed-start");
  });

  it("is monotone for an S-curve shape", () => {
    const fn = createProgressCurveFunction([
      { t: 0.3, progress: 0.05 },
      { t: 0.7, progress: 0.95 },
    ]);
    assertMonotone(fn, "s-curve");
  });

  it("is monotone for many control points", () => {
    const fn = createProgressCurveFunction([
      { t: 0.1, progress: 0.05 },
      { t: 0.3, progress: 0.2 },
      { t: 0.5, progress: 0.5 },
      { t: 0.7, progress: 0.8 },
      { t: 0.9, progress: 0.95 },
    ]);
    assertMonotone(fn, "many-points");
  });

  it("is monotone for delayed pan scenario — flat then ramp", () => {
    // Pan delayed until 50% through the transition
    const fn = createProgressCurveFunction([
      { t: 0.5, progress: 0.0 },
    ]);
    assertMonotone(fn, "delayed-pan");
    // Should be ~0 until 0.5, then ramp to 1
    expect(fn(0.25)).toBeCloseTo(0, 1);
  });

  // ── Output range ───────────────────────────────────────────────

  it("output is always in [0, 1]", () => {
    const extremeCurves = [
      [{ t: 0.1, progress: 0.9 }],
      [{ t: 0.9, progress: 0.1 }],
      [{ t: 0.01, progress: 0.99 }],
      [{ t: 0.5, progress: 0.0 }, { t: 0.51, progress: 1.0 }],
    ];
    for (const points of extremeCurves) {
      const fn = createProgressCurveFunction(points);
      for (let i = 0; i <= 100; i++) {
        const t = i / 100;
        const val = fn(t);
        expect(val).toBeGreaterThanOrEqual(0);
        expect(val).toBeLessThanOrEqual(1);
      }
    }
  });

  // ── Error handling ─────────────────────────────────────────────

  it("throws for duplicate t values", () => {
    expect(() =>
      createProgressCurveFunction([
        { t: 0.5, progress: 0.3 },
        { t: 0.5, progress: 0.7 },
      ]),
    ).toThrow("strictly ascending");
  });

  it("throws for non-ascending t values", () => {
    expect(() =>
      createProgressCurveFunction([
        { t: 0.7, progress: 0.7 },
        { t: 0.3, progress: 0.3 },
      ]),
    ).toThrow("strictly ascending");
  });

  // ── Comparison with easing presets ─────────────────────────────

  it("approximates easeInOut when given matching control points", () => {
    // Sample the easeInOut preset at a few points to get control points
    const easeInOut = createEasingFunction("easeInOut");

    // Use control points that approximate the easeInOut curve
    const fn = createProgressCurveFunction([
      { t: 0.25, progress: easeInOut(0.25) },
      { t: 0.5, progress: easeInOut(0.5) },
      { t: 0.75, progress: easeInOut(0.75) },
    ]);

    // Should be reasonably close at intermediate points
    for (const t of [0.1, 0.2, 0.3, 0.4, 0.6, 0.7, 0.8, 0.9]) {
      expect(fn(t)).toBeCloseTo(easeInOut(t), 1);
    }
  });
});

// ── Integration with generateTrajectory ──────────────────────────

describe("generateTrajectory with ProgressCurve", () => {
  it("produces valid trajectory with ProgressCurve easing", () => {
    const trajectory = generateTrajectory({
      keyframes: [
        { t: 0, positions: { slide: 0 } },
        { t: 5000, positions: { slide: 500 } },
      ],
      axisTypes: { slide: "linear" },
      axes: {
        slide: {
          easing: {
            points: [{ t: 0.5, progress: 0.8 }],
          },
        },
      },
      sample_interval_ms: 10,
    });

    expect(trajectory.points.length).toBeGreaterThan(0);
    // First point at slide=0, last at slide=500
    expect(trajectory.points[0].slide).toBe(0);
    expect(trajectory.points[trajectory.points.length - 1].slide).toBe(500);
    // Times ascending
    for (let i = 1; i < trajectory.points.length; i++) {
      expect(trajectory.points[i].t).toBeGreaterThan(
        trajectory.points[i - 1].t,
      );
    }
  });

  it("produces same output as linear when ProgressCurve has no points", () => {
    const linearTrajectory = generateTrajectory({
      keyframes: [
        { t: 0, positions: { slide: 0 } },
        { t: 2000, positions: { slide: 200 } },
      ],
      axisTypes: { slide: "linear" },
      axes: { slide: { easing: "linear" } },
      sample_interval_ms: 100,
    });

    const progressTrajectory = generateTrajectory({
      keyframes: [
        { t: 0, positions: { slide: 0 } },
        { t: 2000, positions: { slide: 200 } },
      ],
      axisTypes: { slide: "linear" },
      axes: { slide: { easing: { points: [] } } },
      sample_interval_ms: 100,
    });

    expect(progressTrajectory.points.length).toBe(linearTrajectory.points.length);
    for (let i = 0; i < linearTrajectory.points.length; i++) {
      expect(progressTrajectory.points[i].slide).toBeCloseTo(
        linearTrajectory.points[i].slide,
        1,
      );
    }
  });

  it("ease-out curve makes slide values higher at midpoint", () => {
    // Ease-out: fast start, slow end → at midpoint time, should be past midpoint position
    const easeOutTrajectory = generateTrajectory({
      keyframes: [
        { t: 0, positions: { slide: 0 } },
        { t: 2000, positions: { slide: 200 } },
      ],
      axisTypes: { slide: "linear" },
      axes: {
        slide: {
          easing: { points: [{ t: 0.5, progress: 0.8 }] },
        },
      },
      sample_interval_ms: 100,
    });

    // At t=1000 (midpoint), slide should be > 100 (linear midpoint)
    const midpoint = easeOutTrajectory.points.find((p) => p.t === 1000);
    expect(midpoint).toBeDefined();
    expect(midpoint!.slide).toBeGreaterThan(100);
  });

  it("works with rotation axes and ProgressCurve", () => {
    const trajectory = generateTrajectory({
      keyframes: [
        { t: 0, positions: { pan: 0, tilt: 0, roll: 0 } },
        { t: 3000, positions: { pan: 90, tilt: -30, roll: 0 } },
      ],
      axisTypes: { pan: "rotation", tilt: "rotation", roll: "rotation" },
      axes: {
        pan: { easing: { points: [{ t: 0.5, progress: 0.2 }] } },
        tilt: { easing: { points: [{ t: 0.5, progress: 0.2 }] } },
        roll: { easing: { points: [{ t: 0.5, progress: 0.2 }] } },
      },
      sample_interval_ms: 100,
    });

    expect(trajectory.points.length).toBeGreaterThan(0);
    expect(trajectory.points[0].pan).toBe(0);
    expect(trajectory.points[trajectory.points.length - 1].pan).toBe(90);
  });
});
