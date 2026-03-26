# DJI RS 5 RSA Port Wiring

## Physical Layout

Looking into the RSA port on the gimbal:

```
               ┌─────┐
               │ 9 10│
┌──────────────┼─────┤
│ 5  6  7  8   │
│ 1  2  3  4   │
└──────────────┘
```

10 pins total: 4x2 grid + 2 above the right side.

## Pinout

| Pin | Signal | Description | Voltage (powered) | Status |
|-----|--------|-------------|-------------------|--------|
| 1 | VCC | Power output | 9V | **Confirmed** — activated when AD_COM pulled down |
| 2 | SBUS_RX | SBUS input | 0V | **Likely** — 0V flat line, consistent with undriven input |
| 3 | AD_COM | Accessory detect | — | **Confirmed** — 100k pull-down to GND triggers power output |
| 4 | GND | Ground | 0V | **Confirmed** — 0Ω continuity to chassis (gimbal off) |
| 5 | VCC | Power output | 9V | **Confirmed** — duplicate of pin 1 |
| 6 | CANL | CAN Low | 1.76V | **Confirmed** — oscilloscope shows CAN traffic (inverse of pin 7) |
| 7 | CANH | CAN High | 3.23V | **Confirmed** — oscilloscope shows CAN traffic |
| 8 | GND | Ground | 0V | **Confirmed** — 0Ω continuity to chassis (gimbal off) |
| 9 | Unknown | — | ~0V flat | Not identified. No signal observed on oscilloscope. |
| 10 | Unknown | — | ~0V flat | Not identified. No signal observed on oscilloscope. |

## Notes

- **VCC is 9V**, not the 8V ± 0.4V documented for the RS 2. Either the RS 5 raised the voltage or there's tolerance variation.
- **AD_COM is pin 3**, not in the same position as the RS 2's 6-pin layout. The RS 5 rearranged the pins.
- **GND is doubled** on pins 4 and 8 (right column). **VCC is doubled** on pins 1 and 5 (left column).
- **CAN bus** (pins 6 and 7, center-top) confirmed active with oscilloscope. CANH idles at ~3.23V, CANL at ~1.76V. CAN traffic observed without sending any commands — gimbal pushes data.
- **Pins 9 and 10** are new to the RS 5. Both read ~0V with no oscilloscope activity. Possibly reserved, unused, or inputs waiting for data.
- **SBUS_RX** (pin 2) identified by elimination — 0V flat line, no activity. Needs functional test (send SBUS signal) to fully confirm.

## CAN Bus Parameters

| Parameter | RS 2 (documented) | RS 5 (measured) |
|-----------|-------------------|-----------------|
| Baud rate | 1 Mbps | 1 Mbps (confirmed — TWAI driver connected at 1M) |
| Gimbal TX ID | 0x222 | **0x426** |
| Gimbal RX ID | 0x223 | **0x427** (assumed, untested) |
| Heartbeat data | N/A | `55 45 04 DE E5 06 69 4B` (constant, ~fast rate) |
| SDK frame header | 0xAA | TBD — heartbeat uses 0x55, SDK frames may still use 0xAA |

**The RS 5 uses different CAN IDs than the RS 2.** The R SDK v2.5 documents CAN TX=0x222, RX=0x223 for the RS 2. The RS 5 broadcasts on **0x426**. The heartbeat frame starts with `0x55` (not `0xAA`), suggesting it may be a separate keep-alive layer. SDK protocol frames (starting with `0xAA`) may appear once we send an enable-push command on the correct TX ID.

## CAN Bus Connection

Tested working wiring:

```
RS 5 RSA Port          SN65HVD230           ESP32-S3
─────────────          ──────────           ────────
Pin 7 (CANH)  ──────── CANH
Pin 6 (CANL)  ──────── CANL
Pin 4 (GND)   ──────── GND ──────────────── GND
                       3V3 ──────────────── 3V3
                       CTX ──────────────── GPIO 5
                       CRX ──────────────── GPIO 6
Pin 3 (AD_COM)─── 100k resistor ───┐
Pin 4 (GND)   ─────────────────────┘
```

Power (pin 1/5, 9V) available for accessories but needs regulation for 3.3V/5V logic. Do NOT connect VCC to ESP32 or transceiver — they run on USB/3.3V.

## Test Procedure

See [rsa-pinout-test-procedure.md](rsa-pinout-test-procedure.md) for the full verification methodology.

## Source

- [R SDK v2.5 protocol spec](../../external/dji-r-sdk/docs/DJI_R_SDK_Protocol_and_User_Interface_EN_v2.5.md#312-ronin-series-accessories-rsanato-ports) — RS 2 6-pin pinout (original reference)
- Physical testing performed on DJI RS 5, 2026-03-25
