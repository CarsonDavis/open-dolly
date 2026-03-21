# Camera Slider/Motion Control App UI Research - Research Background

**Date:** 2026-03-21
**Topic:** In-depth investigation of edelkrone and iFootage camera slider/motion control software UIs, focusing on keyframe systems, multi-axis control, interaction patterns, and critical user reviews. Research supports OpenDolly V2 web UI design.

## Sources

[1]: https://edelkrone.com/pages/edelkrone-system-wide-features-guide "edelkrone System-Wide Features Guide"
[2]: https://fstoppers.com/astrophotography/fstoppers-reviews-edelkrone-sliderplus-and-headplus-system-366953 "Fstoppers - edelkrone SliderPlus and HeadPlus Review"
[3]: https://apps.apple.com/us/app/edelkrone/id1436389193 "edelkrone App - App Store"
[4]: https://edelkrone.com/blogs/highlights/keypose-mode "edelkrone - Keypose Mode"
[5]: https://edelkrone.com/blogs/edelkrone-news-updates/edelkrone-firmware-and-app-update-smoother-smarter-and-more-reliable-than-ever "edelkrone Firmware and App Update Blog"
[6]: https://github.com/Flexihh/edelkroneWebApp "edelkrone Web App (GitHub Fork)"
[7]: https://edelkrone.com/blogs/highlights/introducing-edelkrone-link-adapter "edelkrone Link Adapter / SDK"
[8]: https://allexpertreviews.com/ifootage-shark-nano-ii-motorized-camera-slider-review-pros-cons-ifootage-shark-slider-nano-ii/ "AllExpertReviews - iFootage Shark Nano II Review"
[9]: https://www.ifootagegear.com/products/sharkslidernano2 "iFootage Shark Slider Nano 2 Product Page"
[10]: https://apps.apple.com/us/app/ifootage-moco/id1214567080 "iFootage Moco App - App Store"
[11]: https://apps.apple.com/us/app/1214567080?see-all=reviews&platform=iphone "iFootage Moco App - User Reviews"
[12]: https://www.techgeartalk.com/edelkrone-sliderplus-and-motion-kit-detailed-review/ "TechGearTalk - edelkrone SliderPLUS Motion Kit Review"
[13]: https://reduser.net/threads/anyone-have-any-experience-with-edelkrone-sliders.173840/ "REDUSER - edelkrone Slider Experience Thread"
[14]: https://www.trustpilot.com/review/edelkrone.com "edelkrone Trustpilot Reviews"
[15]: https://www.newsshooter.com/2020/11/21/ifootage-shark-slider-nano-review/ "Newsshooter - iFootage Shark Slider Nano Review"
[16]: https://www.eoshd.com/comments/topic/52565-slider-edelkrone-or-ifootagegear/ "EOSHD Forum - edelkrone vs iFootage Thread"
[17]: https://apps.apple.com/us/app/arc-ii/id1458646478 "Rhino ARC II App - App Store"
[18]: https://www.dragonframe.com/dragonframe-software/ "Dragonframe Software Features"
[19]: https://www.dragonframe.com/arc-pilot/ "Dragonframe ARC Pilot"
[20]: https://www.caiimagestudio.art/en/post/edelkrone-review-overhyped "Cai Image Studio - edelkrone Overhyped Review"

## Research Log

---

### Search: "edelkrone app keyframe system UI how it works SliderONE HeadPLUS"

- **App auto-detects connected modules** and only shows controls for devices you have paired — no manual configuration of which axes are available ([edelkrone System-Wide Features Guide][1], [Fstoppers Review][2])
- **Pose-based system**: Users program "poses" (positions) rather than traditional animation keyframes. The term is "pose" not "keyframe" in their UI ([Fstoppers Review][2])
- **Core workflow**: Move the hardware to a position, save it as a pose, then the system moves between poses with configurable speed and acceleration ([Fstoppers Review][2])
- **Speed and acceleration are configurable** per move — users can adjust how fast the slider moves and how it accelerates/decelerates ([Fstoppers Review][2])
- **Continuous loop mode** available for repeating movements back and forth ([Fstoppers Review][2])
- **Timelapse mode** with intervalometer and specific interval configuration ([Fstoppers Review][2])
- **Laser Module integration** enables automated focus pulls during moves ([Fstoppers Review][2])
- **Point Tracking** (HeadPLUS): lock onto a 3D point while the body moves — user teaches the point by aiming at the same subject from 2 different angles ([edelkrone System-Wide Features Guide][1])
- **No physical buttons** — all control is through the smartphone app, which one reviewer noted as a limitation ([Fstoppers Review][2])
- **Battery advantage**: Using a phone as the controller means all device battery goes to motors, not screens ([Fstoppers Review][2])
- **Bluetooth pairing** between modules — no cables needed between head and slider ([Fstoppers Review][2])

