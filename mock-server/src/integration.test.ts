/**
 * Integration tests: connect to a real mock server via WebSocket,
 * send commands, verify responses end-to-end.
 */
import { describe, it, expect, beforeAll, afterAll } from "vitest";
import { createServer } from "./server.js";
import { WebSocket } from "ws";
import type { FastifyInstance } from "fastify";

let server: { app: FastifyInstance };
let baseUrl: string;
let wsUrl: string;

beforeAll(async () => {
  server = await createServer({
    profile: "slider-dji",
    port: 0,
    telemetryRate: 50,
    latency: 0,
  });
  await server.app.listen({ port: 0, host: "127.0.0.1" });
  const address = server.app.server.address();
  if (typeof address === "object" && address) {
    baseUrl = `http://127.0.0.1:${address.port}`;
    wsUrl = `ws://127.0.0.1:${address.port}/ws`;
  }
});

afterAll(async () => {
  await server.app.close();
});

function connectWs(): Promise<WebSocket> {
  return new Promise((resolve, reject) => {
    const ws = new WebSocket(wsUrl);
    ws.on("open", () => resolve(ws));
    ws.on("error", reject);
  });
}

function waitForMessage(
  ws: WebSocket,
  predicate: (msg: Record<string, unknown>) => boolean,
  timeoutMs = 3000,
): Promise<Record<string, unknown>> {
  return new Promise((resolve, reject) => {
    const timer = setTimeout(
      () => reject(new Error("Timeout waiting for message")),
      timeoutMs,
    );
    ws.on("message", function handler(data: Buffer) {
      const msg = JSON.parse(data.toString());
      if (predicate(msg)) {
        clearTimeout(timer);
        ws.off("message", handler);
        resolve(msg);
      }
    });
  });
}

describe("GET /api/status", () => {
  it("returns position with all 4 axes", async () => {
    const res = await fetch(`${baseUrl}/api/status`);
    const body = await res.json();

    expect(body.state).toBe("idle");
    expect(body.position).toBeDefined();
    expect(typeof body.position.pan).toBe("number");
    expect(typeof body.position.tilt).toBe("number");
    expect(typeof body.position.roll).toBe("number");
    expect(typeof body.position.slide).toBe("number");

    // No NaN
    expect(Number.isNaN(body.position.pan)).toBe(false);
    expect(Number.isNaN(body.position.tilt)).toBe(false);
    expect(Number.isNaN(body.position.roll)).toBe(false);
    expect(Number.isNaN(body.position.slide)).toBe(false);
  });
});

describe("exact user scenario: +10 slide then +15 pan", () => {
  it("slide stays at 10 after pan move", async () => {
    // Get initial position (all axes start at 0 or clamped min)
    const res0 = await fetch(`${baseUrl}/api/status`);
    const status0 = await res0.json();

    const ws = await connectWs();

    // Step 1: Press +10 on Slide (move_to with only slide axis)
    let idlePromise = waitForMessage(ws, (msg) => msg.evt === "state" && msg.state === "idle");
    ws.send(JSON.stringify({ cmd: "move_to", duration_ms: 200, slide: status0.position.slide + 10 }));
    await idlePromise;

    // Verify slide moved to +10
    const res1 = await fetch(`${baseUrl}/api/status`);
    const status1 = await res1.json();
    expect(status1.position.slide).toBeCloseTo(status0.position.slide + 10, 0);

    // Step 2: Press +15 on Pan (move_to with only pan axis)
    idlePromise = waitForMessage(ws, (msg) => msg.evt === "state" && msg.state === "idle");
    ws.send(JSON.stringify({ cmd: "move_to", duration_ms: 200, pan: status1.position.pan + 15 }));
    await idlePromise;

    // Verify: pan moved, slide MUST still be at +10
    const res2 = await fetch(`${baseUrl}/api/status`);
    const status2 = await res2.json();
    expect(status2.position.pan).toBeCloseTo(status0.position.pan + 15, 0);
    expect(status2.position.slide).toBeCloseTo(status0.position.slide + 10, 0); // THIS IS THE BUG

    ws.close();
  });
});

