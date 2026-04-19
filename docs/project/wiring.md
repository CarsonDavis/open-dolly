# OpenDolly Wiring

*Status: active*
*Scope: Every wire in the current build (slider axis + DJI RS 5 gimbal over CAN). Pinouts, diagrams, and connection tables — a reference document, not a bring-up procedure.*

**Subsystems covered:**

1. Power distribution (24 V PSU, LM2596 buck, rails)
2. ESP32-S3
3. Stepper motor driver (BTT TMC2209 V1.3)
4. Stepper motor (NEMA 17)
5. Limit switches (KW12-3, NC fail-safe)
6. DJI RS 5 gimbal via CAN bus (SN65HVD230 transceiver)

**Legend (applies to all diagrams):**

| Color       | Meaning                           |
|-------------|-----------------------------------|
| Red         | 24 V (motor power bus)            |
| Orange      | 5 V (ESP32 supply)                |
| Yellow      | 3.3 V (logic reference)           |
| Dark grey   | GND (single common node)          |
| Blue        | Digital signal (ESP32 GPIO)       |
| Purple      | Motor coil conductor              |
| Green       | CAN differential pair             |
| Dashed grey | Intentionally unconnected / floating |

---

## 1. Overview

### 1.1 System block diagram

```mermaid
flowchart LR
    classDef v24  fill:#e74c3c,color:#fff,stroke:#922,stroke-width:2px
    classDef v5   fill:#e67e22,color:#fff,stroke:#630,stroke-width:2px
    classDef v33  fill:#f1c40f,color:#000,stroke:#860,stroke-width:2px
    classDef sig  fill:#3498db,color:#fff,stroke:#124,stroke-width:2px
    classDef can  fill:#27ae60,color:#fff,stroke:#063,stroke-width:2px
    classDef dev  fill:#34495e,color:#fff,stroke:#000,stroke-width:2px

    PSU["24 V PSU<br/>(wall brick)"]:::v24
    LM["LM2596 buck<br/>(set to 5.0 V)"]:::v5
    ESP["ESP32-S3"]:::dev
    TMC["TMC2209 V1.3"]:::dev
    MOT["NEMA 17"]:::dev
    SW_MIN["Limit MIN<br/>(KW12-3)"]:::dev
    SW_MAX["Limit MAX<br/>(KW12-3)"]:::dev
    XCVR["SN65HVD230<br/>CAN transceiver"]:::dev
    GIM["DJI RS 5<br/>RSA port"]:::dev

    PSU  -->|"24 V"| LM
    PSU  -->|"24 V (VM)"| TMC
    LM   -->|"5 V"| ESP
    ESP  -->|"3.3 V (VIO)"| TMC
    ESP  -->|"STEP / DIR / EN"| TMC
    TMC  -->|"coils A & B"| MOT
    ESP  -->|"GPIO 7"| SW_MIN
    ESP  -->|"GPIO 8"| SW_MAX
    ESP  -->|"3.3 V + CAN_TX / CAN_RX"| XCVR
    XCVR -->|"CANH / CANL"| GIM
```

### 1.2 ESP32-S3 GPIO map (current build)

Only pins that carry real signals are listed. Source of truth: [`firmware/src/config.h`](../../firmware/src/config.h).

| GPIO | Direction | Net                   | Wired to                                      |
|-----:|-----------|-----------------------|-----------------------------------------------|
|    5 | out       | `PIN_CAN_TX`          | SN65HVD230 `CTX`                              |
|    6 | in        | `PIN_CAN_RX`          | SN65HVD230 `CRX`                              |
|    7 | in (intr) | `PIN_LIMIT_MIN`       | MIN microswitch `C` (+ 10 kΩ pulldown to GND) |
|    8 | in (intr) | `PIN_LIMIT_MAX`       | MAX microswitch `C` (+ 10 kΩ pulldown to GND) |
|   15 | out       | `PIN_STEP`            | TMC2209 `STEP`                                |
|   16 | out       | `PIN_DIR`             | TMC2209 `DIR`                                 |
|   17 | out       | `PIN_EN`              | TMC2209 `EN` (active LOW)                     |
|  3V3 | out       | 3.3 V logic reference | 3.3 V mini-bus (TMC2209 VIO/MS1/MS2, SN65HVD230 3V3, limit switches NC tabs) |
|   5V | in        | 5 V supply            | LM2596 `OUT+`                                 |
|  GND | —         | common GND            | LM2596 `OUT−`, TMC2209 both `GND` pins, SN65HVD230 `GND`, pulldown resistors |

Pin 18 (`PIN_TMC_UART_*`) is defined in firmware but not wired in the current build (Phase 1 uses STEP/DIR only).

