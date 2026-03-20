# Camera Motion Control Landscape - Research Background

**Date:** 2026-03-20
**Topic:** Open-source projects similar to OpenDolly — camera motion control systems combining motorized sliders, gimbal control, web app UIs, and keyframe timeline editors. Covers DIY slider builds, motion control software, gimbal integration, keyframe editors, DJI control projects, and commercial products.

## Sources

[1]: https://github.com/topics/camera-slider "GitHub Topics: camera-slider"
[2]: https://github.com/maxmacstn/DollyDuino-V2 "DollyDuino-V2"
[3]: https://github.com/RajPShinde/CamSlider "CamSlider"
[4]: https://github.com/Adamslab/OpenSlider "OpenSlider"
[5]: https://github.com/bdring/camera-slider "bdring camera-slider"
[6]: https://github.com/colinmanuel/Arduino-Camera-Slider "Arduino-Camera-Slider"
[7]: https://github.com/eballetbo/Slider "eballetbo Slider"
[8]: https://github.com/RobTC/SliderCam "SliderCam"
[9]: https://github.com/bmedicke/arvakr "arvakr"
[10]: https://github.com/Af-Slider/AF.Slider-open-project "AF.Slider"
[11]: https://github.com/digitalbird01/DigitalBird-Camera-Slider "DigitalBird Camera Slider"
[12]: https://github.com/DynamicPerception "Dynamic Perception GitHub"
[13]: https://hackaday.io/project/174685-3-axis-camera-slider "Hackaday 3-Axis Camera Slider"
[14]: https://forum.arduino.cc/t/openmoco-open-source-timelapse-motion-control/8911 "OpenMoCo Arduino Forum"
[15]: https://www.diyphotography.net/this-esp32-based-diy-camera-slider-offers-multi-axis-motion-control-over-wifi/ "ESP32 WiFi Camera Slider - DIYPhotography"
[16]: https://github.com/SasaKaranovic/DIY-Camera-Slider "SasaKaranovic DIY-Camera-Slider"
[17]: https://github.com/cinosh07/ESP32_Camera_Slider "ESP32_Camera_Slider (cinosh07)"
[18]: https://github.com/mikrotron-zg/slidertron "Slidertron"
[19]: https://github.com/isaac879/Pan-Tilt-Mount "Pan-Tilt-Mount (isaac879)"
[20]: https://builds.openbuilds.com/builds/6-axis-motion-control-camera-rig-pan-tilt-roll-slide-focus-zoom.10039/ "6-Axis Motion Control Camera Rig - OpenBuilds"
[21]: https://github.com/zz85/timeliner "Timeliner"
[22]: https://www.theatrejs.com "Theatre.js"
[23]: https://github.com/alyssaxuu/motionity "Motionity"
[24]: https://github.com/ConstantRobotics/DJIR_SDK "DJIR_SDK"
[25]: https://github.com/Hibiki1020/dji_rs3pro_ros_controller "DJI RS3 Pro ROS Controller"
[26]: https://github.com/rileycoyote87/DJI-Ronin-RS2-Log-and-Replay "DJI Ronin RS2 Log and Replay"
[27]: https://github.com/VCHS-R-D/Ronin-SC-Gimbal-Controller "Ronin-SC-Gimbal-Controller"
[28]: https://github.com/Itamare4/dji_ronin "dji_ronin ROS Node"
[29]: https://github.com/bryanrandell/Kinowheels_3-Axis_wireless_for_DJI_Ronin "Kinowheels for DJI Ronin"
[30]: https://edelkrone.com/pages/edelkrone-system-wide-features-guide "edelkrone System Features"
[31]: https://github.com/Flexihh/edelkroneWebApp "edelkrone Web App (community)"
[32]: https://www.dragonframe.com/arc-pilot/ "Dragonframe Arc Pilot"
[33]: https://support.syrp.co.nz/hc/en-us/articles/360001040615-Current-and-Future-Features- "Syrp Features"
[34]: https://hackaday.com/2020/11/24/diy-camera-motion-rig-is-mostly-3d-printed/ "6-Axis 3D Printed Motion Rig - Hackaday"

## Research Log

---

### Search: "camera slider arduino open source github motorized"

