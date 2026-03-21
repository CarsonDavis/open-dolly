# Bill of Materials (Rough)

*Status: draft*
*Last updated: 2026-03-20*

Rough cost estimates for a complete slider + gimbal system. Prices are approximate USD, sourced from common retailers (Amazon, AliExpress, DigiKey).

## Electronics

| Part | Example | Est. Cost | Notes |
|------|---------|-----------|-------|
| Microcontroller | Espressif ESP32-S3-DevKitC-1 (N8R8 or N32R16V) | $15-17 | Official Espressif board. Dual-core 240 MHz, Wi-Fi, BLE, TWAI (CAN), 8+ MB PSRAM, dual USB-C, IPEX antenna. See [ADR-001](adr-001-microcontroller.md) for board selection rationale |
| CAN transceiver | TJA1051 or SN65HVD230 module | $1-3 | 3.3V logic level for ESP32-S3. Required for DJI gimbal communication; not needed for servo-based gimbals |
| Stepper driver (slider) | TMC2209 silent driver | $5-8 | StealthChop for silent motion; UART config; 2.0A RMS matches motor perfectly. See [motor selection research](../research/slider-motor-selection/report.md) |
| Stepper motor (slider) | StepperOnline 17HS19-2004S1 (NEMA 17) | $10-12 | 59 Ncm holding torque, 2.0A rated current, 42x48mm body. Handles 7 kg payload on flat and up to 45-degree inclines. See [motor selection research](../research/slider-motor-selection/report.md) |
| Power supply | 24V 2A DC adapter or 6S LiPo | $15-30 | **24V required** for stepper torque at speed; ESP32 runs off 5V buck converter; battery for portable use |
| Wiring/connectors | JST, Dupont, screw terminals | $5-10 | |
| USB-C breakout (programming/debug) | — | $2 | Usually built into ESP32 dev board |

**Electronics subtotal: ~$50-80**

## Mechanical (Slider)

| Part | Example | Est. Cost | Notes |
|------|---------|-----------|-------|
| Linear rail | Dual rod rail (8mm rods, 500-1000mm) | $20-40 | Or MGN12 linear rail for smoother motion |
| Sliding platform/carriage | Linear bearings + plate | $10-20 | Must carry gimbal + camera weight (~7 kg total) |
| GT2 belt + pulleys | GT2 6mm belt, 20-tooth drive + idler pulleys | $7-11 | Belt drive chosen for speed (0.5-1.0 m/s), simplicity, and low noise. See [motor selection research](../research/slider-motor-selection/report.md) |
| Idler + motor mount | Aluminum brackets or 3D printed | $5-15 | Needs belt tensioning mechanism |
| Feet/base | Tripod mount plate or rubber feet | $5-10 | 1/4-20 thread for tripod compatibility |

**Mechanical subtotal: ~$47-96**

## Gimbal Options

| Option | Example | Est. Cost | Notes |
|--------|---------|-----------|-------|
| DJI RS 4 | — | $430-500 | 3-axis, CAN SDK support, 3kg payload |
| DJI RS 4 Pro | — | $650-750 | 3-axis, CAN SDK support, 4.5kg payload |
| DJI RS 3 Pro | — | $500-600 | 3-axis, CAN SDK support; community ROS projects exist |
| DJI RS 2 (used) | — | $250-350 | 3-axis, community CAN support (older protocol v2.2) |
| Custom servo gimbal | 3x MG996R servos + 3D printed frame | $30-50 | Simpler, no SDK needed, direct PWM from ESP32; lower precision |
| No gimbal (slider only) | — | $0 | System works with any subset of axes |

## Full System Estimates

| Config | What you get | Est. Total |
|--------|-------------|------------|
| **Minimal (slider only)** | Motorized rail + ESP32 + web UI | $100-175 |
| **Budget (slider + servo gimbal)** | 4-axis with custom gimbal | $130-225 |
| **Mid-range (slider + used DJI RS 2)** | 4-axis with proven gimbal | $350-525 |
| **Full (slider + DJI RS 4)** | 4-axis with current-gen gimbal | $530-675 |

## Optional Add-Ons

| Part | Example | Est. Cost | Pins Used | Notes |
|------|---------|-----------|-----------|-------|
| Status display (I2C) | SH1107 1.5" 128x128 OLED | $6-10 | 0 (shares I2C) | Battery, IP, playback status. Shares I2C bus |
| Status display (SPI) | ST7789 1.5-2.0" 240x240 color TFT | $5-12 | 5-6 (SPI) | More pixels and color, but uses more GPIO |
| Focus motor | NEMA 17 + TMC2209 | $15-23 | 3 (STEP/DIR/EN) | Adds focus axis |
| Rotary encoder | KY-040 or similar | $2-4 | 3 (A/B/BTN) | Manual jog control without phone |
| SD card module | SPI breakout | $2-4 | 4 (SPI) | Trajectory logging/storage |

## GPIO Pin Budget

The ESP32-S3-DevKitC-1 has 44 GPIO (N8R8) or 41 (N32R16V). Here's how they get used:

| Subsystem | Pins | Notes |
|-----------|------|-------|
| **Slider stepper** (STEP/DIR/EN) | 3 | Always needed |
| **TWAI / CAN bus** (TX/RX) | 2 | For DJI gimbal |
| **Limit switches** (slider ends) | 2 | Homing and safety |
| **I2C bus** (SDA/SCL) | 2 | Shared by OLED display + any I2C peripherals |
| **Battery voltage** (ADC) | 1 | |
| **Status LED** | 1 | |
| ***Core total*** | ***11*** | *Minimum viable slider* |
| Servo gimbal (pan/tilt/roll PWM) | +3 | Only if not using DJI |
| Focus motor stepper (STEP/DIR/EN) | +3 | |
| SPI SD card (MOSI/MISO/SCK/CS) | +4 | |
| Rotary encoder + button | +3 | |
| SPI display (MOSI/SCK/CS/DC/RST/BL) | +5-6 | I2C display uses 0 extra pins |
| ***Fully loaded total*** | ***~29-30*** | *Every optional subsystem connected* |

11-30 pins used out of 41-44 available. Even the most loaded build leaves 11+ GPIO free.

## Power

The system has consumers at different voltage levels:

| Consumer | Voltage | Current | Source |
|----------|---------|---------|--------|
| Stepper motor + driver | 24V | 1-2A per motor | 6S LiPo (22.2V) or 24V DC adapter |
| ESP32-S3 | 5V via VIN | ~130-200 mA | Buck converter from 24V rail |
| CAN transceiver | 3.3V | ~10 mA | ESP32 3.3V rail |
| DJI gimbal | Own battery | 0A from us | Self-powered |
| Display (optional) | 3.3V | ~20 mA | ESP32 3.3V rail |

For portable use: a single 6S LiPo (1300-2600 mAh) feeds 24V to the stepper driver directly, with a small buck converter ($2) stepping down to 5V for the ESP32's VIN pin. For studio use: a 24V 2A DC adapter. 24V is required for good stepper torque at speed -- 12V causes severe torque dropoff above 300 RPM.

Battery charging is external for v1 — use a standard RC balance charger. On-board 6S charging requires a balance charging IC and is a custom PCB feature for a future version.

## Not Included

- Camera and lens
- Tripod / support
- CAN adapter cable (DJI R Focus Wheel port connector — may need to be custom or 3D-printed)
