import { describe, it, expect } from "vitest";
import { generateTrajectory } from "./trajectory.js";
import type { TrajectoryConfig } from "./types.js";

const FOUR_AXIS_TYPES = {
  pan: "rotation" as const,
  tilt: "rotation" as const,
  roll: "rotation" as const,
  slide: "linear" as const,
};

describe("generateTrajectory", () => {
  // ── Plan Test 1: Linear interpolation (2 keyframes, no easing) ──

  it("test 1: 2 keyframes, no easing, produces linear interpolation", () => {
    const config: TrajectoryConfig = {
      keyframes: [
        { t: 0, positions: { pan: 0, tilt: 0, roll: 0, slide: 0 } },
        { t: 1000, positions: { pan: 90, tilt: 0, roll: 0, slide: 500 } },
      ],
      axisTypes: FOUR_AXIS_TYPES,
      sample_interval_ms: 250,
    };

    const result = generateTrajectory(config);
    expect(result.points.length).toBe(5);

    // Check timestamps
    expect(result.points.map((p) => p.t)).toEqual([0, 250, 500, 750, 1000]);

    // Slide should be exactly linear (2 keyframes -> LERP)
    expect(result.points[0].slide).toBe(0);
    expect(result.points[1].slide).toBe(125);
    expect(result.points[2].slide).toBe(250);
    expect(result.points[3].slide).toBe(375);
    expect(result.points[4].slide).toBe(500);

    // Pan should be ~22.5° increments (SLERP for single-axis = linear in angle)
    expect(result.points[0].pan).toBeCloseTo(0, 0);
    expect(result.points[1].pan).toBeCloseTo(22.5, 0);
    expect(result.points[2].pan).toBeCloseTo(45, 0);
    expect(result.points[3].pan).toBeCloseTo(67.5, 0);
    expect(result.points[4].pan).toBeCloseTo(90, 0);
  });

  // ── Plan Test 2: Easing on slide ──

  it("test 2: easeInOut on slide", () => {
    const config: TrajectoryConfig = {
      keyframes: [
        { t: 0, positions: { pan: 0, tilt: 0, roll: 0, slide: 0 } },
        { t: 1000, positions: { pan: 0, tilt: 0, roll: 0, slide: 1000 } },
      ],
      axisTypes: FOUR_AXIS_TYPES,
      axes: {
        slide: { delay_ms: 0, duration_ms: 1000, easing: "easeInOut" },
      },
      sample_interval_ms: 250,
    };

    const result = generateTrajectory(config);
    expect(result.points.length).toBe(5);

    // easeInOut: starts slow, accelerates, decelerates
    expect(result.points[0].slide).toBe(0);
    expect(result.points[1].slide).toBeLessThan(250); // below linear at t=0.25
    expect(result.points[2].slide).toBeCloseTo(500, 0); // midpoint is exactly 0.5
    expect(result.points[3].slide).toBeGreaterThan(750); // above linear at t=0.75
    expect(result.points[4].slide).toBe(1000);
  });

  // ── Plan Test 3: Per-axis timing (staggered start) ──

  it("test 3: staggered pan and slide", () => {
    const config: TrajectoryConfig = {
      keyframes: [
        { t: 0, positions: { pan: 0, tilt: 0, roll: 0, slide: 0 } },
        { t: 2000, positions: { pan: 45, tilt: 0, roll: 0, slide: 200 } },
      ],
      axisTypes: FOUR_AXIS_TYPES,
      axes: {
        pan: { delay_ms: 0, duration_ms: 1000 },
        tilt: { delay_ms: 0, duration_ms: 2000 },
        roll: { delay_ms: 0, duration_ms: 2000 },
        slide: { delay_ms: 1000, duration_ms: 1000 },
      },
      sample_interval_ms: 500,
    };

    const result = generateTrajectory(config);
    expect(result.points.length).toBe(5);
    expect(result.points.map((p) => p.t)).toEqual([0, 500, 1000, 1500, 2000]);

    // Pan: done by t=1000
    expect(result.points[0].pan).toBeCloseTo(0, 0);
    expect(result.points[1].pan).toBeCloseTo(22.5, 0); // halfway through pan motion
    expect(result.points[2].pan).toBeCloseTo(45, 0);   // pan done
    expect(result.points[3].pan).toBeCloseTo(45, 0);   // holding
    expect(result.points[4].pan).toBeCloseTo(45, 0);   // holding

    // Slide: starts at t=1000
    expect(result.points[0].slide).toBeCloseTo(0, 0);
    expect(result.points[1].slide).toBeCloseTo(0, 0);   // waiting
    expect(result.points[2].slide).toBeCloseTo(0, 0);   // just starting
    expect(result.points[3].slide).toBeCloseTo(100, 0);  // halfway
    expect(result.points[4].slide).toBeCloseTo(200, 0);  // done
  });

  // ── Plan Test 4: 3+ keyframes with Catmull-Rom ──

  it("test 4: Catmull-Rom passes through all keyframes", () => {
    const config: TrajectoryConfig = {
      keyframes: [
        { t: 0, positions: { pan: 0, tilt: 0, roll: 0, slide: 0 } },
        { t: 1000, positions: { pan: 0, tilt: 0, roll: 0, slide: 300 } },
        { t: 2000, positions: { pan: 0, tilt: 0, roll: 0, slide: 400 } },
        { t: 3000, positions: { pan: 0, tilt: 0, roll: 0, slide: 1000 } },
      ],
      axisTypes: FOUR_AXIS_TYPES,
      sample_interval_ms: 1000,
    };

    const result = generateTrajectory(config);
    expect(result.points[0].slide).toBe(0);
    expect(result.points[1].slide).toBe(300);
    expect(result.points[2].slide).toBe(400);
    expect(result.points[3].slide).toBe(1000);
  });

  // ── Plan Test 5: SQUAD rotation through 3 keyframes ──

  it("test 5: SQUAD rotation passes through keyframes", () => {
    const config: TrajectoryConfig = {
      keyframes: [
        { t: 0, positions: { pan: 0, tilt: 0, roll: 0, slide: 0 } },
        { t: 1000, positions: { pan: 90, tilt: 30, roll: 0, slide: 0 } },
        { t: 2000, positions: { pan: 180, tilt: 0, roll: 0, slide: 0 } },
      ],
      axisTypes: FOUR_AXIS_TYPES,
      sample_interval_ms: 1000,
    };

    const result = generateTrajectory(config);
    expect(result.points[0].pan).toBeCloseTo(0, 0);
    expect(result.points[0].tilt).toBeCloseTo(0, 0);

    expect(result.points[1].pan).toBeCloseTo(90, 0);
    expect(result.points[1].tilt).toBeCloseTo(30, 0);

    expect(result.points[2].pan).toBeCloseTo(180, 0);
    expect(result.points[2].tilt).toBeCloseTo(0, 0);
  });

  // ── Property-based tests ──

  it("first and last points match first and last keyframe exactly", () => {
    const config: TrajectoryConfig = {
      keyframes: [
        { t: 0, positions: { pan: 12.3, tilt: -4.5, roll: 6.7, slide: 89.1 } },
        { t: 500, positions: { pan: 45.6, tilt: 10.0, roll: 0.0, slide: 234.5 } },
        { t: 1000, positions: { pan: 78.9, tilt: -20.0, roll: 3.3, slide: 456.7 } },
      ],
      axisTypes: FOUR_AXIS_TYPES,
      sample_interval_ms: 50,
    };

    const result = generateTrajectory(config);
    const first = result.points[0];
    const last = result.points[result.points.length - 1];

    expect(first.pan).toBe(12.3);
    expect(first.tilt).toBe(-4.5);
    expect(first.slide).toBe(89.1);

    expect(last.pan).toBe(78.9);
    expect(last.tilt).toBe(-20);
    expect(last.slide).toBe(456.7);
  });

  it("timestamps are strictly ascending", () => {
    const config: TrajectoryConfig = {
      keyframes: [
        { t: 0, positions: { slide: 0 } },
        { t: 500, positions: { slide: 100 } },
        { t: 1000, positions: { slide: 500 } },
      ],
      axisTypes: { slide: "linear" },
      sample_interval_ms: 7, // odd interval
    };

    const result = generateTrajectory(config);
    for (let i = 1; i < result.points.length; i++) {
      expect(result.points[i].t).toBeGreaterThan(result.points[i - 1].t);
    }
  });

  it("loop flag is passed through", () => {
    const config: TrajectoryConfig = {
      keyframes: [
        { t: 0, positions: { slide: 0 } },
        { t: 1000, positions: { slide: 100 } },
      ],
      axisTypes: { slide: "linear" },
      loop: true,
    };
    expect(generateTrajectory(config).loop).toBe(true);

    config.loop = false;
    expect(generateTrajectory(config).loop).toBe(false);
  });

  // ── Edge cases ──

  it("single keyframe produces 1 point at t=0", () => {
    const config: TrajectoryConfig = {
      keyframes: [{ t: 500, positions: { pan: 45, slide: 200 } }],
      axisTypes: { pan: "rotation", slide: "linear" },
    };

    const result = generateTrajectory(config);
    expect(result.points.length).toBe(1);
    expect(result.points[0].t).toBe(0);
    expect(result.points[0].pan).toBe(45);
    expect(result.points[0].slide).toBe(200);
  });

  it("very short move (10ms)", () => {
    const config: TrajectoryConfig = {
      keyframes: [
        { t: 0, positions: { slide: 0 } },
        { t: 10, positions: { slide: 100 } },
      ],
      axisTypes: { slide: "linear" },
      sample_interval_ms: 10,
    };

    const result = generateTrajectory(config);
    expect(result.points.length).toBe(2);
    expect(result.points[0].slide).toBe(0);
    expect(result.points[1].slide).toBe(100);
  });

  it("constant axis values stay constant", () => {
    const config: TrajectoryConfig = {
      keyframes: [
        { t: 0, positions: { slide: 42 } },
        { t: 500, positions: { slide: 42 } },
        { t: 1000, positions: { slide: 42 } },
      ],
      axisTypes: { slide: "linear" },
      sample_interval_ms: 100,
    };

    const result = generateTrajectory(config);
    for (const point of result.points) {
      expect(point.slide).toBe(42);
    }
  });

  it("slider-only config (no rotation axes)", () => {
    const config: TrajectoryConfig = {
      keyframes: [
        { t: 0, positions: { slide: 0 } },
        { t: 1000, positions: { slide: 500 } },
      ],
      axisTypes: { slide: "linear" },
      sample_interval_ms: 250,
    };

    const result = generateTrajectory(config);
    expect(result.points.length).toBe(5);
    expect(result.points[2].slide).toBe(250);
    // No rotation axes should appear
    expect(result.points[0]).not.toHaveProperty("pan");
  });

  it("long trajectory (30s at 100Hz) completes in reasonable time", () => {
    const config: TrajectoryConfig = {
      keyframes: [
        { t: 0, positions: { pan: 0, tilt: 0, roll: 0, slide: 0 } },
        { t: 30000, positions: { pan: 90, tilt: -10, roll: 5, slide: 900 } },
      ],
      axisTypes: FOUR_AXIS_TYPES,
      sample_interval_ms: 10,
    };

    const start = performance.now();
    const result = generateTrajectory(config);
    const elapsed = performance.now() - start;

    expect(result.points.length).toBe(3001);
    expect(elapsed).toBeLessThan(1000); // under 1 second
  });

  // ── Validation errors ──

  it("throws on empty keyframes", () => {
    expect(() =>
      generateTrajectory({
        keyframes: [],
        axisTypes: { slide: "linear" },
      }),
    ).toThrow("At least 1 keyframe");
  });

  it("throws on non-ascending times", () => {
    expect(() =>
      generateTrajectory({
        keyframes: [
          { t: 0, positions: { slide: 0 } },
          { t: 500, positions: { slide: 100 } },
          { t: 300, positions: { slide: 200 } },
        ],
        axisTypes: { slide: "linear" },
      }),
    ).toThrow("ascending");
  });

  it("throws on invalid rotation axis name", () => {
    expect(() =>
      generateTrajectory({
        keyframes: [
          { t: 0, positions: { yaw: 0 } },
          { t: 1000, positions: { yaw: 90 } },
        ],
        axisTypes: { yaw: "rotation" },
      }),
    ).toThrow('Rotation axis "yaw" is not supported');
  });

  it("throws on duplicate times", () => {
    expect(() =>
      generateTrajectory({
        keyframes: [
          { t: 0, positions: { slide: 0 } },
          { t: 500, positions: { slide: 100 } },
          { t: 500, positions: { slide: 200 } },
        ],
        axisTypes: { slide: "linear" },
      }),
    ).toThrow("ascending");
  });
});
