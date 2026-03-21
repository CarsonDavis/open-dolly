# ADR-003: Linear Drive System

*Status: exploring*
*Date: 2026-03-20*

## Context

The slider needs a linear drive system that moves a camera platform along a rail up to ~1200-1500mm (4-5 feet). The system must serve two very different motion profiles:

1. **Cinematic moves** — fast, smooth pushes and dollies at 50-500+ mm/sec. Absolute position accuracy matters less; smooth motion matters more.
2. **Focus stacking / macro** — tiny, repeatable steps of 10-50 microns. Speed is irrelevant; position repeatability is everything.

The drive system choice affects speed, precision, cost, complexity, noise, and maximum rail length. This decision interacts with motor selection (NEMA 17 vs 23), controller firmware (open-loop vs closed-loop), and the overall mechanical design.

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

## Key Trade-off

The decision reduces to: **do we need fast cinematic moves (300+ mm/sec)?**

- **If yes** — belt is the only option. Add a linear encoder if focus stacking precision matters.
- **If no** — ball screw gives precision for free at lower firmware complexity, but caps speed at ~112 mm/sec at 1200mm.

112 mm/sec is not slow in absolute terms — it's a 10.7-second full-length traverse, which covers most deliberate dolly work. The question is whether the slider should also support fast dramatic pushes, whip transitions, and rapid repositioning.

## Cost Comparison

| Option | Drive Cost | Encoder Cost | Total | Max Speed (1200mm) | Focus Stack Ready |
|--------|-----------|-------------|-------|--------------------|--------------------|
| GT2 belt only | $10-15 | — | $10-15 | 2000+ mm/sec | No (limited repeatability) |
| GT2 belt + linear encoder | $10-15 | $30-60 | $45-75 | 2000+ mm/sec | Yes |
| Ball screw (SFU1605) | $65-160 | — | $65-160 | 112 mm/sec | Yes |
| Lead screw (T8) | $10-20 | — | $10-20 | 43-96 mm/sec | Yes (if short enough) |

## Open Questions

1. What's the actual use case mix? If this is primarily a macro/focus stacking tool, ball screw simplicity wins. If cinematic moves matter, belt is required.
2. How important is rapid repositioning between shots? Even if cinematic speed during recording isn't needed, waiting 10+ seconds to return to start position adds up during a shoot.
3. Can firmware closed-loop control on the ESP32 (reading a linear encoder and adjusting stepper position) be made reliable enough for focus stacking? This needs prototyping.
4. Should the design support both drives (modular carriage plate that accepts either belt or screw)? This adds mechanical complexity but matches the "bring your own hardware" philosophy.

## Decision

**Not yet reached.** Pending prototyping and use case prioritization.

## References

- [Research: Long-Travel Linear Motion](../research/long-travel-linear-motion/report.md) — full analysis with sources
- [Research: Stepper Motor & Drive Selection](../research/slider-motor-selection/background.md) — belt vs screw data from blanch.org
- [ADR-001: Microcontroller Selection](adr-001-microcontroller.md) — ESP32-S3 and TMC2209 context
