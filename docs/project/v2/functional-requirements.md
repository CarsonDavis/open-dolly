# V2 Functional Requirements

Status: **active**

Every capability the V2 UI must support. Written as testable "the user should be able to..." statements, organized by feature area. Derived from [`vision/v2-ui-vision.md`](../vision/v2-ui-vision.md), [`roadmap.md`](../roadmap.md), and [`docs/research/slider-ui-ux/report.md`](../../research/slider-ui-ux/report.md).

---

## 1. Keyframe Management

### Capture
- **FR-KF-1:** The user should be able to capture a keyframe from the current hardware position with one tap.
- **FR-KF-2:** Each captured keyframe stores the exact position of every axis reported by the board at the moment of capture.
- **FR-KF-3:** Keyframes are automatically labeled sequentially ("Keyframe 1", "Keyframe 2", ...) and the label is editable.

### Display
- **FR-KF-4:** Keyframe cards display a summary of all axis values (e.g., "Pan 45.0° | Tilt -10.0° | Slide 500mm").
- **FR-KF-5:** Keyframes are displayed in sequence order — top-to-bottom in portrait, left-to-right or top-to-bottom in landscape/desktop.

### Selection
- **FR-KF-6:** The user should be able to select a keyframe by tapping its card.
- **FR-KF-7:** Selecting a keyframe in interactive mode sends a `move_to` command, moving the hardware to that keyframe's stored position.
- **FR-KF-8:** While a keyframe is selected, any control panel movement (jog, increment, typed input) overwrites that keyframe's stored positions with the new values.
- **FR-KF-9:** Deselecting a keyframe (tap elsewhere or explicit deselect action) returns to free jog mode — control panel movements affect only the hardware, not any stored keyframe.
- **FR-KF-10:** In free jog mode, the "Capture Keyframe" button is visible and active. While a keyframe is selected, it is hidden or replaced with an indicator showing which keyframe is being edited.

### Editing
- **FR-KF-11:** The user should be able to reorder keyframes by drag or move-up/move-down buttons. Reordering preserves or adjusts associated transitions.
- **FR-KF-12:** The user should be able to delete a keyframe. Deleting removes the keyframe and merges or removes its adjacent transitions.
- **FR-KF-13:** The user should be able to rename a keyframe via inline text editing on the card.

### Persistence
- **FR-KF-14:** The complete sequence (keyframes + transitions) persists to localStorage and survives page reloads.
- **FR-KF-15:** V1 localStorage data (flat keyframe array without transitions) is automatically migrated to the V2 sequence format on load.
- **FR-KF-16:** The user should be able to clear the entire sequence (all keyframes and transitions) with a confirmation prompt.

---

## 2. Transitions

### Creation
- **FR-TR-1:** A transition is automatically created between every pair of adjacent keyframes. The user does not create transitions manually.
- **FR-TR-2:** Each transition has a duration in seconds and per-axis curve definitions. Default: linear curves on all axes, duration unset.
- **FR-TR-3:** Duration entry is blocking — the user must set a valid duration (>0) for all transitions before playback. *Current implementation: validated at playback time (`allTransitionsSet()` check in PlaybackControls). UI does not yet prevent navigation or capture with unset durations — unset brackets pulse to draw attention.*

### Simple View
- **FR-TR-4:** The transition simple view shows a duration input field (seconds) as the primary control.
- **FR-TR-5:** Below the duration, the simple view shows derived per-axis speed indicators for each axis that changes between the two keyframes (e.g., "125 mm/s", "4.5 °/s"). Axes with no delta are hidden.
- **FR-TR-6:** Speed indicators use the correct unit: mm/s for linear axes, °/s for rotation axes.
- **FR-TR-7:** The user should be able to tap a speed indicator to switch that axis to speed-primary input mode. Editing the speed auto-adjusts the duration. All other speeds update accordingly.
- **FR-TR-8:** Only one input mode is active at a time: either duration-primary or speed-primary for a specific axis.

### Duration Brackets
- **FR-TR-9:** Between each pair of keyframe cards, a visual bracket displays the transition duration in seconds.
- **FR-TR-10:** Tapping a duration bracket opens the transition simple view for that transition.

### Auto-Open
- **FR-TR-13:** When a 2nd or subsequent keyframe is captured, the transition simple view automatically opens for the new transition, prompting the user to set a duration.

### Advanced Curves
- **FR-TR-11:** The transition simple view has an "Advanced" button that opens the per-axis curve editor for that transition.
- **FR-TR-12:** If the user edits a speed in the simple view after creating a custom curve on that axis, the system warns that this will reset the curve to linear. The user can accept or cancel.

---

## 3. Curve Editor

