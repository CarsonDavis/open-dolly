# V2 User Stories

Status: **active**

End-to-end workflow walkthroughs for key V2 scenarios. Each story walks through what the user does, what the system does, and what the user sees — step by step.

---

## Story 1: First-Time Setup and First Keyframe

**Actor:** New user, phone in hand on set, slider powered on.

1. User connects phone to the slider's WiFi AP.
2. Opens the browser and navigates to the board's IP address.
3. **Sees:** "Connecting to board..." message. After a moment, the main screen appears.
4. **Sees:** An empty keyframe area (top of screen) with text: "No keyframes yet." Below: the control panel tray at the bottom of the screen showing all axes (e.g., Pan, Tilt, Roll, Slide) in jog mode.
5. User drags the Slide jog control to the right. The slider physically moves. The Slide readout updates in real-time from telemetry (e.g., "Slide: 245.3 mm").
6. User taps the Pan value readout, types "45", hits enter. The gimbal pans to 45.0°.
7. Happy with the framing, user taps **"Capture Keyframe"** at the bottom of the control panel.
8. **Sees:** A keyframe card appears at the top: "Keyframe 1 — Pan 45.0° | Tilt 0.0° | Slide 245.3 mm". The control panel remains in free jog mode.

---

## Story 2: Multi-Keyframe Product Shoot with Curve Editing

**Actor:** Product photographer, 3-keyframe sequence on a tabletop setup. Phone in hand.

### Capture Phase

1. User has already captured Keyframe 1 (Story 1). Now moves the slider to a new position, adjusts pan/tilt to reframe the product, taps **"Capture Keyframe"**.
2. **Sees:** Keyframe 2 card appears below Keyframe 1. Between them: a duration bracket showing "—" (unset). The transition simple view slides up from the bottom, covering the control panel.
3. **Sees:** Duration input field (empty, required). Below it: greyed-out speed indicators — e.g., "Slide: — mm/s", "Pan: — °/s" (waiting for duration).
4. User types "8" in the duration field.
5. **Sees:** Speed indicators populate: "Slide: 31.9 mm/s", "Pan: 5.6 °/s". The duration bracket between the two keyframe cards now shows "8.0s".
6. User taps outside the transition view — it closes, revealing the control panel. They jog to a third position, capture Keyframe 3.
7. **Sees:** Keyframe 3 appears. A new duration bracket "—" appears between KF2 and KF3. The transition simple view opens again for the new transition.
8. User types "12" for the duration. Brackets now show: KF1 ←8.0s→ KF2 ←12.0s→ KF3.

### Curve Editing

9. User taps the "8.0s" bracket between KF1 and KF2. The transition simple view opens.
10. User taps **"Advanced"**. The control panel area transforms into the curve editor.
11. **Sees:** Stacked lanes — Slide, Pan, Tilt (only axes with non-zero deltas). Each shows a straight diagonal line (linear).
12. User taps on the Slide lane at the midpoint. A control point appears on the curve.
13. User drags the point upward. The curve bows upward — fast start, gentle coast to stop.
14. **Sees:** The Slide lane now shows an ease-out-like curve. Pan and Tilt remain linear.
15. User checks the Pan checkbox and the Tilt checkbox, then adds a point to the Pan lane at 5 seconds (62.5% of the 8s transition), dragging it down to the baseline.
16. **Sees:** Both Pan and Tilt lanes show a flat line for the first 5 seconds (no rotation) then a steep curve for the last 3 seconds. The slider moves the whole time, but pan/tilt are delayed.
17. User taps **"Done"**. Returns to the keyframe view.

### Playback

18. User taps **"Upload & Play"** in the transport bar.
19. **Sees:** "Computing..." briefly, then "Uploading...", then the progress bar starts moving. The slider and gimbal execute the move.
20. After 20 seconds (8s + 12s), playback completes. The hardware is at KF3's position.

---

## Story 3: Using Buffers for Jitter-Free Stops

**Actor:** Videographer shooting a product reveal. The slider arrives at the final position at moderate speed and the camera shakes slightly from inertia.

1. User has a 2-keyframe sequence: KF1 (start) → KF2 (end), 6-second transition.
2. User opens **Settings** and enables **"Buffers"** (global toggle).
3. Returns to the main screen. The duration bracket now shows "6.0s" with small buffer indicators: "1.0s | 6.0s | 1.0s" (pre-buffer, motion, post-buffer).
4. User taps **"Upload & Play"**.
5. **What happens:** The hardware holds at KF1 for 1 second (pre-buffer), executes the 6-second move, then holds at KF2 for 1 second (post-buffer). Total: 8 seconds of trajectory.
6. In editing software, the user trims the first and last second — clean in-point and out-point with no jitter.