**OpenSlider** (214 stars, GPL-3.0) is the most popular open-source camera slider project on GitHub. It's a **3-axis 3D-printed slider that runs on Marlin firmware** (the 3D printer firmware). This is clever — it reuses existing motion control infrastructure but means the UI is whatever Marlin supports (G-code commands, LCD menus). No web UI or keyframe editor. Last updated June 2021. ([OpenSlider][4])

**DollyDuino-V2** (12 stars) is an **ESP32-based 2-axis slider** (slide + pan) with TMC2226 stepper drivers. Features orbit mode (camera tracks subject while sliding), timelapse with automatic shutter trigger via BLE to Canon EOS cameras, and gamepad control via USB HID. UI is a **16x2 LCD display** — no web interface. Written in C++ with PlatformIO. Last active ~June 2024. ([DollyDuino-V2][2])

**CamSlider** (63 stars) is an Arduino-based 2-axis slider. Simple C/C++ firmware with AccelStepper library. No UI beyond basic serial control. Last updated Sept 2020, only 6 commits. ([CamSlider][3])

Other notable projects from the GitHub topics page ([GitHub Topics][1]):
- **arvakr** (10 stars) — autonomous slider for timelapse with **smartphone control** (Raspberry Pi + Python). One of the few with phone-based UI. ([arvakr][9])
- **AF.Slider** (5 stars) — fully 3D printable, C++, updated March 2024. ([AF.Slider][10])
- **aleiei TX/RX projects** (2-3 stars) — RC transmitter/receiver pair for 3-axis slider with pan/tilt using NRF24L01 radio. Updated March 2026, actively maintained. ([GitHub Topics][1])

**Key observation:** Almost none of these projects have a web UI or keyframe editor. Most use LCD displays, serial commands, gamepad input, or smartphone apps. The software sophistication is generally low — basic move-from-A-to-B or timelapse interval control. OpenSlider's use of Marlin is the most sophisticated motion control approach but it's G-code based, not keyframe-based.

---

### Search: "camera motion control software open source keyframe timelapse github" and "motion control camera slider keyframe software github multi-axis"

**DigitalBird Camera Slider** (111 stars, MIT) is the **most feature-rich open-source camera motion control system found so far**. It's a modular ESP32-based system with slider, 3 pan-tilt head options, cinematic turntable, focus motor, and WiFi controller. Key features: **6-keyframe sequencer with easing curves (linear, ramp, bounce)**, stop-motion, timelapse, PTZ control via VISCA over IP (OBS/vMix compatible), 9 programmable pose presets per camera. UI is via **WiFi remote controller** (custom hardware, not a web app). Actively maintained — v7.05 released Oct 2025, 467 commits. Written in C++. ([DigitalBird][11])

**This is the closest competitor to OpenDolly in terms of feature set** — multi-axis, keyframes, easing — but it uses a custom WiFi remote hardware controller rather than a web UI. No browser-based interface.

**Dynamic Perception** (commercial company with open-source components) has 25 GitHub repos. Key projects: **OMLibraries** (36 stars) — OpenMoCo libraries for AVR; **nanoMoCo_Firmware** (15 stars) — stepper driver/controller firmware; **MX2_DollyEngine** (10 stars); **Graffik** (7 stars) — motion control desktop app; **NMXCommander** (3 stars) — CLI for NMX motor controller in Java. They also published NonLinearFitting libraries for motion curve calculations. This is a **mature ecosystem** but repos appear mostly inactive (last updates years ago). ([Dynamic Perception GitHub][12])

**OpenMoCo** is an older (2010-era) open-source timelapse motion control platform built on Arduino. Architecture: centralized "engine" controlling up to 2 cameras + 3 stepper motors. Includes a **Perl API library and "Slim" scripting engine** for human-readable motion scripts. Interface options ranged from touchscreens to PC control. This was pioneering but appears largely dormant now. ([OpenMoCo Forum][14])

**Hackaday 3-Axis Camera Slider** — 3D-printed with 3x NEMA 17 + TMC2208 drivers, herringbone gears for pan/tilt giving 0.013 degree precision, Hall effect homing sensors. Arduino Nano + Bluetooth control. No web UI or keyframe editor. ([Hackaday][13])

