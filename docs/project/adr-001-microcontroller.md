# ADR-001: Microcontroller Selection

*Status: accepted*
*Date: 2026-03-20*

## Context

We need a control board that sits on the slider platform and:

1. Serves the web UI over Wi-Fi (HTTP server + WebSocket)
2. Talks to a DJI gimbal over CAN bus at 1 Mbps
3. Drives stepper motors with precise pulse timing
4. Stores and walks a pre-computed trajectory table (~60KB)
5. Runs on battery power for portable use
6. Is cheap, well-documented, and accessible to open-source contributors

## Decision

**Use the ESP32-S3 (dual-core Xtensa LX7, 240 MHz)** on a standard dev board (ESP32-S3-DevKitC or equivalent). Use the Arduino framework via PlatformIO for development.

## Why ESP32-S3

The ESP32 family is the only option that satisfies all six requirements in a single chip at low cost. Within the family, the S3 is the right variant.

### Why the S3 over the original ESP32

The S3 and original ESP32 share the same core capabilities (dual-core 240 MHz, TWAI, MCPWM+PCNT, Wi-Fi b/g/n) and the same library support (FastAccelStepper, ESPAsyncWebServer, Arduino framework). The S3 wins on:

- **Native USB-OTG** — no USB-UART bridge chip needed. Plug in a cable and flash. Simpler boards, cheaper custom PCB.
- **No JTAG/TWAI pin conflict** — on the original ESP32, CAN and JTAG share pins, so you can't debug while CAN is active. The S3 has no such limitation.
- **More GPIO** — 45 vs 34. Matters for a "bring your own hardware" project where users may add focus motors, extra steppers, limit switches, LEDs, etc.
- **Newer core** — Xtensa LX7 is a more efficient architecture than the LX6.
- **PSRAM standard** — most S3 boards ship with 2-8MB PSRAM, giving headroom for larger trajectories without seeking out specialty boards.

To be candid: for a v1 build, the S3's advantages are more about future-proofing than necessity. The JTAG/TWAI conflict only matters during debugging (you can test CAN and debug separately). USB-OTG is nice but every dev board has a USB-UART bridge. The extra GPIO won't matter until users actually add peripherals. PSRAM is available on ESP32-WROVER modules. The original ESP32 could absolutely work for v1.

We chose the S3 anyway because the cost difference is ~$1-2 and the S3 is clearly the better long-term platform. For an open-source project where the hardware choice is baked into documentation, examples, and contributor expectations, starting on the newer chip avoids a migration later.

### Wi-Fi + web serving

Built-in 802.11b/g/n. Supports AP mode out of the box — the board creates its own Wi-Fi network, no router needed. ESPAsyncWebServer provides HTTP + WebSocket in one library. mDNS is supported natively for `slider.local` discovery.

### CAN bus (DJI gimbal communication)

