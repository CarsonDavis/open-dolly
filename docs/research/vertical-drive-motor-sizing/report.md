# Vertical Drive Motor Sizing & Brake Mechanisms

**Date:** 2026-03-20

## Recommendation

**Use a GT2 belt drive with a NEMA 17 stepper motor (17HS19-2004D-B070, 52 Ncm, 2.0A) that has an integrated 70 Ncm power-off electromagnetic brake.** This is a drop-in replacement for the previously recommended 17HS19-2004S, adding only 31mm of length and ~$18 in cost. The brake provides fail-safe gravity protection: if power is lost, the spring-engaged brake locks the shaft and holds the payload. During operation, the motor has sufficient torque to drive the 7 kg design load vertically at slow-to-moderate speeds with the brake electrically released.

For sustained vertical operation at higher speeds or reduced thermal load, add an optional constant-force spring counterbalance (~$15-30 in parts) to offset gravity. This is the same approach edelkrone uses in their $690 Vertical Module.

**Why not lead screws or worm gears?** Self-locking lead screws (T8x2) are limited to 3.5-14 mm/s -- unusable for video slides. Worm gears (31:1+) max out at 21-43 mm/s -- barely usable. The belt + brake combination preserves the slider's full speed range (500+ mm/s horizontal, 100+ mm/s vertical) while providing mechanical safety.

## 1. Payload Specification

| Component | Weight |
|-----------|--------|
| DJI RS 5 gimbal (with battery grip & QR plates) | 1,460g |
| Sony A7IV (with battery & card) | 658g |
| Sigma 28-105mm f/2.8 DG DN Art | 990g |
| **Total equipment** | **3,108g** |
| Carriage (plate, MGN12H bearings, hardware) | ~300-500g |
| **Total real payload** | **~3,500g (7.7 lb)** |
| **Design load (2x safety margin)** | **~7,000g (7.0 kg / 15.4 lb)** |

Commercial reference: the iFOOTAGE Shark Slider Nano II rates 15.4 lb horizontal and 7.7 lb vertical (50% derating). Our 3.5 kg real payload falls within the vertical rating of this class of slider.

## 2. Torque Calculations for Vertical Operation

### 2.1 Variables and Constants

| Symbol | Value | Description |
|--------|-------|-------------|
| m | 7.0 kg (design) / 3.5 kg (real) | Total moving mass |
| g | 9.81 m/s^2 | Gravitational acceleration |
| mu | 0.015 | MGN12 linear rail friction coefficient |
| eta_belt | 0.95 | GT2 belt drive efficiency |
| eta_screw | 0.30 (T8x2) / 0.90 (ball screw) | Screw drive efficiency |
| r_20T | 6.37 mm = 0.00637 m | GT2 20-tooth pulley radius |
| r_16T | 5.09 mm = 0.00509 m | GT2 16-tooth pulley radius |
| theta | 90 degrees | Vertical (worst case) |

### 2.2 Gravitational Force

At pure vertical (theta = 90 degrees):

```
F_gravity = m x g = 7.0 x 9.81 = 68.67 N  (design load)
F_gravity = m x g = 3.5 x 9.81 = 34.34 N  (real load)
```

Friction on MGN12 rail is negligible at vertical (acts perpendicular to travel).

### 2.3 Holding Torque (Stationary, Vertical)

The torque required just to hold the payload against gravity with the motor energized:

**GT2 Belt with 20-tooth pulley:**
```
T_hold = F_gravity x r / eta_belt
T_hold = 68.67 x 0.00637 / 0.95 = 0.460 Nm = 46.0 Ncm  (design load)
T_hold = 34.34 x 0.00637 / 0.95 = 0.230 Nm = 23.0 Ncm  (real load)
```

**GT2 Belt with 16-tooth pulley:**
```
T_hold = 68.67 x 0.00509 / 0.95 = 0.368 Nm = 36.8 Ncm  (design load)
T_hold = 34.34 x 0.00509 / 0.95 = 0.184 Nm = 18.4 Ncm  (real load)
```

### 2.4 Acceleration Torque (Moving Upward)

Additional torque for acceleration, reflected to motor shaft through belt:

```
T_accel = m x a x r / eta_belt
```