### Layout
- **FR-CE-1:** The curve editor displays stacked per-axis lanes — one lane per axis, all visible simultaneously (Premiere-style inline lanes).
- **FR-CE-2:** Only axes with a non-zero delta between the two keyframes are shown by default.
- **FR-CE-3:** Each lane spans the duration of the transition on the X-axis (labeled in seconds) and normalized progress [0, 1] on the Y-axis.
- **FR-CE-4:** The default curve for each axis is a straight line from (0, 0) to (1, 1) — linear interpolation.

### Point Manipulation
- **FR-CE-5:** The user should be able to add a control point by clicking/tapping on the curve line.
- **FR-CE-6:** The user should be able to drag a control point to reshape the curve. The point's time (t) is constrained between its neighboring points. Progress is clamped to [0, 1].
- **FR-CE-7:** The user should be able to delete a control point by dragging it off the graph area.
- **FR-CE-8:** Touch interaction: long-press to add a point, drag to move, swipe off to delete.
- **FR-CE-9:** The curve always passes through the user's control points (interpolating, not approximating).

### Curve Math
- **FR-CE-10:** Curves use monotone interpolation — the progress function is non-decreasing. This prevents motor direction reversals within a transition.
- **FR-CE-11:** The curve maps normalized time [0, 1] to normalized progress [0, 1]. Endpoints (0, 0) and (1, 1) are implicit and cannot be moved or deleted.

### Multi-Axis Linking
- **FR-CE-12:** Each axis lane has a checkbox for linking.
- **FR-CE-13:** When multiple checkboxes are selected, editing the curve on any linked axis applies the same curve shape to all linked axes.

### Easing Presets
- **FR-CE-14:** The curve editor provides preset buttons (linear, ease in, ease out, ease in-out) that populate the lane with control points approximating the preset shape.
- **FR-CE-15:** After applying a preset, the user can further edit the generated control points.

### Navigation
- **FR-CE-16:** A "Done" / "Back" button exits the curve editor and returns to the transition simple view / control panel.
- **FR-CE-17:** The curve editor replaces the control panel area while open (on phone). On desktop, it may appear inline or in a panel.

---

## 4. Control Panel

### Movement Modes
- **FR-CP-1:** The control panel provides a toggle between two movement modes: **Position Bar** and **Jog** (default: Jog).
- **FR-CP-2:** **Position Bar mode:** A horizontal bar representing the full range of the axis (min to max). Tapping anywhere on the bar moves the hardware to that position via `move_to`.
- **FR-CP-3:** **Jog mode:** A bidirectional drag control where movement speed is proportional to how far the user drags from center. Releasing stops movement.

### Increment Buttons
- **FR-CP-4:** Each axis has fine and coarse increment buttons at each end of the control:
  - Linear axes: +1 mm (fine), +10 mm (coarse)
  - Rotation axes: +1° (fine), +15° (coarse)
- **FR-CP-5:** Increment buttons move the hardware by the specified amount relative to the current position.

### Typed Input
- **FR-CP-6:** The user should be able to tap the current value readout to enter an exact position via typed numeric input.
- **FR-CP-7:** Typed values are rounded/snapped to the nearest position achievable by the hardware, based on axis resolution reported by capabilities.
- **FR-CP-8:** The UI does not allow precision that the hardware cannot achieve.

### Interactive vs. Static Mode
- **FR-CP-9:** The control panel has a toggle between Interactive mode (default) and Static editing mode.
- **FR-CP-10:** **Interactive mode:** Control panel changes move the hardware in real-time. The displayed values reflect actual sensor readings from the board.
- **FR-CP-11:** **Static editing mode:** Control panel changes update stored values only — no hardware commands are sent. Useful for offline sequence planning.

### Movement Speed
- **FR-CP-12:** Manual repositioning speed defaults to 25% of each axis's maximum speed.
- **FR-CP-13:** Movement speed is configurable per-axis in settings.
- **FR-CP-14:** Movement speed applies only to manual repositioning (control panel), not to keyframe playback.

### Layout
- **FR-CP-15:** The control panel is collapsible to a thin tray showing only the "Capture Keyframe" button.
- **FR-CP-16:** The control panel displays one axis strip per axis reported by the board's capabilities.

---

## 5. Timeline View

### Layout
- **FR-TL-1:** The timeline view displays a horizontal, left-to-right timeline spanning the full sequence duration.
- **FR-TL-2:** One horizontal lane per axis, stacked vertically, each showing the curve for that axis's motion.
- **FR-TL-3:** Keyframe positions appear as vertical markers across all lanes, with small boxes showing axis values at each keyframe.
- **FR-TL-4:** A time scale at the bottom shows second markers.

### Keyframe Retiming
- **FR-TL-5:** The user should be able to drag a keyframe marker left or right to change its timing.
- **FR-TL-6:** Dragging redistributes duration between the two adjacent transitions. The total sequence duration and all other transitions remain unchanged.

### Inline Curve Editing
- **FR-TL-7:** Each axis lane shows a mini progress curve view for each transition.
- **FR-TL-8:** The user should be able to click/tap an axis lane to expand it for curve editing (same interactions as the curve editor).