**Dragonframe** (commercial, not open source) is mentioned as having a **multi-axis bezier spline keyframe interface** for stop-motion animation motion control. This is the gold standard for keyframe-based motion control UI but it's a proprietary desktop app.

---

### Search: "ESP32 camera slider web interface wifi open source github"

**SasaKaranovic DIY-Camera-Slider** (125 stars, GPL-3.0) — ESP32-based WiFi slider with a **web interface served directly from the ESP32**. The codebase is 90% HTML, suggesting a substantial web UI. 3D-printed mechanical parts included. However, documentation is sparse — the README just says "build instructions available in folders." Only 8 commits. Has a YouTube build video. This is notable because it's one of the **highest-starred slider projects with a web UI**. ([SasaKaranovic DIY-Camera-Slider][16])

**ESP32_Camera_Slider by cinosh07** (3 stars, 61 commits) — Universal slider control system designed for **astrophotography and video production**. Web interface accessed via ESP32 hotspot on mobile browser. Uses **WebSocket for real-time bidirectional communication** (no latency from HTTP polling). **JSON configuration files** loaded at boot determine hardware specifics — supports multiple slider hardware including legacy Dynamic Perception equipment. Async server ensures HTTP requests don't block the motor control loop. Language mix: HTML 47%, JS 24%, C++ 14%. This is architecturally interesting — **closest to OpenDolly's web-first approach** even though it's low stars. ([ESP32_Camera_Slider][17])

**Slidertron** (1 star, 13 commits, GPL-3.0) — ESP32 in AP mode, DRV8825 driver, NEMA17 motor, GT2 belt on 2020 extrusion. Web app at 192.168.4.1 for control. Basic but functional web UI. C++ 42%, JS 21%, HTML 6%. PlatformIO build. ([Slidertron][18])

**Key insight for OpenDolly:** The cinosh07 project's architecture (WebSocket communication, JSON hardware config, async server so motor control isn't blocked by HTTP) is the most relevant prior art for a web-first slider controller. However, none of these web UIs have keyframe timeline editors — they're all basic "set speed, set distance, go" interfaces.

---

### Search: "slider gimbal integration pan tilt camera control open source multi-axis"

**Pan-Tilt-Mount by isaac879** (474 stars, MIT) — **The most popular open-source camera motion control project on GitHub**. 3D-printed 3-axis system (pan/tilt/slide) with Arduino Nano, 3x NEMA 17 steppers, TMC2208 drivers. Extremely precise: **0.013 degree pan, 0.037 degree tilt, 0.023mm slide precision** via herringbone gears and 16th microstepping. Serial/Bluetooth control. Max load ~1kg (Canon EOS 250D). Nearly silent operation. 113 forks. CAD files on Thingiverse. No web UI — controlled via serial commands or Bluetooth. ([Pan-Tilt-Mount][19])

**6-Axis Motion Control Camera Rig** on OpenBuilds — **ESP32-based, WiFi-controlled 6-axis rig** (pan, tilt, roll, slide, focus, zoom) built on OpenBuilds C-Beam linear rail. 1500mm of travel using rack-and-pinion drive. Custom mobile app for control. Supports heavy cameras like Sony FX6. Build in progress. This is the most ambitious DIY motion control rig found, but no source code appears publicly available — it's a build showcase. ([6-Axis Rig][20])

**OpenSAM** — 3D-printed self-built 3-axis handheld gimbal stabilizer with brushless motors and joystick control. Different domain (handheld stabilization vs. slider motion control) but shares gimbal engineering concepts.

**Key observation:** The pan-tilt-mount project (474 stars) proves there's strong community interest in multi-axis camera motion control. But even the most popular projects lack sophisticated software — they all use serial commands or basic mobile apps. **No project in this space has a web-based keyframe timeline editor.** OpenDolly would be genuinely novel in this regard.

---

### Search: "keyframe timeline editor web javascript library motion control easing curves" and "web based keyframe editor robotics motion planning timeline UI open source"

