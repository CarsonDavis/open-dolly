# DJI RS Gimbal Control Methods - Complete Landscape

**Date:** 2026-03-25

## Executive Summary

There are exactly **two viable protocols** for programmatically controlling DJI RS series gimbal motion: **CAN bus** (via the DJI R SDK protocol) and **SBUS** (via the RSA port's SBUS_RX pin). Every commercial product, open-source project, and DIY solution ultimately uses one of these two. CAN bus provides full-featured bidirectional control (absolute positioning, speed control, telemetry feedback, focus motor, presets). SBUS provides one-way speed-proportional joystick-like control with no feedback. All other interfaces -- Bluetooth, WiFi, USB-C, the Ronin app -- are dead ends for gimbal motion control. The RSA/NATO port on the gimbal handle is the sole physical gateway.

---

## 1. CAN Bus (DJI R SDK Protocol)

**The primary and most capable control method.**

### What It Is

DJI's official RS SDK uses CAN bus at 1 Mbps to send position commands, speed commands, and receive telemetry from RS series gimbals. The protocol is documented in PDF specs (v2.2 for RS 2, v2.5 adds focus motor control).

### Supported Gimbals

- **Officially listed:** RS 5, RS 4, RS 4 Pro, RS 3 Pro
- **Community-confirmed:** RS 2 (protocol v2.2)
- **Not confirmed:** RS 3 (standard), RS 3 Mini, RS 4 Mini, RSC 2

### Capabilities

| Feature | Details |
|---------|---------|
| Position control | Absolute angles (0.1 deg resolution), all 3 axes |
| Speed control | Continuous rotation at specified deg/s |
| Telemetry | Attitude + joint angles pushed at configurable rate |
| Focus motor | Control attached focus motor position (v2.5+) |
| Camera control | Shutter, recording |
| Calibration | Remote gimbal calibration |
| Min command time | 100 ms per position command |
| Angle range | Yaw/Roll: +/-180 deg, Pitch: -56 to +146 deg (v2.5) |

### Physical Connection

The CAN bus signals (CANH, CANL) are on **pins 2 and 4** of the 6-pin RSA/NATO port. Connection requires:
1. A CAN transceiver (e.g., SN65HVD230, TJA1051, MCP2551) between your microcontroller and the CAN bus lines
2. OR a USB-to-CAN adapter (CANable Pro, Lawicel CANUSB, USBCAN-II C) for PC-based control
3. A physical cable to the RSA port (the DJI R Focus Wheel provides a convenient breakout, or you can make a custom cable)
4. A 10-100k pull-down resistor on pin 5 (AD_COM) to enable power output from the port

### Open Source Implementations

| Project | Language | Platform | Gimbal |
|---------|----------|----------|--------|
| DJIR_SDK | C++ | Desktop (CMake) | RS 2 |
| dji_rs3pro_ros_controller | C++/Python | ROS Noetic / Linux | RS 3 Pro |
| dji_rs2_ros_controller | Python | ROS / Linux | RS 2 |
| DJI-Ronin-RS2-Log-and-Replay | Python | Linux (python-can) | RS 2/3 |
| ArduPilot mount-djirs2-driver.lua | Lua | Flight controllers | RS 2, RS 3 Pro |
| **OpenDolly dji_can** | **C (ESP-IDF)** | **ESP32-S3** | **RS 5 (target)** |

No Arduino or ESP32 library existed before our project. A Teensy 4.1 attempt in late 2024 failed without producing working code.

---

## 2. SBUS

**A simpler but more limited control path.**

### What It Is

SBUS (Futaba Serial Bus) is an RC protocol originally designed for servos and receivers. DJI RS gimbals accept SBUS input on pin 3 of the RSA port, interpreting it as joystick-style speed commands. This is the same input the DJI Focus Wheel sends when its CAN/SBUS switch is set to SBUS mode.

### How It Works

- **Inverted serial UART:** 100 kbaud, 8E2 (8 data bits, even parity, 2 stop bits)
- **16 proportional channels** + 2 digital channels per 25-byte frame
- **Send rate:** 50-100 Hz
- **Value range:** 0-2047 per channel, 1023 = center/neutral
- **Requires signal inverter:** An NPN transistor circuit or a UART with configurable inversion (ESP32 supports this natively)

### DJI-Specific Channel Mapping

The ExpressLRS project reverse-engineered DJI's exact SBUS expectations:

| Channel | Function | Range | Notes |
|---------|----------|-------|-------|
| CH1 | Pan (yaw) | 352-1696 | Speed proportional |
| CH2 | Tilt (pitch) | 352-1696 | Speed proportional |
| CH4 | Roll | 352-1696 | Speed proportional |
| CH5 | Arm switch | -- | Must stay low/unarmed |
| CH6 | Record/Photo | 352-1696 | Toggle center-to-extreme in 0.4s |
| CH7 | Mode select | 352-1696 | 3 stable positions |
| CH8 | Recenter/Selfie | 176-848 | Toggle center-to-extreme in 0.4s |

DJI's SBUS implementation has non-standard endpoint values. Standard SBUS output with default endpoints will not correctly trigger functions like recentering. The ExpressLRS "DJI RS Pro" output mode pre-configures these values.

### Limitations

- **Speed control only** -- no absolute position commands
- **No telemetry feedback** -- you cannot read the gimbal's current angle
- **No focus motor control**
- **No presets or calibration commands**
- **Recentering requires precise timing** -- a 0.4-second toggle gesture on CH8

### Hardware for DIY SBUS Control

For Arduino/ESP32, you need:
- An SBUS library (bolderflight/sbus, TheDIYGuy999/SBUS, or BMC_SBUS)
- A signal inverter circuit (NPN transistor) OR use ESP32's UART inversion capability
- Wire to pin 3 (SBUS_RX) and pin 6 (GND) of the RSA port

---

## 3. Third-Party Commercial Products

All of these connect to the RSA/NATO port and use either CAN bus or SBUS internally.

### Middle Things APC-R

| Attribute | Detail |
|-----------|--------|
| What it does | Turns DJI RS gimbals into network-controllable PTZ cameras |
| Connection | RSA port via included cable; CAN bus (RS series) or SBUS (Ronin-S/SC/2) |
| Network | Ethernet + built-in WiFi |
| Control | Middle Control macOS app, USB game controllers (up to 4), ATEM, Bitfocus Companion |
| Gimbals | RS 2, RS 3 Pro, RS 4, RS 4 Pro, RS 5 (CAN); Ronin-S, Ronin-SC, Ronin-2 (SBUS) |
| CAN features | Position presets, focus motor, remote calibration, sleep/wake, ActiveTrack |
| SBUS features | Basic PTZ movement only |
| Camera control | Bluetooth to Blackmagic cameras |
| API | No public SDK or developer API |
| Price | Standard (free), Pro (199 EUR) + APC-R hardware |

### eMotimo Conductor SA2.6

| Attribute | Detail |
|-----------|--------|
| What it does | Coordinates gimbal + slider/dolly/turntable for automated camera motion |
| Connection | RSI dongle + 6-pin cable to RSA port (likely CAN bus internally) |
| Gimbals | RS 2, RS 3, RS 4, RS 5 |
| Features | Pan/tilt/roll/focus, keyframe animation, Dragonframe integration |
| API | Published eMotimo API; integrates with Bitfocus Companion, SKAARHOJ, Dragonframe |
| Price | $1,549 (with RSI) |

### SKAARHOJ PTZ Pro

| Attribute | Detail |
|-----------|--------|
| What it does | Professional broadcast PTZ controller for DJI RS gimbals |
| Connection | USB or IP; likely requires APC-R or similar bridge hardware |
| Gimbals | RS 3 Pro, RS 4 |
| Features | Joystick pan/tilt, lens zoom, preset recall, ATEM tally integration |
| Protocol | VISCA gimbal control |

### iFlight Gimbal Controller

| Attribute | Detail |
|-----------|--------|
| What it does | Wireless remote control for DJI RS gimbals (designed for cinelifter use) |
| Connection | NATO/RSA port; outputs modified SBUS ("DJI RS Pro" protocol) |
| Radio | ExpressLRS 2.4 GHz or 900 MHz |
| Gimbals | RS 2, RS 3 Pro, RS 4, RS 4 Pro |
| Features | Pan/tilt/roll, record, recenter |
| Size | 47x31x12mm, 28g |
| Price | ~$30-40 |

### RadioMaster GR-01

| Attribute | Detail |
|-----------|--------|
| What it does | ExpressLRS receiver purpose-built for DJI RS gimbals |
| Connection | RSA port; outputs "DJI RS Pro" modified SBUS |
| Gimbals | RS 2, RS 3 Pro (designed); RS 4 Pro (community-confirmed) |
| Price | ~$25-30 |

### SmallRig 3920 Wireless Controller

| Attribute | Detail |
|-----------|--------|
| What it does | Wireless joystick control for DJI RS gimbals |
| Connection | Receiver connects via **USB-C** (not RSA port) |
| Gimbals | RS 2, RS 3 Pro, RS 4, RS 4 Pro |
| Features | Joystick, mode switch, centering, record |
| Range | 100m, <50ms latency |
| Protocol | Unknown/proprietary (USB-C based) |

### Cyanview CI0

| Attribute | Detail |
|-----------|--------|
| What it does | Broadcast camera shading/control system with gimbal integration |
| Connection | SBUS output to RSA port |
| Gimbals | Ronin-S, Ronin-SC (documented); RS series likely compatible |
| Features | Pan/tilt/roll via SBUS, integrated with video router for multi-camera |

---

## 4. Interfaces That Do NOT Provide Gimbal Motion Control

### Bluetooth

Bluetooth on DJI RS gimbals is strictly for:
- Camera shutter control (autofocus, record, photo)
- Ronin app connection (settings, firmware updates, ActiveTrack config)
- Camera pairing (Sony, Canon, Blackmagic)

No gimbal motion commands are sent over Bluetooth. No one has reverse-engineered BLE for motion control. The Ronin app does send gimbal settings/configuration over BLE, but motion commands go through CAN bus only.

### WiFi

DJI RS gimbals do not have WiFi. The Enhanced Intelligent Tracking Module (RS 5) and RavenEye image transmitter use WiFi for video transmission and ActiveTrack, but these are video-only -- no gimbal control channel.

### USB-C Ports

The RS series has multiple USB-C ports, none officially documented for gimbal motion control:
- **Camera Control Port:** For camera shutter/focus/aperture/ISO via wired cable
- **Multifunctional Port:** For DJI accessories (SDR Transmission, Focus Pro Motor, firmware updates)
- **Tracking Module Port:** Firmware updates only

**However:** The SmallRig 3920 wireless controller connects its receiver to a USB-C port and achieves joystick-style gimbal control. This means some USB-C port does carry a gimbal control protocol, but it is completely undocumented and no one has reverse-engineered it.

### DJI Ronin App

The Ronin app has a virtual joystick for gimbal control, but:
- No API, SDK, IPC, intent, or network interface is exposed
- No way to drive it programmatically
- The app communicates with the gimbal over BLE for settings only
- No one has found a way to intercept or automate the app's virtual joystick

### DUML Protocol (USB/Serial)

DJI's internal DUML protocol is used for inter-module communication (firmware updates, calibration, diagnostics) over serial/USB. While partially reverse-engineered for drone products, it has NOT been reverse-engineered for gimbal motion control on RS series. The USB-C connection during firmware updates likely uses DUML, but no one has documented gimbal motion commands over this path.

---

## 5. Physical Interface Summary

### RSA/NATO Port Pinout (6-Pin, RS 2 / RS 3 Pro / RS 4 era)

| Pin | Signal | Description |
|-----|--------|-------------|
| 1 | VCC | 8V +/- 0.4V output (0.8A rated, 1.2A peak) |
| 2 | CANL | CAN bus low |
| 3 | SBUS_RX | SBUS input (inverted UART) |
| 4 | CANH | CAN bus high |
| 5 | AD_COM | Accessory detect (needs 10-100k pull-down for power) |
| 6 | GND | Ground |

The RS 5 introduced a new **8-pin RSA port**. The additional 2 pins are undocumented. The APC-R supports RS 5, so CAN bus is confirmed to work on the new port.

### CAN Bus Parameters

- Baud rate: 1 Mbps
- TX ID: 0x223 (host to gimbal)
- RX ID: 0x222 (gimbal to host)
- Standard CAN frames (not extended)
- CRC16 (poly 0x8005, init 0x3AA3) and CRC32 (poly 0x04C11DB7, init 0x3AA3)

---

## 6. Landscape Summary

| Method | Type | Position Control | Feedback | Focus Motor | Complexity | Status |
|--------|------|-----------------|----------|-------------|------------|--------|
| CAN bus (DJI R SDK) | Wired, bidirectional | Absolute angles | Yes | Yes | Medium | Official, documented |
| SBUS | Wired, one-way | Speed only | No | No | Low | Community reverse-engineered |
| Middle Things APC-R | CAN-to-IP bridge | Via CAN | Via CAN | Via CAN | Low (turnkey) | Commercial, no API |
| eMotimo SA2.6 | CAN via RSI dongle | Yes | Yes | Yes | Low (turnkey) | Commercial, has API |
| iFlight/RadioMaster | SBUS via radio | Speed only | No | No | Low | Commercial |
| SmallRig 3920 | USB-C (unknown protocol) | Speed only | No | No | Low (turnkey) | Commercial |
| Bluetooth | -- | **No** | **No** | **No** | -- | Dead end |
| WiFi | -- | **No** | **No** | **No** | -- | Dead end |
| Ronin app | -- | **No API** | **No API** | **No API** | -- | Dead end |
| USB-C (documented) | Camera control only | **No** | **No** | **No** | -- | Camera shutter/focus only |
| USB-C (undocumented) | SmallRig proves possible | Speed only? | Unknown | Unknown | -- | Undocumented, unresearched |

---

## 7. Key Takeaways for Our Project

1. **CAN bus is the right choice** for the OpenDolly slider project. It is the only method that provides absolute position control and telemetry feedback -- both essential for repeatable keyframe-based camera motion.

2. **SBUS could serve as a fallback or diagnostic tool.** The ESP32-S3 can generate SBUS frames natively (its UART supports signal inversion). This could be useful for quick testing or as a degraded-mode fallback if CAN bus encounters issues.

3. **No one has built an ESP32-native DJI CAN bus library before our project.** The existing open-source projects all target desktop Linux (socketCAN) or specific flight controllers (ArduPilot). Our `firmware/lib/dji_can/` implementation is genuinely novel.

4. **The RS 5's new 8-pin RSA port needs investigation.** The pinout is undocumented, but CAN bus is confirmed to work (Middle Things APC-R supports RS 5). We need to determine the physical connector and pin mapping.

5. **The SmallRig USB-C control path is an interesting unknown.** If someone were to reverse-engineer the USB-C protocol, it could provide an alternative control path that doesn't require accessing the RSA port. But this is unexplored territory and not a priority.
