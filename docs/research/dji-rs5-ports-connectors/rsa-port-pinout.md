# DJI RS 5 RSA Port Pinout (Assumed)

**Status:** draft — pins 7-8 are unverified guesses. Needs physical confirmation.

## Background

The DJI R SDK v2.5 [External Interface Diagram](https://terra-1-g.djicdn.com/851d20f7b9f64838a34cd02351370894/Ronin%E7%B3%BB%E5%88%97/External%20interface%20diagram.pdf) documents a **6-pin** RSA/NATO port for the DJI RS 2. The RS 5 RSA port has **8 pins** — the original 6 plus 2 undocumented additions.

This document maps what we know and what we're guessing.

## Source: RS 2 RSA/NATO Port (6-pin, documented)

From the External Interface Diagram (R SDK v2.5, June 2021):

```
        ┌───────────┐
        │  5     6  │
        │  1     3  │
        │  1     2  │
        │  4     6  │
        └───────────┘
    (pin layout as shown in diagram)
```

| Pin | Signal   | Description      | Notes |
|-----|----------|------------------|-------|
| 1   | VCC      | Power output     | 8V ± 0.4V, rated 0.8A, peak 1.2A |
| 2   | CANL     | CAN Low          | 1 Mbps baud rate |
| 3   | SBUS_RX  | SBUS input       | Inverted serial, 100k baud, 25 bytes/frame |
| 4   | CANH     | CAN High         | 1 Mbps baud rate |
| 5   | AD_COM   | Accessory detect | Built-in 100k pull-up. Needs 10-100k pull-down resistor. Port will NOT output power unless accessory is detected. |
| 6   | GND      | Ground           | |

## RS 5 RSA Port (8-pin, assumed)

**Assumption:** The RS 5 retains pins 1-6 in the same positions and adds 2 new pins (7-8). The Focus Wheel and Tethered Control Handle still work with the RS 5 ([accessories compatibility list](https://support.dji.com/help/content?customId=en-us03400007633)), which strongly suggests backward compatibility.

| Pin | Signal   | Description      | Status | Notes |
|-----|----------|------------------|--------|-------|
| 1   | VCC      | Power output     | **Confirmed** (RS 2 doc) | 8V ± 0.4V, rated 0.8A, peak 1.2A |
| 2   | CANL     | CAN Low          | **Confirmed** (RS 2 doc) | 1 Mbps baud rate |
| 3   | SBUS_RX  | SBUS input       | **Confirmed** (RS 2 doc) | |
| 4   | CANH     | CAN High         | **Confirmed** (RS 2 doc) | 1 Mbps baud rate |
| 5   | AD_COM   | Accessory detect | **Confirmed** (RS 2 doc) | Needs pull-down to enable power |
| 6   | GND      | Ground           | **Confirmed** (RS 2 doc) | |
| 7   | ???      | Unknown          | **Unverified** | New on RS 5 |
| 8   | ???      | Unknown          | **Unverified** | New on RS 5 |

### Speculation on pins 7-8

Possible functions based on new RS 5 features and accessories:

- **USB data (D+/D-)** — The RS 5 introduced the Electronic Briefcase Handle (RS 5 only) and Enhanced Intelligent Tracking Module. These may need higher bandwidth than CAN bus provides.
- **SBUS_TX** — A return SBUS channel for bidirectional communication.
- **Additional power rail** — Different voltage for new accessories (e.g., 5V for USB devices alongside the existing 8V).
- **I2C (SDA/SCL)** — Low-speed data bus for simple sensors or configuration.
- **Reserved/NC** — May be physically present but not yet assigned.

No way to narrow this down without physical measurement or updated documentation from DJI.

## Connection Method

The R SDK v2.5 shows the canonical connection path:

```
DJI RS gimbal
    └─ RSA port (pogo pins)
        └─ DJI Focus Wheel (or custom connector)
            └─ Cable: VCC_5V, CANH, CANL, GND
                └─ CAN adapter (e.g., USB-CAN)
                    └─ PC / microcontroller
```

### Known connectors that mate with the RSA port

| Connector | Source | Notes |
|-----------|--------|-------|
| Riley Harmon 3D-printed housing + Precidip 813-S1-008-10-014101 SLC | [GitHub](https://github.com/rileyharmon/DJI-Ronin-RS2-Log-and-Replay/blob/main/3d-print-ronin-can-connector.stl) | Designed for RS 2 (6-pin). STL only, CC BY-NC 4.0. Would need modification for RS 5's 8 pins. |
| Middle Things APC-R Gimbal Adapter | [Store](https://store.middlethings.co/products/gimbal-adapter) | Commercial, ~$300. Claims RS 5 compatibility. |
| eMotimo RSI + 6-pin cable | [eMotimo](https://emotimo.com/pages/dji-rs-gimbals) | 6-pin, RS 2/3/4 only. RS 5 not listed. |
| DJI Focus Wheel (harvest connector) | DJI Store | ~$150. Wasteful but guaranteed to fit. |

## References

- [DJI R SDK v2.5 Protocol and User Interface (PDF)](https://www.dji.com/rs-sdk) — Section 3, Figure 39
- [External Interface Diagram (PDF)](https://terra-1-g.djicdn.com/851d20f7b9f64838a34cd02351370894/Ronin%E7%B3%BB%E5%88%97/External%20interface%20diagram.pdf)
- [DJI RS 5 Store Page](https://store.dji.com/product/dji-rs-5) — Confirms RSA Port × 1, NATO Port × 2
- [DJI RS Series Accessories Compatibility List](https://support.dji.com/help/content?customId=en-us03400007633)
- [Riley Harmon RS2 CAN Connector](https://github.com/rileyharmon/DJI-Ronin-RS2-Log-and-Replay)