The TWAI peripheral is a native CAN 2.0 controller that supports the 1 Mbps baud rate required by the DJI R SDK. The only additional hardware needed is a CAN transceiver chip (TJA1051 or SN65HVD230, ~$1). No existing ESP32 DJI gimbal project exists — see [Risk #1](#risks) for what this means in practice.

### Stepper motor control

The FastAccelStepper library uses the ESP32-S3's MCPWM and PCNT hardware peripherals to generate step pulses entirely in hardware — up to 200,000 steps/sec with zero CPU involvement for timing. The CPU just feeds target positions and speeds to a hardware queue. Far more than adequate for camera slider speeds.

### Dual-core isolation

The dual-core architecture allows FreeRTOS task pinning: Wi-Fi and the web server on Core 0, motor control and CAN communication on Core 1. This isolation means a burst of WebSocket traffic cannot cause a missed step pulse, and vice versa.

Note: no existing project exercises all three subsystems (TWAI + FastAccelStepper + Wi-Fi) simultaneously on any ESP32 variant. The architecture is sound in theory — each subsystem uses hardware peripherals with minimal CPU involvement — but this specific combination is unproven. Early firmware work should validate this before building further.

### Memory

512 KB SRAM plus 2-8 MB PSRAM (standard on most S3 boards) comfortably holds trajectory tables, web server buffers, CAN buffers, and application state. 4-16 MB flash stores the web UI files and firmware. A 10,000-point trajectory at 20 bytes/point is 200KB — fits easily with PSRAM.

### Power

~130 mA with Wi-Fi active. A 2500 mAh LiPo gives ~19 hours of continuous use. Modem-sleep drops to <1 mA when idle. This is 20x less than a Raspberry Pi Zero 2 W.

### Community and tooling

Largest microcontroller community outside of classic Arduino. PlatformIO provides a modern build system with dependency management and cross-platform support. OTA firmware updates work via ArduinoOTA. The barrier to entry for contributors is as low as it gets — `pio run` and you're building.

## Alternatives Considered

### ESP32 (original)

Same dual-core 240 MHz, TWAI, MCPWM+PCNT, Wi-Fi. Slightly more mature ecosystem and $1-2 cheaper. But: JTAG and TWAI share pins (can't debug while CAN is active), fewer GPIO (34 vs 45), no native USB (needs USB-UART bridge), and most boards lack PSRAM. Viable fallback if S3 boards are unavailable.

### ESP32-C6

Two TWAI controllers (useful for CAN debugging), Wi-Fi 6, lowest deep-sleep power (~5 uA). The C6 has a 160 MHz HP core and a 20 MHz LP core — not truly dual-core in the ESP32-S3 sense, as the LP core can only handle simple tasks like sleep management.

The single-HP-core constraint deserves a fair assessment: FastAccelStepper generates step pulses in MCPWM/PCNT hardware, TWAI handles CAN frames in hardware, and Wi-Fi has its own baseband processor. The CPU's role is mostly feeding data to hardware queues and handling protocol logic. A single core at 160 MHz is likely sufficient for the actual CPU work involved.

The reason we chose dual-core anyway: during trajectory playback the board must simultaneously walk the trajectory table (feeding positions to FastAccelStepper), process CAN responses from the gimbal, and handle WebSocket position updates to the UI. If any of these paths hits an unexpected latency spike (e.g., a Wi-Fi retransmit storm, a CAN bus error recovery sequence), a second core provides a hard guarantee that motor timing is unaffected. For a v1 where we're also porting an unproven DJI protocol, that margin matters.

The C6 remains interesting for a future lightweight build (slider-only, no gimbal) where the dual TWAI controllers and Wi-Fi 6 are more valuable than dual cores.

### Raspberry Pi Pico 2 W

Excellent PIO state machines for motor control, dual-core, $7. But no native CAN bus. Adding an MCP2515 SPI module creates bus contention with the WiFi chip (which also uses SPI on the Pico W). Total cost approaches ESP32 while being functionally worse.

### Raspberry Pi Zero 2 W

Linux makes web serving trivial but makes real-time motor control nearly impossible. Would require a companion microcontroller, adding $5-15 and significant complexity. Draws 3W (20x more than ESP32), takes 15-30 seconds to boot, and has no native CAN. Only makes sense if the web UI needs a full web framework — and for a single-user slider control interface, it does not.

### STM32F407/H7

The gold standard for real-time motor control. Native CAN 2.0B, superb timer peripherals. But zero Wi-Fi capability — requires a two-chip design (STM32 + ESP8266 or WiFi module), a custom PCB, and significantly more development overhead. The ESP32's MCPWM+PCNT stepper peripherals are more than adequate for camera slider speeds. STM32's superior determinism only matters for CNC-class applications with much tighter timing requirements than ours.

### Teensy 4.1

Best CAN support (3 controllers), fastest CPU (600 MHz Cortex-M7), but $32 with no Wi-Fi. Would need a separate WiFi module and custom integration. Designed for audio/CNC applications where raw speed matters more than integration.

### Arduino GIGA R1

$73, dual-core STM32H747 + WiFi + CAN FD. Technically capable but absurdly overpowered and expensive for this application. Poor cost story for an open-source project that wants broad adoption.

### BeagleBone Black

Excellent PRU-based stepper control, native CAN, but $55-75, no Wi-Fi on base model, 1-2W power draw, Linux boot times. Designed for industrial automation, not portable battery-powered devices.

## Development Framework

**Arduino framework via PlatformIO**, not bare ESP-IDF:

- FastAccelStepper, ESPAsyncWebServer, and ESP32-TWAI-CAN libraries work out of the box
- Arduino on ESP32 runs on top of ESP-IDF — we can call ESP-IDF APIs (TWAI driver, MCPWM) directly when needed
- `xTaskCreatePinnedToCore()` is available for pinning tasks to cores
- Lower barrier to entry for contributors
- OTA updates via ArduinoOTA
- If we outgrow Arduino's abstractions, we can migrate to pure ESP-IDF without changing hardware

## Specific Board

Use the **Espressif ESP32-S3-DevKitC-1** — the official reference board, sold directly by Espressif. This avoids counterfeit module risk (third-party Amazon boards have been found with mislabeled flash/PSRAM specs). Two variants work for this project:

| Board | Module | Flash | PSRAM | Usable GPIO | Notes |
|-------|--------|-------|-------|-------------|-------|
| **ESP32-S3-DevKitC-1-N8R8** | WROOM-1 | 8 MB | 8 MB | 44 | Recommended — all GPIO available |
| **ESP32-S3-DevKitC-1-N32R16V** | WROOM-2 | 32 MB | 16 MB | 41 | Also fine — loses GPIO 35/36/37 to octal SPI |

Either board has far more memory than this project needs (firmware + web UI ≈ 2-3 MB flash; largest trajectory ≈ 200 KB PSRAM). Both have dual USB-C (UART + native USB-OTG) and an IPEX antenna connector.

### GPIO Budget

Estimated pin usage for all subsystems, from minimum viable to fully loaded:

| Subsystem | Pins | Notes |
|-----------|------|-------|
| Slider stepper (STEP/DIR/EN) | 3 | Core — always needed |
| TWAI / CAN bus (TX/RX) | 2 | Core — needed for DJI gimbal |
| Limit switches (slider ends) | 2 | Core — homing and safety |
| I2C bus (SDA/SCL) | 2 | Shared by display + any I2C peripherals |
| Battery voltage (ADC) | 1 | |
| Status LED | 1 | |
| **Core total** | **11** | |
| Servo gimbal (pan/tilt/roll PWM) | 3 | Only if not using DJI gimbal |
| Focus motor stepper (STEP/DIR/EN) | 3 | Optional axis |
| SPI SD card (MOSI/MISO/SCK/CS) | 4 | For trajectory logging |
| Rotary encoder + button (manual jog) | 3 | |
| SPI display — ST7789 (MOSI/SCK/CS/DC/RST/BL) | 5-6 | If SPI display chosen; I2C display shares I2C bus above at 0 extra pins |
| **Fully loaded total** | **~29-30** | With SPI display and all expansion |

Both boards have headroom: 44 (N8R8) or 41 (N32R16V) usable GPIO vs ~30 worst case. Either board works even with every optional subsystem connected.

Buy from Espressif's Amazon storefront, DigiKey, Mouser, or Adafruit.

## Hardware Cost

| Component | Cost |
|-----------|------|
| ESP32-S3-DevKitC-1 (N8R8 or N32R16V) | $15-17 |
| TJA1051 CAN transceiver module | $1-2 |
| **Total** | **$16-19** |

For comparison: Pico 2 W + CAN module = $15-17, STM32 + WiFi = $10-19, Teensy + WiFi = $40+.

## Risks

1. **No proven ESP32 + DJI gimbal integration exists — this is the project's biggest technical unknown.** Every existing DJI R SDK implementation runs on PC (DJIR_SDK with USB-CAN adapter), Linux/ROS (dji_rs2_ros_controller, dji_rs3pro_ros_controller), or STM32 flight controllers (ArduPilot via Lua script). No one has ported the protocol to any ESP32 variant. The gap between "TWAI can send CAN frames at 1 Mbps" and "a working DJI gimbal control stack" is significant — it includes the handshake/activation sequence, command framing with CRC, sequence number management, bus error recovery, and gimbal state machine management. The protocol is documented and the reference C++ code is available, but this is a from-scratch port with no community precedent to lean on. **Mitigation:** build a minimal CAN-only test early (just handshake + position command on an S3 + transceiver + DJI gimbal) before investing in the rest of the firmware.

2. **The three-subsystem combination (TWAI + FastAccelStepper + Wi-Fi) is unproven on ESP32.** Each subsystem works individually — FastAccelStepper is well-tested, TWAI is documented, ESPAsyncWebServer is mature. But no known project runs all three simultaneously. The architecture relies on hardware peripheral offloading and dual-core isolation, which is sound in theory but untested in practice. **Mitigation:** an early integration test running all three subsystems under load should be a firmware milestone before building application logic on top.

3. **PSRAM access is slower than SRAM.** The S3's 512 KB SRAM is sufficient for typical trajectories (~60KB at 3,000 points). Larger trajectories (10,000+ points, ~200KB) may spill into PSRAM, which has higher latency. Mitigation: the trajectory walker accesses data sequentially with predictable patterns, so cache performance should be good.

4. **S3 ecosystem is slightly younger than original ESP32.** Most libraries now support the S3, but edge-case issues are more likely. FastAccelStepper, ESPAsyncWebServer, and the TWAI driver are all confirmed working.

## Consequences

- Firmware will be written in C++ using PlatformIO with the Arduino framework
- The DJI R SDK CAN protocol will need to be implemented from scratch on the TWAI peripheral, using DJIR_SDK and ROS controllers as reference
- Motor control uses FastAccelStepper for hardware-accelerated step generation
- Web UI is served directly from the ESP32-S3's flash filesystem (LittleFS or SPIFFS)
- The dual-core split (Core 0 = network, Core 1 = motion) is a fundamental architectural pattern that all firmware code must respect
