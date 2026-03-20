# DJI Gimbal Software Control - Research Report

**Date:** 2026-03-20

## Executive Summary

DJI provides multiple official pathways for programmatic gimbal control, but they vary significantly by product line. **For standalone handheld gimbals (RS series), the DJI RS SDK is the primary official option**, communicating over CAN bus and supporting the RS 5, RS 4, RS 4 Pro, and RS 3 Pro. For drone-mounted gimbals, the Mobile SDK (MSDK), Onboard SDK (OSDK, deprecated), and Payload SDK (PSDK) all provide gimbal control APIs. There is no Bluetooth or WiFi-based API for direct gimbal position control on handheld stabilizers -- Bluetooth is limited to camera shutter control. The community has built several open-source projects around the CAN-based DJI R SDK protocol, primarily in C++ and Python with ROS integration. Commercial products like Middle Things and SKAARHOJ have productized RS gimbal control for cinema/broadcast PTZ workflows.

---

## 1. Official DJI SDKs with Gimbal Control

### 1.1 DJI RS SDK (Handheld Gimbals)

**The most relevant SDK for standalone gimbal control.**

- **Official page:** https://www.dji.com/rs-sdk
- **Supported gimbals:** RS 5, RS 4, RS 4 Pro, RS 3 Pro
- **Communication:** CAN bus (Controller Area Network)
- **Capabilities:**
  - Set gimbal position (absolute angle control)
  - Control gimbal rotation speed
  - Retrieve motor and attitude information
  - Command camera focus and shooting functions
- **Downloads available:** SDK documentation (ZIP), demo software (ZIP), external interface diagram (PDF)
- **Languages:** English and Chinese
- **Primary use cases:** Virtual studio systems (multi-gimbal control from PC/console), slider integration (4D camera control with programmed movement paths)

**Hardware requirements:** A CAN-to-USB adapter is needed to interface with the gimbal. The DJI R Focus Wheel accessory provides the CAN connection point. Common adapters include USBCAN-II C, CANable Pro, Lawicel CANUSB, and SLCANUINO.

### 1.2 Mobile SDK (MSDK)

- **Platforms:** iOS (Objective-C/Swift), Android (Java/Kotlin)
- **GitHub:** https://github.com/dji-sdk (separate repos for iOS and Android, plus MSDK V5)
- **Gimbal support:** Drone-mounted gimbals, Osmo series, Ronin-MX
- **NOT for standalone RS gimbals** (RS 2, RS 3, RS 4, RS 5)

**Gimbal control features:**
- Three work modes: FPV (pitch only), Yaw Follow (pitch + roll), Free (all axes)
- Angle-based control: absolute or relative positioning with duration
- Speed-based control: continuous rotation at specified speed
- State telemetry at up to 10 Hz
- SmoothTrack (Osmo/Ronin-MX): adjustable speed, acceleration, deadband
- Motor control: PID coefficient adjustment for variable payloads
- Orientation lock, selfie mode

**Current status:** MSDK V5 is the current version (released March 2022). V4.16 was the last V4 release; V4 no longer supports new products.

### 1.3 Onboard SDK (OSDK)

- **GitHub:** https://github.com/dji-sdk/Onboard-SDK (C++, 973 stars)
- **Communication:** Serial UART to flight controller
- **Status:** Effectively deprecated. Gimbal APIs (`Gimbal::setAngle`, `Gimbal::setSpeed`) are deprecated in favor of CameraManager APIs.
- **Gimbal control:** Absolute angle control, speed control, free mode
- **Coordinate systems:** Joint angles (body frame) and attitude angles (NED geodetic Euler angles)

### 1.4 Payload SDK (PSDK)

- **GitHub:** https://github.com/dji-sdk/Payload-SDK (C, 473 stars)
- **Purpose:** Building custom payloads for enterprise drones
- **Gimbal relevance:** Works with DJI X-Port (a standard gimbal with built-in SkyPort) and SkyPort adapters
- **Gimbal control:** Absolute angle control (rotate to position in specified time), speed control (0.5s per command)
- **X-Port features:** Built-in communication APIs, gimbal debugging interface, provides power/GPS/flight data to payload
- **Supported drones:** M300 RTK, M30 series, Mavic 3 Enterprise series

### 1.5 Windows SDK

- **GitHub:** https://github.com/dji-sdk/Windows-SDK
- **Status:** Exists but appears less actively developed. Limited documentation found for gimbal control.

### 1.6 Cloud API

