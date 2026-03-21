# Microcontroller Selection for Camera Slider - Research Background

**Date:** 2026-03-20
**Topic:** Comparing microcontroller and single-board computer options for a DIY motorized camera slider with gimbal control. Requirements: Wi-Fi web UI, CAN bus (1 Mbps) for DJI gimbal, stepper motor control, trajectory storage, battery-powered, open-source friendly.

## Sources

[1]: https://www.espboards.dev/blog/esp32-soc-options/ "ESP32 Comparison Chart: All 9 Models (2026) - espboards.dev"
[2]: https://gist.github.com/sekcompsci/2bf39e715d5fe47579fa184fa819f421 "Comparison chips table for ESP32 variants - GitHub Gist"
[3]: https://discuss.ardupilot.org/t/ap-support-for-dji-rs2-rsc2-rs3-gimbals-maybe/91830 "AP support for DJI RS2, RSC2, RS3 gimbals - ArduPilot Discourse"
[4]: https://lastminuteengineers.com/esp32-can-bus-tutorial/ "How to Build a CAN Bus With ESP32 - Last Minute Engineers"
[5]: https://github.com/gin66/FastAccelStepper "FastAccelStepper - GitHub"
[6]: https://www.adafruit.com/product/6087 "Raspberry Pi Pico 2W - Adafruit"
[7]: https://www.hackster.io/ahmsville/raspberry-pi-pico-pio-stepper-motor-control-1ec3e9 "Raspberry Pi Pico PIO Stepper Motor Control - Hackster.io"
[8]: https://www.waveshare.com/pico-can-b.htm "CAN bus Module (B) for Raspberry Pi Pico - Waveshare"
[9]: https://canislabs.com/canpico/ "CANPico - Canis Labs"
[10]: https://forums.raspberrypi.com/viewtopic.php?t=342101 "Control up to 10 stepper motors with one Raspberry Pi - RPi Forums"
[11]: https://alanmajchrowicz.com/cornernet/how-many-stepper-motors-can-a-raspberry-pi-really-handle-tested-limits-and-smarter-setups/ "How Many Stepper Motors Can a Raspberry Pi Really Handle?"
[12]: https://github.com/pkerspe/ESP-StepperMotor-Server "ESP-StepperMotor-Server - GitHub"
[13]: https://community.st.com/t5/stm32-mcus-products/stm32f407-with-esp8266/td-p/282837 "STM32F407 with ESP8266 - ST Community"
[14]: https://forum.arduino.cc/t/espidf-vs-arduino-framework-performance/1137510 "ESP-IDF vs Arduino framework performance - Arduino Forum"
[15]: https://www.pjrc.com/store/teensy41.html "Teensy 4.1 - PJRC"
[16]: https://github.com/luni64/TeensyStep "TeensyStep - Fast Stepper Motor Library for Teensy"
[17]: https://copperhilltech.com/teensy-4-1-triple-can-bus-board-with-240x240-lcd-and-ethernet/ "Teensy 4.1 Triple CAN Bus Board"
[18]: https://www.beagleboard.org/boards/beaglebone-black "BeagleBone Black"
[19]: https://github.com/hzeller/beagleg "BeagleG - G-code stepper controller using BeagleBone PRU"
[20]: https://copperhilltech.com/blog/beaglebone-ai-family-comes-with-two-can-bus-interfaces/ "BeagleBone AI CAN Bus"
[21]: https://store.arduino.cc/products/giga-r1-wifi "Arduino GIGA R1 WiFi - Arduino Store"
[22]: https://docs.arduino.cc/hardware/giga-r1-wifi "Arduino GIGA R1 WiFi - Docs"
[23]: https://github.com/ConstantRobotics/DJIR_SDK "DJIR_SDK - ConstantRobotics GitHub"
[24]: https://github.com/ceinem/dji_rs2_ros_controller "dji_rs2_ros_controller - GitHub"
[25]: https://github.com/Hibiki1020/dji_rs3pro_ros_controller "dji_rs3pro_ros_controller - GitHub"
[26]: https://ardupilot.org/copter/docs/common-djirs2-gimbal.html "DJI RS2 and RS3-Pro Gimbals - ArduPilot"
[27]: https://docs.espressif.com/projects/esp-idf/en/stable/esp32s3/api-reference/peripherals/twai.html "ESP32-S3 TWAI Documentation - Espressif"
[28]: https://lastminuteengineers.com/esp32-sleep-modes-power-consumption/ "ESP32 Sleep Modes & Power Consumption"
[29]: https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-guides/current-consumption-measurement-modules.html "ESP32 Current Consumption - Espressif"