---

### Search: "edelkrone app tutorial walkthrough pose programming interface screenshots 2024 2025"

- **edelkrone uses "Keypose" terminology** — their primary mode is called "Keypose Mode," described as "the simplest way to program your motions" ([edelkrone - Keypose Mode][4])
- **Supports more than 2 keyposes** — both the app and physical controller allow saving multiple poses for easy recall ([edelkrone - Keypose Mode][4])
- **Loop creation**: Simultaneously pressing 2 keypose buttons puts the system in a loop between those poses — a quick way to get repeating motion ([edelkrone - Keypose Mode][4])
- **Keypose Mode is universal** across all edelkrone motion control products ([edelkrone - Keypose Mode][4])
- **Rebuilt numeric controller** in recent update — the interface for editing keyposes was completely redesigned to be "more intuitive, faster, and pleasant to use" ([edelkrone App Update Blog][5])
- **Per-keypose speed assignments** — users can now assign distinct speed settings to individual keypose triggers without changing global speed. This is a relatively new feature (late 2024). ([edelkrone App Update Blog][5])
- **Vibration settling ("Stable Start")** — for HeadPLUS, the system now waits for vibrations to fully settle before initiating movement, reducing shake. This is analogous to OpenDolly's "buffer" concept. ([edelkrone App Update Blog][5])
- **Upcoming "Flow" feature** — will chain keyposes together for continuous playback without stop-and-go interruptions, enabling smoother multi-pose sequences ([edelkrone App Update Blog][5])
- **Controller-free quick setup coming** — tap button 3 times to enter learning mode, perform movement manually, tap twice to loop. Eliminates app dependency for simple moves. ([edelkrone App Update Blog][5])
- **6-axis simultaneous control** possible — HeadPLUS can pair with both JibONE and DollyPLUS simultaneously ([edelkrone App Update Blog][5])

**Key insight for OpenDolly:** edelkrone's model is fundamentally a **position-and-go** system — you set positions, then the system moves between them. There is no timeline, no per-axis curve editing, no duration control visible in any documentation. Speed and acceleration are the only transition parameters. This is a significant gap that OpenDolly V2 can address.

---

### Search: "edelkrone SDK API HTTP documentation programmatic control slider"

- **edelkrone SDK exists** but is beta-only, requires the Link Adapter hardware (USB dongle), and only supports "high-level motion control commands" — joystick commands or keypose commands ([edelkrone Link Adapter / SDK][7])
- **SDK command model**: Store axis values in "keypose" groups and recall them with a specific speed and acceleration. Same position-and-go paradigm as the app. ([edelkrone Link Adapter / SDK][7])
- **SDK runs as a background service** on a computer, translating HTTP API commands to the hardware protocol and vice versa ([edelkrone Link Adapter / SDK][7])
- **Open-source web app included** — the edelkrone Web App source code ships with the SDK. Built with Vue.js (66% Vue, 20% SCSS, 12% JS). MIT licensed. A community fork exists on GitHub. ([edelkrone Web App (GitHub Fork)][6])
- **API documentation is embedded** in the SDK install — each API call used in the edelkrone Web App is "openly documented" ([edelkrone Link Adapter / SDK][7])
- **Python example scripts** included for programmatic control ([edelkrone Link Adapter / SDK][7])

**Key insight for OpenDolly:** The edelkrone SDK confirms that their motion model is fundamentally limited to keyposes with speed/acceleration — there are no curve, duration, or per-axis timing parameters exposed in the API. The API mirrors the app's simplicity.

---

### Search: "iFootage Shark slider app multi-axis control keyframe programming interface UI"

