import { describe, it, expect, vi } from "vitest";
import type { Capabilities, WsEvent, PositionEvent } from "@opendolly/shared";
import { BoardState } from "./state.js";
import { MotionSimulator } from "./motion.js";

const TEST_CAPABILITIES: Capabilities = {
  axes: [
    { name: "pan", min: -180, max: 180, unit: "deg" },
    { name: "tilt", min: -90, max: 90, unit: "deg" },
    { name: "slide", min: 0, max: 1000, unit: "mm" },
  ],
  max_trajectory_points: 10000,
  max_trajectory_duration_ms: 300000,
  telemetry_rate_hz: 50,
  firmware_version: "test",
  board: "test",
};

function createTestSetup() {
  const boardState = new BoardState(TEST_CAPABILITIES);
  const events: WsEvent[] = [];
  const broadcast = (event: WsEvent) => events.push(event);
  const motion = new MotionSimulator(boardState, broadcast, 1000); // 1000Hz for fast tests
  return { boardState, events, motion };
}

describe("MoveToMotion", () => {
  it("partial target fills missing axes from current position", async () => {
    const { boardState, events, motion } = createTestSetup();

    // Set initial position
    boardState.setPosition({ pan: 45, tilt: -10, slide: 500 });

    // Move only pan — tilt and slide should hold steady
    motion.startMoveTo({ pan: 90 }, 100);

    // Wait for motion to complete
    await new Promise((r) => setTimeout(r, 200));
    motion.stop();

    // Check final position: pan should have moved, others should be unchanged
    expect(boardState.position.pan).toBeCloseTo(90, 0);
    expect(boardState.position.tilt).toBeCloseTo(-10, 0);
    expect(boardState.position.slide).toBeCloseTo(500, 0);
  });

  it("position events contain ALL axis names", async () => {
    const { boardState, events, motion } = createTestSetup();

    boardState.setPosition({ pan: 0, tilt: 0, slide: 0 });
    motion.startMoveTo({ pan: 45 }, 50);

    await new Promise((r) => setTimeout(r, 100));
    motion.stop();

    const positionEvents = events.filter(
      (e) => (e as Record<string, unknown>).evt === "position",
    );
    expect(positionEvents.length).toBeGreaterThan(0);

    for (const evt of positionEvents) {
      const e = evt as unknown as Record<string, unknown>;
      expect(e).toHaveProperty("pan");
      expect(e).toHaveProperty("tilt");
      expect(e).toHaveProperty("slide");
      expect(typeof e.pan).toBe("number");
      expect(typeof e.tilt).toBe("number");
      expect(typeof e.slide).toBe("number");
      // No NaN values
      expect(Number.isNaN(e.pan)).toBe(false);
      expect(Number.isNaN(e.tilt)).toBe(false);
      expect(Number.isNaN(e.slide)).toBe(false);
    }
  });

  it("unchanged axes hold steady during motion (not interpolated to 0 or NaN)", async () => {
    const { boardState, events, motion } = createTestSetup();

    boardState.setPosition({ pan: 45, tilt: -30, slide: 750 });

    // Move only slide — pan and tilt must not change
    motion.startMoveTo({ slide: 250 }, 50);

    await new Promise((r) => setTimeout(r, 100));
    motion.stop();

    const positionEvents = events.filter(
      (e) => (e as Record<string, unknown>).evt === "position",
    );

    for (const evt of positionEvents) {
      const e = evt as unknown as Record<string, number | string>;
      // Pan and tilt should stay at their original values throughout
      expect(e.pan).toBeCloseTo(45, 0);
      expect(e.tilt).toBeCloseTo(-30, 0);
    }

    // Slide should have moved to 250
    expect(boardState.position.slide).toBeCloseTo(250, 0);
  });

  it("move_to with ALL axes specified works correctly", async () => {
    const { boardState, events, motion } = createTestSetup();

    boardState.setPosition({ pan: 0, tilt: 0, slide: 0 });
    motion.startMoveTo({ pan: 90, tilt: -45, slide: 500 }, 50);

    await new Promise((r) => setTimeout(r, 100));
    motion.stop();

    expect(boardState.position.pan).toBeCloseTo(90, 0);
    expect(boardState.position.tilt).toBeCloseTo(-45, 0);
    expect(boardState.position.slide).toBeCloseTo(500, 0);
  });

  it("move_to with empty target holds all axes at current position", async () => {
    const { boardState, events, motion } = createTestSetup();

    boardState.setPosition({ pan: 45, tilt: -10, slide: 300 });
    motion.startMoveTo({}, 50);

    await new Promise((r) => setTimeout(r, 100));
    motion.stop();

    expect(boardState.position.pan).toBeCloseTo(45, 0);
    expect(boardState.position.tilt).toBeCloseTo(-10, 0);
    expect(boardState.position.slide).toBeCloseTo(300, 0);
  });
});