- **Protocols:** MQTT, HTTPS, WebSocket
- **Purpose:** Remote operations via DJI Pilot 2 or DJI Dock
- **Gimbal relevance:** Limited; focused on fleet management rather than direct gimbal control

---

## 2. DJI Gimbal Product Lines and SDK Support

| Model | Category | DJI RS SDK (CAN) | Mobile SDK | Other | Notes |
|-------|----------|------------------|------------|-------|-------|
| **RS 5** | Current | YES | No | -- | Official RS SDK, 5th-gen stabilization |
| **RS 4 Pro** | Current | YES | No | -- | Official RS SDK, 4.5 kg payload |
| **RS 4** | Current | YES | No | -- | Official RS SDK, 3 kg payload |
| **RS 4 Mini** | Current | Unknown | No | -- | Not listed in RS SDK docs |
| **RS 3 Pro** | Legacy | YES | No | -- | Official RS SDK + community ROS projects |
| **RS 3** | Legacy | Unknown | No | -- | Not listed in RS SDK docs |
| **RS 3 Mini** | Legacy | Unknown | No | -- | Not listed |
| **RS 2** | Legacy | YES (v2.2) | No | -- | Community C++ and Python SDKs |
| **RSC 2** | Legacy | Unknown | No | -- | No SDK info found |
| **Ronin-SC** | Legacy | No | No | S-Bus | Community S-Bus hack via Arduino |
| **Ronin-S** | Legacy | Unknown | No | -- | No SDK info found |
| **Ronin-MX** | Legacy | No | YES | -- | Drone-mountable, MSDK support |
| **Ronin (original)** | Legacy | No | No | Hack | Screen capture + Arduino hack only |
| **Osmo series** | Various | No | YES | -- | MSDK support (smartphone gimbals) |
| **X-Port** | Enterprise | N/A | Via PSDK | PSDK | Custom payload gimbal for drones |

**Key observations:**
- The RS SDK officially lists only RS 5, RS 4, RS 4 Pro, and RS 3 Pro
- The RS 2 uses an older protocol version (v2.2) but is supported by community tools
- "Mini" variants and the standard RS 3 are not confirmed to support the RS SDK
- Bluetooth on RS gimbals is for camera shutter control only, not gimbal motion

---

## 3. Communication Protocols

### 3.1 DJI R SDK Protocol (CAN Bus)

**The primary protocol for standalone RS gimbal control.**

- **Physical layer:** CAN bus (Controller Area Network)
- **Baud rate:** 1 Mbps (1,000,000 bps)
- **Protocol version:** v2.2 (documented for RS 2); newer versions exist for RS 4/5
- **Connection:** Via DJI R Focus Wheel port (has physical CAN/S-Bus toggle switch)
- **Angle units:** 0.1 degree increments
- **Speed units:** 0.1 degrees/second
- **Ranges:** Yaw/Roll/Pitch: -180.0 to +180.0 degrees; Speed: 0 to 360 deg/s
- **Minimum position command execution time:** 100ms
- **Control modes:** Absolute position control, speed control, position feedback query

**CAN adapter setup (Linux):**
```bash
# Using Lawicel CANUSB or CANable Pro
sudo slcand -o -s8 -t hw -S 1000000 /dev/ttyUSB0
sudo ip link set up slcan0
```

### 3.2 S-Bus Protocol

- **Used for:** Ronin SC (community hack)
- **Method:** Simulates RC remote control signals
- **Hardware:** Arduino + serial inverter (inverts UART to S-Bus signal)
- **Library:** BMC_SBUS
- **Limitation:** Not a native API; acts as simulated joystick input

### 3.3 DUML Protocol (DJI Universal Markup Language)

- **Also called:** MB protocol
- **Purpose:** DJI's internal inter-module communication protocol
- **Transport layers:** Serial/UART, USB, WiFi (UDP)
- **Gimbal relevance:** Can trigger gimbal calibration via serial DUML packets
- **Status:** Partially reverse-engineered; no complete gimbal motion control documentation
- **Tools:** Wireshark dissector available, DUML packet construction tools exist

### 3.4 WiFi/Bluetooth (via MSDK)

- **Used by:** Mobile SDK for drone-mounted gimbals and Osmo
- **Communication:** WiFi between mobile device and drone/Osmo
- **Bluetooth on RS gimbals:** Camera shutter control only; NOT gimbal position control
- **No direct BLE gimbal control API exists**

---

## 4. Open Source Projects

### 4.1 Direct Gimbal Control Projects

