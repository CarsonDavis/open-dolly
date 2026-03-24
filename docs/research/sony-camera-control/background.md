# Sony Alpha Camera Programmatic Control - Research Background

**Date:** 2026-03-23
**Topic:** Comprehensive survey of open-source projects and official SDKs for programmatically controlling Sony Alpha cameras (A7IV, A7III, A7RV, etc.) via WiFi, USB, and Bluetooth. Focus on liveview, autofocus, exposure control, and shutter triggering for integration with an AI object tracking gimbal system.

## Sources

[1]: https://support.d-imaging.sony.co.jp/app/sdk/en/index.html "Sony Camera Remote SDK Official Page"
[2]: https://www.scribd.com/document/846150304/Camera-Remote-SDK-API-Reference-v1-12-00 "Camera Remote SDK API Reference v1.12"
[3]: https://github.com/Bloodevil/sony_camera_api "Bloodevil/sony_camera_api (pysony)"
[4]: https://github.com/storborg/sonypy "storborg/sonypy"
[5]: https://github.com/kota65535/sony_camera_remote_api "kota65535/sony_camera_remote_api (Ruby)"
[6]: https://github.com/kazyx/kz-remote-api "kazyx/kz-remote-api (C#)"
[7]: https://github.com/gphoto/libgphoto2 "gphoto/libgphoto2"
[8]: https://github.com/gphoto/libgphoto2/issues/985 "libgphoto2 issue #985 - Manual focus not implemented for Sony A7IV"
[9]: https://github.com/gphoto/libgphoto2/issues/981 "libgphoto2 issue #981 - Sony A7 II+ USB preview issues"
[10]: https://github.com/ma1co/Sony-PMCA-RE "ma1co/Sony-PMCA-RE"
[11]: https://github.com/jakkuh/sony-alpha-python "jakkuh/sony-alpha-python"
[12]: https://github.com/petabite/libsonyapi "petabite/libsonyapi"
[13]: https://github.com/Staacks/alpharemote "Staacks/alpharemote (Bluetooth BLE remote)"
[14]: https://github.com/frank26080115/alpha-fairy "frank26080115/alpha-fairy"
[15]: https://github.com/coral/freemote "coral/freemote (BLE remote on NRF52840)"
[16]: https://github.com/pixeltris/SonyAlphaUSB "pixeltris/SonyAlphaUSB"
[17]: https://github.com/timelapseplus/node-sony-camera "timelapseplus/node-sony-camera"
[18]: https://github.com/topics/sony-alpha-cameras "GitHub topic: sony-alpha-cameras"
[19]: https://github.com/Sherlock-Photography/QuantumMirror "Sherlock-Photography/QuantumMirror"
[20]: https://www.sony.eu/presscentre/sony-enables-remote-control-of-a-wide-camera-range-through-iso-ptp-protocol-through-proprietary-extension "Sony Camera Remote Command press release"
[21]: https://support.d-imaging.sony.co.jp/app/cameraremotecommand/en/index.html "Camera Remote Command page"
[22]: https://alphauniverse.com/stories/how-to-livestream-with-your-sony-camera/ "Sony Alpha Universe - livestreaming"
[23]: https://github.com/xremix/Camera-Remote-JS "xremix/Camera-Remote-JS"
[24]: https://github.com/abdur75648/AI-Camera "abdur75648/AI-Camera"
[25]: https://github.com/micolous/gst-plugins-sonyalpha "micolous/gst-plugins-sonyalpha"
[26]: https://github.com/maxboels/TrackingPanTiltCam "maxboels/TrackingPanTiltCam"
[27]: https://github.com/rlew631/ObjectTrackingDrone "rlew631/ObjectTrackingDrone"
[28]: https://repair.dji.com/help/content?customId=01700007811 "DJI Ronin autofocus control with cameras"
[29]: https://www.4kshooters.net/2019/06/02/how-to-set-up-sony-a7iii-for-dji-ronin-s-focus-pulling/ "Sony A7III + DJI Ronin-S focus pulling"
[30]: https://github.com/frank26080115/alpha-fairy/blob/main/doc/Camera-Reverse-Engineering.md "Alpha-Fairy Camera Reverse Engineering doc"
[31]: https://github.com/erik-smit/sony-camera-api "erik-smit/sony-camera-api (fork with liveView.py)"
[32]: https://github.com/dzwiedziu-nkg/esp32-a7iv-rc "dzwiedziu-nkg/esp32-a7iv-rc"
[33]: https://github.com/gkoh/furble "gkoh/furble"
[34]: https://gregleeds.com/reverse-engineering-sony-camera-bluetooth/ "Greg Leeds - Reverse Engineering Sony Camera Bluetooth"
[35]: https://github.com/Parrot-Developers/sequoia-ptpy "Parrot-Developers/sequoia-ptpy"

