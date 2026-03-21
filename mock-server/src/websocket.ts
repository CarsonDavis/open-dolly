import type { FastifyInstance } from "fastify";
import type { WebSocket } from "ws";
import type {
  ErrorCode,
  Position,
  SystemState,
  WsCommand,
  WsEvent,
} from "@opendolly/shared";
import type { BoardState } from "./state.js";
import type { TrajectoryStore } from "./trajectory.js";
import type { MotionSimulator } from "./motion.js";

interface WsDeps {
  boardState: BoardState;
  trajectoryStore: TrajectoryStore;
  motion: MotionSimulator;
  latencyMs: number;
}

const ERROR_DETAILS: Record<ErrorCode, (axis?: string) => string> = {
  LIMIT_HIT: (axis) => `Simulated: ${axis ?? "unknown"} max limit reached`,
  MOTOR_STALL: (axis) =>
    `Simulated: ${axis ?? "unknown"} motor stall detected`,
  TRAJECTORY_INVALID: () => "Simulated: trajectory data corrupted",
  OUT_OF_MEMORY: () => "Simulated: insufficient memory for trajectory",
  COMMUNICATION_LOST: () => "Simulated: CAN bus communication timeout",
  INVALID_STATE: () => "Simulated: invalid state transition",
};

/**
 * Command validity matrix.
 * Maps each state to the set of commands that are valid in that state.
 */
const VALID_COMMANDS: Record<SystemState, Set<string>> = {
  idle: new Set(["play", "home", "jog", "move_to", "scrub"]),
  moving: new Set(["move_to", "jog", "stop"]),
  playing: new Set(["pause", "stop", "scrub"]),
  paused: new Set(["resume", "stop", "scrub"]),
  homing: new Set(["stop"]),
  error: new Set(["stop"]),
};

export function registerWebSocket(
  app: FastifyInstance,
  deps: WsDeps,
): {
  broadcast: (event: WsEvent) => void;
} {
  const { boardState, trajectoryStore, motion, latencyMs } = deps;
  const clients = new Set<WebSocket>();

  function broadcast(event: WsEvent): void {
    const msg = JSON.stringify(event);
    for (const ws of clients) {
      if (ws.readyState === ws.OPEN) {
        ws.send(msg);
      }
    }
  }

  // Register state change listener to broadcast state events
  boardState.onStateChange((state) => {
    broadcast({ evt: "state", state });
  });

  const delay = latencyMs > 0
    ? () => new Promise<void>((r) => setTimeout(r, latencyMs))
    : () => Promise.resolve();

  app.register(async (instance) => {
    instance.get("/ws", { websocket: true }, (socket) => {
      clients.add(socket);

      socket.on("close", () => {
        clients.delete(socket);
      });

      socket.on("message", async (data: Buffer) => {
        let parsed: WsCommand;
        try {
          parsed = JSON.parse(data.toString()) as WsCommand;
        } catch {
          broadcast({
            evt: "error",
            code: "TRAJECTORY_INVALID",
            detail: "Malformed JSON",
          });
          return;
        }

        if (latencyMs > 0) await delay();

        const cmd = parsed.cmd;

        // ping is valid from any state — immediate pong, no state change
        if (cmd === "ping") {
          broadcast({ evt: "pong" });
          return;
        }

        // simulate_error is valid from any state
        if (cmd === "simulate_error") {
          handleSimulateError(parsed as { cmd: "simulate_error"; code: ErrorCode; axis?: string });
          return;
        }

        // Validate command against current state
        const state = boardState.state;
        if (!VALID_COMMANDS[state]?.has(cmd)) {
          broadcast({
            evt: "error",
            code: "INVALID_STATE",
            detail: `Command '${cmd}' not valid in state '${state}'`,
          });
          return;
        }

        switch (cmd) {
          case "play":
            handlePlay(parsed as { cmd: "play"; trajectory_id: string });
            break;
          case "pause":
            handlePause();
            break;
          case "resume":
            handleResume();
            break;
          case "stop":
            handleStop();
            break;
          case "scrub":
            handleScrub(parsed as { cmd: "scrub"; t: number });
            break;
          case "jog":
            handleJog(parsed as Record<string, unknown>);
            break;
          case "move_to":
            handleMoveTo(parsed as Record<string, unknown>);
            break;
          case "home":
            handleHome(parsed as { cmd: "home"; axes: string[] });
            break;
        }
      });
    });
  });

  // --- Command handlers ---

  function handlePlay(cmd: { cmd: "play"; trajectory_id: string }): void {
    const traj = trajectoryStore.get(cmd.trajectory_id);
    if (!traj) {
      broadcast({
        evt: "error",
        code: "TRAJECTORY_INVALID",
        detail: `No trajectory with id '${cmd.trajectory_id}'`,
      });
      return;
    }
    boardState.transition("playing");
    motion.startPlayback(traj);
  }

  function handlePause(): void {
    boardState.transition("paused");
    motion.pause();
  }

  function handleResume(): void {
    boardState.transition("playing");
    motion.resume();
  }

  function handleStop(): void {
    motion.stop();
    boardState.transition("idle");
  }

  function handleScrub(cmd: { cmd: "scrub"; t: number }): void {
    if (boardState.trajectoryLoaded === null) {
      broadcast({
        evt: "error",
        code: "TRAJECTORY_INVALID",
        detail: "No trajectory loaded",
      });
      return;
    }
    motion.scrub(cmd.t);
  }

  function handleJog(cmd: Record<string, unknown>): void {
    const state = boardState.state;

    // If in moving state, jog cancels the move
    if (state === "moving") {
      motion.stop();
      boardState.transition("idle");
    }

    const deltas: Record<string, number> = {};
    for (const axis of boardState.axes) {
      if (axis.name in cmd && typeof cmd[axis.name] === "number") {
        deltas[axis.name] = cmd[axis.name] as number;
      }
    }

    if (Object.keys(deltas).length > 0) {
      motion.startJog(deltas);
    }
  }

  function handleMoveTo(cmd: Record<string, unknown>): void {
    const duration_ms = cmd.duration_ms as number;
    if (typeof duration_ms !== "number" || duration_ms <= 0) {
      broadcast({
        evt: "error",
        code: "INVALID_STATE",
        detail: "move_to requires a positive duration_ms",
      });
      return;
    }

    const target: Position = {};
    for (const axis of boardState.axes) {
      if (axis.name in cmd && typeof cmd[axis.name] === "number") {
        target[axis.name] = cmd[axis.name] as number;
      } else {
        // Hold current position for unspecified axes
        target[axis.name] = boardState.position[axis.name]!;
      }
    }

    const err = boardState.validateAxisValues(target);
    if (err) {
      broadcast({
        evt: "error",
        code: "LIMIT_HIT",
        detail: err,
      });
      return;
    }

    motion.startMoveTo(target, duration_ms);
  }

  function handleHome(cmd: { cmd: "home"; axes: string[] }): void {
    const err = boardState.validateAxes(cmd.axes);
    if (err) {
      broadcast({
        evt: "error",
        code: "INVALID_STATE",
        detail: err,
      });
      return;
    }
    motion.startHoming(cmd.axes);
  }

  function handleSimulateError(cmd: {
    cmd: "simulate_error";
    code: ErrorCode;
    axis?: string;
  }): void {
    motion.stop();
    boardState.transition("error");
    const detailFn = ERROR_DETAILS[cmd.code];
    broadcast({
      evt: "error",
      code: cmd.code,
      axis: cmd.axis,
      detail: detailFn ? detailFn(cmd.axis) : `Simulated: ${cmd.code}`,
    });
  }

  return { broadcast };
}
