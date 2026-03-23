# Bill of Materials

*Status: active*
*Last updated: 2026-03-22*

## Ordered Parts

| Qty | Part | Price Paid | Per Slider | Link | Notes |
|-----|------|-----------|------------|------|-------|
| 1 | Espressif ESP32-S3-DevKitC-1-N32R16V | $17.00 | $17.00 | [Amazon](https://www.amazon.com/dp/B0FDG3WJDX) | 32 MB flash, 16 MB PSRAM, Micro-USB, PCB antenna |
| 1 | STEPPERONLINE Nema 17 2A 55Ncm 42x48mm | $12.99 | $12.99 | [Amazon](https://www.amazon.com/gp/product/B0B93HTR87) | Slider motor. 1.8° step angle |
| 2 | BIGTREETECH TMC2209 V1.3 UART Driver | $13.99 (2-pack) | $7.00 | [Amazon](https://www.amazon.com/gp/product/B08SMDY3SQ) | 2.8A peak, StealthChop, UART. 1 for slider, 1 spare |
| 2 | Waveshare SN65HVD230 CAN Board | $16.99 (2-pack) | $8.50 | [Amazon](https://www.amazon.com/gp/product/B00KM6XMXO) | 3.3V CAN transceiver for DJI gimbal. 1 per slider, 1 spare |
| 1 | Auplf 24V 6A DC Power Supply | $16.98 | $16.98 | [Amazon](https://www.amazon.com/gp/product/B0CW598HV8) | 144W, 5.5x2.1mm barrel jack + terminal connector |
| 2 | Seloky LM2596 Buck Converter w/ LED | $8.63 (2-pack) | $4.32 | [Amazon](https://www.amazon.com/dp/B0F1M2SR31) | 4-40V in, adjustable out. Set to 5V for ESP32 |
| 2+4 | ANWOK HGR20 1200mm Linear Rail Kit | $88.02 | $88.02 | [Amazon](https://www.amazon.com/dp/B099WSJQ2J) | 2 rails + 4 HGH20CA carriages. Dual-rail setup on 2020 extrusion |
| 10 | TITGGI 2020 Aluminum Extrusion 48" (1220mm) | $79.98 (10-pack) | $16.00 | [Amazon](https://www.amazon.com/dp/B0CLHVSZFP) | 6063-T5, black anodized. 2 beams per slider |
| 260 | Bemaka M3/M4/M5 T-Slot Nut + Screw Kit | $13.99 | $13.99 | [Amazon](https://www.amazon.com/dp/B0FQ55VFDK) | M3 for rail mounting, M4/M5 for brackets. Includes hex wrenches |
| 1 | Zeelo GT2 Belt + Pulley Kit | $16.99 | $16.99 | [Amazon](https://www.amazon.com/gp/product/B08SMFM3Z6) | 5m 6mm belt, 4x 20T 5mm bore pulleys, 4x idlers, 4x clamps, 8x springs |
| 10 | HiLetgo KW12-3 Micro Limit Switch | $5.99 (10-pack) | $1.20 | [Amazon](https://www.amazon.com/dp/B07X142VGC) | SPDT w/ roller lever. 2 per slider for endstops |
| 1 | STEPPERONLINE Nema 17 Pancake 1A 17Ncm | $10.50 | — | [Amazon](https://www.amazon.com/gp/product/B0B93PNYCP) | Not part of base slider; for focus axis or experimentation |

| | **Totals** | **$285.05** | **~$203** | | |

## Gimbal Options (not yet purchased)

| Option | Est. Cost | Notes |
|--------|-----------|-------|
| DJI RS 4 | $430-500 | 3-axis, CAN SDK support, 3 kg payload |
| DJI RS 4 Pro | $650-750 | 3-axis, CAN SDK support, 4.5 kg payload |
| DJI RS 2 (used) | $250-350 | 3-axis, older CAN protocol v2.2 |
| Custom servo gimbal | $30-50 | 3x MG996R servos + 3D printed frame; no SDK needed |
| No gimbal | $0 | Slider-only configuration |

## Future / Optional (not yet purchased)

| Part | Est. Cost | Notes |
|------|-----------|-------|
| 1/4"-20 brass heat-set inserts | ~$13 | Camera/gimbal mount to carriage plate |
| 3/8"-16 brass heat-set inserts | ~$13 | Tripod mount |
| Status display (SH1107 OLED, I2C) | $6-10 | 128x128, shares I2C bus (0 extra GPIO) |
| Focus motor (pancake motor already owned) | $0-7 | Needs TMC2209 (spare already owned) |
| 6S LiPo battery + buck converter | $20-40 | For portable/field use instead of wall power |

## GPIO Pin Budget

ESP32-S3-DevKitC-1-N32R16V has 41 GPIO available:

| Subsystem | Pins | Notes |
|-----------|------|-------|
| **Slider stepper** (STEP/DIR/EN) | 3 | Core |
| **TWAI / CAN bus** (TX/RX) | 2 | DJI gimbal communication |
| **Limit switches** (min/max) | 2 | Homing and endstops |
| **I2C bus** (SDA/SCL) | 2 | Display + other I2C peripherals |
| **Battery voltage** (ADC) | 1 | |
| **Status LED** | 1 | Onboard RGB on GPIO 48 |
| ***Core total*** | ***11*** | |
| Servo gimbal (PWM x3) | +3 | Only if not using DJI |
| Focus motor (STEP/DIR/EN) | +3 | |
| SPI SD card | +4 | |
| Rotary encoder + button | +3 | |
| SPI display | +5-6 | I2C display uses 0 extra |
| ***Fully loaded*** | ***~29-30*** | 11+ GPIO still free |

## Power

| Consumer | Voltage | Current | Source |
|----------|---------|---------|--------|
| Stepper motor + TMC2209 | 24V | 1-2A typical, 2A peak | 24V 6A power supply |
| ESP32-S3 | 5V via VIN | ~130-200 mA | LM2596 buck converter from 24V |
| CAN transceiver | 3.3V | ~10 mA | ESP32 3.3V rail |
| DJI gimbal | Own battery | — | Self-powered |
| Display (optional) | 3.3V | ~20 mA | ESP32 3.3V rail |

24V is required for stepper torque at speed — 12V causes severe dropoff above 300 RPM. For portable use, a 6S LiPo (22.2V, 1300-2600 mAh) feeds the stepper directly + buck converter for ESP32.

## Not Included

- Camera and lens
- Tripod / support
- DJI gimbal CAN adapter cable (may need custom connector)
- 3D printed parts (motor mount, idler mount, carriage plate, end brackets)
