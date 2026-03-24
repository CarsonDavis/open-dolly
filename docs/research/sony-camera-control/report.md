# Sony Alpha Camera Programmatic Control: Research Report

**Date:** 2026-03-23
**Purpose:** Survey of all known methods for programmatically controlling Sony Alpha mirrorless cameras (A7IV, A7III, A7RV, etc.), with a focus on building an AI object tracking system that combines camera control with gimbal movement.

## 1. Executive Summary

There are five distinct protocols for communicating with Sony Alpha cameras: the official Camera Remote SDK (C/C++ over USB/WiFi/Ethernet), the Camera Remote Command (raw PTP opcodes), the deprecated WiFi JSON-RPC API, USB PTP via libgphoto2, and Bluetooth BLE. Each has different capabilities and limitations.

**The recommended architecture for our AI tracking gimbal project is:**

1. **Video feed:** HDMI clean output via capture card (most reliable, ~150ms latency, 1080p/30fps) -- or USB UVC on cameras that support it (A7IV, A7RV, but not A7III)
2. **Camera control:** Sony Camera Remote SDK over WiFi or USB for focus, exposure, and shutter control -- this is the most capable and officially supported path
3. **Supplementary control:** BLE via the freemote protocol for manual focus nudges (works on A7III and newer, provides focus in/out with variable step sizes)

The key constraint is that **video capture and camera control likely need separate physical interfaces** -- you cannot run UVC streaming and PTP control simultaneously on the same USB-C port. HDMI for video + WiFi for control is the most practical combination.

The **A7III is significantly more limited** than the A7IV and newer cameras: no official SDK support, no UVC streaming, and broken libgphoto2 liveview. If the project targets A7III, the old WiFi JSON-RPC API or BLE are the only viable control paths.

## 2. Protocols Overview

There are five known protocols for communicating with Sony Alpha cameras:

| Protocol | Interface | Camera Support | Capabilities | Status |
|----------|-----------|---------------|-------------|--------|
| **Camera Remote SDK** | USB, WiFi, Ethernet | A7IV, A7RV, A7SM3, A7C, A1, A9II+ | Full: liveview, focus, exposure, shutter, AF tracking, events | Active (v2.01, Feb 2026) |
| **Camera Remote Command** | USB PTP, PTP-IP | Same as SDK | Full: raw PTP opcodes, 100+ command types | Active (v2024.0.0) |
| **WiFi JSON-RPC API** | WiFi (HTTP) | A7III, A7II, A6000-era cameras | Liveview, capture, basic focus/exposure | Deprecated (still works on older cameras) |
| **USB PTP (libgphoto2)** | USB | A7IV (partial), A7III (partial) | Capture, some settings; no reliable liveview or focus | Broken for real-time use |
| **Bluetooth BLE** | BLE | A7III, A7IV, A7RIV, A1, A6400+ | Shutter, AF-ON, manual focus in/out, zoom, record | Active (reverse-engineered) |

### Protocol Detail: Camera Remote SDK / Camera Remote Command

Sony provides two complementary products:
- **Camera Remote SDK:** A high-level C/C++ library that abstracts the PTP protocol. Provides `CameraDevice::get_live_view()`, property getters/setters, event callbacks. Runs on Windows, macOS, and Linux (including ARM). Free to use, commercial applications allowed.
- **Camera Remote Command:** A low-level PTP command reference documenting the actual opcodes. Enables building custom implementations without the SDK library. Added PTP-IP support in 2024.

Both use Sony's proprietary extension of ISO PTP (Picture Transfer Protocol). Key opcodes discovered via reverse engineering:
- `0x9110` -- SetPropertyValue
- `0x9116` -- GetEventData
- `0x9153` -- GetLiveViewImage
- Property codes follow `0xD2??` pattern (e.g., `0xD25D` = zoom percentage)

### Protocol Detail: WiFi JSON-RPC API (Deprecated)

The old API worked via:
1. Camera creates WiFi access point
2. Client discovers camera via UPnP SSDP
3. Commands sent as HTTP POST with JSON-RPC body to port 8080
4. Liveview streamed as framed JPEG data from a dedicated endpoint

Commands included `actTakePicture`, `startLiveview`, `setShootMode`, `actHalfPressShutter`, `setExposureCompensation`, `setFNumber`, `setIsoSpeedRate`, `setShutterSpeed`, `actTrackingFocus`, etc.