### 1.3 Global wiring constraints

These apply to the whole system, not any one subsystem:

- **VM ≠ VIO.** The TMC2209 has a 24 V motor rail (`VM`/`VS`) and a 3.3 V logic rail (`VIO`/`VDD`) on the same pin-header side. Swapping them destroys the driver and usually the ESP32 with it.
- **Single common GND.** Every subsystem's ground is the same electrical node — PSU `−`, LM2596 `OUT−`, ESP32 `GND`, both TMC2209 `GND` pins, SN65HVD230 `GND`, limit-switch pulldowns, and RSA pin 4 (and 8) all tie together.
- **Bulk capacitor on VM.** The BTT V1.3 has one on-board; if a different driver variant is substituted, add a 100–220 µF electrolytic across VM and GND close to the driver.
- **Do not connect RSA pin 1/5 (VCC, 9 V) to anything in this harness.** The gimbal exposes 9 V on those pins; the ESP32 and transceiver run on 5 V / 3.3 V from the LM2596 and would be damaged.

---

## 2. Power distribution

```mermaid
flowchart LR
    classDef v24 fill:#e74c3c,color:#fff,stroke:#922,stroke-width:2px
    classDef v5  fill:#e67e22,color:#fff,stroke:#630,stroke-width:2px
    classDef v33 fill:#f1c40f,color:#000,stroke:#860,stroke-width:2px
    classDef gnd fill:#2c3e50,color:#fff,stroke:#000,stroke-width:2px

    PSU["24 V PSU<br/>(5.5 × 2.1 barrel)"]

    subgraph BB["Breadboard rails"]
        direction TB
        R24(["top-red rail — 24 V"]):::v24
        RG_T(["top-black rail — GND"]):::gnd
        R5(["bot-red rail — 5 V"]):::v5
        RG_B(["bot-black rail — GND"]):::gnd
        R33(["mini-bus row — 3.3 V"]):::v33
    end

    LM["LM2596<br/>(output set to 5.0 V)"]
    ESP["ESP32-S3"]
    TMC["TMC2209 V1.3"]
    XCVR["SN65HVD230"]

    PSU -->|"+ output"| R24
    PSU -->|"− output"| RG_T

    R24 -->|"IN+"| LM
    RG_T -->|"IN−"| LM
    LM  -->|"OUT+"| R5
    LM  -->|"OUT−"| RG_B

    RG_T <-->|"GND bridge jumper"| RG_B

    R5   -->|"5 V pin"| ESP
    RG_B -->|"GND pin"| ESP
    ESP  -->|"3V3 pin (source)"| R33

    R24  -->|"VM / VS"| TMC
    RG_T -->|"GND (top-right col)"| TMC
    R33  -->|"VIO / VDD"| TMC
    RG_B -->|"GND (bottom-right col)"| TMC

    R33  -->|"3V3"| XCVR
    RG_B -->|"GND"| XCVR
```

The 3.3 V mini-bus is a single un-used breadboard row fed from the ESP32's on-board `3V3` regulator. Everything needing 3.3 V (TMC2209 `VIO`, `MS1`, `MS2`; limit-switch `NC` tabs; SN65HVD230 `3V3`) lands on this row.

---

## 3. Stepper motor driver (BTT TMC2209 V1.3)

Pin reference image: [`slider-wiring-images/tmc2209-v1.3-step-dir-pinout.jpg`](slider-wiring-images/tmc2209-v1.3-step-dir-pinout.jpg). All TMC2209 I/O is through **header pins** — this board has no screw terminals.

- **Left column (top → bottom):** `EN, MS1, MS2, PDN, PDN, CLK, STEP, DIR`
- **Right column (top → bottom):** `VM, GND, A2, A1, B1, B2, VDD, GND`

### 3.1 Driver power + microstep configuration