<!-- Add new sources here as you find them. Number sequentially. -->

## Research Log

---

### Search: "ESP32 variants comparison ESP32-S2 S3 C3 C6 specs CAN bus TWAI WiFi 2024 2025"

**ESP32 variant specs relevant to this project:**

| Variant | CPU | Cores | Clock | SRAM | WiFi | TWAI/CAN | GPIOs | Deep Sleep | Price |
|---------|-----|-------|-------|------|------|----------|-------|------------|-------|
| **ESP32** | Xtensa LX6 | 2 | 240 MHz | 520 KB | Wi-Fi 4 | 1 controller | 34 | ~100 uA | $3-10 |
| **ESP32-S2** | Xtensa LX7 | 1 | 240 MHz | 320 KB | Wi-Fi 4 | 1 controller | 43 | ~22 uA | ~$6 |
| **ESP32-S3** | Xtensa LX7 | 2 | 240 MHz | 512 KB | Wi-Fi 4 | 1 TWAI (confirmed, see search below) | 44 | ~24 mA active (no WiFi) | $4-15 |
| **ESP32-C3** | RISC-V | 1 | 160 MHz | 400 KB | Wi-Fi 4 | 1 controller | 22 | ~5 uA | $3-8 |
| **ESP32-C6** | RISC-V | 1+LP | 160 MHz | 512 KB | Wi-Fi 6 | **2 controllers** | 30 | ~5 uA | $4-10 |

Key observations for this project:
- **Original ESP32** has **dual cores at 240 MHz** and **1 TWAI controller** -- the dual-core architecture allows dedicating one core to Wi-Fi/web serving and the other to motor control ([ESP32 Comparison Chart][1])
- **ESP32-C6** is the only variant with **2 TWAI controllers**, which is notable but it's single-core at 160 MHz ([Comparison chips table][2])
- **ESP32-S3** has dual cores and the most SRAM (512 KB) but TWAI/CAN support status was not confirmed across these sources ([ESP32 Comparison Chart][1])
- **ESP32-S2 lacks Bluetooth entirely** and is single-core -- less suitable ([ESP32 Comparison Chart][1])
- All variants with Wi-Fi can serve a web UI directly
- The original ESP32 has the most mature ecosystem and lowest cost floor ($3)

**Follow-up questions:**
- Does ESP32-S3 have TWAI/CAN? Need to check the datasheet.
- What is the ESP32's TWAI max baud rate -- can it do 1 Mbps reliably?
- What dev boards are available at good prices?

---

### Search: "ESP32 TWAI CAN bus DJI gimbal RS3 RS2 1Mbps communication"

**DJI gimbal CAN protocol findings:**
- Only **RS2 and RS3 Pro** use the DJI R SDK CAN protocol -- **standard RS3 and RS3 Mini do NOT** ([AP support for DJI RS2/RS3 gimbals][3])
- ArduPilot developers observed raw CAN frames with start-of-frame byte `0x55`, 8-byte payloads, and sequence numbers in the last two bytes ([AP support for DJI RS2/RS3 gimbals][3])
- A ROS-based controller project (`ceinem/dji_rs2_ros_controller`) has successfully decoded raw CAN frames and sends commands using cmd_type, cmd_set, cmd_id parameters ([AP support for DJI RS2/RS3 gimbals][3])
- The exact baud rate was NOT specified in the ArduPilot thread, though the R SDK docs specify 1 Mbps