### Scrub Preview
- **FR-TL-9:** The timeline has a draggable playhead (vertical line).
- **FR-TL-10:** Dragging the playhead sends `scrub` commands to the board — the hardware moves to the corresponding trajectory position in real-time.
- **FR-TL-11:** Scrub requires a trajectory to be uploaded. The system auto-uploads the trajectory when the sequence changes (debounced).
- **FR-TL-12:** Scrub commands are throttled to 20Hz to avoid flooding the board.

### Availability
- **FR-TL-13:** The timeline view is available on desktop (>1024px) and tablet (640-1024px).
- **FR-TL-14:** On phone (<640px), a simplified timeline (keyframe track + playhead for scrubbing, lanes collapsed by default) is available as an alternative view.

---

## 6. Buffers (Anti-Jitter)

### Behavior
- **FR-BF-1:** When buffers are enabled, the system automatically adds hold periods (settle zones) at keyframe boundaries before motion starts and after motion ends.
- **FR-BF-2:** Default buffer duration is exactly 1 second on each side — designed for clean trim points in video editing software.
- **FR-BF-3:** Buffer duration is configurable per-axis.

### Clearance Checks
- **FR-BF-4:** Before generating buffers, the system checks if each axis has physical room (distance from axis limits) for the buffer travel.
- **FR-BF-5:** When a buffer cannot be generated for an axis at a keyframe (insufficient room), the system displays a clear warning message identifying the axis and keyframe.
- **FR-BF-6:** The user can proceed without the buffer or adjust keyframe positions.

### Configuration
- **FR-BF-7:** Buffers have a global on/off toggle in settings.
- **FR-BF-8:** Buffers can be enabled/disabled per-transition as an override.
- **FR-BF-9:** A stiffness/inertia parameter is configurable (affects how much extra travel the buffer needs). Exact input TBD pending hardware testing.

---

## 7. Playback

- **FR-PB-1:** The user should be able to upload and play the full sequence trajectory with one tap ("Upload & Play").
- **FR-PB-2:** Playback requires at least 2 keyframes and all transitions must have a valid duration (>0).
- **FR-PB-3:** The system generates a dense trajectory point table from all keyframes and transitions, uploads it to the board, then sends a play command.
- **FR-PB-4:** During playback, the user should be able to pause, resume, and stop.
- **FR-PB-5:** A progress bar shows playback position in real-time.
- **FR-PB-6:** Playback is autonomous — once the trajectory is uploaded and play is sent, the board executes without needing network.
- **FR-PB-7:** Errors from the board (limit hit, motor stall, etc.) are displayed clearly during playback.

---

## 8. Responsive Layout

- **FR-RL-1:** **Portrait phone (<640px):** Keyframe cards in a vertical scroll area at top, collapsible control panel tray at bottom. No tab bar.
- **FR-RL-2:** **Landscape phone/tablet (640-1024px):** Split screen — control panel on the left, keyframe list on the right.
- **FR-RL-3:** **Desktop (>1024px):** Same as landscape plus timeline view available below or as a toggle.
- **FR-RL-4:** The keyframe view and timeline view operate on the same data and can be switched between freely.
- **FR-RL-5:** All touch targets meet a minimum size of 44px.
- **FR-RL-6:** The dev server is accessible from a phone on the same Wi-Fi network (already works via Vite).

---

## 9. Settings

- **FR-ST-1:** All existing V1 settings remain: device name, WiFi AP, telemetry rate, home-on-boot.
- **FR-ST-2:** New setting: per-axis manual movement speed (percentage of max, default 25%).
- **FR-ST-3:** New setting: global buffer toggle (on/off, default off).
- **FR-ST-4:** New setting: buffer duration (seconds, default 1.0).
- **FR-ST-5:** New setting: stiffness/inertia parameter for buffer calculation.

---

## 10. Connection and Board Interaction

- **FR-CN-1:** All V1 connection behavior is preserved: WebSocket with auto-reconnect, heartbeat, status polling, capability fetching.
- **FR-CN-2:** The UI adapts to whatever axes the board reports — it does not hardcode axis counts, names, ranges, or units.
- **FR-CN-3:** The control panel enforces precision constraints from the board's reported axis resolution. *Note: `AxisCapability` currently has no resolution field. For now, all axis values round to 0.1 (matching `trajectory.ts` rounding and `AxisStrip` input step). True per-axis resolution deferred to a Board API update.*
- **FR-CN-4:** The `scrub` WebSocket command (already defined in the Board API and implemented in mock server + firmware) is used by the timeline playhead.

---

## Non-Requirements (explicitly out of scope for V2)

- Loop mode (V3)
- Save/load named sequences (V3)
- Visual animation preview (V3)
- Move test mode at reduced speed (V3)
- Shot templates (V3)
- Target tracking (V3)
- Speed as first-class input mode divorced from duration (V3 — V2 has the partial version via clickable speed indicators)