| Acceleration | Design Load (20T) | Design Load (16T) | Real Load (20T) | Real Load (16T) |
|-------------|-------------------|-------------------|-----------------|-----------------|
| 0.1g (0.981 m/s^2) | 4.6 Ncm | 3.7 Ncm | 2.3 Ncm | 1.8 Ncm |
| 0.2g (1.962 m/s^2) | 9.2 Ncm | 7.4 Ncm | 4.6 Ncm | 3.7 Ncm |
| 0.5g (4.905 m/s^2) | 23.0 Ncm | 18.4 Ncm | 11.5 Ncm | 9.2 Ncm |

### 2.5 Total Torque: Moving Upward at Various Accelerations

Total = Holding + Acceleration:

| Scenario | 20T Pulley | 16T Pulley | Motor Margin (59 Ncm) |
|----------|-----------|-----------|----------------------|
| **Design load, hold only** | 46.0 Ncm | 36.8 Ncm | 1.28x / 1.60x |
| **Design load, 0.1g accel up** | 50.6 Ncm | 40.5 Ncm | 1.17x / 1.46x |
| **Design load, 0.2g accel up** | 55.2 Ncm | 44.2 Ncm | 1.07x / 1.33x |
| **Design load, 0.5g accel up** | 69.0 Ncm | 55.2 Ncm | **STALL** / 1.07x |
| **Real load, hold only** | 23.0 Ncm | 18.4 Ncm | 2.57x / 3.21x |
| **Real load, 0.1g accel up** | 25.3 Ncm | 20.2 Ncm | 2.33x / 2.92x |
| **Real load, 0.2g accel up** | 27.6 Ncm | 22.1 Ncm | 2.14x / 2.67x |
| **Real load, 0.5g accel up** | 34.5 Ncm | 27.6 Ncm | 1.71x / 2.14x |

**Key findings:**
- At the **real payload** (3.5 kg), a 59 Ncm NEMA 17 with 20T pulley handles vertical operation with 2.5x margin at hold and 1.7x margin even at aggressive 0.5g acceleration. This is comfortable.
- At the **design load** (7 kg, 2x safety), hold-only barely works (1.28x) and any significant acceleration risks stalling.
- **Switching to a 16-tooth pulley** buys 25% more torque margin at the cost of 20% less top speed. This is a worthwhile trade for vertical operation.

### 2.6 Lead Screw Torque (T8x2, for reference)

For a T8x2 trapezoidal lead screw (2mm lead, ~6.5mm mean diameter, mu=0.15 polymer nut):

```
T_raise = (F x d_mean/2) x (lead + pi x mu x d_mean) / (pi x d_mean - mu x lead)
T_raise = (68.67 x 0.00325) x (0.002 + pi x 0.15 x 0.0065) / (pi x 0.0065 - 0.15 x 0.002)
T_raise = 0.2232 x (0.002 + 0.003063) / (0.02042 - 0.0003)
T_raise = 0.2232 x 0.005063 / 0.02012
T_raise = 0.0562 Nm = 5.6 Ncm  (design load)
```

Very low torque needed because the screw's mechanical advantage is enormous. But the self-locking friction that makes it safe also makes it slow and inefficient (only ~30% efficient).

**Speed limit: At 500mm screw length, max safe RPM is ~106 (rigid/free) to ~424 (fixed/fixed). At 2mm lead: max linear speed = 0.85-3.4 mm/s. Completely unusable for video.**

### 2.7 Ball Screw Torque (SFU1605, for reference)

For a 16mm ball screw with 5mm lead, ~90% efficiency:

```
T_drive = F x lead / (2 x pi x eta)
T_drive = 68.67 x 0.005 / (2 x pi x 0.90)
T_drive = 0.0607 Nm = 6.1 Ncm  (design load)
```

Low torque, higher speed than lead screw. But ball screws are NOT self-locking -- the load will back-drive and fall if the motor loses holding torque.

## 3. Self-Locking and Brake Analysis

The fundamental problem of vertical camera slider operation: **if the motor loses power, loses steps, or the driver faults, gravity pulls the payload down.** With a 3.5 kg camera rig, this means a $5,000+ equipment disaster. The drive mechanism must address this.

### 3.1 Self-Locking Options

