import { describe, it, expect } from "vitest";
import { createEasingFunction, cubicBezier } from "./easing.js";

describe("cubicBezier", () => {
  it("returns 0 at t=0 and 1 at t=1 for all presets", () => {
    const presets = [
      "linear", "easeIn", "easeOut", "easeInOut",
      "easeInCubic", "easeOutCubic", "easeInOutCubic",
    ] as const;
    for (const name of presets) {
      const fn = createEasingFunction(name);
      expect(fn(0)).toBe(0);
      expect(fn(1)).toBe(1);
    }
  });

  it("linear returns identity", () => {
    const fn = createEasingFunction("linear");
    for (let i = 0; i <= 10; i++) {
      const t = i / 10;
      expect(fn(t)).toBeCloseTo(t, 10);
    }
  });

  it("easeInOut is symmetric around (0.5, 0.5)", () => {
    const fn = createEasingFunction("easeInOut");
    expect(fn(0.5)).toBeCloseTo(0.5, 5);
    // f(t) + f(1-t) should equal 1
    for (const t of [0.1, 0.2, 0.3, 0.4]) {
      expect(fn(t) + fn(1 - t)).toBeCloseTo(1.0, 4);
    }
  });

  it("easeIn starts slow (below linear at t=0.25)", () => {
    const fn = createEasingFunction("easeIn");
    expect(fn(0.25)).toBeLessThan(0.25);
  });

  it("easeOut starts fast (above linear at t=0.25)", () => {
    const fn = createEasingFunction("easeOut");
    expect(fn(0.25)).toBeGreaterThan(0.25);
  });

  it("easeInOut at t=0.25 is well below midpoint", () => {
    // CSS cubic-bezier(0.42, 0, 0.58, 1) at t=0.25 ≈ 0.129
    const fn = createEasingFunction("easeInOut");
    expect(fn(0.25)).toBeCloseTo(0.1292, 2);
    expect(fn(0.25)).toBeLessThan(0.25);
  });

  it("easeInOut at t=0.75 is well above midpoint", () => {
    // Symmetric: f(0.75) ≈ 1 - f(0.25) ≈ 0.871
    const fn = createEasingFunction("easeInOut");
    expect(fn(0.75)).toBeCloseTo(0.8708, 2);
    expect(fn(0.75)).toBeGreaterThan(0.75);
  });

  it("standard presets are monotonically non-decreasing", () => {
    const presets = [
      "easeIn", "easeOut", "easeInOut",
      "easeInCubic", "easeOutCubic", "easeInOutCubic",
    ] as const;
    for (const name of presets) {
      const fn = createEasingFunction(name);
      let prev = 0;
      for (let i = 1; i <= 1000; i++) {
        const t = i / 1000;
        const v = fn(t);
        expect(v).toBeGreaterThanOrEqual(prev - 1e-10);
        prev = v;
      }
    }
  });

  it("accepts custom bezier params", () => {
    const fn = createEasingFunction({ x1: 0.42, y1: 0.0, x2: 0.58, y2: 1.0 });
    expect(fn(0)).toBe(0);
    expect(fn(1)).toBe(1);
    expect(fn(0.5)).toBeCloseTo(0.5, 3);
  });

  it("handles edge: t < 0 returns 0, t > 1 returns 1", () => {
    expect(cubicBezier(0.42, 0, 0.58, 1, -0.1)).toBe(0);
    expect(cubicBezier(0.42, 0, 0.58, 1, 1.5)).toBe(1);
  });

  it("throws on unknown preset", () => {
    expect(() => createEasingFunction("bogus" as any)).toThrow("Unknown easing preset");
  });
});
