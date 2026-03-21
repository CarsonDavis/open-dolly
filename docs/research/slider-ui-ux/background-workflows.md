# Camera Slider Workflow Research - Background

**Date:** 2026-03-21
**Topic:** How cinematographers and videographers actually use motorized camera sliders in real-world production workflows. This research informs the OpenDolly V2 UI design by understanding user mental models, common moves, setup sequences, multi-axis coordination, pain points, and solo vs crew workflows.

## Sources

[1]: https://nofilmschool.com/2017/11/watch-5-cinematic-slider-moves-and-how-do-them-right "No Film School - 5 Cinematic Slider Moves"
[2]: https://theslantedlens.com/9-cinematic-camera-moves-to-improve-your-videos/ "The Slanted Lens - 9 Cinematic Camera Moves"
[3]: https://www.slrlounge.com/4-camera-movements-can-slider/ "SLR Lounge - 4 Camera Movements with a Slider"
[4]: https://edelkrone.com/blogs/highlights/keypose-mode "edelkrone - Keypose Mode"
[5]: https://edelkrone.com/blogs/edelkrone-news-updates/improved-keypose-mode-edelkrone-app-v3-3 "edelkrone - Improved Keypose Mode App v3.3"
[6]: https://manuals.plus/edelkrone/slider-one-and-slider-one-pro-manual "edelkrone SliderONE Manual"
[7]: https://www.cined.com/rhino-arc-ii-4-axis-motorized-head-and-slider-for-automated-camera-movement/ "CineD - Rhino Arc II Review"
[8]: https://www.superreel.co.uk/how-to-use-a-camera-slider/ "SuperReel - How to Use a Camera Slider"
[9]: https://fstoppers.com/originals/fstoppers-reviews-edelkrone-sliderone-pro-ultimate-motorized-portable-slider-220441 "Fstoppers - edelkrone SliderONE PRO Review"
[10]: https://reduser.net/threads/anyone-have-any-experience-with-edelkrone-sliders.173840/ "REDUSER - edelkrone slider experiences"
[11]: https://www.premiumbeat.com/blog/lone-videographers-need-motorized-slider/ "PremiumBeat - Solo Videographers Need Motorized Slider"
[12]: https://improvephotography.com/49564/rhino-slider/ "Improve Photography - Rhino Slider Long-term Review"
[13]: https://nofilmschool.com/field-test-sypr-genie-ii "No Film School - Syrp Genie II Field Test"
[14]: https://www.techgeartalk.com/edelkrone-sliderplus-and-motion-kit-detailed-review/ "TechGearTalk - edelkrone SliderPLUS Motion Kit Review"
[15]: https://www.dragonframe.com/dragonframe-software/ "Dragonframe Software Features"
[16]: https://www.manfrotto.com/global-en/stories/6-slider-shots-every-filmmaker-should-know/ "Manfrotto - 6 Slider Shots Every Filmmaker Should Know"
[17]: https://cinetics.com/lynx-3-axis-slider/ "Cinetics Lynx 3 Axis Slider"
[18]: https://support.syrp.co.nz/hc/en-us/articles/360000988375-Genie-II-App-Key-framing-Setup-Tutorial- "Syrp Genie II App Keyframing Tutorial"

<!-- Add new sources here as you find them. Number sequentially. -->

## Research Log

---

### Search: "common cinematic camera slider moves techniques parallax reveal tracking"

**The standard vocabulary of slider moves consists of 4-5 core types:**

- **Lateral slide** — Basic side-to-side movement. Great for establishing shots, introducing environments, showing a process. Foreground elements passing at varying speeds enhance depth ([The Slanted Lens][2], [SLR Lounge][3])
- **Push-in / Pull-out** — Moving toward or away from the subject. Slow push builds tension; fast push creates surprise/alarm. Pulls reveal broader context or communicate "magnitude of challenges ahead." ([The Slanted Lens][2], [SLR Lounge][3])
- **Parallax** — The camera slides in one direction while panning the opposite direction to keep the subject centered. This is the signature "cinematic" slider move — foreground/background move at different speeds creating depth. ([No Film School][1], [SLR Lounge][3])
  - **Key UX implication:** Parallax requires coordinating two axes (slide + pan) in opposite directions simultaneously. This is exactly the multi-axis coordination problem the UI needs to handle well.