### Clearance Warning

7. User edits KF2 so the slide axis is at 995mm (5mm from the rail end at 1000mm).
8. **Sees:** A warning: "Buffer cannot be generated for Slide at Keyframe 2 — only 5mm of travel remaining (need ~25mm for 1.0s buffer at current speed)."
9. User can: (a) proceed without the slide buffer at KF2, (b) reduce buffer duration, or (c) move KF2 to give more room.

---

## Story 4: Editing an Existing Sequence (Retiming and Curve Adjustment)

**Actor:** Cinematographer who shot the sequence in Story 2 but wants to slow down the first move and adjust the pan delay.

### Retiming via Keyframe View

1. User taps the "8.0s" bracket between KF1 and KF2.
2. In the transition simple view, changes duration from 8 to 12.
3. **Sees:** Speed indicators update — everything is now slower. The bracket shows "12.0s".

### Retiming via Timeline (Desktop)

4. On a laptop, user switches to the **Timeline View**.
5. **Sees:** A horizontal timeline showing three keyframe markers at t=0s, t=12s, and t=24s. Per-axis lanes below show the curves.
6. User grabs the middle keyframe marker and drags it left to the 10-second mark.
7. **Sees:** The first transition becomes 10s, the second becomes 14s. The mini curves in the lanes stretch/compress accordingly.

### Curve Adjustment

8. User taps the Pan lane in the first transition on the timeline. It expands to an editable curve.
9. The existing control point (at 62.5% / 5s of the old 8s duration) is now at 50% of the 10s duration. The user drags it right to 70% (7 seconds) — pan starts even later.
10. User taps outside to collapse the lane. Satisfied.

### Scrub Preview

11. User grabs the playhead on the timeline and drags it slowly from left to right.
12. **The hardware physically moves** through the trajectory in real-time as the playhead is dragged. The user can verify the pan delay, the slide easing, and the exact framing at any moment.

---

## Story 5: Phone vs. Desktop Workflows

### Phone (Portrait) — Solo Shooter

1. User holds phone in one hand while adjusting the slider with the other.
2. **Sees:** Keyframe cards stacked vertically in the top portion of the screen. Control panel tray at the bottom. They can minimize the tray (collapse to a thin bar with just "Capture" visible) to see more keyframes.
3. Workflow: Jog → Capture → Set duration → Repeat. All done with thumb on the bottom of the screen.
4. For curve editing: taps a bracket, taps "Advanced", one-handed point manipulation in the curve editor. Phone held vertically — lanes are wide but short.

### Phone (Landscape) — Fine Adjustment

5. User rotates phone to landscape for detailed work.
6. **Sees:** Split screen. Control panel on the left, keyframe list on the right.
7. Can see axis controls and keyframe cards simultaneously. Useful for fine-tuning a selected keyframe — see the keyframe's values update as they jog.

### Desktop — Full Production

8. User connects laptop to slider WiFi. Opens the web UI in a full browser.
9. **Sees:** Keyframe view + control panel (top area) with the timeline view below.
10. Can drag keyframes on the timeline to retime, edit curves inline, scrub the playhead — all with a mouse.
11. The extra screen space shows all transitions at once, with full curve detail in every lane.

---

## Story 6: Solo Shooter — One-Handed Phone Operation

**Actor:** Solo cinematographer running the slider while operating the camera. Phone is mounted near the slider or held in one hand.

1. User has pre-captured all keyframes and set durations. The sequence is ready.
2. Minimizes the control panel tray — just the "Upload & Play" button visible at the bottom.
3. Taps **"Upload & Play"**. Trajectory uploads in <1 second (60KB for a 30-second move).
4. Playback starts. User puts the phone down and operates the camera.
5. The board executes autonomously — no network needed. The phone could disconnect and the move still completes.
6. When the move finishes, the phone (if still connected) shows "Complete" in the transport bar.

### Quick Adjustment Between Takes

7. Director asks for the move to be slower. User picks up the phone.
8. Taps the duration bracket, changes 10 → 15 seconds. Speed indicators update.
9. Taps "Upload & Play" again. New trajectory uploads, playback starts.
10. Total time for the adjustment: ~5 seconds.