- **iFootage Shark Slider Nano 2 supports up to 8 keyframes** — users can set up to 8 customizable motion points in a single path, with fine-tuning of parameters at each point ([iFootage Shark Nano II Review][8], [iFootage Shark Slider Nano 2 Product Page][9])
- **Dual control: touchscreen + app** — the Nano 2 has a built-in IPS touchscreen for on-device control plus the iFootage Moco mobile app for remote control. Both can program keyframes and control movements. ([iFootage Shark Nano II Review][8])
- **"Hands-on programming is very intuitive"** — reviewer specifically called out the ease of the physical programming workflow ([iFootage Shark Nano II Review][8])
- **DJI gimbal integration** — pairs with DJI RS2, RS3 Pro, RS4, RS4 Pro via wired RS Adapter Seat for multi-axis pan/roll/tilt control ([iFootage Shark Slider Nano 2 Product Page][9])
- **AI facial and object tracking** — using the Moco app, users can select a subject on screen and the Nano 2 keeps it centered and in focus ([iFootage Shark Nano II Review][8])
- **Multi-target mode** supporting 4 preset points for automated tracking sequences ([iFootage Shark Nano II Review][8])
- **Auto-save memory** — restores settings after power interruption ([iFootage Shark Nano II Review][8])
- **iOS only for app control** — the Moco app does not work with Android phones, a significant limitation noted in reviews ([iFootage Shark Nano II Review][8])

---

### Search: "iFootage Moco app review walkthrough keyframe speed curve editing interface 2024 2025"

- **Moco app has bezier curve preset timeline functions** — described as supporting "multiple key frame bezier curve preset timeline function, panoramic function, photo stitching function" ([iFootage Moco App - App Store][10])
- **Graph editor for axis programming** — users program each axis separately using a graph editor. One reviewer found this workflow too time-consuming for typical timelapse use. ([iFootage Moco App - User Reviews][11])

> "Having to program each axis separately and using a graph editor just takes too much time" — reviewer requesting a simpler A-to-B timelapse mode ([iFootage Moco App - User Reviews][11])

- **Multi-track timeline** — the app supports timelines with 2 or more tracks (one per axis), though users report bugs when using multi-track mode ([iFootage Moco App - User Reviews][11])
- **Three-axis control** — pan, tilt, and slide axes can be programmed independently ([iFootage Moco App - App Store][10])
- **Target control mode** and **manual control mode** as separate operating modes ([iFootage Moco App - App Store][10])
- **Critical reliability issues** — the app has a 1.6/5 star rating on the App Store. Multiple users report crashes on timelapses over 20 minutes, multi-track playback bugs, and unreliable preset saving. ([iFootage Moco App - User Reviews][11])

> "Any real Timelapse of 30m to 2 hours fails. Not just sometimes, every time" — user review ([iFootage Moco App - User Reviews][11])

- **No instructions/documentation** — multiple users complain about lack of guidance on how to pair devices or use features ([iFootage Moco App - User Reviews][11])
- **Connectivity praised** — the one positive review noted that "connecting to your devices is quick and consistent" ([iFootage Moco App - User Reviews][11])

**Key insight for OpenDolly:** iFootage's Moco app is the most feature-rich slider control app on the market — it actually has bezier curve editing and multi-track timelines, which is exactly what OpenDolly V2 envisions. However, the implementation is deeply unreliable (1.6/5 stars) with crashes, bugs, and no documentation. This is a massive opportunity: the feature set is right but the execution is terrible. OpenDolly can deliver what iFootage promised but failed to execute.

**Critical UX insight:** One reviewer specifically complained that the graph editor approach requires programming each axis separately and is too slow. This validates OpenDolly's approach of having a shared curve editor that shows all axes at once, while also supporting a simpler "set positions and go" mode for quick setups.

---

### Search: "edelkrone app complaints problems UI UX reddit forum 2024 2025" + "reddit edelkrone slider app software experience review frustrating"

- **Bluetooth disconnection is the #1 complaint** — multiple users report the app dropping connection after a few hours of use, requiring restart and reconnection between shots ([edelkrone Trustpilot Reviews][14])
- **Forced firmware updates** — the app sometimes automatically updates slider firmware with no way to postpone, problematic when time-pressed on set ([edelkrone Trustpilot Reviews][14])
- **No clear documentation** — users complain there are "no clear instructions on how to adjust different capturing points" and no instructions on how to use the app, making the learning experience difficult ([edelkrone Trustpilot Reviews][14])
- **App praised as "absurdly easy to program"** by one detailed reviewer — the point-and-click workflow of pushing the carriage, clicking Point A, then Point B was specifically praised ([TechGearTalk Review][12])

