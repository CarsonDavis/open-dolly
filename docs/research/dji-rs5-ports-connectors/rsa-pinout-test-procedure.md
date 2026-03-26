# RS 5 RSA Port Pinout Verification Procedure

**Status:** draft
**Goal:** Verify whether the RS 5 RSA port pinout matches the RS 2 6-pin pinout documented in the [R SDK v2.5](../../../docs/external/dji-r-sdk/docs/DJI_R_SDK_Protocol_and_User_Interface_EN_v2.5.md), and characterize any new pins.

## Equipment Needed

- DJI RS 5 gimbal (powered on, no camera mounted)
- Multimeter with continuity, DC voltage, and resistance modes
- Oscilloscope (optional but very helpful for CAN/SBUS identification)
- Breadboard + jumper wires
- 47k resistor (for AD_COM pull-down test)
- Pogo pin probe or fine-tipped multimeter probes that can reach RSA port contacts
- Notebook to record measurements

## Safety Notes

- Do NOT short pins together. Always probe one pin at a time against a known ground.
- The RSA port outputs up to 8V at 1.2A peak — enough to damage 3.3V logic if connected wrong.
- Power on the gimbal before probing. Some pins are only active when powered.

## Before You Start

Label the pins. Looking into the RSA port on the gimbal, sketch the physical contact positions and number them 1-8 (or however many contacts you see). Take a photo. Count them — we expect 8 based on the assumption that 2 were added to the original 6.

**Record the total pin count and physical arrangement before doing anything else.**

---

## Phase 1: Find GND (no pull-down needed)

GND should be identifiable without triggering the accessory detect circuit.

1. Power on the RS 5.
2. Set multimeter to **DC voltage** mode.
3. Touch the black probe to the gimbal body or battery negative terminal (chassis ground reference).
4. Touch the red probe to each pin one at a time.
5. Record the voltage for every pin.

**Expected results (based on RS 2 doc):**

| Pin | Expected voltage vs chassis | Why |
|-----|----------------------------|-----|
| GND (pin 6) | **0V** | It IS ground |
| AD_COM (pin 5) | ~3.3V or ~5V | Internal 100k pull-up to VCC, floating high with no accessory |
| VCC (pin 1) | **0V or floating** | Power output is disabled until AD_COM detects an accessory |
| CANH (pin 4) | ~2.5V | CAN recessive idle state |
| CANL (pin 2) | ~2.5V | CAN recessive idle state |
| SBUS_RX (pin 3) | ~0V or floating | Input pin, not driven by gimbal |
| Unknown (pin 7) | ? | Record whatever you see |
| Unknown (pin 8) | ? | Record whatever you see |

**Action:** Identify the pin(s) reading 0V with continuity to chassis. That's GND. There may be more than one GND pin (the RS 2 has pin 6 appear twice in the physical layout — top and bottom of one column).

---

## Phase 2: Find AD_COM and trigger power

The AD_COM pin has an internal 100k pull-up. Connecting a pull-down resistor should cause the port to enable power output on VCC.

1. Identify the pin reading ~3.3-5V from Phase 1 — this is likely AD_COM.
2. Connect a **47k resistor** between that pin and the confirmed GND pin.
3. Wait 1-2 seconds.
4. Re-measure all pins against GND.

**Expected results after pull-down:**

| Pin | Expected voltage vs GND | Why |
|-----|------------------------|-----|
| VCC (pin 1) | **~8V** (7.6-8.4V) | Power output now enabled by AD_COM detection |
| AD_COM (pin 5) | Drops to ~1-2V | Voltage divider: internal 100k pull-up + external 47k pull-down |
| CANH (pin 4) | ~2.5V | CAN bus idle (recessive) |
| CANL (pin 2) | ~2.5V | CAN bus idle (recessive) |
| SBUS_RX (pin 3) | ~0V or floating | Input, not driven |
| GND (pin 6) | 0V | Ground |

**Action:** Record which pin jumped to ~8V. That's VCC. Record AD_COM's new voltage.

---

## Phase 3: Confirm CAN bus pins

With power enabled (AD_COM pulled down), the CAN bus should be active.

### Multimeter method

1. Measure the voltage between the two pins you suspect are CANH and CANL.
2. In idle (recessive) state: CANH ≈ 2.5V, CANL ≈ 2.5V, differential ≈ 0V.
3. If the gimbal is sending periodic data (push telemetry), you may see the multimeter reading fluctuate slightly.

### Oscilloscope method (better)

1. Connect scope CH1 to suspected CANH, CH2 to suspected CANL, both referenced to GND.
2. Set timebase to 1µs/div, voltage to 1V/div.
3. You should see:
   - CANH: idle at 2.5V, pulses up to ~3.5V during dominant bits
   - CANL: idle at 2.5V, pulses down to ~1.5V during dominant bits
   - Traffic pattern: bursts of frames if the gimbal is pushing status data