<!-- Add new sources here as you find them. Number sequentially. -->

## Research Log

---

### Search: "Sony Camera Remote SDK official documentation developer"

**Sony's official Camera Remote SDK** is the primary official tool for programmatic camera control ([Sony Camera Remote SDK Official Page][1]).

- **Current version:** 2.01.00 (released February 3, 2026) ([Sony Camera Remote SDK Official Page][1])
- **Cost:** Free of charge, developers may commercialize applications built with it ([Sony Camera Remote SDK Official Page][1])
- **Supported cameras (Alpha series relevant to us):** ILCE-7M5, ILCE-7M4 (A7IV), ILCE-7RM5, ILCE-7RM4A, ILCE-7RM4, ILCE-7CR, ILCE-7SM3, ILCE-7CM2, ILCE-7C, ILCE-9M3, ILCE-9M2, ILCE-1M2, ILCE-1, ILCE-6700, plus cinema/PTZ models ([Sony Camera Remote SDK Official Page][1])
- **Notable: A7III (ILCE-7M3) is NOT listed** -- only A7IV (ILCE-7M4) and newer ([Sony Camera Remote SDK Official Page][1])
- **Connection methods:** USB, Wired LAN (Ethernet), Wireless LAN (Wi-Fi) -- availability varies by model ([Sony Camera Remote SDK Official Page][1])
- **OS support:** macOS 14.1+, Windows 11, Linux 64-bit (ARMv8, x86), Linux 32-bit (ARMv7) -- **Linux ARM support is very promising for embedded use** ([Sony Camera Remote SDK Official Page][1])
- **API capabilities:** Shutter release, live monitoring with OSD emulation, focus position settings, zoom speed control, preset focus/zoom, ISO/exposure settings, interval shooting, AF tracking sensitivity, content/file transfer, event notifications (focus completion, recording start/stop), firmware updates ([Sony Camera Remote SDK Official Page][1])
- **SDK is C/C++ based** (provides precompiled libraries) -- requires registration/download per region ([Sony Camera Remote SDK Official Page][1])

---

### Search: "sony camera remote API" github open source control

Found many projects implementing the **old Sony Camera Remote API** (JSON-RPC over WiFi, accessed via SSDP discovery). This is a **deprecated API**.

- **Bloodevil/sony_camera_api (pysony):** 280 stars, Python, MIT. Liveview, capture, video, timer. WiFi only. ([pysony][3])
- **storborg/sonypy:** 61 stars, Python, MIT. Discovery + basic capture. ([sonypy][4])
- **kota65535/sony_camera_remote_api:** Ruby gem. ([Ruby gem][5])
- **kazyx/kz-remote-api:** C# wrapper, Camera Remote API beta v2.3.0. ([kz-remote-api][6])

**Key insight:** All use the old WiFi-based JSON-RPC API. Replaced by Camera Remote SDK for newer models.

---

### Search: libgphoto2 sony alpha a7iv a7rv USB PTP camera control

**libgphoto2** Sony support is partial and buggy. ([libgphoto2][7])