```mermaid
flowchart LR
    classDef v24    fill:#e74c3c,color:#fff,stroke:#922,stroke-width:2px
    classDef v33    fill:#f1c40f,color:#000,stroke:#860,stroke-width:2px
    classDef gnd    fill:#2c3e50,color:#fff,stroke:#000,stroke-width:2px
    classDef unused fill:#ecf0f1,color:#7f8c8d,stroke:#bbb,stroke-dasharray:5 5

    R24["24 V rail"]:::v24
    R33["3.3 V mini-bus"]:::v33
    GND["GND"]:::gnd

    subgraph TMC["TMC2209 right column (top → bottom)"]
        direction TB
        T_VM["VM / VS"]:::v24
        T_GND_T["GND"]:::gnd
        T_A2["A2  (motor — see §4)"]
        T_A1["A1  (motor — see §4)"]
        T_B1["B1  (motor — see §4)"]
        T_B2["B2  (motor — see §4)"]
        T_VIO["VIO / VDD"]:::v33
        T_GND_B["GND"]:::gnd
    end

    subgraph MSPINS["TMC2209 left column — microstep config"]
        direction TB
        T_MS1["MS1"]:::v33
        T_MS2["MS2"]:::v33
        T_PDN1["PDN — floating"]:::unused
        T_PDN2["PDN — floating"]:::unused
        T_CLK["CLK — floating"]:::unused
    end

    R24 --> T_VM
    GND --> T_GND_T
    R33 --> T_VIO
    GND --> T_GND_B
    R33 -->|"HIGH → 1/16 microstep"| T_MS1
    R33 -->|"HIGH → 1/16 microstep"| T_MS2
```

`MS1` = `MS2` = HIGH selects 1/16 microstepping, which matches `STEPS_PER_MM = 80` in [`firmware/src/stepper.h`](../../firmware/src/stepper.h) (20-tooth GT2 pulley, 200 full-steps/rev). Any other microstep setting requires firmware recalibration.

The 5-pin top-edge UART breakout (if present on your board revision) and the bottom-side `SPREAD` pad are both left untouched — the factory default is StealthChop, which is what we want.

### 3.2 STEP / DIR / EN signals

```mermaid
flowchart LR
    classDef sig fill:#3498db,color:#fff,stroke:#124,stroke-width:2px

    subgraph ESP["ESP32-S3"]
        direction TB
        E15["GPIO 15 — STEP"]:::sig
        E16["GPIO 16 — DIR"]:::sig
        E17["GPIO 17 — EN"]:::sig
    end

    subgraph TMC["TMC2209 left column (bottom of column)"]
        direction TB
        T_EN["EN (active LOW)"]:::sig
        T_STEP["STEP"]:::sig
        T_DIR["DIR"]:::sig
    end

    E15 -->|"step pulse"| T_STEP
    E16 -->|"direction"| T_DIR
    E17 -->|"enable"| T_EN
```

---

## 4. Stepper motor (NEMA 17)

### 4.1 Coil-to-driver wiring

The BTT V1.3 groups the motor pins as **A2 + A1 = coil A (phase A)** and **B1 + B2 = coil B (phase B)**. (Confirmed from BigTreeTech wiki: *"3(A2) Phase A, 4(A1) Phase A, 5(B1) Phase B, 6(B2) Phase B"*; and the Trinamic chip datasheet: *"the stepper is connected with one phase from OA1 to OA2 and the other on OB1 to OB2"*.)

```mermaid
flowchart LR
    classDef motor fill:#9b59b6,color:#fff,stroke:#606,stroke-width:2px

    subgraph TMC["TMC2209 right column (motor pins, top → bottom)"]
        direction TB
        T_A2["A2  — phase A"]:::motor
        T_A1["A1  — phase A"]:::motor
        T_B1["B1  — phase B"]:::motor
        T_B2["B2  — phase B"]:::motor
    end

    subgraph MOT["NEMA 17 (coil pairs identified by Ω-meter — see §4.2)"]
        direction TB
        M_A1["coil A — wire 1"]:::motor
        M_A2["coil A — wire 2"]:::motor
        M_B1["coil B — wire 1"]:::motor
        M_B2["coil B — wire 2"]:::motor
    end

    T_A2 --- M_A1
    T_A1 --- M_A2
    T_B1 --- M_B1
    T_B2 --- M_B2
```

Rules:

- **Both wires of one coil go into one letter-group.** Coil A into `A2`+`A1`; coil B into `B1`+`B2`. Crossing a wire between groups (e.g. a coil-A wire into `B1`) causes the motor to hum without rotating and overheats the driver.
- **Order within a group only affects direction.** Swapping `A2` ↔ `A1` at the driver (both wires of coil A, but reversed) reverses rotation. Safe to swap there or to flip the direction sign in firmware — pick one, not both.

### 4.2 Identifying coil pairs

A coil is a length of wire, so its two ends read **1–5 Ω** through a multimeter on resistance mode. Wires from *different* coils have no electrical path between them — meter reads **OL / infinity / open**. Test every pair combination until you find the two pairs; those are your two coils.

### 4.3 Motor-plug pinout (A-B-A-B vs A-A-B-B)

