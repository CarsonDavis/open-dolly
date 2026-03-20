# DJI Gimbal Software Control - Research Background

**Date:** 2026-03-20
**Topic:** Comprehensive research on interacting with DJI gimbals via software - official SDKs, protocols, open source projects, communication interfaces, and integration examples.

## Sources

[1]: https://developer.dji.com/mobile-sdk/documentation/introduction/component-guide-gimbal.html "DJI Mobile SDK - Gimbal Component Guide"
[2]: https://developer.dji.com/onboard-sdk/documentation/tutorial/gimbal-manager.html "DJI Onboard SDK - Gimbal Management"
[3]: https://developer.dji.com/api-reference/android-api/Components/Gimbal/DJIGimbal.html "DJI Mobile SDK Android API - Gimbal"
[4]: https://developer.dji.com/payload-sdk/documentation/tutorial/gimbal-control.html "DJI Payload SDK - Gimbal Control"
[5]: https://developer.dji.com/onboard-sdk/documentation/guides/component-guide-camera-and-gimbal.html "DJI Onboard SDK - Camera and Gimbal Guide"
[6]: https://github.com/orgs/dji-sdk/repositories "DJI SDK GitHub Organization"
[7]: https://enterprise-insights.dji.com/blog/dji-sdk-guide "DJI SDK Guide - Enterprise Insights"
[8]: https://github.com/dji-sdk/Payload-SDK "DJI Payload SDK GitHub"
[9]: https://github.com/dji-sdk/Onboard-SDK "DJI Onboard SDK GitHub"
[10]: https://github.com/dji-sdk/Windows-SDK "DJI Windows SDK GitHub"
[11]: https://www.dji.com/rs-sdk "DJI RS SDK Official Page"
[12]: https://github.com/ConstantRobotics/DJIR_SDK "DJIR SDK - Third-party C++ SDK for DJI RS 2"
[13]: https://github.com/Itamare4/dji_ronin "DJI Ronin ROS Node"
[14]: https://ardupilot.org/copter/docs/common-djirs2-gimbal.html "ArduPilot DJI RS2/RS3 Gimbal Integration"
[15]: https://forum.dji.com/thread-274563-1-1.html "DJI RS SDK Forum Thread"
[16]: https://github.com/Hibiki1020/dji_rs3pro_ros_controller "DJI RS3 Pro ROS Controller"
[17]: https://github.com/VCHS-R-D/Ronin-SC-Gimbal-Controller "Ronin SC Gimbal Controller (S-Bus)"
[18]: https://developer.dji.com/onboard-sdk/documentation/protocol-doc/open-protocol.html "DJI Onboard SDK Open Protocol"
[19]: https://github.com/samuelsadok/dji_protocol "DJI Protocol - Partial Documentation of Proprietary Protocols"
[20]: https://github.com/o-gs/dji-firmware-tools "DJI Firmware Tools"
[21]: https://github.com/fvantienen/dji_rev "DJI Reverse Engineering"
[22]: https://github.com/vm32/Reverse-Engineering-DJI-drones "Reverse Engineering DJI Drones"
[23]: https://www.dji.com/products/comparison-rs "DJI Ronin Stabilizer Comparison"
[24]: https://www.newsshooter.com/2026/01/14/turn-dji-rs-3-pro-and-rs-4-gimbals-into-real-ptz-cameras-with-skaarhoj-controllers/ "SKAARHOJ Controllers with DJI RS Gimbals"
[25]: https://www.middlethings.co/ "Middle Things - Cinematic & Modular PTZ"
[26]: https://www.middlethings.co/apc-r-controller/ "Middle Things APC-R Controller"
[27]: https://enterprise-insights.dji.com/blog/dji-developer-sdk-new-support-policies "DJI SDK New Support Policies"
[28]: https://developer.dji.com/policies/eula/ "DJI Developer EULA"
[29]: https://emotimo.com/products/ptz-mover "eMotimo PTZ Mover"
[30]: https://www.diyphotography.net/the-new-ifootage-shark-slider-nano-2-controls-your-dji-gimbal/ "iFootage Shark Slider Nano 2"
[31]: https://github.com/rileycoyote87/DJI-Ronin-RS2-Log-and-Replay "DJI RS2 Log and Replay (Python)"
[32]: https://github.com/ceinem/dji_rs2_ros_controller "DJI RS2 ROS Controller"
[33]: https://discuss.ardupilot.org/t/ap-support-for-dji-rs2-rsc2-rs3-gimbals-maybe/91830 "ArduPilot RS2/RSC2/RS3 Discussion"
[34]: https://developer.dji.com/onboard-sdk/documentation/appendix/releaseNotes.html "DJI OSDK Release Notes"
[35]: https://developer.dji.com/payload-sdk-v2/ "DJI Payload SDK v2"
[36]: https://www.heliguy.com/blogs/posts/dji-x-port-opening-the-door-to-fast-payload-to-drone-integration/ "DJI X-Port Overview"