| Mechanism | Self-Locking? | Max Speed (1m slider) | Torque Required | Cost | Verdict |
|-----------|:------------:|----------------------|----------------|------|---------|
| GT2 belt (direct) | NO | 500+ mm/s | 46 Ncm (hold) | $5-15 | Fast but unsafe |
| T8x2 lead screw | YES | 3.5-14 mm/s | 5.6 Ncm | $10-20 | Safe but unusable speed |
| T8x8 lead screw | NO | 14-56 mm/s | Varies | $10-20 | Too slow AND unsafe |
| SFU1605 ball screw | NO | 100-300 mm/s | 6.1 Ncm | $120-275 | Expensive and unsafe |
| 31:1 worm gear + belt | YES | 21-43 mm/s | ~2 Ncm | $25-50 | Safe but slow |
| 5:1 planetary + belt | NO | 67-133 mm/s | ~10 Ncm | $20-35 | Fast but unsafe |
| Belt + EM brake | YES (at stop) | 500+ mm/s | 46 Ncm (motor) | $29-40 total | Fast AND safe |

### 3.2 Self-Locking Criteria (Lead Screws)

A lead screw is self-locking when the lead angle is less than the friction angle:

```
Self-locking condition: lambda < phi
Where: lambda = arctan(lead / (pi x d_mean))
       phi = arctan(mu_friction)
```

| Screw | Lead | Lead Angle | Friction Angle (mu=0.15) | Self-Locking? | Efficiency |
|-------|------|-----------|-------------------------|:-------------:|-----------|
| T8x1 | 1mm | 2.8 deg | 8.5 deg | YES | ~18% |
| T8x2 | 2mm | 5.6 deg | 8.5 deg | YES | ~30% |
| T8x4 | 4mm | 11.1 deg | 8.5 deg | NO | ~50% |
| T8x8 | 8mm | 21.4 deg | 8.5 deg | NO | ~72% |
| SFU1605 | 5mm | 5.7 deg | ~2 deg (ball) | NO | ~90% |

### 3.3 Electromagnetic Brake Solution (Recommended)

**How it works:** A power-off (spring-engaged) brake clamps the motor shaft whenever 24V is not applied to the brake coil. This is fail-safe by design -- any power loss automatically engages the brake.

**Brake torque requirement for our application:**

With GT2 20-tooth belt, the torque the load exerts on the motor shaft through the belt:
```
T_gravity = m x g x r = 3.5 x 9.81 x 0.00637 = 0.219 Nm = 21.9 Ncm  (real load)
T_gravity = 7.0 x 9.81 x 0.00637 = 0.437 Nm = 43.7 Ncm  (design load)
```

The 70 Ncm brake on the 17HS19-2004D-B070 provides:
- **3.2x safety factor** on the real load (70 / 21.9)
- **1.6x safety factor** on the design load (70 / 43.7)

Both are adequate. The brake will hold the load even under design-load conditions.

**Operational sequence:**
1. Power on: brake remains engaged (safe by default)
2. Before moving: ESP32 energizes brake coil via MOSFET (24V, 3.7W) to release
3. Motor moves normally with brake released
4. When stopping: motor holds position, then de-energize brake coil to engage brake
5. Reduce motor current (or de-energize entirely) -- brake holds the load mechanically
6. On power loss / fault: brake engages automatically within milliseconds

**Cost impact:** $29 for motor+brake vs ~$12 for motor alone = **$17 premium** for complete fall protection. Trivial compared to the value of the camera gear.

### 3.4 Why Not Worm Gears?

Worm gears provide self-locking AND torque multiplication, but at a severe speed penalty:

| Worm Ratio | Max Linear Speed (motor at 1000 RPM) | Self-Locking? |
|-----------|--------------------------------------|:------------:|
| 17:1 | 39 mm/s | YES (marginal) |
| 31:1 | 21 mm/s | YES |
| 40:1 | 17 mm/s | YES |

For reference, a direct belt drive at 1000 RPM achieves 667 mm/s. Even slow cinematic slides typically run at 10-50 mm/s, which a 31:1+ worm gear can barely achieve. Fast repositioning between shots (the primary speed benefit of belt drives) is completely eliminated.

The Hirnschall DIY slider used a 40:1 worm gear and identified speed as the primary limitation. For a project that values both cinematic slides AND fast repositioning, worm gears are too compromising.

## 4. Motor Recommendations

### 4.1 Recommended: 17HS19-2004D-B070 (Belt + Brake)

