# Web UI Implementation Plan

*Status: active — V1 implemented*
*Last updated: 2026-03-21*

This document is the implementation plan for the SvelteKit web UI — the primary interface for the camera slider system. The UI runs on the user's phone or laptop browser, connects to the ESP32 board over WiFi, and provides full motion control. During development it runs against a mock server.

### Connection and Discovery

When a user connects their phone to the slider's WiFi access point, a **captive portal** automatically opens the web UI in the device's default browser. Fallback: navigate to `http://192.168.4.1` manually.

- **CapabilitiesProvider** attempts connection to the board URL automatically on mount.
- **Dev mode:** connects to the mock server via the `VITE_BOARD_URL` environment variable.
- **Production:** same-origin (relative URLs, since the board serves the UI directly).

---

## Table of Contents

1. [Project Setup](#project-setup)
2. [Route Structure](#route-structure)
3. [Layout and Wireframes](#layout-and-wireframes)
4. [Component Architecture](#component-architecture)
5. [Svelte Stores](#svelte-stores)
6. [API Client](#api-client)
7. [V1 vs V2 Scope](#v1-vs-v2-scope)
8. [Responsive Design](#responsive-design)
9. [Build and Bundle Size](#build-and-bundle-size)
10. [Development Workflow](#development-workflow)
11. [Test Strategy](#test-strategy)
12. [Accessibility](#accessibility)
13. [File Tree](#file-tree)

---

## Project Setup

### Stack

| Tool | Purpose |
|------|---------|
| SvelteKit | App framework (file-based routing, SSR/SSG, stores) |
| TypeScript | Type safety across components, stores, and API client |
| `@sveltejs/adapter-static` | Builds to static HTML/JS/CSS for ESP32 LittleFS flash |
| Vite | Dev server with HMR |
| Vitest | Unit and component testing |
| `@testing-library/svelte` | Component test utilities |

### CSS Approach

Plain CSS with CSS custom properties (variables) for theming. No Tailwind, no CSS-in-JS, no UI framework. Reasons:

- Tailwind's generated CSS is too large for the 500KB flash budget even with purging.
- Custom properties give us a design-token system with zero runtime cost.
- A single `global.css` file defines the token palette; components use scoped `<style>` blocks.

Token examples:

```css
:root {
  --color-bg: #1a1a2e;
  --color-surface: #16213e;
  --color-primary: #0f3460;
  --color-accent: #e94560;
  --color-text: #eee;
  --color-text-muted: #999;
  --color-success: #4caf50;
  --color-warning: #ff9800;
  --color-error: #f44336;
  --radius: 8px;
  --touch-min: 44px; /* minimum touch target */
  --font-mono: 'SF Mono', 'Menlo', monospace;
}
```

Dark theme by default (outdoor filming, often in dim/mixed lighting). High-contrast mode toggled via a CSS class on `<body>`.

### Init Commands

```bash
npm create svelte@latest ui -- --template skeleton --types typescript
cd ui
npm install -D @sveltejs/adapter-static vitest @testing-library/svelte jsdom
```

---

## Route Structure

**Recommendation: two routes, not a full SPA.**

| Route | Purpose |
|-------|---------|
| `/` | Main control interface — jog, keyframes, playback, status |
| `/settings` | Device config, WiFi, telemetry, firmware info (see details below) |

### Justification

A single monolithic route would need tab switching logic that duplicates what SvelteKit routing already provides. But more than two routes adds navigation overhead on a phone where every tap matters. Two routes hits the sweet spot:

- `/` is the 95% screen. Jog, keyframes, and playback controls are always visible or one tap away via in-page tabs.
- `/settings` is visited once per session at most. Separate route keeps it out of the critical path and avoids loading its components until needed.
- SvelteKit's client-side navigation means switching between the two is instant (no full page reload).

Both routes share a root `+layout.svelte` that provides the WebSocket connection, capabilities context, and the status bar.

### Settings Page Detail

The `/settings` route displays:

| Field | Type |
|-------|------|
| Device name | Editable text |
| AP SSID | Editable text |
| AP password | Editable text |
| Telemetry rate | Editable number (Hz) |
| Jog sensitivity | Slider |
| Home on boot | Toggle |
| Firmware version | Read-only (from capabilities) |
| Board info | Read-only (from capabilities) |

- **Save** button calls `PATCH /api/settings` with all editable fields.
- WiFi changes (SSID or password) display a warning: *"WiFi changes require a device restart to take effect."* A **Restart** button is shown alongside the warning, which calls `PATCH /api/settings` with `{"restart": true}`.

---

## Layout and Wireframes

### Mobile Layout (portrait, primary device)

```
┌──────────────────────────┐
│  StatusBar               │
│  ● Connected  🔋 82%     │
│  idle          ⚙ Settings│
├──────────────────────────┤
│                          │
│   ┌──────────────────┐   │
│   │                  │   │
│   │   JogControl     │   │
│   │   (touch pad)    │   │
│   │                  │   │
│   │    ○ center      │   │
│   │                  │   │
│   └──────────────────┘   │
│                          │
│  [Pan 34.2°] [Tilt -8°] │
│  [Slide 423mm] [Roll 0°] │
│                          │
├────────┬─────────────────┤
│  Jog   │  Keyframes  │ ▶ │  <-- in-page tab bar
├────────┴─────────────────┤
│                          │
│  Keyframe 1   [Go] [X]  │
│  Pan 0° Tilt 0° Slide 0 │
│                          │
│  Keyframe 2   [Go] [X]  │
│  Pan 45° Tilt -10° ...  │
│                          │
│  [+ Capture Current Pos] │
│                          │
├──────────────────────────┤
│  Duration: [___5.0s___]  │
│  Easing: [ease-in-out ▾] │
│  [Upload & Play]         │
└──────────────────────────┘
```

### Mobile Layout (landscape, for timeline V2)

```
┌─────────────────────────────────────────────────┐
│ StatusBar (compact)  ● Connected  idle  🔋 82%   │
├─────────────────────────────────────────────────┤
│                                                 │
│  t=0s        t=5s       t=10s       t=15s       │
│  ├───────────┼──────────┼───────────┤           │
│  │ Pan   ●━━━━━━━●━━━━━━━━━━━━━━●  │           │
│  │ Tilt  ●━━━━━━━━━━━━●━━━━━━━━━●  │           │
│  │ Slide ●━━━━━━━━━━━━━━━━●━━━━━●  │           │
│  │ Roll  ●━━━━━━━━━━━━━━━━━━━━━━●  │           │
│  ├───────────┼──────────┼───────────┤           │
│  │          playhead ▼              │           │
│                                                 │
│  [|◀] [▶ Play] [Stop ■] [▶|]      ⚙           │
└─────────────────────────────────────────────────┘
```

### Desktop/Tablet Layout

```
┌──────────────────────────────────────────────────────────────┐
│  StatusBar  ● Connected   idle   🔋 82%   Firmware 1.0.0  ⚙ │
├────────────────────────────┬─────────────────────────────────┤
│                            │                                 │
│   JogControl               │   KeyframeList                  │
│   ┌────────────────────┐   │                                 │
│   │                    │   │   KF 1: Pan 0° Tilt 0° ...     │
│   │    (touch/drag)    │   │         [Go To] [Delete]        │
│   │                    │   │                                 │
│   └────────────────────┘   │   KF 2: Pan 45° Tilt -10° ...  │
│                            │         [Go To] [Delete]        │
│   Axis readouts:           │                                 │
│   Pan   34.2°  [----|--]   │   KF 3: Pan 90° Tilt 0° ...    │
│   Tilt  -8.1°  [--|----]   │         [Go To] [Delete]        │
│   Slide 423mm  [----|--]   │                                 │
│   Roll   0.0°  [---|---]   │   [+ Capture Current Position]  │
│                            │                                 │
├────────────────────────────┴─────────────────────────────────┤
│  TrajectoryPreview (V2)                                      │
│  ┌─────────────────────────────────────────────────────────┐ │
│  │  slide ▲                                                │ │
│  │        │    ╭──────╮                                    │ │
│  │        │   ╱        ╲                                   │ │
│  │        │──╱          ╲──                                │ │
│  │        └──────────────────▶ time                        │ │
│  └─────────────────────────────────────────────────────────┘ │
├──────────────────────────────────────────────────────────────┤
│  PlaybackControls                                            │
│  Duration: [5.0s]  Easing: [ease-in-out ▾]                  │
│  [Upload]  [|◀ Play ▶|]  [Pause] [Stop]   Progress: ████░░ │
└──────────────────────────────────────────────────────────────┘
```

---

## Component Architecture

### Component Tree

```
+layout.svelte
├── CapabilitiesProvider (context, not visible)
├── StatusBar
└── <slot/> (page content)

+page.svelte (/)
├── TabBar (Jog | Keyframes | Play) -- mobile only, desktop shows all
├── JogControl
│   └── AxisControl (one per axis, from capabilities)
├── KeyframeList
│   └── KeyframeCard (one per keyframe)
├── TrajectoryPreview (V2)
└── PlaybackControls

/settings/+page.svelte
├── ConnectionSettings
├── AxisCalibration
│   └── AxisCalibrationRow (per axis)
└── FirmwareInfo
```

### Component Specifications

#### StatusBar

Persistent across all routes (lives in `+layout.svelte`).

```typescript
// Props: none (reads from stores)

// Reads:
//   $connectionStore  — connection state
//   $stateStore       — system state (idle/moving/playing/paused/etc)
//   $positionStore    — not directly, but progress during playback
//   $capabilitiesStore — battery, firmware version

// Renders:
//   - Connection indicator: green dot (connected), yellow pulse (reconnecting), red dot (disconnected)
//   - System state badge: "idle", "moving", "playing", "paused", "homing", "error"
//   - Battery percentage (from periodic /api/status polls or telemetry)
//   - Playback progress bar (visible only during playing/paused states)
//   - Settings gear icon (link to /settings)

// Events: none (display only)
```

#### CapabilitiesProvider

Invisible context provider. Wraps the app in `+layout.svelte`.

```typescript
// Behavior:
//   1. On mount, subscribes to connectionStore.
//   2. When connected, fetches GET /api/capabilities.
//   3. Sets capabilitiesStore with the response.
//   4. Provides axis list via Svelte context so any descendant can call getContext('capabilities').
//   5. Re-fetches if connection drops and re-establishes.

// Context provided:
type Capabilities = {
  axes: Axis[];
  max_trajectory_points: number;
  max_trajectory_duration_ms: number;
  telemetry_rate_hz: number;
  firmware_version: string;
  board: string;
};

type Axis = {
  name: string;   // "pan", "tilt", "roll", "slide", etc.
  min: number;
  max: number;
  unit: string;   // "deg", "mm", etc.
};
```

#### JogControl

The primary interaction surface. A 2D touch/drag pad that sends jog commands.

```typescript
// Props:
interface JogControlProps {
  axes: Axis[];           // from capabilities — determines which axes are controllable
  primaryAxes: [string, string]; // default: ["pan", "slide"] — the two axes mapped to X/Y drag
}

// Reads:
//   $positionStore    — shows current position as crosshair on pad
//   $connectionStore  — disables if disconnected

// Writes:
//   Sends WebSocket jog commands via websocket client

// Events:
//   on:axisSwitch — dispatched when user changes which axes are mapped to X/Y

// Behavior:
//   - 2D pad: horizontal drag = primary X axis, vertical drag = primary Y axis
//   - Drag distance maps proportionally to jog speed (short drag = slow, edge = fast)
//   - Touch start: begin sending jog commands at throttled rate (20Hz)
//   - Touch end: send final jog with zero velocities, stop sending
//   - Axis switcher: dropdown or toggle to remap X/Y to different axis pairs
//   - Below the pad: AxisControl sliders for all axes (fine numeric control)
//   - Dead zone in center (5% radius) to prevent drift from imprecise touch release

// Size: full width on mobile, left half on desktop. Min height 200px, prefer 40vh.
```

#### AxisControl

Single-axis slider with numeric readout. Reused per axis.

```typescript
// Props:
interface AxisControlProps {
  axis: Axis;              // { name, min, max, unit }
  value: number;           // current position (from positionStore)
  disabled?: boolean;      // true when disconnected or during playback
}

// Events:
//   on:jog — dispatched with { axis: string, delta: number } when user drags slider
//   on:moveTo — dispatched with { axis: string, value: number } when user types a value and hits Enter

// Renders:
//   - Axis label with unit: "Pan (deg)" or "Slide (mm)"
//   - Horizontal range slider: min to max from capabilities
//   - Current value readout: numeric, 1 decimal place
//   - Manual input field: type a number, press Enter to move_to
//   - Visual fill showing current position within range
```

#### KeyframeList

Manages the list of saved keyframes.

```typescript
// Props: none (reads from keyframeStore)

// Reads:
//   $keyframeStore    — array of saved keyframes
//   $positionStore    — current position (for "capture" action)
//   $capabilitiesStore — axis names for display

// Writes:
//   keyframeStore     — add, delete, reorder keyframes

// Events:
//   on:goTo — dispatched with keyframe data, parent sends move_to command

// Renders:
//   - Ordered list of KeyframeCard components
//   - "Capture Current Position" button at bottom
//   - Drag handles for reordering (V1: up/down buttons; V2: drag-and-drop)
//   - Empty state message when no keyframes saved
```

#### KeyframeCard

Single keyframe display within the list.

```typescript
// Props:
interface KeyframeCardProps {
  keyframe: Keyframe;
  index: number;
  axes: Axis[];            // for display formatting
  isActive?: boolean;      // true if this is the current go-to target
}

// Types:
type Keyframe = {
  id: string;              // UUID
  label: string;           // user-editable, default "Keyframe N"
  positions: Record<string, number>;  // { pan: 45.0, tilt: -10.0, ... }
  createdAt: number;       // timestamp
};

// Events:
//   on:goTo — user wants to move to this keyframe's position
//   on:delete — remove this keyframe
//   on:moveUp / on:moveDown — reorder
//   on:updateLabel — rename

// Renders:
//   - Label (editable inline)
//   - Compact axis readout: "Pan 45° | Tilt -10° | Slide 500mm"
//   - [Go To] button — sends move_to for all axes
//   - [Delete] button (with confirmation on mobile)
//   - Drag handle or up/down arrows
```

#### PlaybackControls

Upload trajectory and control playback.

```typescript
// Props: none (reads from stores)

// Reads:
//   $keyframeStore       — needs 2+ keyframes to enable
//   $trajectoryStore     — upload state, computed trajectory
//   $stateStore          — current playback state
//   $connectionStore     — disables if disconnected
//   $capabilitiesStore   — max duration/points for validation

// Writes:
//   trajectoryStore      — triggers computation and upload

// Behavior:
//   Note: "moving" state means a move_to is in progress (not a trajectory playback).
//     During "moving", playback controls are disabled but a [Stop] button is available.
//
//   V1 flow:
//     1. User has 2+ keyframes saved.
//     2. User sets total duration (seconds) and picks easing preset from dropdown.
//     3. User taps "Upload & Play".
//     4. Component calls math library to compute trajectory point table.
//     5. Uploads via POST /api/trajectory.
//     6. Sends WebSocket "play" command.
//     7. Shows progress bar during playback (from telemetry).
//   V2 flow:
//     Trajectory comes from Timeline component instead of simple duration+easing.

// Renders:
//   - Duration input: number field, seconds, validated against max_trajectory_duration_ms
//   - Easing preset dropdown: "linear", "ease-in", "ease-out", "ease-in-out"
//   - [Upload & Play] button (disabled if <2 keyframes or disconnected)
//   - During playback: [Pause] [Stop] buttons, progress bar with time readout
//   - Upload state indicator: "computing...", "uploading...", "ready", "error"
//   - Validation messages: "Need at least 2 keyframes", "Duration exceeds board limit", etc.
//
//   Camera controls (always visible, independent of playback):
//   - [Photo] button — sends DJI shutter command via CAN bridge
//   - [Record] toggle — sends record_start on first press, record_stop on second press;
//     button shows red dot and "Recording..." label while active
```

#### Timeline (V2)

Full timeline editor for multi-keyframe sequences with per-axis control.

```typescript
// Props:
interface TimelineProps {
  keyframes: Keyframe[];
  axes: Axis[];
  totalDuration: number;   // seconds
}

// Reads:
//   $keyframeStore
//   $capabilitiesStore
//   $stateStore           — shows playhead during playback

// Writes:
//   trajectoryStore       — outputs computed trajectory

// Behavior:
//   - Horizontal scrollable timeline, pinch-to-zoom on mobile
//   - One lane per axis
//   - Keyframe dots on each lane, draggable along time axis
//   - Easing curve visualization between keyframes (rendered as SVG path)
//   - Per-axis delay/duration handles: drag start/end of each axis lane
//   - Playhead indicator showing current position during playback
//   - Time ruler at top with tick marks
//   - Snapping: keyframes snap to other keyframes across axes for alignment

// Events:
//   on:trajectoryChange — emitted when any timeline parameter changes, triggers recompute

// Subcomponents:
//   TimelineRuler — time axis with tick marks and labels
//   TimelineLane — one per axis, contains keyframe dots and easing curves
//   EasingCurveHandle — click to change easing type between two keyframes
//   PlayheadIndicator — vertical line showing current time
```

#### TrajectoryPreview (V2)

2D visualization of the computed path.

```typescript
// Props:
interface TrajectoryPreviewProps {
  trajectory: TrajectoryPoint[] | null;
  axes: Axis[];
  mode: 'time-series' | 'top-down';  // time-series: axis value vs time; top-down: pan vs slide
}

// Renders:
//   time-series mode:
//     - One line per axis, value on Y axis, time on X axis
//     - Color-coded per axis
//     - Vertical playhead line during playback
//   top-down mode:
//     - X = slide position, Y = pan angle
//     - Shows the spatial path the camera will trace
//     - Dot spacing indicates speed (close = slow, far = fast)

// Implementation:
//   Canvas 2D or SVG. Canvas preferred for performance with 3000+ point trajectories.
//   Falls back to SVG for <500 points (simpler, accessible).
```

---

## Svelte Stores

All stores live in `src/lib/stores/`. Each store is a separate file for testability.

### connectionStore

```typescript
// src/lib/stores/connection.ts

import { writable, derived } from 'svelte/store';

type ConnectionState = 'disconnected' | 'connecting' | 'connected' | 'reconnecting';

interface ConnectionStoreValue {
  state: ConnectionState;
  url: string | null;           // board WebSocket URL
  lastConnected: number | null; // timestamp
  error: string | null;
  reconnectAttempt: number;
}

// Usage:
//   connectionStore.connect('ws://192.168.4.1/ws')
//   connectionStore.disconnect()
//
// Auto-reconnect is built-in. On disconnect, transitions to 'reconnecting'
// and retries with exponential backoff (1s, 2s, 4s, 8s, max 30s).
//
// Derived:
//   $isConnected = derived(connectionStore, $c => $c.state === 'connected')
```

### capabilitiesStore

```typescript
// src/lib/stores/capabilities.ts

import { writable } from 'svelte/store';

// Set once after successful GET /api/capabilities.
// Reset to null on disconnect.
// All components that need axis info read from this store.

type CapabilitiesStore = Capabilities | null;

// Derived conveniences:
//   $axisNames  = derived(capabilitiesStore, $c => $c?.axes.map(a => a.name) ?? [])
//   $axisMap    = derived(capabilitiesStore, $c => Object.fromEntries($c?.axes.map(a => [a.name, a]) ?? []))
```

### positionStore

```typescript
// src/lib/stores/position.ts

import { writable } from 'svelte/store';

// Updated from WebSocket "position" events at up to 50Hz.
// Value is Record<string, number> — axis name to current value.
// Example: { pan: 34.2, tilt: -8.1, roll: 0.0, slide: 423.5 }
//
// The store setter is throttled on the read side — components update at
// most at requestAnimationFrame rate (~60fps) even if telemetry arrives
// at 50Hz. This avoids unnecessary re-renders.

type PositionStore = Record<string, number>;
```

### stateStore

```typescript
// src/lib/stores/state.ts

import { writable } from 'svelte/store';

type SystemState = 'idle' | 'moving' | 'playing' | 'paused' | 'homing' | 'error';

interface StateStoreValue {
  state: SystemState;
  error: { code: string; axis?: string; detail: string } | null;
  trajectoryId: string | null;      // currently loaded trajectory
  playbackProgress: number | null;  // 0.0-1.0 during playback
  playbackElapsedMs: number | null;
}

// Updated from WebSocket "state" events and "complete" events.
// Also updated from periodic GET /api/status polls as a fallback
// (every 5s when no WebSocket telemetry received).
```

### keyframeStore

```typescript
// src/lib/stores/keyframes.ts

import { writable } from 'svelte/store';

interface Keyframe {
  id: string;
  label: string;
  positions: Record<string, number>;
  createdAt: number;
}

// Persisted to localStorage under key 'slider-keyframes'.
// On init: load from localStorage. On every change: write to localStorage.
//
// Methods (via a custom store wrapper):
//   keyframeStore.add(positions)      — capture current position as new keyframe
//   keyframeStore.remove(id)          — delete by ID
//   keyframeStore.reorder(fromIndex, toIndex)
//   keyframeStore.updateLabel(id, label)
//   keyframeStore.clear()             — remove all
//   keyframeStore.exportJSON()        — serialize for file save (V2)
//   keyframeStore.importJSON(data)    — load from file (V2)
```

### trajectoryStore

```typescript
// src/lib/stores/trajectory.ts

import { writable } from 'svelte/store';

type TrajectoryState = 'empty' | 'computing' | 'computed' | 'uploading' | 'uploaded' | 'error';

interface TrajectoryStoreValue {
  state: TrajectoryState;
  points: TrajectoryPoint[] | null;   // computed trajectory
  trajectoryId: string | null;        // ID returned by board after upload
  error: string | null;
  pointCount: number;
  durationMs: number;
}

type TrajectoryPoint = {
  t: number;                          // milliseconds
  [axisName: string]: number;         // axis values
};

// Methods:
//   trajectoryStore.compute(keyframes, durationMs, easing, capabilities)
//     — calls math library, sets state to 'computing' then 'computed'
//   trajectoryStore.upload()
//     — POST /api/trajectory, sets state to 'uploading' then 'uploaded'
//   trajectoryStore.clear()
//     — DELETE /api/trajectory/:id, reset to 'empty'
```

---

## API Client

### REST Client — `src/lib/api/client.ts`

```typescript
// Thin fetch wrapper. All methods return typed responses.
// Base URL: relative in production ('/api/...'), configurable in dev.

class BoardClient {
  constructor(baseUrl: string);

  async getStatus(): Promise<StatusResponse>;
  async getCapabilities(): Promise<Capabilities>;
  async uploadTrajectory(trajectory: TrajectoryUpload): Promise<TrajectoryUploadResponse>;
  async deleteTrajectory(id: string): Promise<void>;
  async getTrajectoryStatus(id: string): Promise<TrajectoryStatusResponse>;
}

// Base URL resolution:
//   - Dev: import.meta.env.VITE_BOARD_URL ?? ''
//   - Production (on ESP32): '' (same origin, relative paths)
```

### WebSocket Client — `src/lib/api/websocket.ts`

```typescript
// Manages the WebSocket connection lifecycle and message routing.

class BoardWebSocket {
  constructor(url: string, handlers: WebSocketHandlers);

  connect(): void;
  disconnect(): void;
  isConnected(): boolean;

  // Sending (client -> board)
  sendJog(axes: Record<string, number>): void;     // throttled to maxSendRate
  sendMoveTo(positions: Record<string, number>, durationMs: number): void;
  sendPlay(trajectoryId: string): void;
  sendPause(): void;
  sendResume(): void;
  sendStop(): void;
  sendScrub(timeMs: number): void;
  sendHome(axes: string[]): void;
}

interface WebSocketHandlers {
  onPosition: (data: PositionEvent) => void;
  onState: (data: StateEvent) => void;
  onComplete: (data: CompleteEvent) => void;
  onError: (data: ErrorEvent) => void;
  onConnectionChange: (state: ConnectionState) => void;
}

// Internal details:
//   - Auto-reconnect: on close/error, wait backoffMs then reconnect.
//     Backoff: 1000, 2000, 4000, 8000, 16000, 30000 (capped).
//     Reset backoff on successful connection.
//   - Jog throttle: sendJog() is throttled to max 20 calls/sec.
//     Uses trailing-edge throttle so the last position is always sent.
//   - Message format: JSON.stringify({ cmd, ...params })
//   - Parse incoming: JSON.parse, route by 'evt' field to handler.
//   - Heartbeat: if no message received for 5s, send a ping.
//     If no pong in 3s, consider connection dead and reconnect.
```

### Types — shared package

Types are imported from a shared package at the project root (`shared/types.ts` or similar), **not** defined locally in `src/lib/api/types.ts`. Both the web UI and the mock server import from this shared source to keep API types in sync.

```typescript
// Re-exported from shared/types.ts. Matches board-api.md exactly.

// --- Capabilities ---
export interface Capabilities {
  axes: Axis[];
  max_trajectory_points: number;
  max_trajectory_duration_ms: number;
  telemetry_rate_hz: number;
  firmware_version: string;
  board: string;
}

export interface Axis {
  name: string;
  min: number;
  max: number;
  unit: string;
}

// --- Status ---
export interface StatusResponse {
  state: SystemState;
  position: Record<string, number>;
  battery: number;
  trajectory_loaded: string | null;
}

export type SystemState = 'idle' | 'moving' | 'playing' | 'paused' | 'homing' | 'error';

// --- Trajectory ---
export interface TrajectoryUpload {
  points: TrajectoryPoint[];
  loop: boolean;
}

export interface TrajectoryPoint {
  t: number;
  [axis: string]: number;
}

export interface TrajectoryUploadResponse {
  trajectory_id: string;
  point_count: number;
  duration_ms: number;
}

export interface TrajectoryStatusResponse {
  state: 'playing' | 'paused' | 'idle' | 'moving';
  elapsed_ms: number;
  progress: number;
}

// --- WebSocket: Client -> Board ---
export type ClientMessage =
  | { cmd: 'play'; trajectory_id: string }
  | { cmd: 'pause' }
  | { cmd: 'resume' }
  | { cmd: 'stop' }
  | { cmd: 'scrub'; t: number }
  | { cmd: 'jog'; [axis: string]: number | string }  // cmd + axis deltas
  | { cmd: 'move_to'; duration_ms: number; [axis: string]: number | string }
  | { cmd: 'home'; axes: string[] };

// --- WebSocket: Board -> Client ---
export type BoardEvent =
  | PositionEvent
  | StateEvent
  | CompleteEvent
  | BoardErrorEvent;

export interface PositionEvent {
  evt: 'position';
  t: number;
  [axis: string]: number | string;
}

export interface StateEvent {
  evt: 'state';
  state: SystemState;
}

export interface CompleteEvent {
  evt: 'complete';
  trajectory_id: string;
}

export interface BoardErrorEvent {
  evt: 'error';
  code: 'LIMIT_HIT' | 'MOTOR_STALL' | 'TRAJECTORY_INVALID' | 'OUT_OF_MEMORY' | 'COMMUNICATION_LOST';
  axis?: string;
  detail: string;
}
```

### Multiple Clients

Multiple browsers can connect to the board simultaneously. All clients receive telemetry and state updates via their own WebSocket connections. Commands are **last-writer-wins** — there is no locking or ownership mechanism. This is intentional: a common workflow is phone (jog control) + laptop (timeline editing) side by side.

---

## V1 vs V2 Scope

### V1 — Minimum Viable UI

Everything needed to go from "plug in the board" to "execute a smooth camera move."

| Feature | Details |
|---------|---------|
| **Connect to board** | Captive portal auto-opens UI when phone joins slider WiFi AP. Fallback: navigate to `http://192.168.4.1`. Dev mode uses `VITE_BOARD_URL`; production uses same-origin relative URLs. |
| **Jog control** | 2D touch pad for two axes, with axis-pair selector. Per-axis sliders below. Works on phone and laptop. |
| **Axis readouts** | Live position display for all axes, updated from telemetry. |
| **Save keyframes** | Capture current position. Minimum 2 keyframes to define a move. Stored in localStorage. |
| **Manage keyframes** | View list, delete, reorder (up/down buttons), rename, go-to. |
| **Simple move** | Pick duration (seconds) + easing preset (linear, ease-in, ease-out, ease-in-out). Compute trajectory, upload, play. |
| **Playback control** | Play, pause, resume, stop. Progress bar with elapsed time. |
| **Status display** | Connection state, system state, battery. Error display with human-readable messages. |
| **Responsive** | Works on phone (portrait) and laptop. Touch and mouse input. |
| **Camera trigger** | DJI shutter and record_start/record_stop commands. Add trigger markers to the playback timeline. V1: manual buttons (take photo, start/stop recording). Trajectory event triggers are V2. |
| **Settings page** | Device name, AP SSID/password, telemetry rate, jog sensitivity, home-on-boot, firmware info. Save via `PATCH /api/settings`. WiFi changes warn and offer restart. |

**V1 explicitly excludes:**
- Timeline editor
- Per-axis timing
- Custom easing curves (only presets)
- Trajectory preview visualization
- Undo/redo
- File save/load of sequences
- Drag-and-drop keyframe reordering (use up/down buttons)
- Homing UI (homing command exists in API but no UI for it in V1)

### V2 — Full Timeline

Adds the timeline editor described in initial-vision.md.

| Feature | Details |
|---------|---------|
| **Timeline editor** | Horizontal multi-lane timeline. One lane per axis. Keyframes as draggable dots along time axis. |
| **Per-axis timing** | Drag axis lane start/end to offset when each axis begins and finishes its motion. |
| **Custom easing** | Click between two keyframes to open a bezier curve editor. Per-segment easing. |
| **Trajectory preview** | Canvas-based 2D visualization. Time-series mode (value vs time) and top-down mode (pan vs slide). |
| **N keyframes** | Support 10+ keyframes per sequence. |
| **Undo/redo** | Command pattern for all timeline edits. Ctrl+Z / Ctrl+Shift+Z. |
| **Save/load sequences** | Export sequence (keyframes + timing + easing) as JSON file. Import from file. |
| **Drag reorder** | Touch-friendly drag-and-drop in keyframe list. |
| **Scrub preview** | Drag playhead on timeline, board moves to that position in real-time via `scrub` command. |

### V2+ (Future)

- Loop mode with configurable repeat count
- Homing UI with per-axis home buttons and status
- Multi-sequence management (save library of moves)
- Camera trigger trajectory events (auto-trigger at keyframe or time marker)
- Time-lapse mode (slow motion with photo intervals)
- Focus axis control (follow-focus motor driven via DJI CAN)

---

## Responsive Design

### Breakpoints

```css
/* Mobile-first. Base styles target phones (< 640px). */
@media (min-width: 640px)  { /* Tablet  */ }
@media (min-width: 1024px) { /* Desktop */ }
@media (orientation: landscape) and (max-height: 500px) { /* Phone landscape */ }
```

### Layout Strategy

| Viewport | Layout |
|----------|--------|
| Phone portrait | Single column. JogControl on top, tabs below for Keyframes / Playback. StatusBar fixed top. |
| Phone landscape | Timeline view (V2). Controls collapse to minimal strip at bottom. |
| Tablet | Two columns: JogControl left, KeyframeList right. PlaybackControls full-width below. |
| Desktop | Two columns with optional TrajectoryPreview below. More generous spacing. |

### Touch Considerations

- **JogControl touch pad:** Uses `touchstart`, `touchmove`, `touchend` events (not just mouse). Prevents default scroll when touching the pad. Multi-touch: ignore second finger.
- **All buttons and interactive targets:** Minimum 44x44px tap area. Padding around icons if the icon itself is smaller.
- **Swipe gestures:** Avoided to prevent conflicts with browser gestures (back/forward swipe on iOS Safari).
- **Jog sensitivity:** Configurable via a sensitivity slider in settings. Default tuned for phone screen size.

---

## Build and Bundle Size

### Size Budget

| Asset | Budget |
|-------|--------|
| JavaScript (gzipped) | < 300KB |
| CSS (gzipped) | < 30KB |
| HTML | < 10KB |
| Fonts | 0KB (use system fonts) |
| **Total** | **< 340KB** (well within 500KB limit) |

SvelteKit with static adapter produces minimal JS. A typical SvelteKit app with no heavy dependencies gzips to ~50-80KB of framework code. The remainder is application code and the math library.

### Bundle Rules

1. **No large dependencies.** Before adding any npm package, check its bundled size on bundlephobia.com. Hard limit: no single dependency > 50KB minified.
2. **System fonts only.** Use `font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif` and the mono var for numbers.
3. **Tree-shake everything.** Import only what's needed. No barrel imports from large packages.
4. **Math library is local.** `src/lib/math/` — no external math dependency (no mathjs, no glmatrix). Hand-rolled interpolation functions.
5. **Canvas over SVG for large datasets.** TrajectoryPreview uses Canvas for 500+ point trajectories to avoid DOM bloat.
6. **Lazy-load V2 features.** Timeline and TrajectoryPreview are dynamic imports, not included in initial bundle.

### Build Output

```bash
npm run build
# Output: build/
#   index.html
#   settings.html (prerendered)
#   _app/
#     immutable/
#       entry/
#         start-[hash].js
#         app-[hash].js
#       chunks/
#         ...
#     version.json

# Copy to firmware:
cp -r build/* ../firmware/data/
```

The firmware build system copies `build/` contents into `data/` for LittleFS upload to the ESP32.

### Size Monitoring

Add a build step that reports gzipped sizes and fails if any budget is exceeded:

```bash
# In package.json scripts:
"build:check-size": "node scripts/check-bundle-size.js --max-total-kb 500"
```

---

## Development Workflow

### Dev Server Setup

```bash
npm run dev
# Starts SvelteKit dev server on http://localhost:5173
# Proxies /api/* and /ws to mock board server
```

### Mock Board Server

A lightweight Node.js (or Bun) server that implements the board API for development.

Location: `src/mock-server/` (separate from the SvelteKit app, not bundled into production).

```bash
npm run mock
# Starts mock board server on http://localhost:3001
# Implements all REST endpoints and WebSocket with simulated telemetry
```

The mock server:
- Returns realistic capabilities (4 axes: pan, tilt, roll, slide)
- Accepts trajectory uploads, validates format, stores in memory
- Simulates playback: emits position events at 50Hz, walking through uploaded trajectory
- Simulates jog: updates internal position state and emits telemetry
- Simulates move_to: interpolates linearly from current to target over duration_ms
- Simulates errors: special axis value `-999` triggers a `LIMIT_HIT` error
- Simulates disconnect: send `{ cmd: "simulate_disconnect" }` to test reconnection

### Environment Variables

```bash
# .env.development
VITE_BOARD_URL=http://localhost:3001   # mock server URL

# .env.production
# (empty — production uses relative URLs, same origin as ESP32)
```

### Vite Config

```typescript
// vite.config.ts
import { sveltekit } from '@sveltejs/kit/vite';
import { defineConfig } from 'vite';

export default defineConfig({
  plugins: [sveltekit()],
  server: {
    proxy: {
      '/api': {
        target: 'http://localhost:3001',
        changeOrigin: true,
      },
      '/ws': {
        target: 'ws://localhost:3001',
        ws: true,
      },
    },
  },
  test: {
    include: ['src/**/*.test.ts'],
    environment: 'jsdom',
    setupFiles: ['src/test-setup.ts'],
  },
});
```

### NPM Scripts

```json
{
  "scripts": {
    "dev": "vite dev",
    "build": "vite build",
    "build:check-size": "node scripts/check-bundle-size.js --max-total-kb 500",
    "preview": "vite preview",
    "mock": "node src/mock-server/index.js",
    "dev:full": "concurrently \"npm run dev\" \"npm run mock\"",
    "test": "vitest run",
    "test:watch": "vitest",
    "test:coverage": "vitest run --coverage",
    "check": "svelte-kit sync && svelte-check --tsconfig ./tsconfig.json"
  }
}
```

---

## Test Strategy

### Unit Tests (Vitest)

Location: co-located with source files as `*.test.ts`.

| Area | What to test |
|------|-------------|
| Math library (`lib/math/`) | Interpolation functions, easing curves, trajectory generation. Property-based: output array length matches expected point count, values stay within axis bounds, timestamps are monotonically increasing. |
| Store logic (`lib/stores/`) | Keyframe store: add/remove/reorder, localStorage persistence. Trajectory store: state transitions (empty -> computing -> computed -> uploading -> uploaded). Connection store: state machine transitions. |
| API client (`lib/api/client.ts`) | Request URL construction, request body formatting, response parsing, error handling for non-200 responses. Mock `fetch`. |
| WebSocket client (`lib/api/websocket.ts`) | Message serialization, jog throttling (verify max send rate), reconnect backoff timing, handler dispatch by event type. Mock `WebSocket`. |
| Type validation | Validate that API types match board-api.md examples (snapshot tests on example payloads). |

### Component Tests (Vitest + @testing-library/svelte)

Location: co-located as `*.test.ts` alongside component files.

| Component | What to test |
|-----------|-------------|
| StatusBar | Renders correct indicator for each connection state. Shows battery. Shows state badge. |
| AxisControl | Slider reflects value prop. Input field dispatches moveTo event. Disabled state. |
| KeyframeCard | Displays axis values formatted with units. Go-to and delete buttons dispatch events. |
| KeyframeList | Renders correct number of cards. Capture button dispatches with current position. Empty state. |
| PlaybackControls | Disabled when <2 keyframes. Duration validation. State transitions during upload+play. |
| JogControl | Touch events compute correct axis deltas. Dead zone in center. Disabled when disconnected. |

### Integration Tests

Longer tests that wire up multiple components with the mock server.

```
Scenario: Full keyframe-to-playback flow
  1. Connect to mock server
  2. Verify capabilities loaded (4 axes displayed)
  3. Jog to a position (send jog commands, verify position updates)
  4. Capture keyframe 1
  5. Jog to different position
  6. Capture keyframe 2
  7. Set duration to 5s, easing to ease-in-out
  8. Click "Upload & Play"
  9. Verify trajectory uploaded (mock server received POST /api/trajectory)
  10. Verify playback started (state changes to 'playing')
  11. Verify progress bar updates
  12. Verify playback completes (state returns to 'idle')
```

### Manual Testing Checklist

Run through before each release:

- [ ] **Phone (iOS Safari):** Jog pad touch works, no page scroll during drag
- [ ] **Phone (Android Chrome):** Same as above
- [ ] **Laptop (Chrome):** Mouse drag on jog pad, keyboard input on fields
- [ ] **Laptop (Firefox):** WebSocket connects, telemetry updates render
- [ ] **WiFi drop:** Disconnect WiFi, verify reconnecting state, reconnect, verify recovery
- [ ] **Error handling:** Upload invalid trajectory, verify error message displayed
- [ ] **localStorage:** Save keyframes, reload page, verify keyframes persist
- [ ] **Two keyframe move:** Full flow from jog to capture to play to complete
- [ ] **Responsive:** Rotate phone landscape/portrait, resize laptop window
- [ ] **High contrast:** Outdoors in sunlight, verify readability

---

## Accessibility

### Requirements

| Concern | Approach |
|---------|----------|
| **Touch targets** | All interactive elements min 44x44px. Apply `min-height: var(--touch-min); min-width: var(--touch-min)` to buttons and controls. |
| **Keyboard navigation** | All controls reachable via Tab. JogControl: arrow keys move axes. Enter/Space activates buttons. Esc closes modals/dropdowns. |
| **ARIA labels** | Jog pad: `role="application"` with `aria-label="Manual axis control. Use arrow keys or touch to move."`. Sliders: `role="slider"` with `aria-valuemin`, `aria-valuemax`, `aria-valuenow`. Status indicators: `aria-live="polite"` for connection and state changes. |
| **Color contrast** | WCAG AA minimum (4.5:1 for text, 3:1 for large text/graphics). High-contrast mode boosts to AAA. Test with browser devtools contrast checker. |
| **Focus indicators** | Visible focus ring on all interactive elements. Custom focus style: `outline: 2px solid var(--color-accent); outline-offset: 2px`. |
| **Screen reader** | Connection state changes announced via `aria-live`. Error messages announced. Keyframe list uses `role="list"` with meaningful item labels. |
| **Reduced motion** | `@media (prefers-reduced-motion: reduce)` — disable CSS transitions and animations. Telemetry updates still function, just without smooth visual interpolation. |

---

## File Tree

Planned file structure for the `ui/` directory:

```
ui/
├── package.json
├── svelte.config.js
├── tsconfig.json
├── vite.config.ts
├── .env.development
├── static/
│   └── favicon.png
├── scripts/
│   └── check-bundle-size.js
├── src/
│   ├── app.html
│   ├── app.css                          # CSS custom properties, global reset
│   ├── test-setup.ts                    # Vitest setup (jsdom, cleanup)
│   ├── lib/
│   │   ├── api/
│   │   │   ├── client.ts               # REST client (BoardClient class)
│   │   │   ├── client.test.ts
│   │   │   ├── websocket.ts            # WebSocket client (BoardWebSocket class)
│   │   │   ├── websocket.test.ts
│   │   │   └── types.ts                # Re-exports from shared/types.ts
│   │   ├── stores/
│   │   │   ├── connection.ts           # connectionStore
│   │   │   ├── connection.test.ts
│   │   │   ├── capabilities.ts         # capabilitiesStore
│   │   │   ├── capabilities.test.ts
│   │   │   ├── position.ts             # positionStore (50Hz telemetry)
│   │   │   ├── position.test.ts
│   │   │   ├── state.ts                # stateStore (system state)
│   │   │   ├── state.test.ts
│   │   │   ├── keyframes.ts            # keyframeStore (localStorage-backed)
│   │   │   ├── keyframes.test.ts
│   │   │   ├── trajectory.ts           # trajectoryStore (computed trajectory)
│   │   │   └── trajectory.test.ts
│   │   ├── math/                        # Motion math library (separate module)
│   │   │   └── (see plan-motion-math.md)
│   │   └── utils/
│   │       ├── throttle.ts             # Trailing-edge throttle for jog
│   │       └── format.ts              # Axis value formatting (e.g., "45.0°", "500mm")
│   ├── components/
│   │   ├── StatusBar.svelte
│   │   ├── StatusBar.test.ts
│   │   ├── JogControl.svelte
│   │   ├── JogControl.test.ts
│   │   ├── AxisControl.svelte
│   │   ├── AxisControl.test.ts
│   │   ├── KeyframeList.svelte
│   │   ├── KeyframeList.test.ts
│   │   ├── KeyframeCard.svelte
│   │   ├── KeyframeCard.test.ts
│   │   ├── PlaybackControls.svelte
│   │   ├── PlaybackControls.test.ts
│   │   ├── TabBar.svelte               # Mobile tab navigation
│   │   ├── CapabilitiesProvider.svelte
│   │   ├── Timeline.svelte             # V2
│   │   ├── TimelineLane.svelte         # V2
│   │   ├── TrajectoryPreview.svelte    # V2
│   │   └── EasingCurveHandle.svelte    # V2
│   ├── routes/
│   │   ├── +layout.svelte              # Root layout: CapabilitiesProvider + StatusBar + slot
│   │   ├── +layout.ts                  # Prerender config for static adapter
│   │   ├── +page.svelte                # Main control interface
│   │   └── settings/
│   │       └── +page.svelte            # Settings page
│   └── mock-server/
│       ├── index.js                    # Mock board server entry point
│       ├── rest.js                     # REST endpoint handlers
│       ├── websocket.js                # WebSocket handler with simulated telemetry
│       └── state.js                    # Simulated board state
└── build/                              # Static build output (git-ignored)
```