**ESP32 TWAI capabilities:**
- **ESP32 TWAI supports 25 Kbit/s to 1 Mbit/s** on revisions v0.0/v1.0/v1.1, and 12.5 Kbit/s to 1 Mbit/s on v3.0/v3.1 -- **1 Mbps is within spec** ([ESP32 CAN Bus Tutorial][4])
- An **external CAN transceiver is always required** (ESP32 has the controller but not the physical layer driver). TJA1050 or SN65HVD23x recommended ([ESP32 CAN Bus Tutorial][4])
- TX and RX pins can be mapped to **any GPIO** ([ESP32 CAN Bus Tutorial][4])
- Receive FIFO buffer is **64 bytes** ([ESP32 CAN Bus Tutorial][4])
- Standard frames (11-bit IDs) and extended frames (29-bit IDs) both supported ([ESP32 CAN Bus Tutorial][4])

**Follow-up questions:**
- Are there any ESP32-specific DJI gimbal control projects?
- What transceiver board works best for 3.3V logic (TJA1050 is 5V, may need level shifting vs TJA1051 which is 3.3V)?

---

### Search: "ESP32 stepper motor control step direction pulse generation timer interrupt AccelStepper"

**FastAccelStepper library** is the gold standard for ESP32 stepper control ([FastAccelStepper][5]):
- Supports **ESP32, ESP32-S2, ESP32-S3, ESP32-C3, ESP32-C6, ESP32-P4** and also Raspberry Pi Pico/Pico 2
- On ESP32 uses **MCPWM+PCNT** peripherals for hardware-based step pulse generation -- offloads timing from CPU entirely
- Also supports **RMT** peripheral as alternative driver
- **Maximum step rate: 200,000 steps/sec** on ESP32 (vs 50,000 on AVR) ([FastAccelStepper][5])
- Can drive **up to 14 simultaneous steppers** on ESP32 (6 via MCPWM/PCNT + 8 via RMT) ([FastAccelStepper][5])
- Supports acceleration/deceleration profiles with cubic speed function for smooth motion
- Uses **interrupt-driven command queues** -- no periodic polling needed, CPU is free for other tasks
- Hardware-based step counting with PCNT catches and corrects any pulse overruns ([FastAccelStepper][5])

This is very significant for the project: FastAccelStepper means the ESP32 can generate precise stepper pulses via dedicated hardware peripherals while the CPU handles Wi-Fi, web serving, and CAN bus communication concurrently.

**Follow-up questions:**
- How does FastAccelStepper interact with trajectory tables? Can you feed it position/speed targets from a pre-computed table?
- Does MCPWM usage conflict with TWAI on the same ESP32?

---

### Search: "Raspberry Pi Pico W Pico 2 W specs CAN bus stepper motor control web server"

