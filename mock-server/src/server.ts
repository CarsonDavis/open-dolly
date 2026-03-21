import Fastify from "fastify";
import cors from "@fastify/cors";
import websocket from "@fastify/websocket";
import type { Settings } from "@opendolly/shared";
import { type ProfileName, getProfile } from "./profiles.js";
import { BoardState } from "./state.js";
import { TrajectoryStore } from "./trajectory.js";
import { MotionSimulator } from "./motion.js";
import { registerRoutes } from "./routes.js";
import { registerWebSocket } from "./websocket.js";

export interface ServerOptions {
  port: number;
  profile: ProfileName;
  telemetryRate: number;
  latency: number;
}

export async function createServer(opts: ServerOptions) {
  const capabilities = getProfile(opts.profile);
  capabilities.telemetry_rate_hz = opts.telemetryRate;

  const boardState = new BoardState(capabilities);
  const trajectoryStore = new TrajectoryStore(capabilities);

  const settings: Settings = {
    device_name: "dolly-01",
    ap_ssid: "OpenDolly-AP",
    ap_password: "opendolly",
    telemetry_rate_hz: opts.telemetryRate,
    jog_sensitivity: 1.0,
    home_on_boot: false,
  };

  const app = Fastify({ logger: false });

  await app.register(cors, { origin: true });
  await app.register(websocket);

  // WebSocket must be registered first so broadcast is available,
  // but motion needs broadcast. We break the cycle with a deferred ref.
  let broadcastRef: ((event: import("@opendolly/shared").WsEvent) => void) | null =
    null;

  const motion = new MotionSimulator(
    boardState,
    (event) => {
      broadcastRef?.(event);
    },
    opts.telemetryRate,
  );

  const { broadcast } = registerWebSocket(app, {
    boardState,
    trajectoryStore,
    motion,
    latencyMs: opts.latency,
  });

  broadcastRef = broadcast;

  registerRoutes(app, {
    capabilities,
    boardState,
    trajectoryStore,
    settings,
    latencyMs: opts.latency,
  });

  return { app, capabilities };
}