describe("move_to with single axis", () => {
  it("position event contains ALL axes with valid numbers", async () => {
    // Get initial position
    const statusRes = await fetch(`${baseUrl}/api/status`);
    const status = await statusRes.json();
    const initialPan = status.position.pan;
    const initialTilt = status.position.tilt;
    const initialRoll = status.position.roll;
    const initialSlide = status.position.slide;

    // Connect WebSocket
    const ws = await connectWs();

    // Send move_to with ONLY pan — this is the bug scenario
    const positionPromise = waitForMessage(ws, (msg) => msg.evt === "position");

    ws.send(
      JSON.stringify({
        cmd: "move_to",
        duration_ms: 100,
        pan: initialPan + 15,
      }),
    );

    const posEvent = await positionPromise;

    // ALL axes must be present
    expect(posEvent).toHaveProperty("pan");
    expect(posEvent).toHaveProperty("tilt");
    expect(posEvent).toHaveProperty("roll");
    expect(posEvent).toHaveProperty("slide");

    // ALL axes must be valid numbers (not NaN, not undefined)
    expect(typeof posEvent.pan).toBe("number");
    expect(typeof posEvent.tilt).toBe("number");
    expect(typeof posEvent.roll).toBe("number");
    expect(typeof posEvent.slide).toBe("number");
    expect(Number.isNaN(posEvent.pan)).toBe(false);
    expect(Number.isNaN(posEvent.tilt)).toBe(false);
    expect(Number.isNaN(posEvent.roll)).toBe(false);
    expect(Number.isNaN(posEvent.slide)).toBe(false);

    // Unchanged axes should be at their original values
    expect(posEvent.tilt).toBeCloseTo(initialTilt, 0);
    expect(posEvent.roll).toBeCloseTo(initialRoll, 0);
    expect(posEvent.slide).toBeCloseTo(initialSlide, 0);

    ws.close();
  });

  it("after move_to completes, status shows all axes valid", async () => {
    const ws = await connectWs();

    // Send single-axis move
    const statePromise = waitForMessage(ws, (msg) => msg.evt === "state" && msg.state === "idle");

    ws.send(
      JSON.stringify({
        cmd: "move_to",
        duration_ms: 50,
        slide: 400,
      }),
    );

    // Wait for idle (motion complete)
    await statePromise;
    ws.close();

    // Check status via REST
    const res = await fetch(`${baseUrl}/api/status`);
    const body = await res.json();

    expect(typeof body.position.pan).toBe("number");
    expect(typeof body.position.tilt).toBe("number");
    expect(typeof body.position.roll).toBe("number");
    expect(typeof body.position.slide).toBe("number");

    expect(Number.isNaN(body.position.pan)).toBe(false);
    expect(Number.isNaN(body.position.tilt)).toBe(false);
    expect(Number.isNaN(body.position.roll)).toBe(false);
    expect(Number.isNaN(body.position.slide)).toBe(false);

    expect(body.position.slide).toBeCloseTo(400, 0);
  });

  it("sequential single-axis moves preserve all other axes", async () => {
    // Get baseline
    const res1 = await fetch(`${baseUrl}/api/status`);
    const status1 = await res1.json();

    const ws = await connectWs();

    // Move pan
    let idlePromise = waitForMessage(ws, (msg) => msg.evt === "state" && msg.state === "idle");
    ws.send(JSON.stringify({ cmd: "move_to", duration_ms: 50, pan: 90 }));
    await idlePromise;

    // Move tilt
    idlePromise = waitForMessage(ws, (msg) => msg.evt === "state" && msg.state === "idle");
    ws.send(JSON.stringify({ cmd: "move_to", duration_ms: 50, tilt: -45 }));
    await idlePromise;

    // Move slide
    idlePromise = waitForMessage(ws, (msg) => msg.evt === "state" && msg.state === "idle");
    ws.send(JSON.stringify({ cmd: "move_to", duration_ms: 50, slide: 750 }));
    await idlePromise;

    ws.close();

    // Check final state: all three moves should have stuck
    const res2 = await fetch(`${baseUrl}/api/status`);
    const status2 = await res2.json();

    expect(status2.position.pan).toBeCloseTo(90, 0);
    expect(status2.position.tilt).toBeCloseTo(-45, 0);
    expect(status2.position.slide).toBeCloseTo(750, 0);
    // Roll was never moved, should be at initial value
    expect(status2.position.roll).toBeCloseTo(status1.position.roll, 0);
  });
});