**Timeliner by zz85** (732 stars, MIT) — **The most directly relevant UI library**. A lightweight browser-based animation timeline tool that provides a graphical interface similar to After Effects. Features: keyframe creation by double-clicking on timeline, **easing/tweening function selection between keyframes**, multi-layer support for animating multiple properties simultaneously, data persistence (autosave, localStorage, file export/import), undo/redo, playback controls. Written in pure JavaScript (88%) + HTML (12%). **Version 2.0 recently refactored to ES modules.** 130 commits, 126 forks. This could potentially be adapted or used as inspiration for OpenDolly's timeline editor. ([Timeliner][21])

**Theatre.js** (7,700 stars) — Professional JavaScript animation library with a **sequence editor (dope sheet) and graph editor**. Features: keyframe management across multiple properties, easing presets, custom bezier curve editing, multi-property animation, extension system for custom UIs. Designed to be framework-agnostic — works by changing JavaScript variables. Has been described as "the spiritual successor to Flash animation." This is extremely polished but is focused on web animation rather than hardware motion control. Could be used as a reference for UI/UX patterns. ([Theatre.js][22])

**Motionity** (4,000+ stars, MIT) — Open-source web-based motion graphics editor described as "a mix of After Effects and Canva." Features keyframing with custom easing curves, filtering, audio, text animation. JavaScript 88%, CSS 9%, HTML 3%. 30 commits. More focused on video editing than motion control, but the keyframe UI patterns are relevant. ([Motionity][23])

**Key insight for OpenDolly:** The timeline editor UI is a solved problem in web animation tooling — Theatre.js and Timeliner provide production-quality examples. The gap is that **nobody has connected these UI patterns to hardware motor control**. Timeliner (732 stars, MIT, pure JS) would be the most practical starting point or reference for building a keyframe editor that outputs motor control commands instead of CSS animations.

---

### Search: "DJI RS gimbal SDK API software control open source github" and "DJI ronin gimbal bluetooth serial control python arduino custom software"

**DJIR_SDK** (89 stars, 12 forks) — C++ library implementing **DJI R SDK protocol v2.2** for controlling DJI RS 2 gimbals. Communicates via **CAN bus through a USBCAN-II converter**. Three control methods: **position control** (absolute angles in 0.1 degree units with execution time), **speed control** (rotational velocities up to 360 deg/s), and **status retrieval** (real-time orientation query). Angle range -180 to +180 degrees on all axes. C++11 with CMake. Only 6 commits, last May 2021. ([DJIR_SDK][24])

**DJI RS3 Pro ROS Controller** (7 stars) — Python-based ROS controller for DJI RS3 Pro. Connects via **CAN-USB converter (Lawicel CANUSB)** through a custom cable to the DJI Focus Wheel port. Integrates with IMU (Xsens MTi-30) and RealSense camera. Docker containerization. 64 commits. Ubuntu 20.04 + ROS Noetic. ([RS3 Pro ROS][25])

**DJI Ronin RS2 Log and Replay** (53 stars, CC BY-NC 4.0) — **The most interesting DJI project for OpenDolly's use case.** Python scripts that **record gimbal movements (yaw, roll, pitch + focus motor position) to a log file, then replay them with frame-accurate timing**. Uses Socket-based CAN communication via CANable Pro adapter. Includes 3D-printable connector as alternative to DJI focus wheel. Audio feedback during recording/playback. This is essentially a **teach-and-replay motion control system** — you manually move the gimbal, record it, then replay it exactly. ([Log and Replay][26])

**Ronin-SC-Gimbal-Controller** (13 stars) — Controls DJI Ronin SC via **S-Bus signals** generated by Arduino with serial inverter. Also supports Raspberry Pi + ROS integration. C++ 60%. 69 commits. ([Ronin-SC Controller][27])

**dji_ronin ROS Node** — ROS node for controlling DJI Ronin 3-axis gimbal via Arduino. Serial communication at 9600 baud. ([dji_ronin][28])

**Kinowheels** — 3-axis wireless kino wheels to command DJI Ronin via SBUS protocol using Arduino boards. Custom physical controller. ([Kinowheels][29])

