/** Device settings as returned by GET /api/settings. */
export interface Settings {
  device_name: string;
  ap_ssid: string;
  ap_password: string;
  telemetry_rate_hz: number;
  jog_sensitivity: number;
  home_on_boot: boolean;
}

/**
 * PATCH /api/settings request body.
 * All fields optional; only provided keys are updated.
 * Send `restart: true` to trigger a device restart.
 */
export type SettingsUpdate = Partial<Settings> & {
  restart?: boolean;
};

/**
 * PATCH /api/settings response.
 * Full settings object, optionally with restart_required flag.
 */
export type SettingsResponse = Settings & {
  restart_required?: boolean;
};
