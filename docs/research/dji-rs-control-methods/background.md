# DJI RS Gimbal Control Methods - Research Background

**Date:** 2026-03-25
**Topic:** Comprehensive survey of ALL methods for programmatically controlling DJI RS series gimbals (RS 2 through RS 5), including official SDK, third-party adapters, reverse-engineered protocols, Bluetooth/WiFi, USB-C, SBUS, Arduino/ESP32 libraries, and any other creative approaches found in forums and maker communities.

**Prior research:** `docs/research/dji-gimbal-software-control/` covers official SDKs and open-source CAN bus projects. This research goes deeper on under-documented methods.

## Sources

[1]: https://www.middlethings.co/apc-r-controller/ "Middle Things APC-R Controller"
[2]: https://www.middlethings.co/apcr-support/ "APC-R Support - Middle Things"
[3]: https://www.middlethings.co/support/docs/gimbal-control/ "Middle Things Gimbal Control Documentation"
[4]: https://www.digidow.eu/publications/2021-christof-masterthesis/Christof_2021_MasterThesis_DJIProtocolReverseEngineering.pdf "DJI WiFi Protocol Reverse Engineering - Master Thesis (2021)"
[5]: https://www.dji.com/rs-4/faq "DJI RS 4 FAQ"
[6]: https://dl.djicdn.com/downloads/DJI_RS_5/20260115/UM_2/DJI_RS_5_User_Manual_en.pdf "DJI RS 5 User Manual"
[7]: https://terra-1-g.djicdn.com/851d20f7b9f64838a34cd02351370894/Ronin%E7%B3%BB%E5%88%97/External%20interface%20diagram.pdf "DJI External Interface Diagram"
[8]: https://forum.arduino.cc/t/arduino-nano-every-sbus-dji-ronin-s-how-to-reset-position/699934 "Arduino Nano Every + SBUS + DJI Ronin S"
[9]: https://github.com/VCHS-R-D/Ronin-SC-Gimbal-Controller "Ronin SC Gimbal Controller (SBUS)"
[10]: https://radiomasterrc.com/products/gr-01-expresslrs-2-4ghz-receiver-for-dji-rs-camera-gimbals "RadioMaster GR-01 ExpressLRS Receiver for DJI RS"
[11]: https://github.com/ExpressLRS/ExpressLRS/discussions/2743 "GR-01 ExpressLRS Support for DJI RS4 Pro"
[12]: https://github.com/bolderflight/sbus "Bolder Flight SBUS Library (Arduino/CMake)"
[13]: https://github.com/TheDIYGuy999/SBUS "TheDIYGuy999 SBUS Library (Arduino)"
[14]: https://github.com/ConstantRobotics/DJIR_SDK "DJIR SDK - C++ SDK for DJI RS 2"
[15]: https://ardupilot.org/copter/docs/common-djirs2-gimbal.html "ArduPilot DJI RS2/RS3 Gimbal Integration"
[16]: https://www.dji.com/support/product/rs-4 "DJI RS 4 Support"
[17]: https://www.dji.com/support/product/rs-3 "DJI RS 3 Support"
[18]: https://www.newsshooter.com/2026/01/14/turn-dji-rs-3-pro-and-rs-4-gimbals-into-real-ptz-cameras-with-skaarhoj-controllers/ "SKAARHOJ DJI RS Gimbal PTZ Control"
[19]: https://emotimo.com/pages/dji-rs-gimbals "eMotimo DJI RS Gimbals Integration"
[20]: https://emotimo.com/products/conductorsa26 "eMotimo Conductor SA2.6"
[21]: https://shop.iflight.com/Gimbal-Controller-Pro2229 "iFlight Gimbal Controller for DJI RS"
[22]: https://github.com/ExpressLRS/ExpressLRS/pull/2140 "ExpressLRS PR: DJI RS 2 SBUS Support"
[23]: https://www.expresslrs.org/software/serial-protocols/ "ExpressLRS Serial Protocols Documentation"
[24]: https://support.cyanview.com/docs/Configuration/advanced/SBUS "Cyanview SBUS Configuration"
[25]: https://www.cyanview.com/products/ci0/ "Cyanview CI0"
[26]: https://forum.arduino.cc/t/looking-for-paid-help-with-teensy-4-1-and-dji-rsdk/1315005 "Teensy 4.1 + DJI RSDK Forum Post"
[27]: https://www.dragonframe.com/ufaqs/how-do-i-integrate-a-motion-control-system-with-dragonframe/ "Dragonframe Motion Control Integration"
[28]: https://www.bhphotovideo.com/c/product/1711291-REG/smallrig_3920_wireless_controller_for_dji.html "SmallRig 3920 Wireless Controller for DJI RS"
[29]: https://store.dji.com/product/r-multi-camera-control-cable-usb-c "DJI RS Multi-Camera Control Cable (USB-C)"