- **Boom / vertical** — Slider mounted vertically for up/down motion. Used for reveals (come up to reveal something) and perspective changes. ([SLR Lounge][3], [The Slanted Lens][2])
- **Low mode** — Operating from ground level for dramatic angles ([No Film School][1])

**Additional creative variations:**
- **Outward curve** — Camera moves across while arcing outward, creating a larger apparent move than the slider length allows. "Gives you much more than the 48 inches of your slider...like six or eight feet" ([The Slanted Lens][2])
- **Parallax with Dutch angle** — Parallax combined with a tilted frame for emotional unease ([The Slanted Lens][2])
- **Flyover** — Overhead slider movement, good for revealing maps/layouts ([The Slanted Lens][2])

**Speed is a key creative variable:** Slow movements convey introspection; faster speeds create energy. Speed ramping during a move enhances visual interest. ([The Slanted Lens][2])

**Lens choice matters:** Tighter focal lengths amplify slider movement and help hide the slider itself in push-in shots. Anamorphic lenses add cinematic feel. ([No Film School][1], [The Slanted Lens][2])

**How these map to keyframes:** Most standard moves are **2-keyframe sequences** (start position -> end position). The creative variation comes from:
1. Speed/easing (ease in/out vs linear)
2. Multi-axis coordination (parallax = slide + counter-pan)
3. Speed ramping within the move

---

### Search: "edelkrone SliderONE workflow setup keyframe how to use tutorial" and "edelkrone keypose sequencer workflow"

**edelkrone's workflow model is built around "Keyposes" (their term for keyframes):**

- **Saving a keypose:** Press and hold a keypose button to save the current position of all axes. Both the app and the physical controller support saving "more than 2 poses." ([edelkrone Keypose Mode][4])
- **Recalling a keypose:** Tap the button again to transition the system back to that pose smoothly ([edelkrone Keypose Mode][4])
- **Looping:** Simultaneously pressing 2 keypose buttons puts the system in a loop between those poses — described as "perfect for long interview sessions, podcasts, educational videos" ([edelkrone Keypose Mode][4])
- **Sequencer Mode:** Once you have 3+ keyposes saved, sequencer mode triggers them in any order with separate transition speeds between each pose, and optional back-and-forth loop. "Keyposes act like keyframes on a timeline" ([edelkrone Keypose Mode][4])

**edelkrone setup process:**
- No on/off button — attach battery, auto-calibration detects slider limits ([edelkrone SliderONE Manual][6])
- App auto-pairs via Bluetooth ([edelkrone SliderONE Manual][6])
- Real-time manual control: drag finger left/right in app, slider follows with speed proportional to finger distance from center ([edelkrone SliderONE Manual][6])
- Variable speed and range can be saved for repeatable motions — useful for VFX work ([edelkrone SliderONE Manual][6])

**Multi-axis with HeadPLUS:** When paired with a body device (slider, jib, dolly), you can program 3-5 axis camera motions. The HeadPLUS target tracking feature uses a "teach" approach: aim at the same point from 2 different angles, and the system calculates how to track that point during movement.

**Key UX insight:** edelkrone's mental model is **"save positions, then play between them"** rather than "define a trajectory." The user thinks in terms of compositions/framings, not in terms of motion paths. The system figures out the path. This is a fundamentally different approach from timeline-first tools.

**Another insight:** The "teach by showing" approach for target tracking (aim from 2 angles) is a brilliant alternative to manual multi-axis coordination. Instead of thinking "slide left, pan right by X degrees," the user thinks "I want the camera to stay pointed at this thing while it slides."

---