> "I found that I never had a need for a user-manual, or a help section for the app" — positive review ([TechGearTalk Review][12])

- **Timelapse auto-calculation praised** — the app lets you set final FPS, interval, or duration and auto-calculates the rest. "You can modify any of these variables and it will auto adjust the rest." ([TechGearTalk Review][12])
- **Target tracking workflow well-received** — point camera at subject, set as target, system tracks subject as slide moves with automatic pan/tilt adjustments ([TechGearTalk Review][12])
- **Hardware noise is a software-related concern** — slider motor noise is too loud for use close to audio recording, which relates to speed control in the app ([REDUSER Thread][13])

**Key insight for OpenDolly:** The edelkrone app polarizes users. When it works and the connection holds, it's praised as extremely intuitive. When the connection drops (which happens frequently), it's infuriating. The lesson: **connection reliability is a UX feature**. OpenDolly's WebSocket-based architecture over Wi-Fi may be more reliable than Bluetooth. Also, the timelapse auto-calculation pattern (change one variable, others adapt) is a strong interaction pattern worth borrowing.

---

### Search: "iFootage Shark Slider Nano 2 touchscreen interface review keyframe setup speed curve YouTube"

- **Touchscreen main menu** has four modes: Video, Timelapse, Stop Motion, and Settings ([Newsshooter Review][15])
- **Real-time position tracking** on the touchscreen display — shows how many seconds into the move and total move duration ([Newsshooter Review][15])
- **A/B point workflow on touchscreen**: Select A and B points by simultaneously pressing Power and Function buttons. Icons flash on screen, user manually repositions slider, presses Function to set each point. ([Newsshooter Review][15])
- **Critical limitation: Cannot use motor to position before setting points** — "you can't move the slider using the motor into the position you want and then set your A and B points." User must manually push the carriage. ([Newsshooter Review][15])
- **Pan control during slide** — user can pan the head to start position for Point A, then adjust pan position at Point B, enabling parallax moves without extra equipment ([Newsshooter Review][15])
- **Speed and time are configurable** — interface lets users "change the speed at which it moves and the time it takes for the slider travel," described as "very straightforward" ([Newsshooter Review][15])

> "Setting A and B points is a lot more complicated than it should be" — Newsshooter reviewer ([Newsshooter Review][15])

- **App freezes are a recurring issue** — the Moco app "just ends up freezing up and not letting me make changes," leading the reviewer to prefer the on-device touchscreen over the app ([Newsshooter Review][15])
- **Five control modes total** across the ecosystem: Video, Timelapse, Panorama, Stop Motion, and Macro ([iFootage Shark Slider Nano 2 Product Page][9])

**Key insight for OpenDolly:** The fact that iFootage's touchscreen requires manual slider positioning before setting points (can't use motor to position, then capture) is exactly the kind of workflow friction OpenDolly V1 already solves. OpenDolly's jog-then-capture workflow is clearly superior. Also notable: even iFootage's own reviewer abandoned the app in favor of the touchscreen due to app freezing -- app reliability is paramount.

---

### Search: "edelkrone vs iFootage slider" + "Rhino Arc II app review keyframe"

- **edelkrone app won't connect most of the time** — user on EOSHD forum stated: "Spend more time trying to get the app to connect and the slider to work than I did on the entire rest of the job" ([EOSHD Forum][16])
- **One user switched away entirely** — "I'll never buy another edelkrone product" after persistent connectivity issues ([EOSHD Forum][16])
- **iFootage Nano praised as "much more intuitive and easy to use"** — specifically noted that "you don't really need to use the app for standard slides" due to the touchscreen ([EOSHD Forum][16])
- **iFootage significantly cheaper** than edelkrone for comparable functionality ([EOSHD Forum][16])
- **Rhino Arc II app supports up to 5 keyframes** with a trackpad-based interface for positioning — described as "intuitive controls that make it extremely fast to setup and get filming" ([Rhino ARC II App][17])
- **Rhino Arc II timelapse mode was promised but not delivered** — users who owned the product for 3+ years reported the app still shows "coming very soon" for timelapse features, same message as at launch ([Rhino ARC II App][17])
- **Rhino uses 4-axis control** through the Arc II head (pan, tilt, slide, focus) ([Rhino ARC II App][17])