<!-- Add new sources here as you find them. Number sequentially. -->

## Research Log

---

### Search: "DJI SDK gimbal control official documentation Mobile SDK Onboard SDK Payload SDK"

**DJI has three main SDKs that support gimbal control:**

1. **Mobile SDK (MSDK)** - For iOS and Android. Supports gimbal control on DJI drones and handheld products (Osmo, Ronin-MX). Three work modes: FPV Mode (pitch only), Yaw Follow Mode (pitch and roll), Free Mode (all axes independent). Two control methods: angle-based (absolute/relative positioning) and speed-based (clockwise/counter-clockwise). State info transmits at up to 10 Hz. ([DJI Mobile SDK Gimbal Guide][1])

2. **Onboard SDK (OSDK)** - For embedded systems on DJI aircraft. Supports absolute angle control, speed control, and free mode. Uses joint angles (body-coordinate) and attitude angles (NED geodetic Euler angles). Speed control executes for 0.5s per command. ([DJI Onboard SDK Gimbal Management][2])

3. **Payload SDK (PSDK)** - For third-party payload development on DJI enterprise drones. ([DJI Payload SDK Gimbal Control][4])

---

### Search: "DJI developer platform all SDKs list Windows SDK MSDK v5 OSDK PSDK GitHub repositories"

**DJI SDK GitHub org: 46 repos.** Key: Payload SDK (C, 473 stars), MSDK Android (Java, 1090), MSDK iOS (Obj-C, 596), OSDK (C++, 973), Windows SDK, Edge SDK (C++, 54), RoboMaster SDK (Python, 394). ([DJI SDK GitHub Organization][6])

Enterprise SDKs: MSDK, PSDK, Cloud API (MQTT/HTTPS/WebSocket). 100K+ developers. ([DJI SDK Guide][7])

---

### Search: "DJI Ronin SDK RS SDK gimbal API developer control RS3 RS4 programmatic"

**DJI RS SDK: official product.** Supports RS 5, RS 4, RS 4 Pro, RS 3 Pro. CAN bus protocol. Position, rotation, motor/attitude info, camera control. Downloads: docs ZIP, demo ZIP, interface PDF. ([DJI RS SDK Official Page][11])

Third-party DJIR SDK: C++, protocol v2.2, CAN/USBCAN-II C, RS 2. ([DJIR SDK][12])

---

### Search: "DJI RS SDK CAN bus protocol UART serial communication gimbal control protocol specification"

ArduPilot: CAN bus, Lua script, ArduPilot 4.4+, yaw earth-frame only. ([ArduPilot DJI RS2/RS3][14])
Ronin SC: S-Bus via Arduino + serial inverter. ([Ronin SC Gimbal Controller][17])

---

### Search: "DJI DUSS protocol reverse engineer gimbal serial protocol open source GitHub"

DUML protocol documented partially. ([DJI Protocol][19]) Gimbal calibration via DUML serial. ([DJI Firmware Tools][20]) No complete motion control RE.

---

### Search: "DJI gimbal Bluetooth BLE control Python library RS3 RS4 Ronin wireless protocol"