| Spec | Value |
|------|-------|
| Motor holding torque | 52 Ncm (0.52 Nm) |
| Brake holding torque | 70 Ncm (0.70 Nm) |
| Rated current | 2.0A per phase |
| Phase resistance | 1.4 ohm |
| Inductance | 3.0 mH |
| Step angle | 1.8 deg (200 steps/rev) |
| Body size | 42 x 42 x 79 mm (motor + brake) |
| Brake voltage | 24V DC |
| Brake power | 3.7W |
| Brake type | Power-off (spring-engaged, electrically released) |
| Price | ~$29 |

This motor has slightly less motor holding torque (52 vs 59 Ncm) than the non-brake 17HS19-2004S1 because the dual-shaft version has different construction. At the real payload (3.5 kg), it provides 2.26x margin for vertical holding -- adequate for slow video slides but not for aggressive acceleration.

### 4.2 Upgrade Option: 17HS24-2004D-B070

For more aggressive vertical performance:

| Spec | Value |
|------|-------|
| Motor holding torque | 72 Ncm (0.72 Nm) |
| Brake holding torque | 70 Ncm (0.70 Nm) |
| Rated current | 2.0A per phase |
| Body size | 42 x 42 x 91 mm |
| Price | ~$35 |

This provides 3.1x margin at real payload for vertical holding, and handles moderate acceleration at full design load. The 12mm longer body is the trade-off.

### 4.3 Ultimate Option: 17E1KBK05-07 (Closed-Loop + Brake)

| Spec | Value |
|------|-------|
| Motor holding torque | 72 Ncm |
| Brake holding torque | 70 Ncm |
| Built-in encoder | 1000-line (4000 CPR) |
| Price | ~$50-60 |

Adds closed-loop step verification. The encoder detects missed steps in real time and corrects them before the payload drops. Combined with the brake, this provides two independent safety layers. The closed-loop control also reduces power consumption during holding by only applying as much current as the load requires, rather than full rated current.

### 4.4 Pulley Size Trade-off for Vertical

| Pulley | Holding Torque (real 3.5kg) | Motor Margin (52 Ncm) | Max Speed (1000 RPM) |
|--------|---------------------------|----------------------|---------------------|
| 20-tooth (6.37mm radius) | 23.0 Ncm | 2.26x | 667 mm/s |
| 16-tooth (5.09mm radius) | 18.4 Ncm | 2.83x | 533 mm/s |

**Recommendation for vertical use: switch to 16-tooth pulley.** The 25% torque margin improvement is worth the 20% speed reduction. At 533 mm/s top speed, the slider is still faster than any commercial competitor.

## 5. Counterbalance Analysis

### 5.1 The Edelkrone Approach

Edelkrone's $690 Vertical Module uses constant-force springs in a tension-wire system. This counterbalances the camera weight so the motor only provides acceleration force, not gravitational holding force. The result: the same small motor works for both horizontal and vertical operation.

### 5.2 DIY Constant-Force Spring Counterbalance

**Concept:** A constant-force spring (coiled steel strip, like a tape measure spring) attached to the carriage via a cable over a pulley. The spring pulls upward with a force matching the payload weight, neutralizing gravity.

**Sizing:**
- Real payload: 3.5 kg x 9.81 = 34.3 N
- Need constant-force springs totaling ~35N
- Available on Amazon/AliExpress: 10-50N springs for $5-15 each
- 1-2 springs in parallel for real payload

**Benefits:**
- Reduces vertical holding torque to near-zero (only rail friction remains)
- Reduces motor current draw from ~80% to ~5% when holding
- Reduces motor heat from ~11W to <1W
- Extends battery life dramatically for vertical time-lapses
- Motor can handle aggressive acceleration even vertically

**Drawbacks:**
- Must be tuned to the specific payload weight -- if you change camera/lens, you need to re-balance
- Adds mechanical complexity (cable routing, pulley, spring mount)
- Adds ~200-500g of weight to the slider assembly
- Spring can interfere with horizontal operation if not disengageable

**Recommendation:** Design the counterbalance as an optional bolt-on accessory, not a core requirement. The electromagnetic brake alone provides adequate safety. The counterbalance adds efficiency for users who do significant vertical work.

### 5.3 Other Counterbalance Approaches