**Key insight for OpenDolly:** The Rhino Arc II's failed promise of timelapse mode (3+ years and counting) reinforces a pattern: **every major slider maker struggles with software execution**. Hardware is their strength; software is consistently their weakness. This is a structural advantage for an open-source, software-first project like OpenDolly.

---

### Search: "Dragonframe motion control keyframe curve editor" (bonus competitor for reference)

- **Dragonframe ARC workspace** is the gold standard for motion control keyframe programming — multi-axis bezier spline keyframe interface integrated directly within the software ([Dragonframe Software][18])
- **Axis-by-axis curve programming** with simplified bezier handles — "Adjust the smoothness in each direction with a simplified 'bezier' handle," giving a large degree of control while maintaining efficiency ([Dragonframe Software][18])
- **Axes can be organized into groups** via the Arc hamburger menu, with channels dragged into groups — a useful pattern for organizing multi-axis systems ([Dragonframe Software][18])
- **Move testing built in** — users can run a move test within the Arc workspace that captures video assist frames only, speeding up the test process ([Dragonframe Software][18])
- **ARC Pilot** — free iOS app that connects via QR code, shows live video stream from Dragonframe, and allows jogging moves with a GameVice controller. Provides tactile control without sitting at the computer. ([Dragonframe ARC Pilot][19])
- **Dragonframe is stop-motion focused** — the motion control is primarily designed for frame-by-frame capture, not real-time video moves. However, the keyframe/curve editing UI is the most sophisticated in the camera motion control space. ([Dragonframe Software][18])

**Key insight for OpenDolly:** Dragonframe's ARC is the north star for keyframe curve editing in camera motion control. Its axis grouping, bezier spline curves, and integrated move testing are all patterns worth studying. The key difference is that Dragonframe targets stop-motion (move-shoot-move), while OpenDolly targets real-time video (continuous motion), which affects how curves translate to physical movement.

---

### Search: "edelkrone SliderPLUS Amazon review app software complaints wishlist 2024"

- **App described as "beta software"** — saving settings inconsistently and experiencing inexplicable Bluetooth disconnections, making the software feel unfinished despite hardware costing nearly $4,000 ([Cai Image Studio Review][20])
- **Users want a dedicated physical controller** — suggestion for a phone-sized device with dedicated buttons rather than relying solely on the app, for more reliable and intuitive control ([Cai Image Studio Review][20])
- **edelkrone's timelapse programming frustrations** — one user reported the app was "near impossible to set to very long duration slides for timelapses" ([EOSHD Forum][16])

---

## Synthesis: Competitive Landscape Summary

### What Each Product Does

| Feature | edelkrone | iFootage Nano 2 | Rhino Arc II | Dragonframe ARC |
|---------|-----------|-----------------|--------------|-----------------|
| Max keyframes | 4-6 ("keyposes") | 8 | 5 | Unlimited |
| Transition control | Speed + acceleration only | Speed + time | Speed | Bezier spline curves |
| Per-axis curves | No | Bezier (in Moco app) | No | Yes |
| Timeline view | No | Multi-track (buggy) | No | Yes |
| Duration control | No (speed only) | Yes (time setting) | Yes | Yes |
| Loop mode | Yes | Yes | Unknown | N/A |
| Connectivity | Bluetooth (unreliable) | Bluetooth + touchscreen | Bluetooth | USB/wired |
| Platform | iOS + Android | iOS only (app), touchscreen | iOS only | Desktop (Mac/Win/Linux) |
| DJI gimbal integration | No (own head system) | Yes (RS2/RS3/RS4) | No | Via hardware |
| SDK/API | Yes (HTTP, beta) | No | No | Yes (comprehensive) |
| App rating | ~3.5/5 | 1.6/5 | ~2.5/5 | N/A (desktop software) |

### What They Do Well (Patterns Worth Borrowing)

1. **edelkrone: Auto-detection of connected modules** — the app discovers what hardware is present and adapts its UI accordingly. OpenDolly already does this via the Board API capabilities endpoint, but edelkrone's execution is praised for its seamlessness.