No BLE gimbal control library. CAN at 1 Mbps via CANUSB. ([DJI RS3 Pro ROS Controller][16]) BT = shutter only.

---

### Search: "DJI gimbal models list RS 2 RS 3 RS 4 RS 5 Ronin Osmo SDK support comparison which programmable"

SDK support table compiled. RS SDK: RS5, RS4, RS4 Pro, RS3 Pro. MSDK: Ronin-MX, Osmo. S-Bus hack: Ronin SC. ([DJI Ronin Stabilizer Comparison][23])

---

### Search: "DJI RS gimbal cinema broadcast automation virtual studio PTZ control integration slider motorized"

Middle Things: macOS PTZ app, APC-R controller, VISCA/IP, up to 8 gimbals, free/199 EUR. ([Middle Things][25], [APC-R Controller][26])
SKAARHOJ: RS3 Pro/RS4, USB/IP, VISCA. ([SKAARHOJ Controllers][24])
Sliders: eMotimo, iFootage, CAME-TV. ([eMotimo][29], [iFootage][30])

---

### Search: "DJI SDK licensing restrictions limitations enterprise agreement developer terms gimbal control"

EULA: execution form only, no source redistribution, no open source licensing of SDK. ([DJI Developer EULA][28])
Nov 2021: PSDK Production Mode free, no member tiers, hardware discounts. ([DJI SDK New Support Policies][27])

---

### Search: "GitHub DJI gimbal control open source Python CAN bus RS2 RS3 serial protocol library 2024 2025"

**DJI RS2 Log and Replay (Python):** ronin-record/ronin-play scripts, python-can, CANable Pro adapter, Ubuntu 20.04. ([DJI RS2 Log and Replay][31])

**DJI RS2 ROS Controller (ceinem):** Python ROS node, CAN bus, CANusb/SLCANUINO, socketcan_bridge. ([DJI RS2 ROS Controller][32])

**All open source projects:**

| Project | Language | Gimbal | Protocol | Framework |
|---------|----------|--------|----------|-----------|
| DJIR_SDK | C++ | RS 2 | CAN/DJI R SDK v2.2 | Standalone |
| dji_rs3pro_ros_controller | C++/Python | RS3 Pro | CAN 1Mbps | ROS Noetic |
| dji_rs2_ros_controller | Python | RS 2 | CAN | ROS |
| DJI-Ronin-RS2-Log-and-Replay | Python | RS 2/3 | CAN (python-can) | Standalone |
| Ronin-SC-Gimbal-Controller | C++/Python | Ronin SC | S-Bus | ROS/Arduino |
| dji_ronin | Python | Ronin (orig) | ADB screen hack | ROS |
| dji-firmware-tools | Python | Various | DUML serial | Standalone |
| dji_protocol | Python | Various | DUML/UDP/USB | Standalone |

---

### Search: "DJI Onboard SDK deprecated replaced MSDK v5 Payload SDK gimbal control XPort drone gimbal communication"

**OSDK status:** Gimbal::setAngle and Gimbal::setSpeed are deprecated; replaced by CameraManager APIs. MSDK V4.16 is last V4 version; V5 is current (released March 2022, not backward-compatible with V4). OSDK appears to be in maintenance/deprecated status. ([DJI OSDK Release Notes][34])

**Payload SDK gimbal (XPort/SkyPort):** ([DJI X-Port Overview][36], [DJI Payload SDK v2][35])
- X-Port is a standard gimbal with built-in SkyPort, communication APIs, and gimbal debugging interface
- Provides power, communication links, GPS, flight attitude data
- Gimbal control: absolute angle control (rotate to position in specified time) and speed control (0.5s per command)
- Controllable via MSDK, OSDK, and DJI Pilot
- Angle control limited by max rotation speed, max acceleration, and gimbal limit angles
- Combined with PSDK, developers can build custom gimbaled payloads for enterprise drones

---

<!-- Continue appending search entries below. One entry per search, every time. -->