| Project | Language | Target Gimbal | Protocol | Framework | GitHub |
|---------|----------|--------------|----------|-----------|--------|
| **DJIR_SDK** | C++ | RS 2 | CAN / DJI R SDK v2.2 | Standalone (CMake) | [ConstantRobotics/DJIR_SDK](https://github.com/ConstantRobotics/DJIR_SDK) |
| **dji_rs3pro_ros_controller** | C++/Python | RS 3 Pro | CAN 1 Mbps | ROS Noetic | [Hibiki1020/dji_rs3pro_ros_controller](https://github.com/Hibiki1020/dji_rs3pro_ros_controller) |
| **dji_rs2_ros_controller** | Python | RS 2 | CAN | ROS | [ceinem/dji_rs2_ros_controller](https://github.com/ceinem/dji_rs2_ros_controller) |
| **DJI-Ronin-RS2-Log-and-Replay** | Python | RS 2/3 | CAN (python-can) | Standalone | [rileycoyote87/DJI-Ronin-RS2-Log-and-Replay](https://github.com/rileycoyote87/DJI-Ronin-RS2-Log-and-Replay) |
| **Ronin-SC-Gimbal-Controller** | C++/Python | Ronin SC | S-Bus | ROS/Arduino | [VCHS-R-D/Ronin-SC-Gimbal-Controller](https://github.com/VCHS-R-D/Ronin-SC-Gimbal-Controller) |
| **dji_ronin** | Python | Ronin (original) | ADB screen capture hack | ROS | [Itamare4/dji_ronin](https://github.com/Itamare4/dji_ronin) |

### 4.2 Protocol Reverse Engineering Projects

| Project | Focus | GitHub |
|---------|-------|--------|
| **dji_protocol** | DUML/MB protocol, UDP, USB protocols, Wireshark dissector | [samuelsadok/dji_protocol](https://github.com/samuelsadok/dji_protocol) |
| **dji-firmware-tools** | DUML tools, gimbal calibration, serial packet tools | [o-gs/dji-firmware-tools](https://github.com/o-gs/dji-firmware-tools) |
| **dji_rev** | Firmware extraction, key derivation, FC/ESC analysis | [fvantienen/dji_rev](https://github.com/fvantienen/dji_rev) |
| **Reverse-Engineering-DJI-drones** | Custom djictl CLI, protocol analysis | [vm32/Reverse-Engineering-DJI-drones](https://github.com/vm32/Reverse-Engineering-DJI-drones) |

### 4.3 Notable Project Details

**DJIR_SDK (Best standalone C++ library):**
- API: `connect()`, `disconnect()`, `move_to(yaw, roll, pitch, time)`, `set_speed(yaw, roll, pitch)`, `get_current_position()`, `set_move_mode()`
- Requires USBCAN-II C adapter
- C++11, CMake build

**DJI-Ronin-RS2-Log-and-Replay (Best Python example):**
- Two scripts: `ronin-record` (logs yaw/roll/pitch/focus) and `ronin-play` (replays)
- Uses `python-can` library via socketCAN
- Requires CANable Pro with candlelight firmware
- Includes 3D-printable CAN connector STL file

**ArduPilot Integration (Best autopilot integration):**
- Lua script: `mount-djirs2-driver.lua`
- ArduPilot 4.4+, CAN bus configuration
- Limitation: yaw is always "earth frame" follow mode

---

## 5. Integration Examples and Commercial Products

### 5.1 Cinema/Broadcast PTZ Systems

**Middle Things (Middle Control)**
- **What:** macOS app that turns DJI RS gimbals into cinematic PTZ systems
- **Supported gimbals:** RS 2, RS 3 Pro, RS 4, RS 4 Pro
- **Hardware:** APC-R controller (connects via cable to gimbal, provides Ethernet/WiFi network control)
- **Protocols:** VISCA over IP, ATEM switcher sync, Bitfocus Companion, TCP API/SDK
- **Features:** Up to 8 gimbals from one laptop, preset recall, DJI ActiveTrack remote triggering, camera control via Bluetooth (Blackmagic cameras)
- **Pricing:** Standard (free), Pro (199 EUR one-time)
- **Website:** https://www.middlethings.co/

**SKAARHOJ Controllers**
- Integrates with DJI RS 3 Pro and RS 4
- Joystick pan/tilt, lens zoom, preset recall
- USB and IP control, VISCA protocol

### 5.2 Motorized Slider Integration

- **eMotimo PTZ Mover:** Motion control system compatible with DJI gimbals
- **iFootage Shark Slider Nano 2:** Motorized slider with DJI gimbal control; creates 5-axis PTZ with keyframe programming
- **CAME-TV Slider:** Remote-controlled slider for DJI RS 2, 3.47m track

### 5.3 Robotics (ROS) Integration

Three ROS-based projects exist for direct gimbal control (see Section 4.1). All use CAN bus. The RS3 Pro ROS controller also integrates with Xsens MTi-30 IMU for stabilization compensation and Intel RealSense D435 cameras, with Docker-based deployment.

### 5.4 Autopilot Integration

ArduPilot 4.4+ supports DJI RS2 and RS3 Pro gimbals via a Lua script driver, using CAN bus communication for autonomous gimbal control during flight.

---

## 6. Licensing and Limitations

### 6.1 SDK Licensing

- **No per-unit or per-app fees** for SDK usage
- **RS SDK documentation is freely downloadable** from dji.com/rs-sdk
- PSDK Production Mode is free (since Nov 2021)
- All developers receive equal benefits (tiered membership system eliminated)
- **EULA restrictions:**
  - SDK object code must be distributed only in execution form, as part of your application
  - Cannot distribute SDK source code or documentation
  - Cannot release SDK code under an open source license
  - Cannot charge separate fee for SDK code
  - DJI trademarks cannot be used in product names

### 6.2 Known Limitations

**RS SDK limitations:**
- CAN bus only -- no WiFi, Bluetooth, or USB serial for gimbal position control
- Requires physical CAN adapter hardware ($20-100)
- DJI R Focus Wheel or custom connector needed for CAN access
- Not all RS models are supported (Mini variants, standard RS 3 not listed)
- Protocol documentation provided only as downloadable ZIP, not online

**MSDK limitations (for drone gimbals):**
- Requires DJI mobile device connection (via WiFi/USB to drone remote)
- Only works through iOS/Android apps
- MSDK V5 not backward-compatible with V4
- Does not support standalone RS gimbals

**OSDK limitations:**
- Effectively deprecated
- Gimbal APIs deprecated in favor of CameraManager
- UART serial to flight controller only

**ArduPilot limitations:**
- Yaw rate control is always "follow" (earth frame), not body-frame
- Only RS 2 and RS 3 Pro confirmed working

**General limitations:**
- No REST API or network-based API for direct gimbal control from DJI
- No Python library from DJI for gimbal control
- Bluetooth is limited to camera shutter, not gimbal motion
- Complete DUML protocol for gimbal motion control has not been reverse-engineered

---

## 7. Summary of Available Approaches

For someone looking to control a DJI gimbal from custom software, here are the practical options ranked by maturity:

1. **DJI RS SDK (CAN bus)** -- Best official path for RS 5/4/4 Pro/3 Pro. Requires CAN adapter hardware. Documentation freely available. Use the official demo software as starting point, or the community DJIR_SDK (C++) or DJI-Ronin-RS2-Log-and-Replay (Python) projects.

2. **Middle Things / APC-R** -- Best turnkey commercial solution for cinema/broadcast. Provides network-based control with VISCA, presets, and multi-gimbal management. Supports RS 2/3 Pro/4/4 Pro.

3. **Community CAN bus projects** -- Multiple open-source ROS and standalone implementations for RS 2 and RS 3 Pro. All require Linux + CAN adapter + python-can or socketCAN.

4. **DJI Mobile SDK** -- For drone-mounted gimbals and Osmo products only. Requires iOS/Android app development. Well-documented with comprehensive API.

5. **S-Bus hack** -- For Ronin SC only. Requires Arduino + serial inverter. Simulates RC input rather than direct API control.

6. **DUML serial tools** -- For calibration and firmware analysis only. Not a viable path for gimbal motion control.

---

## Key URLs

| Resource | URL |
|----------|-----|
| DJI RS SDK | https://www.dji.com/rs-sdk |
| DJI Developer Portal | https://developer.dji.com/ |
| DJI SDK GitHub Organization | https://github.com/dji-sdk |
| MSDK Gimbal Documentation | https://developer.dji.com/mobile-sdk/documentation/introduction/component-guide-gimbal.html |
| OSDK Gimbal Documentation | https://developer.dji.com/onboard-sdk/documentation/tutorial/gimbal-manager.html |
| PSDK Gimbal Documentation | https://developer.dji.com/payload-sdk/documentation/tutorial/gimbal-control.html |
| ArduPilot RS2/RS3 Integration | https://ardupilot.org/copter/docs/common-djirs2-gimbal.html |
| DJI SDK Forum | https://sdk-forum.dji.net |
| Middle Things | https://www.middlethings.co/ |
| DJI Developer EULA | https://developer.dji.com/policies/eula/ |
