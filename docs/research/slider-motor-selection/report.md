# Stepper Motor & Drive System Selection for Camera Slider

**Date:** 2026-03-20

## Recommendation

**Use a NEMA 17 stepper motor (StepperOnline 17HS19-2004S1, 59 Ncm, 2.0A) with a GT2 belt and 20-tooth pulley, driven by the TMC2209 at 24V.** This combination handles the 15 lb (7 kg) payload with large margin on flat and moderate inclines, achieves fast repositioning speeds (0.5-1.0 m/s), runs near-silent with StealthChop, and provides sufficient precision for focus stacking at moderate macro magnifications. Total motor + drive cost: under $25.

The math is unambiguous: horizontal sliding with 7 kg on ball-bearing rails requires only 0.5 Ncm of torque. The motor provides 59 Ncm -- over 100x headroom. Even at a 45-degree incline, the motor has a 1.8x safety factor. NEMA 23 is unnecessary for this application.

**Key risk:** Focus stacking precision. Belt elasticity limits real-world positioning accuracy to roughly 50-100 microns per step, adequate for moderate macro work but not for extreme macro. Users needing sub-10-micron precision should pair this slider with a dedicated macro rail on the carriage.

## Motor Selection

### Recommended: StepperOnline 17HS19-2004S1

| Spec | Value |
|------|-------|
| Holding torque | 59 Ncm (84 oz-in) |
| Rated current | 2.0A per phase |
| Phase resistance | 1.4 ohms |
| Inductance | 3.0 mH |
| Step angle | 1.8 deg (200 steps/rev) |
| Body size | 42 x 42 x 48 mm |
| Weight | 390g |
| Price | ~$10-12 (Amazon, StepperOnline) |

This is the most popular high-torque NEMA 17 on the market. The 2.0A rated current is a perfect match for the TMC2209's 2.0A RMS limit. The 2.8V phase voltage and 3.0 mH inductance fall within the Watterott-recommended range for quiet StealthChop operation (phase voltage <= 4V, inductance <= 4 mH).

### Alternatives

| Motor | Torque | Current | Body | Trade-off |
|-------|--------|---------|------|-----------|
| **17HS16-2004S1** | 45 Ncm | 2.0A | 42x42x40mm | 8mm shorter body, 24% less torque. Still sufficient for flat and mild inclines. Better for compact builds. |
| **17HS24-2104S** | 65 Ncm | 2.1A | 42x42x60mm | Maximum NEMA 17 torque. 2.1A slightly exceeds TMC2209's 2.0A RMS -- run at 95% current (losing ~3 Ncm) or use TMC2226. 12mm longer body. Only needed if regularly tilting beyond 45 degrees. |

### Why Not NEMA 23?

NEMA 23 motors provide 100-300 Ncm torque, which is dramatically more than needed. The problems:

1. **TMC2209 incompatible.** Most NEMA 23 motors draw 2.8-4.2A per phase. The TMC2209 maxes out at 2.0A RMS. You would need to upgrade to a TMC5160 ($15-25), increasing cost and board complexity.
2. **Physically larger.** 58mm frame vs 42mm, roughly 2-3x the weight.
3. **Unnecessary.** Our torque calculations show 59 Ncm handles everything up to 45-degree inclines. Even vertical (90-degree) operation needs only 46 Ncm at constant velocity.

Low-current NEMA 23 motors (1.0-2.0A) exist that would work with the TMC2209, but they only provide 80-130 Ncm -- not enough improvement over the 59 Ncm NEMA 17 to justify the larger frame.

## Drive Mechanism: GT2 Belt

### Recommendation: GT2 belt with 20-tooth pulley

This is the same drive system used by Rhino sliders, most DIY builds, and the majority of commercial camera sliders.

### Comparison Table

| Factor | GT2 Belt (20T) | T8 Lead Screw (8mm lead) | Ball Screw (16mm, C7) |
|--------|----------------|--------------------------|----------------------|
| **Max speed** | 2+ m/s | 43-96 mm/s (length dependent) | 100-300 mm/s |
| **Full-step resolution** | 200 um | 40 um | 40 um |
| **16x microstep resolution** | 12.5 um | 2.5 um | 2.5 um |
| **Backlash** | None (belt stretch instead) | Depends on anti-backlash nut | 5-52 um (grade dependent) |
| **Efficiency** | ~98% | ~50-70% | ~90% |
| **Noise** | Quiet | Moderate (screw whine) | Moderate |
| **Cost** | $5-15 | $10-20 | $120-275 |
| **Complexity** | Simple tensioning | Bearing alignment, coupling | Precision alignment required |
| **Best for** | Speed + general precision | High precision, slow moves | High precision + moderate speed |