### Protocol Detail: Bluetooth BLE

Sony cameras expose a custom GATT service (`8000FF00-FF00-FFFF-FFFF-FFFFFFFFFFFF`) with two characteristics for bidirectional communication. Commands are 2-3 byte sequences. The protocol supports:
- Shutter: half-press, full-press (must follow specific sequence)
- AF-ON button
- Manual focus: in/out with variable step sizes (0x00-0x8f)
- Zoom: telephoto/wide with variable steps
- Record start/stop
- Camera sends back: focus acquired/lost, shutter readiness, recording status

## 3. Sony Official SDKs and Documentation

### Camera Remote SDK (v2.01.00, Feb 2026)

- **Download:** https://support.d-imaging.sony.co.jp/app/sdk/en/index.html (requires registration) or https://support.d-imaging.sony.co.jp/app/sdk/licenseagreement_d/en.html (direct, no registration)
- **Cost:** Free, commercial use allowed
- **Language:** C/C++ library with sample code
- **OS:** Windows 11, macOS 14.1+, Linux x86_64, Linux ARMv8, Linux ARMv7
- **Connections:** USB, WiFi, Ethernet (varies by model)

**Supported Alpha cameras:**
ILCE-7M5, ILCE-7M4 (A7IV), ILCE-7RM5 (A7RV), ILCE-7RM4A, ILCE-7RM4, ILCE-7CR, ILCE-7SM3, ILCE-7CM2, ILCE-7C, ILCE-9M3, ILCE-9M2, ILCE-1M2, ILCE-1, ILCE-6700

**Not supported:** A7III (ILCE-7M3), A7RII, A7II, and older

**Key capabilities:**
- Shutter release (half-press, full-press)
- Live view monitoring (MJPEG stream) with OSD emulation
- Focus position settings (absolute and relative)
- Preset focus and zoom positions
- AF tracking sensitivity adjustment
- ISO, aperture, shutter speed, exposure compensation
- Interval shooting
- Event notifications (focus completion, recording state changes)
- Content/file transfer
- Firmware updates

### Camera Remote Command (v2024.0.0)

- **Download:** https://support.d-imaging.sony.co.jp/app/cameraremotecommand/en/index.html
- **Cost:** Free
- **Format:** Command reference documentation + sample code
- **Protocol:** Sony proprietary extension of ISO PTP
- **Connections:** USB PTP, PTP-IP (added in 2024)
- Over 100 command types including recording media settings, ISO AUTO min/max

This is the lower-level alternative to the SDK -- you implement the PTP communication yourself using the documented opcodes. More work, but more control and no dependency on Sony's binary library.

## 4. GitHub Repository Catalog

### Tier 1: Most Relevant to Our Project

