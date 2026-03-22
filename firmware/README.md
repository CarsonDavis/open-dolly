# Firmware

ESP32-S3 firmware for the camera slider controller board. Implements the [Board API](../docs/project/board-api.md) — serves the web UI over Wi-Fi, accepts trajectory uploads, drives a stepper motor and DJI gimbal, and reports telemetry.

## Architecture

```
  Core 0 (network)                     Core 1 (motion)

  WiFi AP + mDNS                       Trajectory Walker (100 Hz)
        |                                      |
  Web Server (HTTP + WS)              Motion Control (stepper + DJI CAN)
        |  cmd_queue  ──────────────►          |
        |  ◄──────────────  telemetry_queue    |
  Telemetry Broadcaster                DJI CAN (TWAI driver)

  Display Update (5 Hz)
```

Six FreeRTOS tasks across two cores. Core 0 handles networking — HTTP, WebSocket, telemetry broadcast, and display updates. Core 1 handles motion — trajectory playback at 100 Hz, stepper motor control, and DJI gimbal communication over CAN bus.

Inter-task communication uses two FreeRTOS queues: `cmd_queue` (commands from the web server to motion control) and `telemetry_queue` (position updates from motion control to WebSocket broadcast). Shared system state is protected by a mutex.

## Hardware

| Component | Details |
|-----------|---------|
| MCU | ESP32-S3-DevKitC-1 v1.1 (dual-core, 16MB PSRAM, 32MB OPI flash) |
| Stepper driver | TMC2209 (UART config, StealthChop) |
| CAN transceiver | TJA1051 or SN65HVD230 (for DJI gimbal communication) |
| Display | SH1107 128x128 OLED (I2C, optional) |
| Limit switches | Active LOW with internal pullup |

### Pin Assignments

| Function | GPIO |
|----------|------|
| Step | 15 |
| Dir | 16 |
| Enable | 17 |
| TMC UART | 18 (single-wire TX/RX) |
| CAN TX | 5 |
| CAN RX | 6 |
| Limit min | 7 |
| Limit max | 8 |
| I2C SDA | 1 |
| I2C SCL | 2 |
| Battery ADC | 4 |
| Status LED | 48 (onboard RGB) |

## Build Profiles

| Environment | Axes | Use case |
|-------------|------|----------|
| `esp32s3` (default) | slide, pan, tilt, roll | Slider + DJI RS gimbal |
| `slider_only` | slide | Slider rail only, no gimbal |
| `slider_servo` | slide, pan, tilt | Slider + servo-based pan/tilt head |
| `native` | (test) | Host-side unit tests, no hardware |

## Prerequisites