### Why Belt Wins for This Application

1. **Speed.** Lead screws are physically limited to ~100 mm/s at slider lengths. Belt drives can do 2+ m/s. For fast repositioning between shots, belt is the only viable option.
2. **Simplicity.** Belt tensioning is straightforward. Lead screws need careful bearing alignment, couplings, and anti-backlash nuts.
3. **Industry standard.** Rhino, most eMotimo configurations, and nearly every DIY camera slider uses belt drives. This means better community support and parts availability.
4. **Cost.** A GT2 belt + two pulleys costs $5-15 total vs $120+ for a quality ball screw.
5. **Noise.** Belt drives are inherently quieter than lead screws, which produce screw whine at speed.

### Belt Specification

- **Belt:** GT2, 6mm wide, 2mm pitch (standard, widely available)
- **Drive pulley:** 20-tooth GT2, 5mm bore (matches NEMA 17 shaft)
- **Idler pulley:** 20-tooth GT2 smooth idler or toothed
- **Pitch diameter:** 12.73mm
- **Linear travel per revolution:** 40mm
- **Linear travel per full step:** 0.2mm (200 microns)

## Precision Analysis

### Theoretical Resolution

| Microstepping | Steps/rev | Linear resolution per microstep |
|---------------|-----------|--------------------------------|
| Full step | 200 | 200 um |
| 16x | 3,200 | 12.5 um |
| 32x | 6,400 | 6.25 um |
| 256x (TMC2209 interpolation) | 51,200 | 0.78 um |

### Real-World Precision

Theoretical microstep resolution is misleading. Testing shows that under load, stepper motor positioning accuracy degrades to +/- 1-2 full steps regardless of microstep level. The incremental torque at 256 microsteps is only 0.61% of full-step torque, meaning any friction can displace the shaft by many microsteps.

**However**, our application has two saving graces:

1. **Extremely low load.** Horizontal sliding on ball-bearing rails generates only 0.69 N of friction force -- less than 1% of the motor's capacity. At such low loads, microstepping accuracy is significantly better than the worst-case Hackaday measurements (which tested at 25% load).
2. **TMC2209's 256x interpolation** is always active, smoothing the current waveform even when commanding 16x or 32x microsteps. This produces smoother sine waves in the driver current, reducing the jagged torque peaks that cause positional error.

**Realistic focus stacking precision with GT2 belt:** 50-100 microns per commanded step. This is the combined effect of microstepping inaccuracy and belt elasticity (the belt absorbs very small movements as stretch before transmitting them to the carriage). This is adequate for:
- Landscape focus stacking (step sizes typically 100-500 um)
- Moderate macro work (1:4 to 1:1 magnification)

It is NOT adequate for:
- Extreme macro (> 1:1 magnification, needing 5-20 um steps)
- Scientific/industrial focus stacking

For extreme precision, users should mount a dedicated macro rail (WeMacro, Novoflex CASTEL-MICRO) on top of the slider carriage. These use lead screws with 50:1 gear ratios to achieve 1 micron steps.

## Speed Analysis

### Configuration: 17HS19-2004S1 + GT2 20T + TMC2209 at 24V

| Mode | Motor RPM | Linear speed | Torque available | Use case |
|------|-----------|-------------|-----------------|----------|
| Slow cinematic | 5-50 RPM | 3-33 mm/s | ~59 Ncm (full) | Smooth video slides |
| Medium video | 50-300 RPM | 33-200 mm/s | 50-59 Ncm | Active tracking shots |
| Fast reposition | 300-1000 RPM | 200-667 mm/s | 15-30 Ncm | Moving between positions |
| Max speed (light load) | 1000-1500 RPM | 667-1000 mm/s | 10-15 Ncm | Only horizontal, no payload accel |

**Practical fast repositioning target:** 500-800 mm/s (0.5-0.8 m/s). This is competitive with commercial sliders -- Rhino achieves roughly 1 m/s at maximum.

**ESP32-S3 step rate:** FastAccelStepper can generate up to 200,000 steps/sec. At 16x microstepping, this corresponds to 12,500 full steps/sec = 3,750 RPM = 2.5 m/s linear -- far beyond what the motor can mechanically achieve. The step rate is not the bottleneck; motor torque at speed is.