Many pre-terminated NEMA 17 cables use a 4-pin connector (XH2.54 or similar) wired as **A+, B+, A−, B−** (interleaved — A-B-A-B). The TMC2209 V1.3 header groups the motor pins as **A2, A1, B1, B2** (both phase-A pins adjacent, then both phase-B — A-A-B-B). These do not match. If a cable wired A-B-A-B is pushed directly onto the driver header, each H-bridge drives one wire of coil A against one wire of coil B — the "hum, no rotation, driver overheats" failure.

Three fixes:

1. **Re-pin the plug housing.** Release the pin retention tabs, slide pins out, reinsert in A-A-B-B order.
2. **Cut the plug off, use four F-F jumpers** landed on the four header pins directly.
3. **Build a crossover pigtail** — 4-pin female matching the motor plug → 4 wires reordered → 4-pin female matching the driver header.

Before doing any of the above, verify the cable order by probing the connector's pins with the Ω-meter (§4.2). Wire *colors* are not reliable — the coil mapping lives in the *pin positions* of the connector.

---

## 5. Limit switches (KW12-3, NC fail-safe)

Both switches wire identically; only the ESP32 GPIO differs. The KW12-3 has three solder tabs: `C` (common, middle), `NC` (normally closed), `NO` (normally open — unused, tape off).

```mermaid
flowchart LR
    classDef v33    fill:#f1c40f,color:#000,stroke:#860,stroke-width:2px
    classDef gnd    fill:#2c3e50,color:#fff,stroke:#000,stroke-width:2px
    classDef sig    fill:#3498db,color:#fff,stroke:#124,stroke-width:2px
    classDef unused fill:#ecf0f1,color:#7f8c8d,stroke:#bbb,stroke-dasharray:5 5

    BUS33["3.3 V mini-bus"]:::v33
    GPIO["ESP32 GPIO<br/>(MIN → GPIO 7, MAX → GPIO 8)"]:::sig
    GND["GND"]:::gnd

    subgraph SW["KW12-3 SPDT microswitch"]
        direction TB
        SW_NC["NC tab"]
        SW_C["C tab"]
        SW_NO["NO tab — taped off, unused"]:::unused
    end

    R10k["10 kΩ pulldown"]

    BUS33 --> SW_NC
    SW_NC -->|"closed at rest<br/>opens when lever is pressed"| SW_C
    SW_C  --> GPIO
    SW_C  --> R10k
    R10k  --> GND
```

State table:

| State                                  | C–NC contact | GPIO reads         | Firmware behavior              |
|----------------------------------------|--------------|--------------------|--------------------------------|
| Carriage clear of switch               | closed       | HIGH (3V3 via NC)  | Idle / motion allowed          |
| Carriage hits switch                   | open         | LOW (10 kΩ pulls)  | `FALLING` interrupt → motor cut |
| Wire breaks / connector falls off      | open         | LOW (10 kΩ pulls)  | Same as pressed → motor cut (fail-safe) |