### Search: "Rhino slider Arc motorized workflow keyframe setup multi-axis coordination" and "motorized camera slider workflow keyframe shot process step by step"

**Rhino Arc II workflow model:**

- **Keyframe-based, up to 5 keyframes** with the Rhino Arc app, playable at different speeds ([CineD - Rhino Arc II Review][7])
- **4-axis setup:** motorized pan/tilt head + focus motor + slider motor. All controlled through integrated joysticks or wireless iOS app ([CineD - Rhino Arc II Review][7])
- **Subject tracking available:** App can automatically track subject movement, similar to edelkrone's approach ([CineD - Rhino Arc II Review][7])
- **Motor options affect workflow:** High Speed Motor (5"/sec) vs High Torque Motor (1"/sec) — this hardware choice constrains what shots are possible (from Rhino product pages)

**Rhino's mental model is similar to edelkrone:** Set keyframes by positioning, then play back. Both products converge on this approach, suggesting it is the natural workflow for slider users.

**Practical slider operation tips (from SuperReel guide):**

- **Fluid head on carriage is essential** — larger heads provide stability but add flex risk ([SuperReel][8])
- **Disable IBIS** on mirrorless cameras — in-body stabilization fights the slider movement and creates artifacts ([SuperReel][8])
- **Touch technique matters:** Push with "just one or two fingers" rather than gripping. Maintain consistent grip without pushing downward ([SuperReel][8])
- **Start and end are the danger zones:** "Watch the start and ends carefully, as this is where the biggest judder occurs" ([SuperReel][8])
- **Stance for manual slides:** "Stand in the middle" of the slider, use leg pivots instead of arm motion. Execute "one well judged, controlled step" at most ([SuperReel][8])
- **Optimal slider length:** 1 meter balances maneuverability, portability, and stability ([SuperReel][8])
- **Setup time scales dramatically with length:** 2-meter sliders require ~10 hours setup vs ~2 hours for standard configs ([SuperReel][8])

**UX implications:**
- The "judder at start/end" problem directly validates the buffer system design in OpenDolly V2
- Motorized sliders solve the "one or two fingers" problem — consistent speed without human variability
- The common workflow across products (Rhino, edelkrone, generic) is: position -> save -> position -> save -> set speed/duration -> play

---

### Search: "reddit motorized slider frustrations pain points edelkrone rhino syrp problems workflow"

**edelkrone pain points (from user forums):**

- **Noise:** "Too loud to use close if you need audio" — motor noise is a recurring complaint. Motion control modules described as "loud, imprecise" ([REDUSER][10])
- **Play/slop in pan mechanism** when reversing direction ([REDUSER][10])
- **Weight capacity:** Designed for DSLRs; struggles with heavier cinema cameras. "For RED... Don't even try!" ([REDUSER][10])
- **Bending under load:** Bends at both ends when weight exceeds limits, clicking/bumping at center point when overloaded ([REDUSER][10])
- **Battery ecosystem:** Each edelkrone module requires a different battery type — NPF for slider motor, Canon LP-E6 for panning head. Complicated on-set logistics (from search snippets)
- **Belt swapping:** 10 minutes to switch between manual and motorized modes on some models (from search snippets)
- **Connection drops:** Occasional Bluetooth disconnections requiring battery reset to re-pair ([Fstoppers][9])
- **Plastic wheels collect dust** requiring regular cleaning ([REDUSER][10])

**edelkrone positives:**
- "Quick to setup, precise, and mostly reliable" during 6 weeks of production ([Fstoppers][9])
- All controls "visible on one screen" — intuitive app layout ([Fstoppers][9])
- App connects "instantly without menu navigation" ([Fstoppers][9])
- Ease in/out curves available with full track utilization ([Fstoppers][9])
- Solo operators can run it as a "second camera operator" — automated slider on one angle while manually operating gimbal for another ([Fstoppers][9])

**Rhino pain points:**
- Weight distribution problems on incline shots — risk of tipping with ballhead + camera + lens ([Improve Photography][12])