| Method | Pros | Cons | Suitability |
|--------|------|------|-------------|
| Constant-force spring | Compact, constant force, cheap | Needs tuning per payload | Best for our project |
| Counterweight on cable | Simple, no tuning needed (weight = payload) | Doubles moving mass, needs pulley path | Poor (increases inertia) |
| Gas strut | Off-the-shelf, compact | Force varies with position, wears out | Marginal |
| Regular spring | Cheapest | Force varies linearly with extension | Poor |

## 6. Thermal Analysis

### 6.1 Holding a Vertical Load with Motor Current

Open-loop stepper motors apply full rated current regardless of load. For the 17HS19-2004D-B070 at 2.0A, 1.4 ohm:

```
P_hold = I^2 x R x 2 (phases) = 4.0 x 1.4 x 2 = 11.2W continuous
```

At 11.2W with ~12 C/W thermal resistance, motor temperature rises ~134C above ambient. This **will overheat** without heatsinking. However:

- Mounting to aluminum extrusion provides excellent heatsinking (reduces thermal resistance to ~5-7 C/W)
- TMC2209 StealthChop at standstill uses slightly more efficient current regulation
- In practice, aluminum-mounted NEMA 17 motors run 40-60C case temperature during continuous hold

### 6.2 Using the Brake to Eliminate Holding Heat

With the electromagnetic brake, the operational pattern changes:

1. **Moving:** Motor energized, brake released (3.7W brake coil power) -- motor generates heat normally
2. **Holding position:** Engage brake, then reduce motor current to zero -- **zero motor heat**
3. **Only the brake coil dissipates 3.7W** during the hold phase (and this is outside the motor body)

This is a massive thermal advantage. During a timelapse (long holds between moves), the motor stays cool because the brake does all the holding work.

### 6.3 Using Counterbalance to Reduce Holding Current

With a counterbalance offsetting gravity, the motor only needs to overcome rail friction:

```
F_friction = m x g x mu = 3.5 x 9.81 x 0.015 = 0.515 N
T_friction = F x r / eta = 0.515 x 0.00637 / 0.95 = 0.0035 Nm = 0.35 Ncm
```

This is <1% of the motor's capacity. The TMC2209's CoolStep could theoretically reduce current to near-zero, but CoolStep doesn't work at standstill. In practice, the firmware can manually reduce the holding current via IHOLD register to 10-25% of rated, which would drop power dissipation to:

```
P_reduced = (0.5A)^2 x 1.4 x 2 = 0.7W  (at 25% current)
```

## 7. What Commercial Vertical Sliders Do

| Brand/Model | Drive | Vertical Solution | Horizontal Payload | Vertical Payload | Derating |
|-------------|-------|------------------|-------------------|-----------------|----------|
| edelkrone SliderPLUS + Vertical Module | Proprietary | Constant-force spring counterbalance (patented tension wire) | 17.6 lb | Not published | Uses counterbalance |
| edelkrone SliderONE v3 | Proprietary | Disables hand-control; software-limited | 4.4 lb | ~4 lb (estimated) | ~0% (software-limited instead) |
| iFOOTAGE Shark Nano II | Brushless motor | Stronger motor, software | 15.4 lb | 7.7 lb | 50% |
| iFOOTAGE Shark Nano (orig) | Brushless motor | Stronger motor, software | 7.7 lb | 4.4 lb | 57% |
| Syrp Genie II Linear | Belt + stepper | Explicitly supports vertical | Not published | Not published | Not published |
| Rhino ROV | Coreless DC motor | Not specified | 5 lb | Not published | Not published |

**Industry pattern:** Most commercial sliders either (a) derate vertical payload 50%, (b) use a counterbalance mechanism, or (c) simply don't officially support vertical operation. Nobody uses self-locking lead screws because of the speed penalty.

## 8. Design Options Summary

### Option A: Belt + Brake Motor (Recommended)

**Motor:** 17HS19-2004D-B070 (52 Ncm + 70 Ncm brake) or 17HS24-2004D-B070 (72 Ncm + 70 Ncm brake)
**Drive:** GT2 belt, 16-tooth pulley
**Safety:** Power-off electromagnetic brake (fail-safe)
**Cost delta vs horizontal-only:** +$17 (brake motor premium)

