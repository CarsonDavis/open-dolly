import { Command } from "commander";
import { PROFILE_NAMES, type ProfileName } from "./profiles.js";
import { createServer } from "./server.js";

const program = new Command()
  .name("mock-server")
  .description("Mock implementation of the Slider board API")
  .option("--port <number>", "Server port", "3001")
  .option(
    "--profile <name>",
    `Capability profile: ${PROFILE_NAMES.join(", ")}`,
    "slider-dji",
  )
  .option("--telemetry-rate <hz>", "Telemetry broadcast rate in Hz", "50")
  .option(
    "--latency <ms>",
    "Artificial latency on all responses in ms",
    "0",
  )
  .parse();

const opts = program.opts();

const port = parseInt(opts.port, 10);
const profile = opts.profile as ProfileName;
const telemetryRate = parseInt(opts.telemetryRate, 10);
const latency = parseInt(opts.latency, 10);

if (!PROFILE_NAMES.includes(profile)) {
  console.error(
    `Unknown profile '${profile}'. Valid profiles: ${PROFILE_NAMES.join(", ")}`,
  );
  process.exit(1);
}

const { app, capabilities } = await createServer({
  port,
  profile,
  telemetryRate,
  latency,
});

await app.listen({ port, host: "0.0.0.0" });

console.log(`Mock board server started
  Port:           ${port}
  Profile:        ${profile} (${capabilities.axes.length} axes)
  Telemetry rate: ${telemetryRate} Hz
  Latency:        ${latency} ms
  REST:           http://localhost:${port}/api/status
  WebSocket:      ws://localhost:${port}/ws`);
