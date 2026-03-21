# ADR-003: Linear Drive System

*Status: exploring*
*Date: 2026-03-20*

## Context

The slider needs a linear drive system that moves a camera platform along a rail up to ~1200-1500mm (4-5 feet). The system must serve two very different motion profiles:

1. **Cinematic moves** — fast, smooth pushes and dollies at 50-500+ mm/sec. Absolute position accuracy matters less; smooth motion matters more.
2. **Focus stacking / macro** — tiny, repeatable steps of 10-50 microns. Speed is irrelevant; position repeatability is everything.

The slider must support **vertical operation** — mounted on a tripod pointing straight up or down. This is a hard requirement, not a nice-to-have. It means the drive system must hold the full payload against gravity, and if power is lost the camera must not fall.

The drive system choice affects speed, precision, cost, complexity, noise, and maximum rail length. This decision interacts with motor selection, controller firmware (open-loop vs closed-loop), brake mechanisms, and the overall mechanical design.

## Payload

| Component | Weight |
|-----------|--------|
| DJI RS 5 gimbal (with battery grip & QR plates) | 1,460g |
| Sony A7IV (with battery & card) | 658g |
| Sigma 28-105mm f/2.8 DG DN Art (E-mount) | 990g |
| **Total equipment** | **3,108g (6.85 lb)** |
| Carriage (plate, MGN12H bearings, hardware) | ~300-500g |
| **Total real payload** | **~3,500g (7.7 lb)** |
| **Design load (2x safety margin)** | **~7,000g (7.0 kg / 15.4 lb)** |

Commercial reference: the iFOOTAGE Shark Slider Nano II rates 15.4 lb horizontal and 7.7 lb vertical (50% derating). Our real payload matches their vertical rating.

## Options Under Consideration

### Option A: GT2 Belt Drive

A continuous GT2 timing belt looped around pulleys at each end, driven by a stepper motor. The dominant approach in DIY CNC machines above 1000mm travel.

**Strengths:**
- Speed is effectively unlimited at any rail length — 500+ mm/sec is easy, 2000 mm/sec is possible
- Cheapest drive mechanism ($10-15 for belt + pulleys)
- No length-dependent speed limits (no whip, no critical RPM)
- Every major DIY CNC (LowRider, WorkBee, ShapeOko) uses belts above 1000mm
- Simple mechanically — fewer precision parts than screw drives

**Weaknesses:**
- Belt stretch limits inherent positional accuracy. Under light camera loads stretch is small, but hysteresis when reversing direction is real.
- Manufacturing pitch inconsistency — the LowRider project measured +/-6mm over 2400mm from belt tolerances alone
- Repeatability for focus stacking likely requires adding a linear encoder ($30-60) to close the loop on actual carriage position
- Belt tension needs to be managed — spring-loaded or adjustable tensioner adds a design element

**Resolution math (GT2, 20-tooth pulley, TMC2209 at 1/256 interpolation):**
- 40mm per revolution / (200 steps x 256 microsteps) = 0.78 microns per microstep (theoretical)
- Real-world accuracy limited by belt elasticity, not step resolution

**Cost: ~$10-15** (belt + pulleys + tensioner), or **~$45-75** with magnetic linear encoder for precision mode.

### Option B: Ball Screw (SFU1605)

A 16mm diameter ball screw with 5mm lead, supported by bearing blocks at each end. The precision option used by the PrintNC and similar CNC builds.

**Strengths:**
- Inherent precision without encoders — C7 rolled ball screws deliver ~30 microns accuracy
- Near-zero backlash with preloaded nuts
- No stretch — rigid mechanical coupling between motor and carriage
- Focus stacking works out of the box with open-loop stepper control
- Self-contained — no encoder, no firmware complexity for position feedback

**Weaknesses:**
- Speed-limited by critical whip speed, which drops with length:

| Length | Critical RPM (fixed-supported) | Max linear speed (5mm lead) |
|--------|-------------------------------|----------------------------|
| 900mm | 2,816 RPM | 234 mm/sec |
| 1200mm | 1,681 RPM | 112 mm/sec |
| 1500mm | 1,073 RPM | 71 mm/sec |

- At 1200mm, **112 mm/sec is the ceiling**. That covers slow-to-medium cinematic dollies but rules out fast dramatic pushes.
- Heavier and bulkier than belt (screw + bearing blocks + coupling)
- Needs lubrication and dust protection
- Slightly more expensive

**Resolution math (5mm lead, 200 steps, 1/256 microstepping):**
- 5mm / (200 x 256) = 0.098 microns per microstep (theoretical)