**Syrp Genie II specific issues:**
- **Battery swap is cumbersome:** Must remove the pan/tilt head to swap the linear motor battery. No dedicated field charger — must charge via USB between takes ([No Film School][13])
- **Rope-driven system limits travel:** Usable range is only about 85-90% of slider length before rope tension becomes problematic ([No Film School][13])
- **Repeated moves drain battery fast**, forcing USB charging between takes ([No Film School][13])

**Cross-product recurring themes:**
1. **Setup complexity** — batteries, calibration, pairing, leveling
2. **Noise** — motor noise kills audio, especially for interviews
3. **Weight limits** — most compact motorized sliders can't handle cinema cameras
4. **Reliability** — Bluetooth drops, motor inconsistencies

---

### Search: "solo videographer camera slider workflow one person shoot setup tips motorized" and "camera slider multi-axis pan tilt coordination workflow"

**Solo operator workflow — key differences from crew:**

- **Motorized slider as "second camera operator":** Solo shooters use the automated slider as an unmanned camera angle, running it on loop while they manually operate a primary handheld/gimbal camera. This is one of the most valuable use cases — it doubles their output with no extra crew ([PremiumBeat][11], [Fstoppers][9])
- **Setup time trade-off:** Adds 5-10 minutes to setup, but saves time vs reshoots from manual operation errors ([PremiumBeat][11])
- **4K crop trick:** Shoot 4K on the motorized slider, then crop/reframe in post to simulate multiple camera angles from a single position ([PremiumBeat][11])
- **VFX repeatability:** Motorized precision enables VFX shots that require accurate, repeatable movement (e.g., split-screen character doubling) ([PremiumBeat][11])
- **Interviews improved:** Motorized sliders are "perfect for interviews" — smooth subtle movement adds production value without needing a dedicated slider operator ([PremiumBeat][11])

**Multi-axis coordination mental models:**

Products like ZEAPON AXIS handle multi-axis by having the user "simply set start and end points" — the system automatically synchronizes pan with slider travel. No calibration or guesswork required (from search snippets). This confirms the "position-based" mental model: users think in terms of start/end compositions, not per-axis motion paths.

**Key insight for OpenDolly UI:** The dominant workflow pattern across all products and user types is:
1. **Physically position** the camera at the start framing (all axes)
2. **Save** that position as a keyframe
3. **Physically position** the camera at the end framing
4. **Save** that position
5. **Set duration/speed**
6. **Preview/play** the move
7. **Tweak** if needed (adjust positions, speed, easing)
8. **Shoot** the final take

The user never thinks in terms of "slide axis moves 200mm while pan axis rotates -15 degrees." They think: "I want the camera here at the start and here at the end." The system resolves that into per-axis motion.

**Solo vs crew differences:**
- **Solo:** More likely to use loop mode, run slider unmanned, value quick setup/teardown, rely on app control exclusively (no physical controller), shoot 4K for reframing flexibility
- **Crew:** More likely to have someone fine-tuning the slider in real-time, adjusting between takes, potentially using a physical controller/joystick for faster iteration

---

### Search: "Syrp Genie Mini II workflow setup keyframe speed easing how to program moves"

**Syrp Genie II keyframing workflow:**

- **Up to 10 keyframes** supported in advanced keyframing mode ([Syrp Keyframing Tutorial][18])
- **Per-axis speed control:** The keyframe editor controls each axis's path and speed independently ([Syrp Keyframing Tutorial][18])
- **Easing via bezier:** Clicking a "bezier" symbol at the bottom of the app causes the slider to "ease" through the keyframe more gradually, smoothing transitions ([Syrp Keyframing Tutorial][18])
- **Templates:** The app provides preset templates (Clouds, People, Night Traffic, Long/Short video tracking) that can be adjusted or used as starting points (from B&H review)
- **Setup process:** Wireless via Bluetooth/WiFi, uses in-app joystick control to position to start/end points (from B&H review)
- **Gyroscope control:** Can mimic phone motion using gyroscope/accelerometer for intuitive positioning ([Cinetics Lynx][17])