- A7IV: Capture/download/ISO/WB/aperture/shutter work. **Manual focus broken**. No liveview. ([libgphoto2 issue #985][8])
- Older A7 II+: USB preview causes camera freezes/reboots. Race condition in PTP events. ([libgphoto2 issue #981][9])
- **Not viable** for real-time tracking.

---

### Search: pmca-re OpenMemories sony camera reverse engineering github

- **Sony-PMCA-RE** (2.6k stars): Firmware hacking tool, not camera control. Dormant since 2022. ([Sony-PMCA-RE][10])
- **jakkuh/sony-alpha-python** (6 stars, Python): PTP 3 over IP with SSH auth. FX30/A6700. Incomplete. ([sony-alpha-python][11])
- **petabite/libsonyapi** (76 stars, Python): Old API wrapper. Full feature set. Dormant since 2020. ([libsonyapi][12])

---

### Search: "sony camera remote SDK github wrapper python open source ILCE"

- **Staacks/alpharemote** (120 stars, Kotlin, GPLv3, Aug 2025): BLE remote. Shutter/AF/focus buttons. A7III, A7IV, A7RIV, A6700. ([alpharemote][13])
- **frank26080115/alpha-fairy** (197 stars, C, May 2023): ESP32 WiFi remote. PTP + HTTP. Focus stacking. ([alpha-fairy][14])
- **coral/freemote** (160 stars, C++, Dec 2025): BLE on NRF52840. Focus/zoom/shutter/record. ([freemote][15])
- **pixeltris/SonyAlphaUSB** (57 stars, C#, archived): USB PTP for A7III on Windows. ([SonyAlphaUSB][16])
- Other: QuantumMirror (39 stars), FocusBracket (81 stars), BetterManual (43 stars) ([sony-alpha-cameras topic][18])

---

### Search: Deep-dive on SonyAlphaUSB, alpha-fairy, and freemote

**alpha-fairy** uses TWO protocols: PTP over WiFi (reverse-engineered from Imaging Edge Remote via Wireshark) and HTTP (old Camera Remote API). Features focus stacking, focus pull with knob, 9-point focus. ([alpha-fairy][14])

**freemote** BLE details: Sony GATT service `8000FF00-FF00-FFFF-FFFF-FFFFFFFFFFFF`. Controls: shutter half/full press, record, AF-ON, C1, zoom (variable steps), **manual focus** (in/out, variable steps). Camera sends: focus states, shutter readiness. ([freemote][15])

**SonyAlphaUSB** uses Windows WIA API over PTP. Maintains Sony opcode reference in `Ids.cs`. Archived March 2026. ([SonyAlphaUSB][16])

---

### Search: Camera Remote Command protocol, AI tracking, and UVC streaming

**Camera Remote Command** enables remote control using Sony's proprietary PTP extension. 300+ commands. USB/LAN/WiFi. **Corporate customers only.** ([Camera Remote Command page][21])

**UVC Streaming:** A7IV, A7RV, A7SM3, A7C, ZV-E10 support USB UVC/UAC -- standard webcam. A7III does NOT support UVC. UVC and PTP cannot coexist on same USB. ([Sony Alpha Universe livestreaming][22])

---

### Search: SDK liveview, AI tracking projects, DJI gimbal + Sony integration

**SDK Liveview:** Motion JPEG stream via `get_live_view()`. Resolution/framerate not public. OSD emulation supported. ([Sony Camera Remote SDK Official Page][1])

**No existing projects combine AI tracking with Sony camera control.** All tracking projects just move the camera. ([TrackingPanTiltCam][26], [ObjectTrackingDrone][27])

**DJI + Sony:** Ronin gimbals trigger Sony AF via USB-C cable. RS3 Pro adds LiDAR AF. ([DJI autofocus support][28])

---

### Search: Alpha-fairy reverse engineering documentation

PTP protocol details from Alpha-Fairy's Camera Reverse Engineering doc ([Camera RE doc][30]):
- Sony uses PTP (ISO 15740) + TCP/IP transport (CIPA DC-X005) + proprietary extensions
- Property codes: `0xD2??` pattern. `0xD25D` = zoom percentage.
- Focus steps: limited to **1, 3, or 7 only**
- Focus point: X/Y packed as 16-bit integers
- Pairing: irreversible without factory reset -- use "Connect Without Pairing"
- Some cameras force "save to PC + Camera" mode = 25-120 MB image data per capture
- HTTP JSON-RPC API **lacks manual focus** -- PTP required
- Key PTP-IP opcodes: GetEventData = `0x9116`, GetLiveViewImage = `0x9153`, SetPropertyValue = `0x9110`

HDMI capture: ~150ms latency, 1080p/30fps typical, requires USB capture card for OpenCV.

---

### Search: ESP32/Arduino BLE projects + BLE protocol documentation

**dzwiedziu-nkg/esp32-a7iv-rc** (12 stars, C++, Feb 2024): ESP32 BLE remote for Sony A7IV. Built on freemote. PlatformIO project. Proves **ESP32 can talk BLE to A7IV.** ([esp32-a7iv-rc][32])

**gkoh/furble** (155 stars, C++, active): Multi-brand BLE camera remote on ESP32. Sony ZV-1F confirmed; "most modern Sony cameras should be supported." Shutter, focus, GPS tagging, intervalometer. ([furble][33])

**Sony BLE Protocol Reference** ([Greg Leeds BLE RE][34]):
- Service UUID: `8000-ff00-ff00-ffffffffffffffffffff`
- Writable characteristic: `0xff01`
- Command table:
  - Focus down/up: `0x0107` / `0x0106`
  - Shutter down/up: `0x0109` / `0x0108`
  - AutoFocus down/up: `0x0115` / `0x0114`
  - Zoom in: `0x026d20` / `0x026c00`, Zoom out: `0x026b20` / `0x026a00`
  - C1 button: `0x0121` / `0x0120`
  - Record toggle: `0x010e`
  - Focus in: `0x024720` / `0x024600`, Focus out: `0x024520` / `0x024400`
- Capture sequence: Focus Down -> Shutter Down -> Shutter Up -> Focus Up
- Pairing required; unpaired devices get disconnected
- Tested on A6100; uniform across modern Sony cameras

**sequoia-ptpy** ([sequoia-ptpy][35]):
- Python PTP library. Sony vendor ID 0x00000011 listed but **not actually implemented** -- timeouts with Sony cameras.
- Useful only as generic PTP reference.

---

<!-- Continue appending search entries below. One entry per search, every time. -->