4. If you enabled parameter push (or the gimbal pushes by default), you'll see periodic CAN frames.

**Action:** Confirm CANH and CANL. Record which physical pin position maps to which.

---

## Phase 4: Identify SBUS_RX

SBUS_RX is an **input** pin — the gimbal listens on it, it doesn't drive it. This makes it harder to identify passively.

1. It should read near 0V or float (high impedance) with no signal applied.
2. If you have an oscilloscope, check for any signal on the remaining unidentified pins. SBUS_RX should show no activity (it's an input waiting for data).
3. **Elimination method:** After identifying GND, VCC, AD_COM, CANH, and CANL, SBUS_RX is the remaining original pin that shows no voltage and no signal.

**Action:** Record which pin you believe is SBUS_RX by elimination.

---

## Phase 5: Characterize unknown pins (7, 8)

These are the pins not present on the RS 2. For each unknown pin:

### Voltage measurement
1. Measure DC voltage vs GND (with AD_COM pulled down so port is active).
2. Record the value.

### Resistance measurement
1. **Power off the gimbal.**
2. Measure resistance between each unknown pin and GND.
3. Measure resistance between each unknown pin and VCC.
4. High impedance (>1MΩ) suggests a data/signal line or unused pin.
5. Low impedance to GND might indicate another GND.
6. Low impedance to VCC might indicate another power rail.

### Oscilloscope check
1. Power the gimbal back on (with AD_COM pull-down).
2. Probe each unknown pin.
3. Look for:
   - **Steady DC voltage** (e.g., 3.3V, 5V) → likely a secondary power rail
   - **Digital signal** → protocol line (capture the waveform, note frequency/pattern)
   - **Nothing** → unused, reserved, or an input waiting for data

### Possible identities

| If you see... | Likely function |
|---------------|----------------|
| 5V steady | USB VBUS or secondary power rail |
| 3.3V steady | Logic-level power output |
| Differential pair (~1.5V and ~3.3V toggling) | USB D+/D- |
| ~0V, high impedance | Reserved/NC or an input pin |
| Matches GND | Additional ground pin |

---

## Recording Template

Fill this in during testing:

```
RS 5 RSA Port Physical Layout
==============================
Date: ___________
Gimbal FW version: ___________

Physical pin positions (sketch looking INTO the port on the gimbal):

     ┌───────────┐
     │  ?    ?   │
     │  ?    ?   │
     │  ?    ?   │
     │  ?    ?   │
     └───────────┘

Total contact points counted: ___

Phase 1 — Voltage vs chassis (power on, no pull-down):
  Pin 1: ___V    Pin 5: ___V
  Pin 2: ___V    Pin 6: ___V
  Pin 3: ___V    Pin 7: ___V
  Pin 4: ___V    Pin 8: ___V

GND identified at pin(s): ___

Phase 2 — After 47k pull-down on suspected AD_COM (pin ___):
  Pin 1: ___V    Pin 5: ___V
  Pin 2: ___V    Pin 6: ___V
  Pin 3: ___V    Pin 7: ___V
  Pin 4: ___V    Pin 8: ___V

VCC (~8V) identified at pin(s): ___
AD_COM confirmed at pin: ___

Phase 3 — CAN bus:
  Suspected CANH pin: ___  Voltage: ___V
  Suspected CANL pin: ___  Voltage: ___V
  Oscilloscope confirms CAN traffic: Y / N

Phase 4 — SBUS_RX:
  Identified by elimination at pin: ___

Phase 5 — Unknown pins:
  Pin 7: ___V DC, resistance to GND: ___Ω, scope: ___________
  Pin 8: ___V DC, resistance to GND: ___Ω, scope: ___________

Summary — RS 5 RSA Pinout:
  Pin 1: ___________
  Pin 2: ___________
  Pin 3: ___________
  Pin 4: ___________
  Pin 5: ___________
  Pin 6: ___________
  Pin 7: ___________
  Pin 8: ___________

Matches RS 2 for pins 1-6: Y / N
Notes: _________________________________
```

## What To Do With Results

- If pins 1-6 match the RS 2 doc exactly → update `rsa-port-pinout.md` with confirmed status
- If any pins 1-6 differ → document the difference and flag it in the forum post
- For pins 7-8 → add findings to `rsa-port-pinout.md` speculation section
- If CAN traffic is confirmed → proceed to send a test command (e.g., obtain gimbal information, CmdSet=0x0E CmdID=0x02) to verify the R SDK protocol still works on RS 5