| Repo | Protocol | Language | Features | Stars | Last Active | Use for Us |
|------|----------|----------|----------|-------|-------------|------------|
| [frank26080115/alpha-fairy](https://github.com/frank26080115/alpha-fairy) | PTP/WiFi + HTTP | C (ESP32) | Focus stacking, focus pull, shutter, 9-point focus, movie, IR fallback | 197 | May 2023 | **Best reference** for PTP-over-WiFi reverse engineering; has protocol docs |
| [coral/freemote](https://github.com/coral/freemote) | BLE | C++ (NRF52840) | Shutter, AF-ON, manual focus in/out, zoom, record | 160 | Dec 2025 | **BLE focus control** reference; MF step commands that USB PTP lacks |
| [Staacks/alpharemote](https://github.com/Staacks/alpharemote) | BLE | Kotlin (Android) | Shutter, half-press, AF-ON, focus adjust, trigger-on-focus | 120 | Aug 2025 | BLE protocol reference for Android; focus-on-trigger pattern |
| [jakkuh/sony-alpha-python](https://github.com/jakkuh/sony-alpha-python) | PTP 3 over IP (SSH) | Python | Zoom control (FX30, A6700) | 6 | Recent | Modern PTP-IP protocol; closest to what SDK uses internally |

### Tier 2: Historical / Reference Value

| Repo | Protocol | Language | Features | Stars | Last Active | Notes |
|------|----------|----------|----------|-------|-------------|-------|
| [Bloodevil/sony_camera_api](https://github.com/Bloodevil/sony_camera_api) | WiFi JSON-RPC | Python | Liveview, capture, video, settings | 280 | Nov 2022 | Most popular; deprecated API |
| [petabite/libsonyapi](https://github.com/petabite/libsonyapi) | WiFi JSON-RPC | Python | Touch AF, tracking focus, liveview, full exposure | 76 | Aug 2020 | Best feature coverage of old API |
| [pixeltris/SonyAlphaUSB](https://github.com/pixeltris/SonyAlphaUSB) | USB PTP (WIA) | C# | Shutter, video, full exposure, drive mode | 57 | Archived 2026 | Sony PTP opcode reference (`Ids.cs`) |
| [storborg/sonypy](https://github.com/storborg/sonypy) | WiFi JSON-RPC | Python | Discovery + capture | 61 | Old | Simplest example of old API |
| [kota65535/sony_camera_remote_api](https://github.com/kota65535/sony_camera_remote_api) | WiFi JSON-RPC | Ruby | Full API wrapper | -- | Old | Ruby alternative |
| [kazyx/kz-remote-api](https://github.com/kazyx/kz-remote-api) | WiFi JSON-RPC | C# | API v2.3.0 wrapper | -- | Old | .NET alternative |
| [timelapseplus/node-sony-camera](https://github.com/timelapseplus/node-sony-camera) | WiFi JSON-RPC | JavaScript | Camera control | 49 | Oct 2018 | Node.js alternative |
| [xremix/Camera-Remote-JS](https://github.com/xremix/Camera-Remote-JS) | WiFi JSON-RPC | JavaScript | WiFi control | 14 | May 2020 | Browser-based |
| [micolous/gst-plugins-sonyalpha](https://github.com/micolous/gst-plugins-sonyalpha) | WiFi JSON-RPC | C (GStreamer) | Liveview JPEG demuxer | -- | Old | GStreamer pipeline for liveview |

### Tier 3: Related / Niche

| Repo | What it Does | Stars | Notes |
|------|-------------|-------|-------|
| [ma1co/Sony-PMCA-RE](https://github.com/ma1co/Sony-PMCA-RE) | Firmware hacking, custom app install | 2,600 | Not useful for camera control; older cameras only |
| [Sherlock-Photography/QuantumMirror](https://github.com/Sherlock-Photography/QuantumMirror) | "AI-powered" Sony camera app | 39 | Runs on camera's Android subsystem |
| [obs1dium/FocusBracket](https://github.com/obs1dium/FocusBracket) | Focus bracketing | 81 | OpenMemories-based |
| [obs1dium/BetterManual](https://github.com/obs1dium/BetterManual) | Manual mode enhancement | 43 | Runs on camera |
| [falk0069/sony-pm-alt](https://github.com/falk0069/sony-pm-alt) | WiFi file transfer | -- | Alternative to PlayMemories |
| [gphoto/libgphoto2](https://github.com/gphoto/libgphoto2) | Generic camera control | -- | Sony support is broken for our needs |

### Tier 4: AI/CV Tracking Projects (Not Sony-Specific)

| Repo | What it Does | Notes |
|------|-------------|-------|
| [maxboels/TrackingPanTiltCam](https://github.com/maxboels/TrackingPanTiltCam) | YOLOv8 + Kalman + Arduino servo pan-tilt | No camera control, USB webcam only |
| [rlew631/ObjectTrackingDrone](https://github.com/rlew631/ObjectTrackingDrone) | Drone gimbal + RPi + OpenCV | Pixhawk-controlled, no camera control |
| [abdur75648/AI-Camera](https://github.com/abdur75648/AI-Camera) | YOLOv8 real-time detection/tracking | General purpose, not camera-specific |

## 5. Live View / Video Feed Approaches

There are four ways to get real-time video from a Sony Alpha camera into a computer for AI processing:

### Option A: HDMI Clean Output + Capture Card (Recommended)

- **How:** Camera HDMI out -> capture card (e.g., Elgato CamLink 4K) -> USB -> computer -> OpenCV
- **Resolution:** 1080p at 30fps (typical clean HDMI output)
- **Latency:** ~150ms end-to-end
- **Pros:** Most reliable path; works on all cameras with HDMI; leaves USB-C free for camera control; no special software needed; capture card appears as standard V4L2/DirectShow device
- **Cons:** Extra hardware (~$100 for capture card); HDMI cable adds bulk; 150ms may be too much for fast tracking
- **Camera support:** All Alpha cameras with micro-HDMI output

### Option B: USB UVC Streaming

- **How:** Camera USB-C -> computer -> OpenCV `VideoCapture`
- **Resolution:** Up to 1080p (camera-dependent)
- **Latency:** Lower than HDMI path (no capture card)
- **Pros:** No extra hardware; simple software integration; camera appears as webcam
- **Cons:** **Cannot simultaneously use USB for camera control (PTP mode vs UVC mode)**; not all cameras support it
- **Camera support:** A7IV, A7RV, A7SM3, A7C, ZV-E10, A7CM2 -- **NOT A7III**

### Option C: Camera Remote SDK Liveview (MJPEG)

- **How:** SDK's `get_live_view()` returns MJPEG frames over USB or WiFi
- **Resolution:** Unknown from public docs (likely lower than full sensor resolution -- common MJPEG liveview is 640x480 or 1024x680)
- **Latency:** Variable; depends on connection and processing
- **Pros:** Single interface for both video and control; includes optional OSD overlays (focus peaking, etc.)
- **Cons:** Resolution likely too low for good AI detection; MJPEG decode overhead; SDK is closed-source binary
- **Camera support:** Same as SDK (A7IV and newer)

### Option D: Old WiFi API Liveview

- **How:** JSON-RPC `startLiveview` command, then stream framed JPEGs from HTTP endpoint
- **Resolution:** Variable (camera-dependent, likely 640x480 typical)
- **Latency:** WiFi-dependent; variable framerate
- **Pros:** Works on older cameras (A7III); well-documented by community
- **Cons:** Low resolution; high latency; deprecated API; WiFi only
- **Camera support:** A7III and older cameras with WiFi Remote API

### Recommendation for Our Project

**HDMI capture card for video + WiFi/BLE for camera control** is the most robust architecture. This gives us:
- Full 1080p video for AI processing
- Independent camera control channel (not fighting for the USB port)
- Works with the widest range of cameras
- Predictable, well-understood latency

If the 150ms HDMI latency is acceptable (and it likely is for gimbal tracking where the gimbal itself adds response delay), this is the clear winner.

## 6. Focus and Exposure Control

### Autofocus Control

| Method | AF Trigger | Focus Position | MF In/Out | AF-ON | Touch AF | AF Tracking |
|--------|-----------|---------------|-----------|-------|----------|-------------|
| Camera Remote SDK | Yes | Yes (absolute) | Yes | Yes | Yes | Yes (sensitivity) |
| Camera Remote Command | Yes | Yes | Yes | Yes | Yes | Yes |
| WiFi JSON-RPC (old) | Yes | No | No | No | Yes (touch) | Yes (tracking) |
| BLE (freemote) | Yes (half-press) | No | **Yes (variable step)** | **Yes** | No | No |
| USB PTP (libgphoto2) | Yes | **No (broken)** | **No (broken)** | No | No | No |
| DJI Ronin USB-C | Yes (half-press) | No | No (motor-based) | No | No | No |

**Key finding:** The Camera Remote SDK is the only method that provides full focus control including absolute focus position and AF tracking sensitivity. BLE is the only reverse-engineered method that provides working manual focus stepping.

For our AI tracking project, the approach to focus is:
1. **Let the camera's built-in AF do most of the work.** Sony's Real-time Tracking AF (available on A7IV, A7RV, A1) is already AI-powered and excellent at subject tracking. Our system should configure and trigger the camera's native AF rather than trying to replicate it.
2. **Use SDK to set AF mode and tracking sensitivity.** Configure the camera for continuous AF with subject tracking enabled.
3. **Use SDK or BLE for focus nudges** if the camera's AF loses track.

### Exposure Control

| Method | ISO | Aperture | Shutter Speed | Exposure Comp | White Balance |
|--------|-----|----------|--------------|--------------|---------------|
| Camera Remote SDK | Yes | Yes | Yes | Yes | Yes |
| Camera Remote Command | Yes | Yes | Yes | Yes | Yes |
| WiFi JSON-RPC (old) | Yes | Yes | Yes | Yes | Yes |
| BLE (freemote) | No | No | No | No | No |
| USB PTP (libgphoto2) | Yes | Yes | Yes | Yes | Yes |

Exposure control is well-supported by all methods except BLE (which is button-level control only). The Camera Remote SDK provides the most comprehensive and reliable exposure control.

## 7. Key Findings and Recommendations for Our AI Tracking Gimbal Project

### Architecture Recommendation

```
+-------------------+     HDMI      +------------------+
|   Sony Alpha      |-------------->| HDMI Capture     |
|   Camera          |               | Card (USB)       |
|   (on DJI RS5)    |               +------------------+
|                   |                       |
|   WiFi/BLE        |               +------------------+
|   <-------------- |<--------------| Compute Unit     |
+-------------------+     WiFi      | (RPi 5 / Jetson) |
                                    |                  |
                                    | - OpenCV/YOLO    |
                                    | - Camera control |
                                    | - Gimbal control |
                                    +------------------+
                                           |
                                    +------------------+
                                    | DJI RS5 Gimbal   |
                                    | (via CAN/UART)   |
                                    +------------------+
```

### Interface Assignments

| Function | Interface | Protocol | Why |
|----------|-----------|----------|-----|
| Video feed for AI | HDMI + capture card | V4L2 / OpenCV | Reliable 1080p, leaves USB-C free |
| Camera settings + focus | WiFi | Camera Remote SDK | Full control, no cable needed |
| Quick focus nudges | BLE | Freemote protocol | Low-overhead MF steps, works alongside WiFi |
| Shutter trigger | WiFi | Camera Remote SDK | Most reliable |
| Gimbal control | CAN/UART | DJI R SDK | Existing slider firmware |

### Camera Selection Guidance

| Camera | SDK Support | UVC | BLE | WiFi API (old) | Recommendation |
|--------|------------|-----|-----|---------------|----------------|
| **A7IV (ILCE-7M4)** | Yes | Yes | Yes | Unknown | **Best choice** -- full SDK, all interfaces |
| **A7RV (ILCE-7RM5)** | Yes | Yes | Yes | Unknown | Excellent -- same as A7IV |
| **A7III (ILCE-7M3)** | **No** | **No** | Yes | Yes | Limited -- BLE + old WiFi API only |
| **A1 (ILCE-1)** | Yes | Yes | Yes | Unknown | Overkill but fully supported |
| **A7SM3** | Yes | Yes | Yes | Unknown | Good for video-focused use |

### Key Risks and Mitigations

1. **WiFi latency for focus commands:** The camera's built-in AF tracking is the primary solution; external commands are supplementary. Test actual round-trip latency with SDK.

2. **HDMI capture latency (~150ms):** For gimbal tracking, this is likely acceptable since the gimbal's mechanical response time adds similar delay. The AI model can predict subject motion to compensate.

3. **SDK is closed-source binary:** If the SDK has bugs or limitations, we cannot fix them. Camera Remote Command (raw PTP opcodes) is the fallback for building a custom implementation.

4. **Camera USB-C port conflict:** Cannot use USB for both UVC video and PTP control simultaneously. HDMI for video + WiFi for control solves this cleanly.

5. **BLE + WiFi simultaneous use:** Needs testing. Sony cameras may not support both radios active simultaneously. If they do, BLE for fast focus commands + WiFi for everything else is ideal.

### Recommended Development Sequence

1. **Download and evaluate the Camera Remote SDK.** Register at Sony's site, download for Linux ARM, build the sample code, verify liveview and focus control work with the target camera.

2. **Test HDMI capture pipeline.** Connect camera HDMI to capture card, verify OpenCV can read frames at 30fps, measure actual end-to-end latency.

3. **Prototype BLE focus control.** Port freemote protocol to ESP32 (or use NRF52840), test manual focus stepping speed and reliability.

4. **Integrate with gimbal.** Connect AI detection output to gimbal control. Start with simple center-of-frame tracking before adding predictive motion.

5. **Evaluate whether SDK liveview can replace HDMI.** If the SDK's MJPEG stream is sufficient resolution (720p+) and low enough latency, it simplifies the hardware by eliminating the capture card.

### Most Valuable Open-Source References

1. **alpha-fairy** -- The gold standard for reverse-engineering Sony PTP-over-WiFi. Read the Camera-Reverse-Engineering.md doc and the PTP handshake code for each camera model.

2. **freemote** -- The definitive BLE protocol reference. The GATT service/characteristic UUIDs, command byte sequences, and step sizes are all documented.

3. **SonyAlphaUSB** -- The `Ids.cs` file contains a comprehensive mapping of Sony PTP device property codes. Invaluable if building a custom PTP implementation.

4. **sony-alpha-python** -- The only project attempting the modern PTP 3 / PTP-IP protocol that the official SDK uses internally. Early stage but shows the connection flow.