**Cinetics Lynx 3-axis workflow:**
- "Sets up in seconds" with preset programs runnable from controller or app ([Cinetics Lynx][17])
- **Saved programs:** Users can save and reuse programs quickly — important for repeatable shots or returning to a previously used setup ([Cinetics Lynx][17])
- 3-axis synchronized motion (slide + pan + tilt) with 2"/sec slide speed, 20 deg/sec pan/tilt ([Cinetics Lynx][17])

---

### Search: "Dragonframe motion control keyframe workflow timeline"

**Dragonframe represents a different, more sophisticated workflow model:**

- **Graphical timeline with per-axis curves** — users mark positions they want to hit, then adjust keyframes to move through them ([Dragonframe][15])
- **Simplified bezier handles** for adjusting smoothness in every direction — similar to the curve editor OpenDolly V2 envisions ([Dragonframe][15])
- **"Feathering" tool** adds extra smoothing of acceleration/deceleration — analogous to the OpenDolly buffer concept ([Dragonframe][15])
- **Motion test mode:** Run a test within the Arc workspace to see how a programmed move looks. Motion test captures only video assist frames to speed up testing ([Dragonframe][15])
- **Increment editor:** Plot and preview motion paths before execution ([Dragonframe][15])

**Key difference from edelkrone/Rhino:** Dragonframe is a timeline-first tool where the trajectory itself is the primary editing surface. Users work with curves and timing directly, not just start/end positions. This is closer to the V2 timeline view concept but may be overkill for typical video slider work (Dragonframe is primarily for stop motion).

---

### Search: "YouTube tutorial motorized camera slider beginner workflow" and "edelkrone SliderPLUS Motion Kit detailed review"

**edelkrone SliderPLUS + Motion Kit workflow (detailed review):**

- **Setup is push-based:** "You literally push it over to where you want it to start, click on A to set it, then push it to where you want it to stop, click B." No programming needed for basic moves ([TechGearTalk][14])
- **Target tracking eliminates per-axis thinking:** Position camera at two locations while pointing at same subject — system calculates pan/tilt tracking automatically during movement ([TechGearTalk][14])
- **Up to 6 targets** can be set and switched between dynamically ([TechGearTalk][14])
- **"I'm not programming keyframes for each axis separately"** — the interface handles multi-axis coordination transparently ([TechGearTalk][14])
- **Focus module adds 3rd axis:** Sets focus on a subject via app; system calculates distance and maintains focus during movement. Eliminates autofocus hunting ([TechGearTalk][14])
- **Timelapse calculator:** Automatically calculates intervals based on desired FPS, shot duration, and image count — modifying any variable auto-adjusts others ([TechGearTalk][14])

**Manfrotto slider shot types and setup tips:**

- **Low Pull Shot:** Lock the panning axis to prevent accidental left-right movement — focus only on tilt ([Manfrotto][16])
- **Track & Pan:** Increase drag on video head for smoother panning while tracking ([Manfrotto][16])
- **Dolly Zoom:** Simultaneously zoom in/out while tracking in opposite direction — requires precise coordination ([Manfrotto][16])
- **Pull, Tilt & Roll:** Requires keyframe synchronization across multiple devices through dedicated software ([Manfrotto][16])
- **Flywheel mechanism** provides "even level of resistance" for smooth movement ([Manfrotto][16])

**edelkrone SliderONE v3 workflow evolution:**
- Can tap directly on SliderONE's body to set speed and poses — no app needed for basic operation (from edelkrone product page)
- Apple Watch control available — "this workflow will become second nature in no time" (from edelkrone product page)
- Physical push-to-position: "you don't have to use a dial or joystick to point the camera, you can literally just push it" (from review snippets)
- Rhino's physical controller described as "dead simple" with users "up and running in minutes" ([Improve Photography][12])

---

<!-- End of research log -->