- [PlatformIO](https://platformio.org/) (CLI or VS Code extension)
- Micro-USB cable to ESP32-S3-DevKitC-1 (use the **UART** port, not the USB port)

## Build and Flash

From the project root:

```bash
# Build and flash everything (firmware + web UI)
npm run deploy

# Flash only the firmware (no web UI rebuild)
npm run deploy:firmware

# Rebuild and flash only the web UI
npm run deploy:ui

# Monitor serial output
uv run --with pyserial python3 -c "
import serial, time
ser = serial.Serial('/dev/cu.usbserial-110', 115200, timeout=1)
ser.reset_input_buffer(); ser.dtr = False; time.sleep(0.1); ser.dtr = True
time.sleep(4); print(ser.read(4000).decode('utf-8', errors='replace')); ser.close()
"
```

**Note:** `pio device monitor` does not work in non-interactive terminals. Use the pyserial snippet above or run `! pio device monitor` from an interactive shell.

### Board details

The ESP32-S3-DevKitC-1 v1.1 has **octal flash (OPI)** and **16MB embedded PSRAM**. The `platformio.ini` is configured with `board_build.arduino.memory_type = opi_opi` to match. Using the wrong flash mode will cause a boot loop.

## Running Tests

Tests run on the host machine — no ESP32 hardware needed.

```bash
# Run all test suites via PlatformIO
pio test -e native

# Or build and run the DJI CAN tests directly with make
cd test/test_dji_gimbal
make test
```

### Test Suites

| Suite | What it tests |
|-------|---------------|
| `test_state_machine` | All valid/invalid state transitions, error codes |
| `test_trajectory` | JSON parsing, validation (timestamps, axis ranges), linear interpolation |
| `test_message_parsing` | WebSocket JSON command parsing for all command types |
| `test_dji_crc` | CRC16/CRC32 against DJI SDK test vectors |
| `test_dji_protocol` | Frame build/parse round-trips, CRC corruption detection |
| `test_dji_gimbal` | Full gimbal state machine: handshake, telemetry parsing, timeout/retry/reconnect, all command payloads, position rate limiting, CAN frame segmentation, reassembly |

## Flash Partition Layout

8MB flash, dual OTA slots:

| Partition | Size | Purpose |
|-----------|------|---------|
| nvs | 20 KB | Settings (device name, Wi-Fi, telemetry rate) |
| app0 | 3 MB | Firmware slot A |
| app1 | 3 MB | Firmware slot B (OTA updates) |
| littlefs | 1.875 MB | Web UI static files |
| coredump | 64 KB | Crash dumps |

## DJI CAN Library

The DJI gimbal protocol is implemented as a standalone library in `lib/dji_can/` with no ESP32 dependencies. It communicates with DJI RS-series gimbals over CAN bus using the DJI R SDK protocol.

The library handles:
- CRC16/CRC32 computation (table-driven, ported from DJI SDK reference)
- Frame building, validation, and parsing (SOF, Ver/Length, CmdType, SEQ, CRC16 header, CmdSet/CmdID, payload, CRC32)
- CAN frame segmentation (splitting SDK packets into 8-byte CAN frames) and reassembly
- Connection state machine (DISCONNECTED -> CONNECTING -> ACTIVE -> ERROR) with automatic retry
- Position control, speed control (with 400ms auto-refresh), camera trigger, angle limits, motor stiffness
- Position command rate limiting (100ms minimum interval per DJI SDK spec)
- Telemetry parsing (attitude angles, joint angles, limits, stiffness at ~50 Hz)

The abstract `CanBusInterface` is implemented by the ESP32 TWAI driver in `src/main.cpp`. Tests use a `MockCanBus` that simulates the gimbal's CAN responses.

See [plan-dji-can.md](../docs/project/plans/plan-dji-can.md) for the full protocol specification and hardware integration guide.

## Project Structure

```
firmware/
  platformio.ini          Build config (4 environments)
  partitions.csv          Flash layout
  src/
    main.cpp              Entry point, FreeRTOS tasks, trajectory walker
    config.h/.cpp         Pin definitions, axis profiles
    commands.h            Inter-task command/telemetry types
    state_machine.h/.cpp  System state management
    trajectory.h/.cpp     Trajectory upload, storage, interpolation
    stepper.h/.cpp        Stepper motor control, homing
    display.h/.cpp        OLED display (optional)
    telemetry.h/.cpp      WebSocket telemetry broadcast
    web_server.h/.cpp     REST API + WebSocket handler
  lib/
    dji_can/              DJI RS gimbal CAN protocol (portable, no ESP32 deps)
  data/                   Web UI files (LittleFS, built from ../web/)
  test/
    mocks/                ESP32/Arduino stubs for host testing
    test_state_machine/   State transition tests
    test_trajectory/      Trajectory parse + interpolation tests
    test_message_parsing/ WebSocket command parsing tests
    test_dji_crc/         CRC test vectors
    test_dji_protocol/    Frame build/parse tests
    test_dji_gimbal/      Full gimbal integration tests (48 tests)
```

## Related Docs

- [Board API spec](../docs/project/board-api.md) — the REST + WebSocket contract this firmware implements
- [Architecture](../docs/project/architecture.md) — system layers and design decisions
- [Firmware plan](../docs/project/plans/plan-firmware.md) — detailed implementation plan
- [DJI CAN plan](../docs/project/plans/plan-dji-can.md) — DJI protocol specification and integration strategy
- [BOM](../docs/project/bom.md) — bill of materials and wiring