**Key insights for OpenDolly:**
- DJI gimbals are controlled via **CAN bus** (RS2/RS3) or **S-Bus** (Ronin/Ronin-SC). No Bluetooth API for programmatic control.
- The Log-and-Replay project (53 stars) demonstrates that **recording and replaying gimbal movements via CAN bus is proven and reliable**. OpenDolly could support this "teach" mode alongside keyframe-based programming.
- CAN bus requires a USB adapter ($30-50) and custom cable. Not plug-and-play but well-documented.
- The DJIR_SDK's position control with execution time parameter is useful — you can tell the gimbal "go to this angle in X milliseconds," which maps well to keyframe interpolation.

---

### Search: "edelkrone slider motion control app keyframe software API", "Rhino slider motion control software Arc keyframe timeline", "edelkrone SDK API documentation HTTP REST motion control", "Syrp Genie motion control app software features keyframe", and "Dragonframe motion control keyframe interface arc motion controller"

**edelkrone** — Premium commercial motion control. Key software features: **Keyposes act like keyframes on a timeline** in Sequencer Mode. 3+ keyposes triggers sequencer mode with separate transition speeds between each pose. Mobile app (iOS/Android) for wireless control. **Most importantly: edelkrone has an HTTP-based SDK** (beta) that translates API commands to hardware. The SDK supports joystick commands and **keypose commands** (store multi-axis values, recall with specific speed/acceleration). Also integrates with Dragonframe. A community-built **Vue.js web app** exists that interfaces with the edelkrone SDK. ([edelkrone Features][30], [edelkrone Web App][31])

**Rhino ARC II** — 4-axis motorized pan/tilt head paired with Rhino Slider. Mobile app (iOS) for control. Supports **up to 5 keyframes** (since firmware 2.0, March 2022). Keyframe workflow: position slider + head at desired position, save keyframe, repeat, then play back. Features loop mode, timelapse, light-lapse (night-to-day), and facial tracking. No open API documented.

**Syrp Genie II** — Mobile app with **programmable keyframe editor supporting up to 10 keyframes** for Genie Mini II. Independent speed control per axis. Features: bounce-back/repeat for video, ease-in/ease-out ramping, stop-motion, camera settings control via USB (ISO, shutter, aperture). Keyframe limits vary by device: 2 (original Genie), 5 (Genie Mini), 10 (Genie Mini II). No open API documented. ([Syrp Features][33])

**Dragonframe** — The gold standard for keyframe-based motion control. Desktop application with **Arc Motion Control workspace**: multi-axis bezier spline keyframe interface, channel groups, jog motors, set keyframes, run move tests. **Arc Pilot** is a free iPad app (with GameVice controller) for programming moves. Since 2024, supports Arduino-based real-time motion control (Arduino Giga R1 or Portenta H7). Compatible hardware: eMotimo spectrum ST4, Kessler CineShooter, edelkrone. This is the UI/UX benchmark for OpenDolly. ([Dragonframe Arc][32])

**Chris Desrondiers' 6-Axis Motion Control Rig** — DIY motorized camera jib reaching 11 feet tall, with gimbal on end + focus + zoom control. **All axes controlled via G-code** (like a 3D printer/CNC). Raspberry Pi 3 sends G-code to Arduino Mega + 6-axis GRBLDuino CNC shield. Uses slip rings for unlimited rotation. Files on Thingiverse. Almost entirely 3D-printed. ([6-Axis Rig Hackaday][34])

**Key insights for OpenDolly:**
- **edelkrone's HTTP SDK is the closest commercial analog** to what OpenDolly is building — a REST API for motion control with keypose/keyframe commands. OpenDolly's web UI approach is validated by edelkrone's direction, but edelkrone requires their proprietary hardware.
- Commercial products max out at 5-10 keyframes with simple mobile apps. OpenDolly's timeline-based approach with unlimited keyframes and easing curves would exceed what commercial products offer in software sophistication.
- Dragonframe's Arc workspace is the UI/UX gold standard to study — channel groups, bezier curves, motor jogging, move testing. But it's a $300+ desktop app, not web-based.
- The G-code approach (OpenSlider, 6-axis rig) is proven for multi-axis coordination but is not user-friendly for non-technical users. OpenDolly's visual timeline editor would be a major UX improvement over G-code.

---

<!-- End of research log -->