**Cost: ~$65-160** (screw + nut + BK12/BF12 bearing blocks + coupling).

### Option C: Lead Screw (T8)

An 8mm lead screw with brass or anti-backlash nut. The budget precision option common in 3D printers and short-travel CNC.

**Strengths:**
- Cheapest screw option ($10-20)
- Good precision at short lengths
- Self-locking — holds position with power off (depending on lead angle)

**Weaknesses:**
- Critical whip speed is severely limiting: **43 mm/sec at 1500mm, 96 mm/sec at 500mm**
- Not viable above 500-600mm for any application needing speed
- Lower efficiency than ball screws (50-70% vs 90%+), meaning more motor torque wasted as heat
- Backlash unless using anti-backlash nut (adds cost and drag)

**Verdict:** Not a serious contender at 1200mm+. Only viable for a sub-600mm time-lapse-only build.

### Option D: Belt + Linear Encoder (Hybrid)

Belt drive for motion, with a magnetic linear encoder strip on the rail and a read head on the carriage for true position feedback.

**Strengths:**
- Full speed range of belt drive (500+ mm/sec for cinematic, sub-mm/sec for focus stacking)
- True carriage position feedback — captures belt stretch, backlash, everything
- Magnetic encoder strips achieve 5-micron resolution (200 counts/mm) and +/-5-10 micron accuracy with PID tuning
- Encoder is robust (magnetic, handles dust/moisture)

**Weaknesses:**
- Added firmware complexity — the ESP32 must run a position feedback loop
- Two sensing systems (motor encoder for dynamics, linear encoder for position)
- More wiring and calibration
- The linear encoder only matters for focus stacking — cinematic moves don't need it

**Cost: ~$45-75** total (belt drive + encoder strip + read head).

## Vertical Operation: The Gravity Problem

Vertical operation introduces two challenges that don't exist on a horizontal slider:

1. **Continuous load**: the motor must actively fight gravity to hold position or move upward. This requires sustained torque, generates heat, and drains battery.
2. **Fail-safe**: if the motor loses power, loses steps, or the driver faults, gravity pulls a $5,000+ camera rig downward. The drive system must prevent this mechanically.

These constraints significantly affect the drive system choice. A belt drive is not self-locking — the load will fall if the motor stops holding. A lead screw with a low enough lead angle IS self-locking, but too slow. A ball screw is NOT self-locking despite being a screw (the ball bearings are too efficient).

### Self-Locking Analysis

A lead screw is self-locking when its lead angle is less than the friction angle:

```
Self-locking condition: lambda < phi
Where: lambda = arctan(lead / (pi x d_mean))
       phi = arctan(mu_friction)
```

| Screw | Lead | Lead Angle | Friction Angle (mu=0.15) | Self-Locking? | Efficiency | Max Speed (1200mm) |
|-------|------|-----------|-------------------------|:-------------:|-----------|-------------------|
| T8x1 | 1mm | 2.8 deg | 8.5 deg | YES | ~18% | 1.8 mm/s |
| T8x2 | 2mm | 5.6 deg | 8.5 deg | YES | ~30% | 3.5-14 mm/s |
| T8x4 | 4mm | 11.1 deg | 8.5 deg | NO | ~50% | 28 mm/s |
| T8x8 | 8mm | 21.4 deg | 8.5 deg | NO | ~72% | 43-96 mm/s |
| SFU1605 ball screw | 5mm | 5.7 deg | ~2 deg (balls) | NO | ~90% | 112 mm/s |

The only self-locking screws (T8x1 and T8x2) are limited to 3.5-14 mm/s — completely unusable for video. Worm gears (31:1+) are self-locking but cap speed at 21-43 mm/s. **No self-locking mechanism can deliver the speeds this slider needs.**

### The Solution: Electromagnetic Brake

A power-off (spring-engaged) electromagnetic brake clamps the motor shaft whenever power is not applied to the brake coil. This is fail-safe by design — any power loss automatically engages the brake.

NEMA 17 stepper motors are available with integrated electromagnetic brakes. The brake adds ~31mm of motor body length and ~$17 to cost. It is a drop-in replacement for a standard NEMA 17.

**Operational sequence:**
1. Power on: brake remains engaged (safe by default)
2. Before moving: ESP32 energizes brake coil via MOSFET (24V, ~155mA, 3.7W) to release
3. Motor moves normally with brake released
4. When stopping: motor holds position, then engage brake, then reduce/cut motor current
5. On power loss / firmware crash: brake engages automatically within 5-15ms

The motor must hold the load during the 5-15ms engagement window, which is trivial since it was already energized for the move.

## Torque Analysis

### Variables