**Raspberry Pi Pico 2 W specifications** ([Pico 2W - Adafruit][6]):
- **CPU:** Dual Arm Cortex-M33 or dual RISC-V Hazard3 at **150 MHz** (switchable architecture)
- **SRAM:** 520 KB (double the original Pico's 264 KB)
- **Flash:** 4 MB QSPI
- **WiFi:** 802.11n single-band 2.4 GHz (WPA3), Soft AP supports up to 4 clients
- **Bluetooth:** BLE and Classic (via Infineon CYW43439, SPI at 33 MHz)
- **GPIO:** 26 multi-purpose pins, 4 ADC capable
- **PIO:** 12 state machines (3 blocks x 4) -- can implement custom protocols including CAN in software
- **Peripherals:** 2x UART, 2x SPI, 2x I2C, 24x PWM, USB 1.1 host/device
- **Price:** **$7.00** (without headers)
- **No native CAN bus controller** -- requires external MCP2515 module via SPI ([Waveshare Pico CAN][8], [CANPico][9])
- M33 cores are approx **2x faster** than RP2040's M0+ cores

**CAN bus on Pico:**
- Requires **external SPI-based CAN controller** (MCP2515) + transceiver
- Waveshare Pico-CAN-B module: ~$8-10 add-on ([Waveshare Pico CAN][8])
- CANPico from Canis Labs: carrier board with CAN FD controller, more expensive ([CANPico][9])
- CAN via SPI adds latency vs native TWAI on ESP32 -- SPI bus is shared with WiFi chip (CYW43439 also on SPI)

**Stepper motor control on Pico:**
- PIO state machines can generate precise step pulses in hardware -- similar concept to ESP32's MCPWM
- FastAccelStepper library **also supports Pico and Pico 2** ([FastAccelStepper][5])
- PIO-based stepper control demonstrated on Hackster.io ([PIO Stepper Control][7])

**Web server on Pico W:**
- MicroPython async web server works but is slower than ESP32's native HTTP server
- C/C++ SDK web server also possible but less documented
- Soft AP mode supports only **4 concurrent clients** -- adequate for a single-user control UI

**Key comparison with ESP32:**
- Pico 2 W has more PIO state machines (12 vs ESP32's peripherals) but **no native CAN**
- ESP32 has native TWAI CAN, making it simpler and lower-latency for CAN bus
- Pico 2 W is $7 vs ESP32 dev boards at $3-10
- Both support FastAccelStepper for stepper control
- SPI bus contention between WiFi chip and CAN module on Pico is a concern

**Follow-up questions:**
- How much latency does MCP2515 over SPI add vs native TWAI?
- Can Pico's PIO implement CAN bus directly without MCP2515?

---

### Search: "Raspberry Pi Zero 2 W motor control real-time Linux stepper CAN bus power consumption battery"

**Raspberry Pi Zero 2 W for this project:**
- **CPU:** Quad-core Arm Cortex-A53 at 1 GHz (BCM2710A1)
- **RAM:** 512 MB LPDDR2
- **WiFi:** 802.11b/g/n, Bluetooth 4.2/BLE
- **Price:** ~$15
- **Power consumption:** ~0.6A at 5V (3W) with WiFi active -- **5-10x more than ESP32** ([RPi stepper limits][11])
- **Boot time:** 15-30 seconds (Linux boot) vs instant-on for microcontrollers
- Runs full Linux -- can serve sophisticated web UIs (React, etc.) with ease

**Real-time motor control is a major weakness:**
- Linux kernel is **not real-time** -- jitter in GPIO timing makes precise stepper control unreliable ([RPi stepper limits][11], [Control 10 steppers][10])
- Forum consensus: "Raspberry Pi is completely useless for CNC and other applications where some sort of realtime is required" ([Control 10 steppers][10])
- Workaround: use Pi as **host controller** sending commands to a **dedicated MCU** (Arduino/ESP32/STM32) that does the actual step pulse generation
- LinuxCNC with a PRU (BeagleBone) or external step generator is the proven pattern for real-time on Linux SBCs

**CAN bus on Pi Zero 2 W:**
- No native CAN controller -- requires external MCP2515 via SPI (same as Pico)
- Linux SocketCAN driver available for MCP2515, well-documented

**For this project:** Pi Zero 2 W would need a companion MCU for motor control, making the system more complex and expensive. The 3W power draw is significant for battery use. The only advantage is running a full Linux web server.

**Follow-up questions:**
- Would a dual-board design (Pi Zero 2 W + ESP32) be worth the complexity?
- What about PREEMPT_RT kernel for the Pi?

---

### Search: "STM32F4 STM32H7 WiFi module CAN bus stepper motor control comparison ESP32 2024"

**STM32 for this project:**
- STM32F4 series: Cortex-M4 at **168 MHz**, ~210 DMIPS, **native CAN 2.0B** (2 controllers on STM32F407), excellent timer peripherals for motor control
- STM32H7 series: Cortex-M7 at up to **480 MHz**, >1000 DMIPS, **CAN FD** support, massively overpowered for this application
- STM32F407 dev boards (e.g., "Black Pill"): ~$8-15 on AliExpress
- STM32H743 dev boards: ~$15-30

**WiFi is the STM32's Achilles heel for this project:**
- **No built-in WiFi** on any STM32F4/H7 variant
- Must add a separate WiFi module: ESP8266 ($2) or ESP32 ($4) connected via UART/SPI ([STM32F407 with ESP8266][13])
- This creates a **two-chip design** -- STM32 for control + ESP for WiFi
- AT command interface between STM32 and ESP8266 is clunky and limited
- Alternatively, use a full ESP32 as the WiFi co-processor, but then why not just use the ESP32 for everything?

**STM32 advantages:**
- Superior **real-time determinism** -- STM32 dominates in flight controllers (ArduPilot, PX4) and CNC applications
- Advanced timer peripherals (16-bit with dead time generation, center-aligned PWM)
- Hardware FPU on all F4/H7 for fast math
- **STM32CubeIDE** free IDE with debugger, but steeper learning curve than Arduino/ESP-IDF
- HAL libraries are comprehensive but verbose

**STM32 disadvantages for this project:**
- Two-chip design adds cost, complexity, and board space
- Community is more professional/industrial, less hobbyist-friendly than ESP32/Arduino
- No built-in web server -- would need to implement HTTP/WebSocket on the ESP side or bridge through UART
- OTA updates much harder without integrated WiFi

**ESP-StepperMotor-Server** is a notable ESP32 project ([ESP-StepperMotor-Server][12]):
- Turns ESP32 into complete stepper motor control server with **built-in web UI, REST API, and serial CLI**
- Supports step/dir drivers with limit switches and rotary encoders
- Includes OTA firmware update capability
- Caveat: **not suitable for synchronized multi-axis CNC** -- motors move asynchronously
- Good reference architecture for this project's web UI + motor control pattern

**Follow-up questions:**
- Are there any STM32 + WiFi integrated boards (like WeAct or similar)?
- Is the STM32WB (with BLE) relevant?

---

### Search: "ESP-IDF vs Arduino framework ESP32 web server WebSocket performance real-time motor control tradeoffs"

**ESP-IDF vs Arduino framework for this project:**

Performance benchmark from Arduino Forum ([ESP-IDF vs Arduino performance][14]):
- Same neural network code: Arduino = 3.69s, ESP-IDF default = 18.96s, ESP-IDF with -O2 = 5.66s
- The disparity was due to **compiler optimization flags**, not framework overhead
- With proper optimization flags, ESP-IDF and Arduino are comparable in raw performance

**Key tradeoffs:**

| Factor | Arduino Framework | ESP-IDF |
|--------|------------------|---------|
| **Ease of use** | Simpler, familiar API | Steeper learning curve |
| **Library ecosystem** | Huge (FastAccelStepper, ESPAsyncWebServer, etc.) | Smaller, more low-level |
| **FreeRTOS access** | Available but abstracted | Full direct access |
| **Web server** | ESPAsyncWebServer (popular, async) | esp_http_server (built-in, sync) |
| **WebSocket** | AsyncWebSocket library | esp_websocket component |
| **OTA updates** | ArduinoOTA (easy) | ESP-IDF OTA (more control) |
| **Debugging** | Serial.print mostly | JTAG, OpenOCD, GDB |
| **Build system** | PlatformIO / Arduino IDE | CMake / idf.py |
| **TWAI/CAN** | arduino-esp32 TWAI wrapper | Full TWAI driver API |
| **Configuration** | Limited sdkconfig access | Full menuconfig |

**WiFi latency considerations:**
- Best-case WiFi round-trip latency: **~2ms** (500 Hz) on uncrowded network
- WebSocket broadcast to multiple clients can interfere with HTTP request processing
- Motor control should use hardware peripherals (MCPWM), not be dependent on WiFi timing

**Recommendation for this project:**
- **Arduino framework with PlatformIO** is the pragmatic choice for open-source contributors
- Use ESP-IDF APIs directly where needed (TWAI, MCPWM) -- Arduino framework on ESP32 allows mixing in ESP-IDF calls
- FastAccelStepper handles motor timing in hardware, so the framework choice doesn't affect step precision
- The web UI runs on its own FreeRTOS task on the second core, isolated from motor control

**Follow-up questions:**
- Can Arduino framework on ESP32 use both cores explicitly (xTaskCreatePinnedToCore)?
- What's the memory footprint of ESPAsyncWebServer + WebSocket?

---

### Search: "Teensy 4.1 CAN bus WiFi stepper motor control specs price power consumption" and "BeagleBone Black Green AI CAN bus WiFi stepper motor PRU real-time price 2024 2025"

**Teensy 4.1** ([Teensy 4.1 - PJRC][15]):
- **CPU:** ARM Cortex-M7 at **600 MHz** (NXP iMXRT1062) -- fastest MCU in this comparison
- **RAM:** 1024 KB (512 KB tightly coupled for zero-wait-state access)
- **Flash:** 7936 KB (nearly 8 MB)
- **CAN bus:** **3 CAN controllers (1 with CAN FD)** -- best native CAN support of any option ([Teensy 4.1 - PJRC][15])
- **WiFi:** **None built-in** -- requires external module (ESP32 or WiFi shield)
- **GPIO:** 55 I/O pins, 35 PWM outputs
- **Power:** ~100 mA at 600 MHz (~500 mW)
- **Price:** ~$32 (from PJRC/SparkFun/Adafruit)
- **Stepper control:** TeensyStep library provides fast coordinated multi-axis stepper control ([TeensyStep][16])
- Triple CAN bus board available from Copperhill Tech with LCD and Ethernet ([Teensy 4.1 Triple CAN][17])
- Real-time: 4 dedicated interval timers, nested interrupt controller, dual-issue superscalar = excellent determinism

**Teensy 4.1 assessment for this project:**
- Massively overpowered CPU (600 MHz M7) for the task
- Excellent CAN bus support (3 controllers!) but **no WiFi** is a dealbreaker for single-board solution
- Expensive at $32 + WiFi module
- Smaller community than ESP32/Arduino, PJRC-specific toolchain
- Best option if real-time motor control were the primary constraint, but this project values WiFi integration more

**BeagleBone Black** ([BeagleBone Black][18]):
- **CPU:** TI AM3358, 1 GHz Cortex-A8 + **2x PRU** (200 MHz real-time coprocessors)
- **RAM:** 512 MB DDR3L
- **Storage:** 4 GB eMMC + microSD
- **CAN bus:** DCAN controllers available on AM335x (2 controllers), but require pin mux configuration
- **WiFi:** **None built-in** on BBB -- BeagleBone Green Gateway variant has WiFi
- **Price:** ~$55-75 (expensive!)
- **Power:** 1-2W typical
- **PRU stepper control:** beagleg project demonstrates G-code stepper control at high speeds using PRU for deterministic timing ([BeagleG][19])
- BeagleBone AI family has 2 classical CAN bus interfaces with CAN FD planned ([BeagleBone AI CAN][20])

**BeagleBone assessment for this project:**
- PRU coprocessors are excellent for real-time stepper control -- proven by beagleg CNC project
- But: **expensive** ($55-75), **no WiFi on base model**, **large form factor**, **small community** compared to ESP32
- Linux-based = same real-time issues as Pi (mitigated by PRU for steppers)
- Overkill for this application -- designed for industrial/CNC use cases
- Poor choice for battery-powered portable use

---

### Search: "Arduino Nano ESP32 Arduino Giga R1 specs CAN bus WiFi price" and "DJI gimbal ESP32 CAN bus control project open source"

**Arduino Nano ESP32:**
- Based on **ESP32-S3** (NORA-W106 module) -- dual-core Xtensa at 240 MHz
- WiFi + BLE built-in
- Classic Arduino Nano form factor
- Price: ~$20 (premium over generic ESP32-S3 boards at $5-10)
- Benefit: official Arduino support, familiar IDE
- Drawback: expensive for what it is; a generic ESP32-S3 board does the same for less

**Arduino GIGA R1 WiFi** ([Arduino GIGA R1 WiFi][21], [GIGA R1 Docs][22]):
- **CPU:** STM32H747XI dual-core -- **480 MHz Cortex-M7 + 240 MHz Cortex-M4**
- **RAM:** 1 MB + 8 MB SDRAM
- **Flash:** 2 MB
- **WiFi:** 802.11b/g/n via ESP32-based module (Murata 1DX)
- **CAN bus:** 1 FDCAN controller
- **GPIO:** 76 pins
- **Price:** **$73** -- by far the most expensive option
- Massively overpowered, designed for edge computing, machine learning, and advanced robotics

**Arduino GIGA R1 assessment:**
- Has both WiFi and CAN FD in one board, but at **$73 it's 10-15x the cost of an ESP32**
- The STM32H747 dual-core is serious overkill for this application
- Poor value proposition for a DIY camera slider
- Much smaller community than ESP32

**DJI gimbal open-source projects -- ESP32 port status:**
- **DJIR_SDK** ([DJIR_SDK][23]): C++ SDK for DJI RS2 based on R SDK protocol v2.2. Runs on PC only (Qt/CMake), uses USB-CAN adapter. **Not ported to ESP32 or any MCU.**
- **dji_rs2_ros_controller** ([dji_rs2_ros_controller][24]): ROS node for RS2 control over CAN. Linux/ROS only. **Not ported to ESP32.**
- **dji_rs3pro_ros_controller** ([dji_rs3pro_ros_controller][25]): Based on the RS2 controller, adapted for RS3 Pro. Also Linux/ROS only.
- **ArduPilot** ([ArduPilot DJI gimbal docs][26]): Supports RS2 and RS3 Pro via Lua script, runs on STM32-based flight controllers with native CAN. Not ESP32-based.
- **No existing ESP32 + DJI gimbal CAN project found.** The protocol (DJI R SDK v2.2) would need to be ported from the C++ DJIR_SDK or the ROS controllers to run on ESP32 with TWAI. The protocol itself is well-documented in the R SDK, so porting is feasible but would be new work.

**Follow-up questions:**
- Is the DJI R SDK protocol simple enough that porting to ESP32 is straightforward?
- What's the minimum viable implementation -- just position control commands?

---

### Search: "ESP32-S3 TWAI CAN bus support datasheet peripheral"

**ESP32-S3 TWAI confirmed** ([ESP32-S3 TWAI Documentation][27]):
- ESP32-S3 **does have 1 TWAI controller**, compatible with ISO 11898-1 (CAN 2.0)
- Supports **1 Kbit/s to 1 Mbit/s** baud rate
- Standard (11-bit) and extended (29-bit) frame formats
- TX, RX, BUS-OFF, and CLKOUT signals routable to any GPIO via GPIO matrix
- **Does NOT support CAN FD** -- will interpret FD frames as errors
- Requires external transceiver (same as all ESP32 variants)

This resolves the earlier uncertainty. Updated ESP32-S3 row:

| ESP32-S3 | Xtensa LX7 | 2 | 240 MHz | 512 KB | Wi-Fi 4 | **1 TWAI** | 44 | TBD | $4-15 |

ESP32-S3 is now confirmed as a viable candidate: dual-core 240 MHz + WiFi + TWAI + 512 KB SRAM.

---

### Search: "ESP32 power consumption WiFi active web server milliamps battery life comparison"

**ESP32 power consumption data** ([ESP32 Sleep Modes][28], [ESP32 Current Consumption][29]):

| Mode | Current Draw | Notes |
|------|-------------|-------|
| WiFi TX burst | up to **240 mA** peak | 0.5ms spikes at WiFi beacon rate |
| WiFi connected active | **130 mA** average | Module-level measurement |
| WiFi connected (dev kit) | **40 mA** average | Includes voltage regulator losses |
| Modem-sleep (WiFi DTIM3) | **~0.9 mA** | Maintains WiFi connection, 300ms sleep intervals |
| Light-sleep | **~0.8 mA** | Can maintain WiFi |
| Deep-sleep | **5-100 uA** | Variant-dependent; ESP32-C3/C6 as low as 5 uA |
| ESP32-S3 active mode | **~24 mA** | Without WiFi active |

**Battery life estimates** (2500 mAh LiPo):
- Continuous WiFi active: ~2500/130 = **~19 hours**
- With modem-sleep between commands: significantly longer
- WiFi connect from cold boot takes **~3.9 seconds** and ~350 mA burst

**For this project:**
- During a slider move (maybe 30 seconds to a few minutes), the ESP32 needs WiFi + motor + CAN simultaneously -- expect ~150-200 mA total draw from the MCU (not counting motors)
- Between moves, modem-sleep can reduce to <1 mA
- A 2500 mAh battery could sustain many hours of intermittent use
- ESP32 power draw is **15-20x less** than Pi Zero 2 W (3W vs ~0.65W)

---

<!-- Research complete. Proceeding to synthesis. -->