Firmware coupling: [`firmware/src/stepper.cpp`](../../firmware/src/stepper.cpp) configures both GPIOs as `INPUT_PULLDOWN` with a `FALLING` interrupt. The external 10 kΩ supplements the internal pulldown (belt + suspenders — the external resistor gives a defined pulldown even if the MCU's internal pulls change between revisions).

---

## 6. DJI RS 5 gimbal (CAN bus)

The gimbal is powered by its own battery. The only connection between the slider's control board and the gimbal is a CAN bus through the gimbal's **RSA port** (10-pin pogo-pin connector on the top handle).

### 6.1 RSA port pinout

Looking into the RSA socket on the gimbal:

```
               ┌─────┐
               │ 9 10│
┌──────────────┼─────┤
│ 5  6  7  8   │
│ 1  2  3  4   │
└──────────────┘
```

| Pin | Signal     | Voltage (idle) | Notes |
|----:|------------|----------------|-------|
|   1 | **VCC**    | 9 V            | Power out; only active when AD_COM is pulled down. **Do not connect to this harness.** |
|   2 | SBUS_RX    | 0 V            | SBUS input (not used for CAN control) |
|   3 | **AD_COM** | —              | Accessory detect; 100 kΩ → GND enables power rails and CAN (see §6.3) |
|   4 | **GND**    | 0 V            | |
|   5 | **VCC**    | 9 V            | Duplicate of pin 1. Not used. |
|   6 | **CANL**   | ~1.76 V        | CAN low; oscilloscope-verified traffic |
|   7 | **CANH**   | ~3.23 V        | CAN high; oscilloscope-verified traffic |
|   8 | **GND**    | 0 V            | Duplicate of pin 4 |
|   9 | Unknown    | ~0 V           | New in RS 5, no observed activity |
|  10 | Unknown    | ~0 V           | New in RS 5, no observed activity |

Pins 1/5 (VCC 9 V), 2 (SBUS_RX), 9, and 10 are unused in the current build. Only pins **3, 4, 6, 7** are connected to the harness.

Detailed verification methodology and scope traces: [`../research/dji-rs5-ports-connectors/wiring.md`](../research/dji-rs5-ports-connectors/wiring.md) and [`../research/dji-rs5-ports-connectors/rsa-pinout-test-procedure.md`](../research/dji-rs5-ports-connectors/rsa-pinout-test-procedure.md).

### 6.2 CAN bus wiring

```mermaid
flowchart LR
    classDef v33  fill:#f1c40f,color:#000,stroke:#860,stroke-width:2px
    classDef gnd  fill:#2c3e50,color:#fff,stroke:#000,stroke-width:2px
    classDef sig  fill:#3498db,color:#fff,stroke:#124,stroke-width:2px
    classDef can  fill:#27ae60,color:#fff,stroke:#063,stroke-width:2px

    subgraph ESP["ESP32-S3"]
        direction TB
        E33["3V3"]:::v33
        E5["GPIO 5 — CAN_TX"]:::sig
        E6["GPIO 6 — CAN_RX"]:::sig
        EG["GND"]:::gnd
    end

    subgraph XCVR["SN65HVD230 CAN transceiver"]
        direction TB
        X_3V3["3V3"]:::v33
        X_GND["GND"]:::gnd
        X_CTX["CTX"]:::sig
        X_CRX["CRX"]:::sig
        X_CANH["CANH"]:::can
        X_CANL["CANL"]:::can
    end

    subgraph RSA["RS 5 RSA port"]
        direction TB
        P3["Pin 3 — AD_COM"]
        P4["Pin 4 — GND"]:::gnd
        P6["Pin 6 — CANL"]:::can
        P7["Pin 7 — CANH"]:::can
    end

    R100k["100 kΩ pulldown<br/>(AD_COM → GND)"]

    E33 --> X_3V3
    EG  --> X_GND
    E5  --> X_CTX
    X_CRX --> E6

    X_CANH --- P7
    X_CANL --- P6
    X_GND  --- P4

    P3 --> R100k
    R100k --> P4
```

### 6.3 AD_COM activation

The gimbal's RSA port stays in a passive state (pins 1/5 read 0 V, CAN not driven at full strength) until an accessory is detected. Detection is a built-in 100 kΩ pull-up on pin 3 inside the gimbal — when an external 100 kΩ resistor between pin 3 and pin 4 (GND) forms a voltage divider, the gimbal interprets it as an attached accessory and enables the 9 V power rails and CAN output.

Implementation: a single 100 kΩ resistor wired **between RSA pin 3 and pin 4 on the cable side**. No connection to the ESP32 or transceiver — it's a passive detector the gimbal watches for.

### 6.4 CAN bus parameters

Once the transceiver is wired and AD_COM is pulled low, the bus operates at:

| Parameter             | Value                                                                 |
|-----------------------|-----------------------------------------------------------------------|
| Baud rate             | **1 Mbps** (confirmed on RS 5)                                        |
| SDK TX ID (to gimbal) | `0x223` (same as RS 2)                                                |
| SDK RX ID (from gimbal) | `0x222` (same as RS 2)                                              |
| SDK frame header      | `0xAA`                                                                |
| Heartbeat ID          | `0x426` (new on RS 5; `55 45 04 DE E5 06 xx 54` + 56-byte pad, ~1 Hz) |

These are protocol values, not wiring, but they're useful to have in one place alongside the physical layer. The R SDK framing is handled by [`firmware/lib/dji_can/`](../../firmware/lib/dji_can/); protocol spec at [`docs/external/dji-r-sdk/`](../external/dji-r-sdk/INDEX.md).

---

## References

- [`firmware/src/config.h`](../../firmware/src/config.h) — authoritative GPIO pin definitions
- [`firmware/src/stepper.cpp`](../../firmware/src/stepper.cpp) — limit-switch pin modes and interrupt configuration
- [`docs/project/slider-wiring-images/README.md`](slider-wiring-images/README.md) — BTT TMC2209 V1.3 vendor pinout + microstep table
- [`docs/research/dji-rs5-ports-connectors/wiring.md`](../research/dji-rs5-ports-connectors/wiring.md) — RSA port verification (scope traces, voltage measurements)
- [`docs/research/dji-rs5-ports-connectors/rsa-pinout-test-procedure.md`](../research/dji-rs5-ports-connectors/rsa-pinout-test-procedure.md) — how the RSA pinout was verified
- [`docs/project/bom.md`](bom.md) — parts, prices, sources