<!-- Add new sources here as you find them. Number sequentially. -->

## Research Log

---

### Search: "Middle Things APC-R controller DJI RS gimbal how it works teardown protocol"

**Key finding: APC-R operates in two distinct modes -- CAN bus and SBUS.** ([APC-R Support][2])

- **CAN bus mode** supports RS 2, RS 3 Pro, RS 4, RS 4 Pro, and **RS 5** (confirmed). CAN bus mode enables the full feature set: position presets, focus motor control, remote calibration, sleep/wake, and ActiveTrack triggering. ([APC-R Support][2])
- **SBUS mode** supports older gimbals: Ronin-S, Ronin-SC, and Ronin-2. SBUS mode provides only "basic PTZ movement" -- no presets, no focus motor, no calibration, no ActiveTrack. ([APC-R Support][2])
- **Physical connection:** APC-R connects to the gimbal's **RSA port** via a single included cable. The gimbal supplies power to the APC-R. ([APC-R Controller][1], [APC-R Support][2])
- **Network interfaces:** Ethernet (wired) and built-in WiFi. The APC-R auto-switches to WiFi when Ethernet is disconnected. This means the APC-R acts as a bridge: CAN bus on the gimbal side, IP network on the control side. ([APC-R Controller][1])
- **Control protocols exposed to the user:** USB gaming controllers (up to 4), ATEM Switcher integration, Bitfocus Companion, Middle Control desktop app (macOS). ([APC-R Controller][1])
- **Camera control:** Bluetooth to Blackmagic cameras for iris/shutter/WB/gain/focus. ([APC-R Controller][1])
- **Focus control** requires either a DJI Focus Wheel or the APC-R Gimbal Adapter for RS-series gimbals. ([APC-R Support][2])
- **No public SDK or developer API documented** -- Middle Things does not expose a programmatic interface for third-party developers.
- **APC-R Mini** also exists (newer, smaller form factor), listed on B&H.

