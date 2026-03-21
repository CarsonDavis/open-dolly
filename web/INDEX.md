# Web UI Index

SvelteKit 5 app (runes mode) for browser-based slider control. Builds to static HTML/JS/CSS (184KB) for ESP32 LittleFS flash. Imports `@opendolly/shared` for API types and `@opendolly/motion-math` for trajectory generation.

## Key Config

| Path | Purpose | Key Details |
|------|---------|-------------|
| `package.json` | Package config (`@opendolly/web`) | Deps: `@opendolly/shared`, `@opendolly/motion-math`; devDeps: SvelteKit, adapter-static, vitest |
| `svelte.config.js` | SvelteKit config | `adapter-static` with `fallback: 'index.html'` for SPA mode |
| `vite.config.ts` | Vite config | Dev proxy: `/api` → `localhost:3001`, `/ws` → `ws://localhost:3001` |
| `.env.development` | Dev environment vars | `VITE_BOARD_URL`, `VITE_BOARD_WS_URL` pointing to mock server |
| `src/app.css` | Global styles and design tokens | Dark theme, CSS custom properties, 44px touch targets, system fonts, reduced-motion support |
| `src/app.html` | HTML shell | Viewport with `viewport-fit=cover`, theme-color meta |
| `src/routes/+layout.ts` | Layout config | `ssr = false`, `prerender = false` (SPA) |

## API Layer

| Path | Purpose | Key Details |
|------|---------|-------------|
| `src/lib/api/client.ts` | REST client for board API | `BoardClient` class; typed methods for status, capabilities, trajectory CRUD, settings; `ApiError` class for error handling |
| `src/lib/api/websocket.ts` | WebSocket client for real-time control | `BoardWebSocket` class; auto-reconnect with exponential backoff (1s→30s cap); jog throttled at 20Hz (50ms); heartbeat every 5s with 3s timeout; JSON ping |

## Stores

| Path | Purpose | Key Details |
|------|---------|-------------|
| `src/lib/stores/connection.ts` | WebSocket connection lifecycle | Creates `BoardWebSocket`, routes position/state/complete/error events to other stores; tracks playback progress from position event `t` field; exports `isConnected` derived store |
| `src/lib/stores/capabilities.ts` | Board capabilities (`GET /api/capabilities`) | Nullable writable; derived `axisNames` and `axisMap` for component convenience |
| `src/lib/stores/position.ts` | Live axis positions | `Record<string, number>` updated from WebSocket position events at up to 50Hz |
| `src/lib/stores/state.ts` | System state and playback tracking | Tracks `SystemState`, error details, trajectory ID, playback progress/elapsed, battery level; derived `isIdle`, `isPlaying`, `hasError` |
| `src/lib/stores/keyframes.ts` | Saved keyframes (localStorage-backed) | Custom store with `add`, `remove`, `reorder`, `updateLabel`, `clear` methods; persists to `opendolly-keyframes` key |
| `src/lib/stores/trajectory.ts` | Trajectory computation/upload lifecycle | States: empty → computing → computed → uploading → uploaded; stores points array, duration, trajectory ID |

## Utilities

| Path | Purpose | Key Details |
|------|---------|-------------|
| `src/lib/utils/throttle.ts` | Trailing-edge throttle | Guarantees last call is always executed; used for jog rate limiting |
| `src/lib/utils/format.ts` | Axis value formatting | `formatAxisValue()` (e.g. "45.0°"), `formatPositionSummary()` (e.g. "Pan 45.0° | Slide 500.0mm") |

## Components

| Path | Purpose | Key Details |
|------|---------|-------------|
| `src/components/CapabilitiesProvider.svelte` | Connection bootstrapper | Connects WebSocket on mount; fetches capabilities with 3-retry backoff; polls `/api/status` every 5s for battery; provides capabilities via Svelte context |
| `src/components/StatusBar.svelte` | Top status bar | Connection indicator (green/yellow/red), system state badge, battery %, settings gear link |
| `src/components/JogControl.svelte` | 2D touch/drag jog pad | Configurable X/Y axis pair (defaults to pan/slide or first available); 5% dead zone; zero-velocity stop on release; per-axis `AxisControl` sliders below |
| `src/components/AxisControl.svelte` | Single-axis slider with readout | Range slider sends `move_to` (absolute); click-to-edit numeric input; displays value with unit |
| `src/components/KeyframeList.svelte` | Keyframe list manager | Capture disabled until position data received; go-to sends `move_to`; delete, reorder (up/down buttons) |
| `src/components/KeyframeCard.svelte` | Single keyframe display | Inline-editable label, compact position summary, go-to/delete buttons, reorder arrows |
| `src/components/PlaybackControls.svelte` | Trajectory generation and playback | Uses `@opendolly/motion-math` `generateTrajectory()` with easing presets; upload via REST; play/pause/resume/stop via WebSocket; progress bar from telemetry |
| `src/components/TabBar.svelte` | Mobile tab navigation | Three tabs: Jog, Keyframes, Play |

## Routes

| Path | Purpose | Key Details |
|------|---------|-------------|
| `src/routes/+layout.svelte` | Root layout | Wraps app in `CapabilitiesProvider` + `StatusBar`; imports `app.css` |
| `src/routes/+page.svelte` | Main control page | Responsive: mobile (< 640px) shows `TabBar` with tab content; desktop shows two-column grid (jog left, keyframes right) with playback below; detects viewport via `$effect` on mount |
| `src/routes/settings/+page.svelte` | Settings page | Device name, WiFi AP (SSID/password with restart warning), telemetry rate, jog sensitivity slider, home-on-boot toggle, read-only firmware/board info; saves via `PATCH /api/settings` |