| Symbol | Value | Description |
|--------|-------|-------------|
| m_real | 3.5 kg | Real payload (equipment + carriage) |
| m_design | 7.0 kg | Design load (2x safety margin) |
| g | 9.81 m/s^2 | Gravitational acceleration |
| mu_rail | 0.015 | MGN12 linear rail friction coefficient |
| eta_belt | 0.95 | GT2 belt drive efficiency |
| r_20T | 6.37 mm | GT2 20-tooth pulley effective radius |
| r_16T | 5.09 mm | GT2 16-tooth pulley effective radius |

### Vertical Holding Torque

The torque required to hold the payload stationary against gravity:

```
T_hold = m x g x r / eta_belt
```

| Pulley | Real load (3.5 kg) | Design load (7.0 kg) |
|--------|-------------------|---------------------|
| 20-tooth (r = 6.37mm) | 23.0 Ncm | 46.0 Ncm |
| 16-tooth (r = 5.09mm) | 18.4 Ncm | 36.8 Ncm |

### Total Torque: Vertical Hold + Acceleration

```
T_total = (m x g + m x a) x r / eta_belt
```

**With 16-tooth pulley (recommended for vertical):**

| Scenario | Real load (3.5 kg) | Design load (7.0 kg) |
|----------|-------------------|---------------------|
| Hold only | 18.4 Ncm | 36.8 Ncm |
| + 0.1g acceleration | 20.2 Ncm | 40.5 Ncm |
| + 0.2g acceleration | 22.1 Ncm | 44.2 Ncm |
| + 0.5g acceleration | 27.6 Ncm | 55.2 Ncm |

**With 20-tooth pulley:**

| Scenario | Real load (3.5 kg) | Design load (7.0 kg) |
|----------|-------------------|---------------------|
| Hold only | 23.0 Ncm | 46.0 Ncm |
| + 0.1g acceleration | 25.3 Ncm | 50.6 Ncm |
| + 0.2g acceleration | 27.6 Ncm | 55.2 Ncm |
| + 0.5g acceleration | 34.5 Ncm | 69.0 Ncm |

### Horizontal Torque (for comparison)

On a horizontal rail, the motor only fights friction, not gravity:

```
T_horizontal = m x g x mu_rail x r / eta_belt
         = 7.0 x 9.81 x 0.015 x 0.00509 / 0.95
         = 0.0055 Nm = 0.55 Ncm  (design load, 16T)
```

Horizontal operation requires negligible torque. Vertical operation requires ~67x more torque just to hold position. This is why vertical is the sizing constraint.

### Ball Screw Torque (for reference)

For SFU1605 (16mm, 5mm lead, ~90% efficiency):

```
T_drive = F x lead / (2 x pi x eta)
        = 68.67 x 0.005 / (2 x pi x 0.90)
        = 6.1 Ncm  (design load)
```

The ball screw's mechanical advantage reduces the torque requirement by ~6x compared to a 16T belt pulley. But ball screws are not self-locking — the load back-drives the screw and falls. A brake would still be needed.

## Motor Analysis

All motors must work within the TMC2209's 2.0A RMS limit.

### NEMA 17 with Integrated Brake (Recommended)

**17HS19-2004D-B070** — the standard choice:

| Spec | Value |
|------|-------|
| Holding torque (motor) | 52 Ncm (0.52 Nm) |
| Holding torque (brake) | 70 Ncm (0.70 Nm) |
| Rated current | 2.0A per phase |
| Phase resistance | 1.4 ohm |
| Inductance | 3.0 mH |
| Step angle | 1.8 deg (200 steps/rev) |
| Body length | 79mm (motor + brake) |
| Brake voltage | 24V DC |
| Brake power | 3.7W |
| Brake type | Power-off (spring-engaged) |
| Price | ~$29 |

**Torque margins with this motor (52 Ncm) and 16-tooth pulley:**

| Scenario | Torque needed | Margin |
|----------|--------------|--------|
| Real load, hold vertical | 18.4 Ncm | **2.83x** |
| Real load, 0.2g accel up | 22.1 Ncm | **2.35x** |
| Real load, 0.5g accel up | 27.6 Ncm | **1.88x** |
| Design load, hold vertical | 36.8 Ncm | **1.41x** |
| Design load, 0.2g accel up | 44.2 Ncm | **1.18x** |
| Design load, 0.5g accel up | 55.2 Ncm | **0.94x — STALL** |

