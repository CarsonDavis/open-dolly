import type { FastifyInstance } from "fastify";
import type {
  Capabilities,
  Settings,
  SettingsResponse,
  StatusResponse,
  TrajectoryUpload,
} from "@opendolly/shared";
import type { BoardState } from "./state.js";
import { TrajectoryStore, TrajectoryValidationError } from "./trajectory.js";

interface RouteDeps {
  capabilities: Capabilities;
  boardState: BoardState;
  trajectoryStore: TrajectoryStore;
  settings: Settings;
  latencyMs: number;
}

export function registerRoutes(
  app: FastifyInstance,
  deps: RouteDeps,
): void {
  const { capabilities, boardState, trajectoryStore, latencyMs } = deps;

  const delay = latencyMs > 0
    ? () => new Promise<void>((r) => setTimeout(r, latencyMs))
    : () => Promise.resolve();

  // GET /api/status
  app.get("/api/status", async (_req, reply) => {
    await delay();
    const body: StatusResponse = {
      state: boardState.state,
      position: boardState.position,
      battery: 82,
      trajectory_loaded: boardState.trajectoryLoaded,
    };
    return reply.send(body);
  });

  // GET /api/capabilities
  app.get("/api/capabilities", async (_req, reply) => {
    await delay();
    return reply.send(capabilities);
  });

  // POST /api/trajectory
  app.post("/api/trajectory", async (req, reply) => {
    await delay();
    try {
      const upload = req.body as TrajectoryUpload;
      const result = trajectoryStore.upload(upload);
      boardState.setTrajectoryLoaded(result.trajectory_id);
      return reply.send(result);
    } catch (err) {
      if (err instanceof TrajectoryValidationError) {
        return reply.status(400).send({
          error: "TRAJECTORY_INVALID",
          detail: err.message,
        });
      }
      throw err;
    }
  });

  // DELETE /api/trajectory/:id
  app.delete<{ Params: { id: string } }>(
    "/api/trajectory/:id",
    async (req, reply) => {
      await delay();
      const { id } = req.params;

      if (!trajectoryStore.has(id)) {
        return reply.status(404).send({
          error: "NOT_FOUND",
          detail: `No trajectory with id '${id}'`,
        });
      }

      const state = boardState.state;
      if (
        (state === "playing" || state === "paused") &&
        boardState.trajectoryLoaded === id
      ) {
        return reply.status(409).send({
          error: "CONFLICT",
          detail: "Cannot delete trajectory while playback is active",
        });
      }

      trajectoryStore.delete(id);
      if (boardState.trajectoryLoaded === id) {
        boardState.setTrajectoryLoaded(null);
      }
      return reply.status(204).send();
    },
  );

  // GET /api/trajectory/:id/status
  app.get<{ Params: { id: string } }>(
    "/api/trajectory/:id/status",
    async (req, reply) => {
      await delay();
      const { id } = req.params;
      const traj = trajectoryStore.get(id);

      if (!traj) {
        return reply.status(404).send({
          error: "NOT_FOUND",
          detail: `No trajectory with id '${id}'`,
        });
      }

      const isActive = boardState.trajectoryLoaded === id;
      const state = boardState.state;

      if (isActive && (state === "playing" || state === "paused")) {
        // In a real implementation we'd track elapsed precisely.
        // For the mock, return a reasonable approximation.
        return reply.send({
          state: state === "playing" ? "playing" : "paused",
          elapsed_ms: 0,
          progress: 0,
        });
      }

      return reply.send({
        state: "idle" as const,
        elapsed_ms: 0,
        progress: 0,
      });
    },
  );

  // GET /api/settings
  app.get("/api/settings", async (_req, reply) => {
    await delay();
    return reply.send(deps.settings);
  });

  // PATCH /api/settings
  app.patch("/api/settings", async (req, reply) => {
    await delay();
    const updates = req.body as Record<string, unknown>;
    let restartRequired = false;

    if (updates.restart === true) {
      console.log("[mock] Restart requested (simulated)");
      delete updates.restart;
    }

    const settableKeys: Array<keyof Settings> = [
      "device_name",
      "ap_ssid",
      "ap_password",
      "telemetry_rate_hz",
      "jog_sensitivity",
      "home_on_boot",
    ];

    for (const key of settableKeys) {
      if (key in updates) {
        if (key === "ap_ssid" || key === "ap_password") {
          restartRequired = true;
        }
        // eslint-disable-next-line @typescript-eslint/no-explicit-any
        (deps.settings as any)[key] = updates[key];
      }
    }

    const response: SettingsResponse = {
      ...deps.settings,
      ...(restartRequired ? { restart_required: true } : {}),
    };

    return reply.send(response);
  });
}