2. **edelkrone: "Absurdly easy" A-to-B workflow** — move the slider, tap to save Point A, move again, tap to save Point B. The simplicity of this flow is universally praised. OpenDolly V1 has this; V2 must not lose it.

3. **edelkrone: Timelapse auto-calculation** — set any one of FPS/interval/duration and the others auto-adjust. This "change one, derive the rest" pattern is excellent for interrelated parameters.

4. **edelkrone: Per-keypose speed assignments** — recently added, lets users set different speeds for different transitions without changing global settings. OpenDolly V2's per-transition curve model goes much further.

5. **edelkrone: Stable Start (vibration settling)** — waits for vibrations to settle before starting a move. Directly analogous to OpenDolly's buffer concept.

6. **iFootage: Built-in touchscreen fallback** — having an on-device control surface means users aren't stranded when the app fails. OpenDolly's web-based approach (any browser on any device) is even better.

7. **iFootage: Multi-track timeline with per-axis graph editing** — the right concept, even though the execution is terrible. The Moco app proves there's demand for this workflow.

8. **Dragonframe: Bezier spline keyframe curves** — the gold standard for motion control curve editing. Simplified bezier handles balance power with usability.

9. **Dragonframe: Axis grouping** — organizing axes into logical groups for complex multi-axis setups.

### What Users Hate (Anti-Patterns to Avoid)

1. **Bluetooth connectivity drops** (edelkrone, iFootage) — the single most complained-about issue across all products. Users spend more time reconnecting than shooting. OpenDolly's Wi-Fi/WebSocket architecture should be inherently more reliable, but connection state management and auto-reconnection must be bulletproof.

2. **App freezing and crashes** (iFootage Moco) — the app that has the most features (bezier curves, timelines) is also the most broken. 1.6/5 stars. Features without reliability are worse than no features.

3. **Per-axis graph editing is too slow** (iFootage Moco) — having to program each axis separately in its own graph editor is tedious. Users want a simpler default with the option to go deep.

4. **Can't motorize to position before capturing keyframe** (iFootage) — requiring users to manually push the slider carriage before setting A/B points is absurd. Motor-assisted positioning is basic.

5. **No documentation** (edelkrone, iFootage) — both platforms have users complaining about lack of instructions. Onboarding and discoverability matter.

6. **Forced firmware updates** (edelkrone) — updating firmware mid-shoot with no postpone option is unacceptable.

7. **iOS only** (iFootage, Rhino) — excluding Android users and locking to a single platform. OpenDolly's web-based approach is platform-agnostic by design.

8. **Promised features never delivered** (Rhino Arc II timelapse) — 3+ years of "coming very soon" destroys user trust.

### The Opportunity for OpenDolly V2

The competitive landscape reveals a massive gap:

- **edelkrone** has excellent UX for simple moves but no curve editing, no timeline, and crippling Bluetooth issues.
- **iFootage** attempted curve editing and timelines but executed so poorly the app is rated 1.6/5.
- **Rhino** has a decent keyframe system but can't deliver promised features.
- **Dragonframe** has the best curve editing but is desktop-only, expensive, and designed for stop-motion.

**No one has delivered a reliable, mobile-friendly motion control app with per-axis curve editing and timeline-based keyframe management.** This is exactly what OpenDolly V2 targets. The feature set iFootage promised, with the reliability users demand, built on a web platform that works on any device.

### Key Design Principles Derived from Research

1. **Simple default, powerful option** — The basic workflow (position, capture, set duration, play) must be as easy as edelkrone's. Curve editing should be opt-in, not required.

2. **Connection must never fail** — Connection state is the #1 UX issue in this market. Auto-reconnection, clear status indicators, and offline editing mode are non-negotiable.

3. **All axes visible at once** — iFootage's per-axis graph editing is too slow. Show all axis curves simultaneously with the ability to focus on one.

4. **Motor-assisted positioning is basic** — Never require manual slider positioning. Jog controls are mandatory for keyframe capture.

5. **Platform agnostic** — Web-based approach is a massive advantage over iOS-only apps. Every device with a browser is a controller.

6. **Derived parameters** — When the user changes one value (speed, duration, distance), auto-calculate what can be derived. Don't make users do math.