At the real payload (what we'll actually run), margins are comfortable. At the design load (2x safety), hold and gentle acceleration work but aggressive acceleration doesn't. This is expected — the 2x safety margin is for holding, not for high-acceleration vertical launches.

**Brake margins:**

| Scenario | Gravity torque at shaft | Brake margin (70 Ncm) |
|----------|------------------------|----------------------|
| Real load | 17.5 Ncm | **4.0x** |
| Design load | 35.0 Ncm | **2.0x** |

The brake holds comfortably at the full design load.

**17HS24-2004D-B070** — higher torque upgrade:

| Spec | Value |
|------|-------|
| Holding torque (motor) | 72 Ncm (0.72 Nm) |
| Holding torque (brake) | 70 Ncm (0.70 Nm) |
| Rated current | 2.0A per phase |
| Body length | 91mm |
| Price | ~$35 |

This pushes the design-load hold margin to 1.96x and handles 0.2g acceleration at design load with 1.63x margin. The trade-off is 12mm more length.

### Closed-Loop + Brake Option

**17E1KBK05-07:**

| Spec | Value |
|------|-------|
| Holding torque (motor) | 72 Ncm |
| Holding torque (brake) | 70 Ncm |
| Built-in encoder | 1000-line (4000 CPR) |
| Price | ~$50-60 |

Adds encoder-verified positioning: the driver detects and corrects missed steps in real time before the payload drops. Two independent safety layers (encoder + brake). The encoder also enables reduced holding current — the driver only applies as much current as the load requires, rather than full rated current, which dramatically reduces heat.

Note: the motor shaft encoder corrects for motor errors but does NOT capture belt stretch. For true carriage position feedback (focus stacking), a linear encoder on the rail is still needed.

### NEMA 23: Why Not

Most NEMA 23 motors draw 2.8-4.2A — above the TMC2209's 2.0A limit. Moving to NEMA 23 would require a different driver (e.g., TMC5160, ~$15-25) and a larger, heavier mechanical design. Given that NEMA 17 with a 16T pulley provides adequate margins at our payload, NEMA 23 is unnecessary complexity.

If the payload grew significantly (e.g., a large cine camera on a heavy gimbal), NEMA 23 + TMC5160 would be the path. But for mirrorless + DJI RS class setups, NEMA 17 is the right size.

### Pulley Size: 16-Tooth vs 20-Tooth

| Pulley | Circumference | Vertical hold margin (52 Ncm, real) | Max speed (1000 RPM) |
|--------|--------------|--------------------------------------|---------------------|
| 20-tooth | 40mm | 2.26x | 667 mm/s |
| 16-tooth | 32mm | 2.83x | 533 mm/s |

The 16-tooth pulley trades 20% top speed for 25% more torque margin. At 533 mm/s, the slider is still faster than any commercial competitor (iFOOTAGE Shark maxes at ~50 mm/s for cinematic moves). **16-tooth is the clear choice for a vertical-capable slider.**

### Torque at Speed

Stepper holding torque is the static rating. Running torque drops as speed increases due to back-EMF and inductance limiting current rise time. For a typical NEMA 17 at 12V supply:

| Speed | Approx torque retention |
|-------|------------------------|
| 0 RPM (hold) | 100% (52 Ncm) |
| 200 RPM (107 mm/s) | ~80% (42 Ncm) |
| 500 RPM (267 mm/s) | ~55% (29 Ncm) |
| 1000 RPM (533 mm/s) | ~30% (16 Ncm) |

At top speed the motor only produces ~16 Ncm, which is less than the 18.4 Ncm needed to hold the real payload vertically. This means: **you can't do 533 mm/s vertically at full load.** The practical vertical speed limit is around 200-300 RPM (107-160 mm/s) where torque margin is still positive. This is fine — fast vertical moves with a heavy camera rig would be unusual anyway.

Horizontal operation has no such limit — the 0.55 Ncm friction load is trivial even at full speed.

Higher supply voltage (24V instead of 12V) pushes the torque curve up significantly, extending the usable speed range. The TMC2209 supports up to 29V.

## Thermal Analysis

### Holding a Vertical Load

Open-loop steppers apply full rated current regardless of load:

```
P_hold = I^2 x R x 2 phases = 2.0^2 x 1.4 x 2 = 11.2W continuous
```

An unmounted NEMA 17 at 11.2W will overheat (~134C rise above ambient). Mounted to aluminum extrusion (which acts as a heatsink), typical case temperature is 40-60C — warm but safe.

### Using the Brake to Eliminate Holding Heat

This is one of the strongest arguments for the brake motor:

1. **Moving**: motor energized, brake released (3.7W brake coil). Motor generates heat normally.
2. **Holding position**: engage brake, reduce motor current to zero. **Zero motor heat.** Only the brake coil dissipates 3.7W.

For time-lapse work (long holds between moves), the brake keeps the motor cool and extends battery life significantly.

### Optional: Counterbalance

A constant-force spring (like edelkrone's $690 Vertical Module uses) can offset gravity, reducing the motor's job to acceleration only:

```
With counterbalance:
T_hold = m x g x mu_rail x r / eta = 0.35 Ncm  (rail friction only)
P_hold = (0.25A)^2 x 1.4 x 2 = 0.18W         (at ~12% holding current)
```

This is a bolt-on accessory, not a core requirement. The brake alone provides adequate safety. A counterbalance adds efficiency for heavy vertical work at ~$15-30 in parts (constant-force springs + cable + pulley). Drawback: must be tuned to the specific payload weight.

## Key Trade-offs

The decision reduces to: **do we need fast cinematic moves (300+ mm/sec)?**

- **If yes** — belt is the only option. Add a linear encoder if focus stacking precision matters. Add a brake motor for vertical safety.
- **If no** — ball screw gives precision for free at lower firmware complexity, but caps speed at ~112 mm/sec at 1200mm. Still needs a brake for vertical operation (ball screws are not self-locking).

112 mm/sec is not slow in absolute terms — it's a 10.7-second full-length traverse, which covers most deliberate dolly work. The question is whether the slider should also support fast dramatic pushes, whip transitions, and rapid repositioning between shots.

## Cost Comparison

| Option | Drive | Motor | Brake | Encoder | Total | Max Speed (1200mm) | Vertical Safe | Focus Stack |
|--------|-------|-------|-------|---------|-------|--------------------|--------------:|------------|
| Belt only | $10-15 | $12 | — | — | $22-27 | 2000+ mm/s | NO | No |
| Belt + brake | $10-15 | $29 | included | — | $39-44 | 2000+ mm/s | YES | No |
| Belt + brake + linear encoder | $10-15 | $29 | included | $30-60 | $69-104 | 2000+ mm/s | YES | Yes |
| Belt + closed-loop brake | $10-15 | $50-60 | included | — | $60-75 | 2000+ mm/s | YES | Partial |
| Ball screw + brake | $65-160 | $29 | included | — | $94-189 | 112 mm/s | YES | Yes |

## Brake Control Hardware

The ESP32-S3 controls the brake via a single GPIO + N-channel MOSFET:

```
ESP32 GPIO --> Gate of IRLZ44N MOSFET --> Brake coil (24V, 155mA)
                                          1N4007 flyback diode across coil
```

The brake coil is a simple inductive load. The flyback diode protects the MOSFET from the voltage spike when the coil is de-energized. Total additional BOM: ~$1.

**Firmware logic:**
1. On boot: GPIO low = brake engaged (fail-safe default)
2. Pre-move: GPIO high, wait 10-20ms for brake release
3. Move executes normally
4. Post-move: motor holds, GPIO low (brake engages in 5-15ms), then optionally reduce motor current
5. Watchdog: if firmware hangs, GPIO defaults low = brake engages

This adds one GPIO to the pin budget (12 core pins instead of 11). Still well within the ESP32-S3's capacity.

## Open Questions

1. **Motor fitment**: the brake motor is 79mm (or 91mm for the higher-torque version) vs 48mm for a standard NEMA 17. The motor mount must accommodate this length.
2. **Brake engagement during fast downward moves**: sudden braking could jolt the camera. Firmware should decelerate first, then engage the brake. Hard-brake at speed is a last resort (power loss only).
3. **24V supply**: the brake needs 24V. If the slider runs on 12V (3S LiPo), a small boost converter is needed for the brake coil. Alternatively, run the whole system on 24V (TMC2209 supports up to 29V) — this also improves stepper torque at speed.
4. **Can firmware closed-loop control on the ESP32 (reading a linear encoder and adjusting stepper position) be made reliable enough for focus stacking?** This needs prototyping.
5. **Should the design support both belt and ball screw drives** (modular carriage plate)? This matches "bring your own hardware" but adds mechanical complexity.

## Decision

**Not yet reached.** Pending prototyping and use case prioritization.

## References

- [Research: Long-Travel Linear Motion](../../research/long-travel-linear-motion/report.md) — full analysis with sources
- [Research: Vertical Drive Motor Sizing](../../research/vertical-drive-motor-sizing/report.md) — torque equations, brake analysis, motor comparison
- [Research: Stepper Motor & Drive Selection](../../research/slider-motor-selection/background.md) — belt vs screw data from blanch.org
- [ADR-001: Microcontroller Selection](adr-001-microcontroller.md) — ESP32-S3 and TMC2209 context
