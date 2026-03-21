import { describe, it, expect } from "vitest";
import { createCatmullRomSpline } from "./catmull-rom.js";

describe("createCatmullRomSpline", () => {
  it("single knot returns constant", () => {
    const spline = createCatmullRomSpline([{ t: 0, value: 42 }]);
    expect(spline(0)).toBe(42);
    expect(spline(0.5)).toBe(42);
    expect(spline(1)).toBe(42);
  });

  it("two knots: linear interpolation", () => {
    const spline = createCatmullRomSpline([
      { t: 0, value: 0 },
      { t: 1, value: 100 },
    ]);
    expect(spline(0)).toBe(0);
    expect(spline(0.25)).toBeCloseTo(25, 5);
    expect(spline(0.5)).toBeCloseTo(50, 5);
    expect(spline(0.75)).toBeCloseTo(75, 5);
    expect(spline(1)).toBe(100);
  });

  it("passes through all keyframe values exactly (3 knots)", () => {
    const spline = createCatmullRomSpline([
      { t: 0, value: 0 },
      { t: 0.5, value: 300 },
      { t: 1, value: 1000 },
    ]);
    expect(spline(0)).toBeCloseTo(0, 5);
    expect(spline(0.5)).toBeCloseTo(300, 5);
    expect(spline(1)).toBeCloseTo(1000, 5);
  });

  it("passes through all keyframe values exactly (4 knots, plan test 4)", () => {
    // Plan Test 4: slide axis with 4 keyframes
    const spline = createCatmullRomSpline([
      { t: 0, value: 0 },
      { t: 1 / 3, value: 300 },
      { t: 2 / 3, value: 400 },
      { t: 1, value: 1000 },
    ]);
    expect(spline(0)).toBeCloseTo(0, 5);
    expect(spline(1 / 3)).toBeCloseTo(300, 3);
    expect(spline(2 / 3)).toBeCloseTo(400, 3);
    expect(spline(1)).toBeCloseTo(1000, 5);
  });

  it("mid-segment values differ from linear (showing curve smoothing)", () => {
    const spline = createCatmullRomSpline([
      { t: 0, value: 0 },
      { t: 1 / 3, value: 300 },
      { t: 2 / 3, value: 400 },
      { t: 1, value: 1000 },
    ]);
    // Between knots 0 and 1: linear would give 150 at t=1/6
    // Catmull-Rom should differ due to curvature
    const midVal = spline(1 / 6);
    expect(midVal).not.toBeCloseTo(150, 0);
    // But it should be in a reasonable range
    expect(midVal).toBeGreaterThan(50);
    expect(midVal).toBeLessThan(250);
  });

  it("clamps to boundary values outside range", () => {
    const spline = createCatmullRomSpline([
      { t: 0, value: 10 },
      { t: 0.5, value: 50 },
      { t: 1, value: 90 },
    ]);
    expect(spline(-0.5)).toBe(10);
    expect(spline(1.5)).toBe(90);
  });

  it("handles constant value across all knots", () => {
    const spline = createCatmullRomSpline([
      { t: 0, value: 42 },
      { t: 0.5, value: 42 },
      { t: 1, value: 42 },
    ]);
    for (let i = 0; i <= 10; i++) {
      expect(spline(i / 10)).toBeCloseTo(42, 5);
    }
  });

  it("continuity: adjacent samples don't jump wildly (5 knots)", () => {
    const spline = createCatmullRomSpline([
      { t: 0, value: 0 },
      { t: 0.25, value: 100 },
      { t: 0.5, value: 80 },
      { t: 0.75, value: 200 },
      { t: 1, value: 150 },
    ]);
    const samples = 1000;
    let prev = spline(0);
    for (let i = 1; i <= samples; i++) {
      const t = i / samples;
      const v = spline(t);
      // Max delta per step should be reasonable
      expect(Math.abs(v - prev)).toBeLessThan(5);
      prev = v;
    }
  });

  it("throws on empty knots", () => {
    expect(() => createCatmullRomSpline([])).toThrow();
  });
});