**Follow-up questions:**
- Does Middle Things expose VISCA over IP? (Previous research mentioned this but wasn't confirmed in the docs.)
- What exactly is the "APC-R Gimbal Adapter" vs using the Focus Wheel?
- Can the APC-R be replaced with a DIY CAN bus adapter + custom software?

---

### Search: "DJI RS gimbal Bluetooth BLE reverse engineer protocol sniff control pan tilt" and "DJI Ronin RS3 RS4 Bluetooth app protocol"

**Bluetooth on DJI RS gimbals is strictly for camera shutter/recording control, not gimbal motion.** ([DJI RS 4 FAQ][5])

- **Dual-Mode Bluetooth** (RS 4 and newer): Press halfway to autofocus, press once to start/stop recording, press and hold to take a photo. Previously paired cameras reconnect automatically. ([DJI RS 4 FAQ][5])
- **No gimbal motion control over Bluetooth.** All searches confirmed BT is camera-only.
- **DJI WiFi protocol RE thesis** (Christof, 2021): Covers DJI drone WiFi protocols and DUML, but focuses on UAVs, not Ronin/RS gimbals. Found that DJI uses encryption layers that make RE difficult. ([DJI WiFi Protocol RE Thesis][4])
- **No evidence of anyone successfully reverse-engineering RS gimbal BLE for motion control.** The DJI Ronin app communicates with the gimbal over BLE for settings/configuration, but motion commands appear to require CAN bus.
- **DJI protocol RE projects** (vm32, fvantienen) focus on drones, not handheld gimbals.

**Conclusion: Bluetooth is a dead end for gimbal motion control on RS series.**

---

### Search: "DJI RS 5 USB-C ports" and DJI RS 5 User Manual / External Interface Diagram

**The RS 5 has multiple USB-C ports, but none are documented for external gimbal motion control.** ([DJI RS 5 User Manual][6])

- **Camera Control Port (USB-C):** For wired camera control cables (shutter, recording, focus). Labeled "RSS Camera Control Port" on RS 5.
- **Multifunctional Port (USB-C):** Connects accessories like DJI SDR Transmission, DJI Focus Pro Motor. Also used for firmware updates when connected to PC.
- **Enhanced Intelligent Tracking Module USB-C:** For firmware updates only.
- **No USB-C port exposes CAN bus or any programmatic gimbal control interface.**
- **The SmallRig 3920 wireless controller** connects its receiver to the gimbal via USB-A-to-USB-C cable, providing wireless joystick control, mode switching, centering, and record start/stop from up to 100m away with <50ms latency. Compatible with RS 2, RS 3 Pro, RS 4, RS 4 Pro. This suggests the USB-C port carries some form of control protocol, but it is undocumented and proprietary. ([SmallRig 3920][28])
- **DJI's own Multi-Camera Control Cable** (USB-C) connects the gimbal's camera control port to the camera for shutter/focus/aperture/ISO control -- this is camera control, not gimbal control. ([DJI Multi-Camera Control Cable][29])

**The RSA/NATO port is the only documented external gimbal motion control interface.** From the official External Interface Diagram for DJI RS 2 (which established the pinout used across the series): ([External Interface Diagram][7])

| Pin | Signal | Description | Notes |
|-----|--------|-------------|-------|
| 1 | VCC | Power output | 8V +/- 0.4V, rated 0.8A, peak 1.2A |
| 2 | CANL | CAN Low | CAN bus data line |
| 3 | SBUS_RX | SBUS input | Accepts SBUS RC signals |
| 4 | CANH | CAN High | CAN bus data line |
| 5 | AD_COM | Accessory detect | Built-in pull-up; needs 10-100k pull-down for power output |
| 6 | GND | Ground | -- |

**Critical detail:** The RSA port carries BOTH CAN bus (pins 2, 4) AND SBUS (pin 3) on the same 6-pin connector. This is how the APC-R can support both protocols -- it depends which pins it talks to.

**Note:** The RS 5 introduced a new 8-pin RSA port (per existing project docs), but official pinout for the 8-pin variant has not been published. The 6-pin pinout above is for RS 2 / RS 3 Pro era.

---

### Search: "DJI RS gimbal SBUS control Arduino ESP32 RC receiver simulate joystick pan tilt"

**SBUS is a viable control method for DJI RS gimbals, providing speed-based (joystick-like) control via the RSA port's SBUS_RX pin.** Multiple implementations exist.

**SBUS channel mapping** (from Arduino Nano Every + Ronin S implementation): ([Arduino SBUS Forum][8])
- **Channel 1:** Pan (yaw) -- speed-proportional
- **Channel 2:** Tilt (pitch) -- speed-proportional
- **Channel 4:** Roll -- speed-proportional
- **Channels 5-7:** Speed, mode, trigger functions

**SBUS value range:** 0-2047, with **1023 as center/neutral**. Below 1023 = move one direction (1022 = slowest, 0 = fastest). Above 1023 = move other direction (1024 = slowest, 2047 = fastest). This is **speed control only, not absolute position control.** ([Arduino SBUS Forum][8])

**Key limitation:** No way to send a "recenter" command via SBUS -- a user tried toggling channel values to simulate a double-click trigger but failed. ([Arduino SBUS Forum][8])

**SBUS signal requirements:**
- Inverted serial UART (100 kbaud, 8E2)
- Requires hardware signal inverter (NPN transistor circuit) between Arduino TX and gimbal SBUS_RX pin
- Send rate: ~50-100 Hz

**Arduino SBUS libraries:**
- **bolderflight/sbus** -- Arduino and CMake library for SBUS TX/RX ([Bolder Flight SBUS][12])
- **TheDIYGuy999/SBUS** -- Arduino library for SBUS receivers and servos ([TheDIYGuy999 SBUS][13])
- **BMC_SBUS** -- used in the Ronin SC controller project ([Ronin SC Controller][9])

**RadioMaster GR-01:** A commercial ExpressLRS 2.4 GHz receiver designed specifically for DJI RS gimbals (RS 2, RS 3 Pro). Connects to RSA port, outputs SBUS to gimbal. Has a "DJI RS Pro" output protocol mode in ExpressLRS. Works with RS 4 Pro when the gimbal's control switch is in the correct position. ([RadioMaster GR-01][10], [ExpressLRS Discussion][11])

**SBUS vs CAN bus comparison:**
| Feature | SBUS | CAN Bus (DJI R SDK) |
|---------|------|---------------------|
| Control type | Speed (joystick-like) | Position (absolute angle) AND speed |
| Position feedback | No | Yes (attitude + joint angles) |
| Focus motor | No | Yes |
| Presets | No | Yes (via protocol commands) |
| Calibration | No | Yes |
| Hardware complexity | Simple (UART + inverter) | CAN transceiver required |
| Latency | Low | Low |

---

### Search: "ESP32 CAN bus DJI RS gimbal control" and "GitHub DJI RS2 RS3 ESP32 Arduino gimbal CAN control library"

**No ESP32-specific DJI RS CAN bus library exists.** All existing open-source CAN bus implementations target desktop Linux or ROS environments.

- **DJIR_SDK** (ConstantRobotics): C++ standalone library for DJI RS 2. Uses DJI R SDK protocol v2.2. Requires USBCAN-II C adapter. Desktop-focused (CMake build). ([DJIR SDK][14])
- **ArduPilot integration:** Lua script `mount-djirs2-driver.lua` for ArduPilot 4.4+. Runs on flight controllers (STM32-based), not ESP32. Uses CAN bus via ArduPilot's DroneCAN driver. ([ArduPilot RS2/RS3][15])
- **ROS controllers** (Hibiki1020, ceinem): Python/C++ for desktop Linux with socketCAN. Not portable to microcontrollers.
- **Teensy 4.1 attempt** (Oct 2024): A forum poster sought paid help to implement DJI R SDK on Teensy 4.1 for RS 2/3/4 control. Had collected Python code from DJI and other developers, written partial Arduino code, but **did not succeed**. Thread closed without resolution. ([Teensy 4.1 + DJI RSDK][26])
- **ESP32's native TWAI controller** is fully capable of 1 Mbps CAN bus communication. Only needs an external transceiver (SN65HVD230 or TJA1051). No DJI-specific library wraps TWAI for RS gimbal control, but the protocol is documented and could be ported.
- **Our own project** (`firmware/lib/dji_can/`) is implementing a DJI CAN protocol library for ESP32-S3 using TWAI. This appears to be one of the first (if not the first) ESP32-native implementations.

---

### Search: "DJI Ronin app API programmatic interface" and "DJI Ronin app WiFi Bluetooth communication"

**The DJI Ronin app does NOT expose any programmatic interface, API, or SDK.** ([DJI RS 4 Support][16], [DJI RS 3 Support][17])

- **Ronin app connects to gimbals via Bluetooth** for initial setup, activation, firmware updates, and settings configuration. Default pairing password: 12345678.
- **The app provides a virtual joystick** for manual gimbal control from the phone, but this is a UI-only feature -- there is no API to drive it programmatically.
- **Settings accessible through the app:** Motor parameters, SmoothTrack settings, remote setup, axis modes, ActiveTrack configuration, firmware updates.
- **No REST API, WebSocket, IPC, or intent-based interface** has been found for the Ronin app on any platform.
- **DJI forum users have repeatedly asked** for a programmable API for the Ronin app with no official response.
- **The only known "app hack"** was for the original Ronin (not RS series): a ROS node that ran the Android Ronin app, captured the screen via ADB, and used OCR to read gimbal state. This is not a viable approach for modern RS gimbals.

**Conclusion: The Ronin app is a dead end for programmatic control. CAN bus via RSA port is the only viable path.**

---

### Search: "SKAARHOJ controller DJI RS gimbal integration" and "eMotimo Conductor SA2.6 RSI" and "iFlight Gimbal Controller DJI RS"

**Three additional commercial products control DJI RS gimbals, all connecting via the RSA/NATO port:**

**SKAARHOJ PTZ Pro:** ([SKAARHOJ DJI RS][18])
- Controls DJI RS 3 Pro and RS 4 gimbals as PTZ cameras
- Provides joystick pan/tilt, lens zoom, preset recall
- Uses **VISCA gimbal control** protocol -- mentioned explicitly in the Newsshooter article
- Connection options: USB vs IP control
- **Unclear whether SKAARHOJ connects directly to the gimbal or requires an intermediary (like the APC-R)** -- the article doesn't specify the physical bridge hardware
- Integrates with video switchers (ATEM) for routing and tally

**eMotimo Conductor SA2.6:** ([eMotimo DJI RS][19], [Conductor SA2.6][20])
- Motion controller that coordinates gimbal with slider/dolly/turntable
- Connects to DJI RS gimbals via **RSI (Robotic Stabilization Interface) dongle + 6-pin RSI cable**
- Compatible with RS 2, RS 3, RS 4, RS 5
- Controls pan, tilt, roll, and focus
- **4 microprocessors + 2 motion coprocessors** inside the SA2.6
- Has CAN 2 expansion ports, FIZ port (Tilta Nucleus M), LANC shutter, slider motor port, turntable motor port, RJ45 Ethernet
- **The RSI dongle likely implements CAN bus communication** to the gimbal (6-pin cable matches the RSA port pinout), though eMotimo doesn't document the protocol
- Connects to Dragonframe for stop-motion automation via DFMoco/DMC protocols ([Dragonframe Integration][27])
- Published eMotimo API for third-party integration (Bitfocus Companion, SKAARHOJ, Dragonframe)
- Price: $1,549 (with RSI), $1,999 (with RSI + RigWheels integration)

**iFlight Gimbal Controller:** ([iFlight Gimbal Controller][21])
- Small (47x31x12mm, 28g) wireless controller for DJI RS gimbals
- Built-in ExpressLRS 2.4 GHz or 900 MHz receiver (ESP8285 MCU, SX1281 RF chip)
- Connects to gimbal via **NATO interface** (RSA port)
- Uses **"DJI RS Pro" output protocol** -- which is a modified SBUS (see below)
- Compatible with RS 2, RS 3 Pro, RS 4, RS 4 Pro
- Features: remote pan/tilt/roll, record start/stop, gimbal recenter
- Power: 5V (powered by gimbal)
- $30-40 price range

---

### Search: "DJI RS Pro protocol ExpressLRS" -- Deep dive into the modified SBUS protocol

**"DJI RS Pro" is NOT a separate protocol -- it is a modified SBUS with DJI-specific endpoint values and channel mappings.** ([ExpressLRS PR #2140][22], [ExpressLRS Serial Protocols][23])

**Why it exists:** DJI's SBUS implementation on RS gimbals demands precise channel endpoint values that differ from standard SBUS. Standard SBUS output with default endpoints won't trigger functions like recentering correctly. The ExpressLRS team created a dedicated SBUS variant pre-configured for DJI's expected values. ([ExpressLRS PR #2140][22])

**Detailed channel mapping (from ExpressLRS):** ([ExpressLRS Serial Protocols][23])

| SBUS Channel | Function | Notes |
|-------------|----------|-------|
| CH1 | Pan (yaw) | Speed-proportional, range 352-1696 us |
| CH2 | Tilt (pitch) | Speed-proportional, range 352-1696 us |
| CH4 | Roll | Speed-proportional, range 352-1696 us |
| CH5 (AUX1) | Arm switch | **Must stay in unarmed/low position** |
| CH6 (AUX2) | Record/Photo | Toggle center-to-high (0.4s) = record; center-to-low = photo |
| CH7 (AUX3) | Mode select | 3-position switch: Mode 1, 2, or 3 |
| CH8 (AUX4) | Recenter/Selfie | Toggle center-to-one-side (0.4s) = recenter; other side = selfie |
| CH16 | AUX1 remapped | Moved from standard position |

**Key technical details:**
- **Endpoint values:** Pan/tilt/roll: 352-1696; Recenter/selfie: 176-848 (narrower range)
- **Trigger mechanism:** Record, photo, recenter, and selfie are triggered by **toggling the switch from center to high/low and back to center within 0.4 seconds** -- this mimics a physical switch flick
- **Failsafe:** DJI's implementation reads the SBUS failsafe bit, so failsafe behavior is gimbal-controlled regardless of receiver settings ([ExpressLRS PR #2140][22])
- **Configuration:** ExpressLRS transmitter should be set to 100 Hz Full with 8 channels, output endpoints -100 to 100

**This is the most detailed documentation of DJI's SBUS implementation for RS gimbals found anywhere.** The ExpressLRS PR and related code effectively reverse-engineered the exact values DJI expects.

---

### Search: "Cyanview DJI RS gimbal control SBUS" and "Dragonframe DJI RS gimbal"

**Cyanview CI0:** Broadcast camera control system that can control DJI Ronin-S/SC gimbals via SBUS for pan, tilt, and roll. Used in live production workflows to integrate mini cameras on gimbals as PTZ-like sources. The RCP touchscreen provides joystick-style control, with video router integration for automatic camera selection. ([Cyanview SBUS][24], [Cyanview CI0][25])

**Dragonframe:** Stop-motion animation software with Arc motion control workspace. Does not directly integrate with DJI RS gimbals, but **can control them indirectly through the eMotimo Conductor SA2.6** and its RSI interface. Uses DFMoco or DMC protocol to communicate with motion controllers. ([Dragonframe Integration][27])

---

### Search: "SmallRig wireless controller DJI RS" and "DJI RS USB-C multifunctional port accessory control"

**The SmallRig 3920 wireless controller receiver connects to the gimbal via USB-C** (USB-A to USB-C cable). This is notable because it means the USB-C port does carry some form of gimbal control protocol -- but it is undocumented and proprietary. ([SmallRig 3920][28])

**SmallRig features via USB-C:**
- Remote joystick control of pan/tilt
- Mode switching
- Centering/locking
- Record start/stop
- Sleep mode
- Range: up to 100m, latency <50ms
- Compatible with RS 2, RS 3 Pro, RS 4, RS 4 Pro

**This is a significant finding:** The USB-C port on DJI RS gimbals can be used for gimbal motion control by at least one third-party accessory. However, the protocol is completely undocumented and would require reverse engineering to use programmatically. No one appears to have attempted this.

**DJI's own USB-C accessories** (Multi-Camera Control Cable) use the camera control port for camera functions only (shutter, focus, aperture, ISO), not gimbal motion. ([DJI Multi-Camera Control Cable][29])

---

### Search: "DJI RS2 RS3 RS4 creative control method hack" and "DJI RS Python library CAN bus 2025 2026"

**No fundamentally new control methods discovered beyond CAN bus and SBUS.** The community landscape remains:
- CAN bus (DJI R SDK) for full-featured position/speed control
- SBUS for simple speed-proportional joystick control
- Everything else (Bluetooth, WiFi, USB-C, DUML) is either camera-only, undocumented, or not reverse-engineered

**Teensy 4.1 attempt** (Oct 2024): Someone tried to port DJI R SDK to Teensy 4.1 but failed after collecting code and writing partial implementation. Thread closed with no resolution. Budget of $500 was deemed insufficient by the community. ([Teensy 4.1 + DJI RSDK][26])

**The only open-source CAN bus implementations remain:** DJIR_SDK (C++, RS 2), dji_rs3pro_ros_controller (ROS, RS 3 Pro), dji_rs2_ros_controller (ROS, RS 2), DJI-Ronin-RS2-Log-and-Replay (Python, RS 2/3), and ArduPilot Lua script (RS 2/RS 3 Pro).

---

<!-- End of research log -->
