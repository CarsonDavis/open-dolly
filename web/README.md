# Web UI

Browser-based control interface for the camera slider. Connect your phone to the slider's WiFi, open the page, and you have full motion control — jog, keyframes, playback.

Built with SvelteKit 5 and compiled to a 184KB static bundle that fits on the ESP32's flash.

## Quick start

From the project root:

```bash
# Start both the mock board server and the UI dev server
npm run dev
```

Or run the UI alone (requires the mock server running separately):

```bash
cd web
npm run dev
```

The dev server runs at `http://localhost:5173` and proxies `/api` and `/ws` to the mock board server at `localhost:3001`.

## Scripts

| Command | Description |
|---------|-------------|
| `npm run dev` | Start Vite dev server with HMR |
| `npm run build` | Build static site to `build/` |
| `npm run preview` | Preview the production build locally |
| `npm run check` | Run `svelte-check` for type errors |
| `npm run test` | Run tests via Vitest |

## How it connects

**Development:** The Vite dev server proxies API requests to the mock board server. Environment variables in `.env.development` set `VITE_BOARD_URL` and `VITE_BOARD_WS_URL`.

**Production (on the ESP32):** The board serves the UI files from LittleFS flash. All API calls use relative URLs (same origin), so no configuration is needed. When a phone joins the slider's WiFi access point, a captive portal opens the UI automatically.

## Architecture

```
+layout.svelte
├── CapabilitiesProvider   ← connects WebSocket, fetches capabilities, polls status
├── StatusBar              ← connection state, system state, battery, settings link
└── <page>
    ├── / (main)
    │   ├── JogControl     ← 2D touch pad + per-axis sliders
    │   ├── KeyframeList   ← capture, reorder, delete, go-to
    │   └── PlaybackControls ← duration, easing, upload & play, transport
    └── /settings
        └── device name, WiFi AP, telemetry rate, jog sensitivity, firmware info
```

### Data flow

```
WebSocket events ──→ connectionStore ──→ positionStore (axis positions)
                                    ──→ stateStore (system state, battery, progress)

User actions ──→ connectionStore.getWebSocket() ──→ jog / move_to / play / stop
             ──→ BoardClient (REST) ──→ upload trajectory / update settings

Keyframes ──→ keyframeStore (localStorage) ──→ generateTrajectory() ──→ trajectoryStore
```

### Stores

| Store | Purpose |
|-------|---------|
| `connectionStore` | WebSocket lifecycle, routes events to other stores |
| `capabilitiesStore` | Board axes, limits, firmware version |
| `positionStore` | Live axis positions from telemetry |
| `stateStore` | System state, errors, playback progress, battery |
| `keyframeStore` | Saved positions, persisted to localStorage |
| `trajectoryStore` | Trajectory computation and upload lifecycle |

## Responsive layout

- **Phone (< 640px):** Single column with tab bar (Jog / Keyframes / Play)
- **Desktop (>= 640px):** Two-column grid (jog left, keyframes right) with playback controls below

## Design

- Dark theme (outdoor filming, often dim lighting)
- CSS custom properties for all colors, radii, font stacks
- 44px minimum touch targets
- System fonts only (no web font downloads)
- `prefers-reduced-motion` respected
- 184KB total build size (well under the 500KB flash budget)

## Production build

```bash
npm run build
# Output: build/

# Copy to firmware for ESP32 flash:
cp -r build/* ../firmware/data/
```

## V1 scope

What's implemented:

- Connect to board (WebSocket with auto-reconnect)
- Jog control (2D touch pad with axis pair selector, per-axis sliders)
- Live axis readouts from telemetry
- Save/manage keyframes (capture, reorder, rename, delete, go-to)
- Trajectory generation with easing presets (via `@opendolly/motion-math`)
- Playback control (play, pause, resume, stop) with progress bar
- Status display (connection, system state, battery, errors)
- Settings page (device name, WiFi AP, telemetry rate, jog sensitivity)

What's planned for V2:

- Timeline editor with per-axis timing
- Custom easing curves (bezier editor)
- Trajectory preview visualization
- Undo/redo
- Save/load sequences to file
- Scrub preview (drag playhead, board moves in real-time)
