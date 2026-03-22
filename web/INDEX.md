# Web UI Index

SvelteKit 5 app (runes mode) for browser-based slider control. Builds to static HTML/JS/CSS (204KB) for ESP32 LittleFS flash. Imports `@opendolly/shared` for API types and `@opendolly/motion-math` for trajectory generation. V2 UI: per-axis control panel, transition editing with duration/speed, SVG curve editor, timeline with scrub, buffer support.

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
| `src/lib/stores/sequence.ts` | V2 sequence store (keyframes + transitions, localStorage-backed) | Replaces V1 `keyframes.ts`; `Sequence` = `SequenceKeyframe[]` + `SequenceTransition[]`; methods: addKeyframe, removeKeyframe, reorderKeyframe, updateKeyframePositions, updateKeyframeLabel, updateTransition, clear; V1 migration from `opendolly-keyframes`; exports `selectedKeyframeId` writable |
| `src/lib/stores/trajectory.ts` | Trajectory computation/upload lifecycle | States: empty → computing → computed → uploading → uploaded; stores points array, duration, trajectory ID |

## Utilities

| Path | Purpose | Key Details |
|------|---------|-------------|
| `src/lib/utils/throttle.ts` | Trailing-edge throttle | Guarantees last call is always executed; used for jog rate limiting |
| `src/lib/utils/format.ts` | Axis value formatting | `formatAxisValue()` (e.g. "45.0°"), `formatPositionSummary()` (e.g. "Pan 45.0° | Slide 500.0mm") |
| `src/lib/utils/trajectory-builder.ts` | V2 trajectory builder | `buildTrajectory(sequence, axisTypes)` — calls `generateTrajectory()` once per transition, concatenates results; `getSequenceDuration()`, `allTransitionsSet()` helpers |

## Components

| Path | Purpose | Key Details |
|------|---------|-------------|
| `src/components/CapabilitiesProvider.svelte` | Connection bootstrapper | Connects WebSocket on mount; fetches capabilities with 3-retry backoff; polls `/api/status` every 5s for battery; provides capabilities via Svelte context |
| `src/components/StatusBar.svelte` | Top status bar | Connection indicator (green/yellow/red), system state badge, battery %, settings gear link |
| `src/components/ControlPanel.svelte` | V2 control panel (replaces JogControl) | Per-axis AxisStrip components; Interactive/Static mode toggle; collapsible tray; capture keyframe button; keyframe selection editing indicator; handles jog and move_to commands |
| `src/components/AxisStrip.svelte` | V2 per-axis control strip (replaces AxisControl) | Jog/Position mode toggle; fine/coarse increment buttons (+1/+10 mm, +1/+15 deg); click-to-type value input; position bar with marker; jog bar with pointer drag |
| `src/components/TransitionEditor.svelte` | V2 transition simple view | Duration input (blocking), derived per-axis speed indicators, speed-primary mode toggle, Advanced button (stub for Phase 4 curve editor) |
| `src/components/CurveEditor.svelte` | V2 per-axis curve editor container | Stacked CurveLane components; easing preset buttons (linear, easeIn, easeOut, easeInOut); multi-axis linking via checkboxes; Done button |
| `src/components/CurveLane.svelte` | V2 SVG curve lane (single axis) | Draws monotone progress curve via createProgressCurveFunction; click to add points, drag to move (constrained), drag off to delete; compact mode for timeline mini-views; grid, axis labels |
| `src/components/TimelineView.svelte` | V2 horizontal timeline (desktop/tablet) | Keyframe track with draggable markers for retiming; per-axis lanes with mini/expandable CurveLane; draggable playhead sends scrub commands (20Hz throttled); auto-uploads trajectory on sequence changes (500ms debounce) |
| `src/components/KeyframeList.svelte` | Keyframe list manager | Uses `sequenceStore`; capture disabled until position data received; go-to sends `move_to`; delete, reorder (up/down buttons) |
| `src/components/KeyframeCard.svelte` | Single keyframe display | Inline-editable label, compact position summary, go-to/delete buttons, reorder arrows; imports `SequenceKeyframe` type |
| `src/components/PlaybackControls.svelte` | Transport controls and trajectory upload | V2: uses `buildTrajectory()` from trajectory-builder; shows "Set transition durations" when transitions are unset; play/pause/resume/stop via WebSocket; progress bar from telemetry |

## Routes

| Path | Purpose | Key Details |
|------|---------|-------------|
| `src/routes/+layout.svelte` | Root layout | Wraps app in `CapabilitiesProvider` + `StatusBar`; imports `app.css` |
| `src/routes/+page.svelte` | Main control page | V2 layout: portrait phone (keyframe list + ControlPanel tray + transport), landscape/desktop (ControlPanel left, keyframes right, transport below); no more TabBar; breakpoints at 640px and 1024px |
| `src/routes/settings/+page.svelte` | Settings page | Device name, WiFi AP (SSID/password with restart warning), telemetry rate, jog sensitivity slider, home-on-boot toggle, read-only firmware/board info; saves via `PATCH /api/settings` |
