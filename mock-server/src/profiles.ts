import type { Capabilities } from "@slider/shared";

export type ProfileName = "slider-only" | "slider-servo" | "slider-dji";

const PROFILES: Record<ProfileName, Capabilities> = {
  "slider-only": {
    axes: [
      { name: "slide", min: 0.0, max: 1000.0, unit: "mm" },
    ],
    max_trajectory_points: 10000,
    max_trajectory_duration_ms: 300000,
    telemetry_rate_hz: 50,
    firmware_version: "1.0.0-mock",
    board: "esp32-s3-mock",
  },

  "slider-servo": {
    axes: [
      { name: "pan", min: -180.0, max: 180.0, unit: "deg" },
      { name: "tilt", min: -90.0, max: 90.0, unit: "deg" },
      { name: "roll", min: -45.0, max: 45.0, unit: "deg" },
      { name: "slide", min: 0.0, max: 1000.0, unit: "mm" },
    ],
    max_trajectory_points: 10000,
    max_trajectory_duration_ms: 300000,
    telemetry_rate_hz: 50,
    firmware_version: "1.0.0-mock",
    board: "esp32-s3-mock",
  },

  "slider-dji": {
    axes: [
      { name: "pan", min: -330.0, max: 330.0, unit: "deg" },
      { name: "tilt", min: -135.0, max: 200.0, unit: "deg" },
      { name: "roll", min: -90.0, max: 90.0, unit: "deg" },
      { name: "slide", min: 0.0, max: 1000.0, unit: "mm" },
    ],
    max_trajectory_points: 10000,
    max_trajectory_duration_ms: 300000,
    telemetry_rate_hz: 50,
    firmware_version: "1.0.0-mock",
    board: "esp32-s3-mock",
  },
};

export const PROFILE_NAMES = Object.keys(PROFILES) as ProfileName[];

export function getProfile(name: ProfileName): Capabilities {
  return structuredClone(PROFILES[name]);
}
