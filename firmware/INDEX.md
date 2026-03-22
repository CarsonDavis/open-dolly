# Firmware Index

ESP32-S3 firmware implementing the [Board API](../docs/project/board-api.md). See [README.md](README.md) for build instructions, hardware setup, and project overview.

## Reading Guide

| Task | Read First |
|------|------------|
| Understanding the architecture | `src/main.cpp`, `src/config.h` |
| State machine and transitions | `src/state_machine.h`, `src/state_machine.cpp` |
| Trajectory upload and playback | `src/trajectory.h`, `src/trajectory.cpp`, walker in `src/main.cpp` |
| DJI gimbal protocol | `lib/dji_can/dji_gimbal.h`, `lib/dji_can/dji_protocol.h` |
| REST API and WebSocket | `src/web_server.cpp` |
| Adding a new axis type | `src/config.h` (axis profiles) |
| Running tests | `test/` directory, `platformio.ini` native env |

## Source Files

| Path | Purpose | Key Details |
|------|---------|-------------|
| `platformio.ini` | PlatformIO build config | 4 environments: esp32s3 (default 4-axis), slider_only, slider_servo, native (host tests); OPI flash/PSRAM mode (`opi_opi`); GitHub URLs for ESPAsyncWebServer/AsyncTCP |
| `partitions.csv` | Flash partition table | 3MB×2 app slots, 1.875MB LittleFS (partition named `spiffs`), 20KB NVS |
| `src/main.cpp` | Entry point, FreeRTOS tasks, trajectory walker | 6 tasks across 2 cores; walker at 100Hz hw timer; DJI rate-limited to 10Hz; WiFi AP with `WIFI_PS_NONE` (no modem sleep); WiFi event logging; guarded by `#ifndef NATIVE_TEST` |
| `src/config.h` | Pin definitions, axis config, build profiles | 3 profiles: slider-only (1 axis), slider-servo (3), slider-dji (4, default); AxisType enum; AxisConfig struct |
| `src/config.cpp` | Runtime axis config globals | Defines `g_axis_config` and `g_axis_count` from compile-time constants |
| `src/commands.h` | Inter-task command/telemetry types | Command union with play/jog/move_to/home/scrub variants; TelemetryPoint struct |
| `src/state_machine.h/.cpp` | State enum, transition table, changeState | 6 states, 6×6 bool transition table, mutex-protected SystemState global |
| `src/trajectory.h/.cpp` | Trajectory storage, JSON parsing, interpolation | Points in PSRAM via ps_malloc; ArduinoJson parser; linear interp with binary search for scrub |
| `src/stepper.h/.cpp` | FastAccelStepper wrapper, homing, limits | 80 steps/mm (GT2 20T, 1/16 microstep); 3-phase homing; limit switch ISR |
| `src/display.h/.cpp` | Abstract display interface + implementations | OledDisplay (SH1107 128×128 I2C) and NullDisplay; auto-detect via I2C scan at 0x3C |
| `src/telemetry.h/.cpp` | WebSocket telemetry broadcaster task | Reads from telemetry_queue; sends `{"evt":"position", "slide":..., "pan":...}` per Board API spec; broadcasts to all WS clients |
| `src/web_server.h/.cpp` | REST endpoints + WebSocket handler | All Board API endpoints; CORS; connectivity probe handlers (Android/Apple/Windows); command parsing → cmd_queue; settings in-memory (TODO: NVS) |

## DJI CAN Library (`lib/dji_can/`)

Portable C++ library — no ESP32 dependencies. Testable on host.

| Path | Purpose | Key Details |
|------|---------|-------------|
| `dji_types.h` | Enums, structs, constants | GimbalState, CommandResult, CameraAction, Attitude, GimbalTelemetry, callback types, CAN IDs (TX=0x223, RX=0x222), timing constants |
| `can_bus_interface.h` | Abstract CAN bus interface | Pure virtual send/receive; ESP32 TWAI implements in main.cpp; tests use MockCanBus |
| `dji_crc.h/.cpp` | CRC16/CRC32 implementations | Table-driven; init 0x3AA3/0x00003AA3; validated against SDK test vectors |
| `dji_protocol.h/.cpp` | Frame build/parse/validate | buildFrame, validateFrame, parseFrame, extractFrameLength, SequenceCounter (wraps 0x0002→0xFFFD) |
| `dji_gimbal.h/.cpp` | High-level gimbal controller | PIMPL pattern; `update(now_ms)` for platform-independent timing; Phase 1 (position, camera), Phase 2 (speed with 400ms auto-refresh), Phase 3 (limits, stiffness); CAN frame reassembly; retry logic (5 retries, 2s interval); position command rate limiting (100ms min interval per DJI SDK spec) |

## Tests (`test/`)

All tests use Unity framework, run on host via PlatformIO `native` environment.

| Path | Purpose | Key Details |
|------|---------|-------------|
| `mocks/platform_mock.h` | ESP32/Arduino/FreeRTOS stubs | ps_malloc→malloc, mutex no-ops, Serial stub, GPIO stubs |
| `test_state_machine/` | State transition validation | All valid/invalid transitions, changeState with error codes |
| `test_trajectory/` | Trajectory parsing + interpolation | JSON parse, validation failures (timestamps, range), linear interpolation |
| `test_message_parsing/` | WebSocket JSON → Command struct | All command types, partial jog, error cases |
| `test_dji_crc/` | CRC16/CRC32 against SDK test vectors | Position header CRC16=0x42A2, full frame CRC32=0xBE97407B |
| `test_dji_protocol/` | Frame build/parse round-trip | Enable push, position command, CRC corruption, field extraction |
| `test_dji_gimbal/` | DJI state machine with MockCanBus | 48 tests: CRC vectors, frame build/validate/parse, sequence wrapping, state machine (connect, timeout, retry, reconnect, max retries), all command payloads, rate limiting, CAN segmentation, reassembly edge cases |