| Pro | Con |
|-----|-----|
| Full speed range preserved | 31mm longer motor body |
| Fail-safe on power loss | Requires brake control MOSFET + logic |
| Drop-in replacement for standard motor | Slight motor torque reduction (52 vs 59 Ncm) |
| Zero holding heat (brake engaged at stop) | Brake coil draws 3.7W when released |
| Compatible with TMC2209, no driver change | |

### Option B: Belt + Brake + Counterbalance (Premium)

Same as Option A plus constant-force spring counterbalance.

| Pro | Con |
|-----|-----|
| All Option A benefits | Additional mechanical complexity |
| Near-zero motor load during holds | Must tune spring to payload weight |
| Can use lighter-duty motor | Adds ~$15-30 and 200-500g |
| Extended battery life for timelapses | Optional complexity for horizontal use |

### Option C: Worm Gear (Not Recommended)

**Motor:** NEMA 17 + 31:1 worm gearbox
**Drive:** GT2 belt through worm gear output
**Safety:** Inherent self-locking

| Pro | Con |
|-----|-----|
| Inherently self-locking, no brake needed | Max speed ~21-43 mm/s (vs 500+ mm/s belt) |
| Massive torque output | 1-2 degree backlash at output |
| Simple control (no brake logic) | Noisy at speed |
| | 90-degree shaft requires different mounting |
| | 40-60% efficiency wastes power |

## 9. Brake Control Implementation

The ESP32-S3 controls the brake via a single GPIO + N-channel MOSFET:

```
ESP32 GPIO --> Gate of MOSFET --> Brake coil (24V, 155mA)
                                  Flyback diode across coil
```

**Software logic:**
1. On boot: brake stays engaged (GPIO low = MOSFET off = no 24V to brake coil = spring clamps)
2. Before any move command: energize brake (GPIO high), wait 10-20ms for release
3. Execute move with motor
4. After move completes: motor holds position, then engage brake (GPIO low), then optionally reduce motor current
5. Watchdog timer: if firmware crashes, GPIO defaults low = brake engages

**Timing considerations:**
- Brake release time: typically 10-30ms (electromagnetic force overcomes spring)
- Brake engagement time: typically 5-15ms (spring force clamps immediately when field collapses)
- During the 5-15ms engagement gap, the motor must still hold the load -- this is not a problem since the motor is already energized

## 10. Bill of Materials (Vertical Drive)

| Part | Model | Est. Cost |
|------|-------|-----------|
| Stepper motor with brake | 17HS19-2004D-B070 | $29 |
| Stepper driver | TMC2209 module | $5-8 |
| GT2 belt | 6mm wide, 2mm pitch | $3-5 |
| Drive pulley | GT2 16-tooth, 5mm bore | $2-3 |
| Idler pulley | GT2 16-tooth idler | $2-3 |
| Brake MOSFET + flyback diode | IRLZ44N + 1N4007 | $1 |
| **Subtotal (vertical-capable)** | | **$42-50** |
| *Optional: counterbalance spring* | *Constant-force spring, ~35N* | *$10-20* |
| *Optional: closed-loop upgrade* | *17E1KBK05-07 replaces motor* | *+$25-30* |

Compare to the previous horizontal-only BOM of $22-31. The vertical capability adds approximately $20.

## 11. Open Questions

1. **Brake motor fitment.** The 17HS19-2004D-B070 is 79mm long vs 48mm for the standard motor. The motor mount design must accommodate this. If using the longer 17HS24-2004D-B070 (91mm), the mount needs even more space.

2. **Brake engagement during emergency stop.** If the motor is moving fast downward when the brake engages, the sudden stop could jolt the camera. Software should decelerate first, then engage the brake. Only engage the brake at speed as a last resort (power loss).

3. **Counterbalance spring selection.** Need to prototype with specific springs to validate that force is truly constant enough across the full slider travel. Springs that vary more than 10-15% will cause the motor to fight the imbalance.

4. **Closed-loop driver integration.** The CLN17 (Creapunk) board integrates TMC2209 + encoder + IMU in a compact form factor. If we go closed-loop, this is worth evaluating as an alternative to separate encoder + driver.

5. **Real-world testing needed.** The torque calculations show adequate margin at the real payload, but stepper torque drops significantly at speed. Need to verify vertical operation at 100+ mm/s with the actual hardware.