**24V supply is essential.** At 12V, torque drops off sharply above 300 RPM due to the motor's back-EMF. At 24V, usable torque extends to 800-1000 RPM. Always use 24V for this system.

## Torque Budget

Calculated for GT2 belt with 20-tooth pulley (6.37mm radius), 7 kg payload, belt efficiency 95%.

| Scenario | Required torque | Motor margin (59 Ncm) | Verdict |
|----------|----------------|----------------------|---------|
| Horizontal, constant velocity | 0.5 Ncm | 118x | Trivial |
| Horizontal, 1 m/s^2 acceleration | 5.2 Ncm | 11x | Easy |
| Horizontal, 3 m/s^2 acceleration | 15 Ncm | 4x | Comfortable |
| 30-degree incline, constant velocity | 24 Ncm | 2.5x | Good |
| 45-degree incline, constant velocity | 33 Ncm | 1.8x | Adequate |
| 45-degree incline + acceleration | 38 Ncm | 1.6x | Marginal at speed |
| Vertical (90 deg), constant velocity | 46 Ncm | 1.3x | Risky -- holding only |
| Vertical + any acceleration | > 50 Ncm | < 1.2x | Will stall at speed |

**Design limit: 45-degree incline at slow speed.** This covers the vast majority of real-world slider use cases. Vertical operation is not recommended with this motor/belt combination at 7 kg payload.

## Noise Considerations

The TMC2209 with StealthChop is one of the quietest stepper driver solutions available. Combined with the right motor, this system will be functionally silent during cinematic slides.

**Factors that maximize silence:**
1. **StealthChop mode** at low-to-medium speeds (our primary use case)
2. **Motor selection** with phase voltage <= 4V and inductance <= 4 mH (the 17HS19-2004S1 at 2.8V/3.0mH is ideal)
3. **24V supply** reduces noise even in SpreadCycle mode
4. **256 microstep interpolation** always active, eliminating full-step resonance
5. **GT2 belt** is inherently quieter than lead screws (no screw whine)

**When it gets louder:** Fast repositioning moves will use SpreadCycle mode, which is audible but not objectionable. This is acceptable because fast moves happen between shots, not during recording.

## What Commercial Sliders Use

| Brand | Drive | Motor | Notes |
|-------|-------|-------|-------|
| **Rhino EVO** | Belt | Compact stepper (integrated) | Claims 0.7 um resolution; belt-driven; 7-hour battery |
| **eMotimo Spectrum** | Belt | NEMA 17 (direct or geared 5:1 to 27:1) | Geared options for slopes; NEMA 23 for Dana Dolly (heavy) |
| **edelkrone SliderPLUS** | Proprietary | Integrated stepper | CNC-level precision; 0.6 mm/s min speed; no published specs |
| **DIY builds** | Belt (overwhelmingly) | NEMA 17 | Hirnschall uses worm gear; most use direct belt |

The industry consensus is clear: belt drives with NEMA 17 steppers are the standard for camera sliders. Planetary gearboxes (5:1 to 14:1) are used when more torque is needed for inclines, but direct drive is the default.

## Final Bill of Materials (Motor + Drive)

| Part | Specific Model | Est. Cost |
|------|---------------|-----------|
| Stepper motor | StepperOnline 17HS19-2004S1 | $10-12 |
| Stepper driver | TMC2209 module (BigTreeTech or similar) | $5-8 |
| GT2 belt | 6mm wide, 2mm pitch, length = 2x slider + wrap | $3-5 |
| Drive pulley | GT2 20-tooth, 5mm bore, aluminum | $2-3 |
| Idler pulley | GT2 20-tooth idler with bearing | $2-3 |
| **Total** | | **$22-31** |

## Open Questions

1. **Belt tension mechanism.** Need to design a tensioning system -- spring-loaded idler or adjustable motor mount.
2. **Pulley tooth count trade-off.** A 16-tooth pulley would give 25% more torque at the expense of 20% less speed. Worth considering if incline performance is poor in testing.
3. **Focus stacking validation.** Real-world testing needed to confirm belt-driven precision at low speeds. If insufficient, a T8 lead screw with anti-backlash nut could be offered as an optional swap (same motor mount, different drive).
4. **Power supply.** 24V is essential for performance. Need to spec a 24V supply or 6S LiPo for portable use. Current draw: ~2A peak, ~0.5A typical = 12-48W.
