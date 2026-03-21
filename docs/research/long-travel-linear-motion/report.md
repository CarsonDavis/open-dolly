# Long-Travel Linear Motion for a Camera Slider (900-1500mm)

**Date:** 2026-03-20

## Executive Summary

For a 3-5 foot motorized camera slider that needs both fast cinematic moves and sub-0.1mm repeatability for focus stacking, **a GT2/GT3 belt drive with closed-loop motor control is the most practical approach**. Belt drives are the only option that can achieve the high speeds needed for cinematic work (500+ mm/sec) at these distances, while closed-loop control (encoder on motor) and careful belt tensioning can deliver the repeatability needed for focus stacking. Ball screws offer superior inherent precision but are speed-limited by critical whip speed at these lengths, and lead screws are even worse. A hybrid design using belt drive with a magnetic linear encoder for position verification would give the best of both worlds, though at added complexity.

The recommended build: **GT2 9mm belt on MGN12H linear rails mounted to 2040 or 4040 aluminum extrusion**, driven by a **closed-loop NEMA 17 stepper with TMC2209 driver**. For focus stacking, add a magnetic linear encoder strip for true position feedback.

---

## 1. Belt Drives at Long Distances (1200mm+)

Belt drives are the dominant choice for DIY linear motion above 1000mm. Every major open-source CNC project (LowRider, ShapeOko, WorkBee) uses belts for their long axes.

### What Works

- **GT2 belts** (2mm pitch, curvilinear tooth profile) are the standard for DIY motion. GT3 is a newer revision with the same pitch but slightly improved tooth profile -- functionally interchangeable.
- **10mm width** is the sweet spot for long-travel CNC machines. The LowRider V4 uses 10mm GT2 for full-sheet (2440mm) travel. 6mm works for lighter loads (camera sliders). 15mm provides more stiffness but is rarely needed.
- **Belt tension** of 7-12 lbs (3-5 kg) is typical. Too loose causes tooth skip; too tight accelerates bearing wear. The LowRider recommends ~7 lbs; camera slider guides recommend 8-12 N (0.8-1.2 kg) for lighter payloads.
- **Steel-core belts are NOT recommended** -- they are stiffer but can damage pulleys and are harder to tension properly.

### The Stretch Problem

Belt stretch is the primary accuracy limitation:

- A 3mm pitch, 15mm-wide belt stretches 0.1% at ~30 kg load. Over 1200mm, 0.1% stretch = 1.2mm of error.
- A 5mm pitch, 15mm-wide belt stretches 0.1% at ~92 kg load -- 3x stiffer.
- Camera slider loads are very light (3-5 kg total), so actual stretch under load is small. The bigger issue is **belt pitch inconsistency** -- the LowRider measured +/-6mm variation over 2400mm from belt manufacturing tolerances alone.
- For a camera slider, stretch under load is manageable but **repeatability** (returning to the exact same position) is limited by belt elasticity and hysteresis when changing direction.

### Speed Advantage

This is where belts dominate. A 20-tooth GT2 pulley (40mm circumference) at a typical stepper max of 3000 RPM yields **2000 mm/sec** -- an order of magnitude faster than any screw drive. Even at conservative speeds, belts easily achieve 500+ mm/sec for fast cinematic pushes.

### Resolution

With a 200-step motor, 20-tooth GT2 pulley, and 1/32 microstepping (TMC2209):

- Steps per revolution: 200 * 32 = 6400
- Distance per revolution: 40mm
- **Resolution: 0.00625mm (6.25 microns) per microstep**

This theoretical resolution is more than adequate for focus stacking (which typically needs 10-50 micron steps). The practical limitation is not resolution but **repeatability** -- whether the belt actually delivers that position consistently.

---

## 2. Lead Screws at Long Distances

Lead screws are the budget option but have severe limitations above 500mm.

### Critical Speed / Whip

A rotating lead screw develops dangerous resonance at high RPM. For an 8mm T8 lead screw:

| Support Type | 500mm | 1000mm | 1500mm |
|-------------|-------|--------|--------|
| Fixed-Supported | 2,902 RPM | 725 RPM | 322 RPM |
| Fixed-Fixed | 6,471 RPM | 1,618 RPM | 719 RPM |

At 1500mm with typical fixed-supported mounting, max safe speed is **322 RPM**. With an 8mm lead, that is only 43 mm/sec -- far too slow for cinematic camera moves.

### Practical Maximum Length

- **T8 lead screws** (8mm diameter): practical limit around **500-600mm** before whip becomes a real issue.
- **TR10x2 or TR12x3** (10-12mm diameter): can push to 800-1000mm but still slow.
- **Supported lead screws** (with intermediate bearing supports) help but add complexity and cost -- at which point a ball screw is a better investment.

### Verdict

Lead screws are not viable for a 1200mm+ camera slider that needs fast moves. They could work for a slow-only slider (time-lapse only, no cinematic moves) at 900mm if budget is extremely tight.

---

## 3. Ball Screws at Long Distances

Ball screws offer the best inherent precision of any screw drive but still face speed limits at long lengths.

### Critical Speed Calculations

Using the standard formula (nc = fc * dr / Lc^2 * 10^7) with fixed-supported mounting (fc = 15.1):

| Ball Screw | 1200mm Critical | 1200mm Safe (80%) | 1500mm Critical | 1500mm Safe (80%) |
|-----------|----------------|-------------------|----------------|-------------------|
| SFU1605 (16mm, 5mm lead) | 1,681 RPM | 1,345 RPM | 1,073 RPM | 858 RPM |
| SFU2005 (20mm, 5mm lead) | 2,101 RPM | 1,681 RPM | 1,344 RPM | 1,075 RPM |

**Maximum linear speeds with 5mm lead:**

| Ball Screw | 1200mm | 1500mm |
|-----------|--------|--------|
| SFU1605 | 112 mm/sec | 71 mm/sec |
| SFU2005 | 140 mm/sec | 90 mm/sec |

These speeds are adequate for slow, precise moves but **too slow for fast cinematic pushes** (which want 300-500 mm/sec). Using a higher-lead screw (SFU1610, 10mm lead) would double the speed but halve the precision.

### Availability and Cost

- **SFU1605 at 1200mm**: $25-95 for screw + nut (C7 rolled, from Chinese suppliers)
- **SFU1605 at 1500mm**: ~$94 (Maker Store USA)
- **Bearing blocks** (BK12 + BF12): ~$40-65
- **Complete kit at 1200mm**: roughly **$70-170** depending on source quality

### Quality Considerations

- **C7 rolled** (most common Chinese): ~30 microns accuracy, 30-70 microns backlash
- **C5 ground with preload**: ~$274 for short lengths, eliminates backlash but much more expensive
- Ball screws need **lubrication** and protection from contamination
- The PrintNC community recommends SFU2010 (20mm, 10mm lead) for axes over 1500mm

### Driven Nut Alternative

A **rotating nut** design (screw is fixed, nut rotates) completely eliminates whip. Available as Chinese "captive linear stepper motors." This removes the length limitation entirely but makes backlash elimination more complex.

### Verdict

Ball screws are excellent for precision at 900-1200mm but hit speed limitations for fast cinematic moves. An SFU1605 at 1200mm gives ~112 mm/sec max -- acceptable for slow pushes and focus stacking, but not for fast whip pans. At 1500mm, it gets worse (71 mm/sec).

---

## 4. Rack and Pinion

Rack and pinion is used in larger CNC machines and excels above 4 feet.

### Characteristics

- **More accurate than belt** drives but **less accurate than ball screws**
- **High rigidity** on long travel applications -- no stretch or whip
- **Almost impervious to dust and debris** -- good for field use
- Costs **less than ball screws**
- Backlash is the main challenge; addressed via split-pinion drives or spring-preloaded dual pinions
- Higher friction and wear than ball screws

### For This Project

Rack and pinion is overkill and adds unnecessary complexity for a 1200-1500mm camera slider. It shines on machines where travel exceeds 2000mm (industrial CNC routers, plasma cutters). The backlash management (split pinion, preloading) adds cost and complexity that isn't justified at this scale.

---

## 5. Linear Rail Options

### MGN Rails on Aluminum Extrusion

This is the standard approach for DIY camera sliders and works well at 1200-1500mm.

| Rail | Width | Weight/m | Best Extrusion | Notes |
|------|-------|----------|----------------|-------|
| MGN9 | 9mm | 380 g/m | 2020 (fits within profile) | Lightest, lowest load |
| **MGN12** | 12mm | 650 g/m | **2020 or 2040** (4mm clearance/side) | Sweet spot for sliders |
| MGN15 | 15mm | 1060 g/m | 2040+ (only 2.5mm clearance on 2020) | Heavier, higher load |

**MGN12H** (standard-length carriage) is the most common choice. Use **H** carriages for stability. **C** carriages are shorter and lighter but less stable under moment loads.

### Alignment at Long Distances

- 3D-printable alignment jigs that reference the extrusion edge are the standard method
- The extrusion itself must be straight -- aluminum extrusions are typically straight to within ~0.5mm/m
- For 1200mm+ rails, align section by section using the carriage as a reference
- The rail is "only as good and straight as what you fixture it to" -- the extrusion quality matters

### V-Slot Wheels vs Linear Rails

V-slot wheels (OpenBuilds style) are inferior to MGN linear rails:
- V-wheels develop **play over time** and need periodic retightening
- Delrin wheels can **degrade after ~2 years**
- Linear rails are **more precise, lower maintenance, and often cheaper**
- The only advantage of V-wheels is that the V-slot extrusion IS the rail -- no separate rail to align

### HGR Rails

HGR15/HGR20 rails (4-raceway design) are used by the PrintNC community for heavy CNC milling. Massive overkill for a camera slider. Weight and cost are much higher than MGN.

### Recommendation

**MGN12H on 2040 aluminum extrusion** with two carriages. Use a 3D-printed alignment jig. The 2040 extrusion provides mounting width for both the rail and a belt channel or ball screw.

---

## 6. Hybrid and Closed-Loop Approaches

### Closed-Loop Stepper Motors

Closed-loop steppers (stepper + rotary encoder + smart driver) solve the lost-step problem:

- Detect and correct missed steps in real time
- Higher torque, lower heating than open-loop
- No PID tuning required (unlike servos)
- **$50-120** for NEMA 23 motor + encoder + driver kit
- NEMA 17 closed-loop options are lighter and sufficient for camera slider loads

**Critical limitation:** The encoder is on the **motor shaft**, not the carriage. It corrects for motor errors but NOT belt stretch or backlash between motor and carriage.

### Linear Encoder for Position Verification

For true closed-loop carriage position control:

- **Magnetic linear encoder strips** with read heads provide 5-micron resolution (200 counts/mm)
- Achievable accuracy: +/-5-10 micrometers with proper PID tuning
- Magnetic scales are robust enough for field use (resist dust, moisture)
- Requires a more sophisticated controller (LinuxCNC, or custom firmware) for the position feedback loop
- The linear encoder handles the **position verification** layer while the rotary encoder handles the **dynamic control** layer

### Practical Hybrid for Camera Slider

The most practical hybrid:

1. **Belt drive** for speed (fast cinematic moves at 500+ mm/sec)
2. **Closed-loop stepper** (motor encoder) for reliable step tracking
3. **Optional magnetic linear encoder** for sub-0.1mm repeatable positioning (focus stacking mode)

This gives full speed range for cinematic work while enabling precision positioning when needed. The linear encoder is only needed for focus stacking -- cinematic moves don't need sub-0.1mm accuracy.

---

## 7. What the DIY CNC Community Does

### Summary of Long-Axis Drive Systems

| Project | Drive System | Max Length | Precision | Notes |
|---------|-------------|-----------|-----------|-------|
| **PrintNC** | Ball screw (SFU1610/SFU2010) | 1500mm+ | High (CNC milling metal) | Upgrades to 20mm diameter for >1500mm |
| **LowRider V4** | GT2 10mm belt | 2440mm+ (full sheet) | +/-6mm over full length | Designed for wood; 1mm is "very good" |
| **OpenBuilds WorkBee** | Belt (>1000mm) or screw (<1000mm) | 1500mm | Moderate (wood CNC) | Explicitly switches to belt above 1000mm |
| **MPCNC** | Belt | ~600mm practical | Low | Conduit-based, not designed for long travel |
| **ShapeOko** | Belt | 1000mm+ | Moderate | GT2 belt on V-wheels or linear rails |

**The pattern is clear:** Commercial and open-source CNC kits switch from screw to belt above 1000mm. Only the PrintNC maintains ball screws at long lengths, and it requires upsizing to 20mm diameter.

### DIY Camera Slider Builds

- Most use **GT2 6mm belt on 2020 aluminum extrusion** with MGN12 rails
- **ESP32 + TMC2209** is the emerging standard controller combination (WiFi control, silent microstepping)
- Modular designs allow building to any length
- Focus stacking builds add limit switches for homing and precise step counting

---

## 8. Recommendations for This Project

### Primary Recommendation: Belt Drive with Closed-Loop Control

| Component | Specification | Est. Cost |
|-----------|--------------|-----------|
| Drive | GT2 9mm belt, 20-tooth pulley | $10-15 |
| Linear guide | MGN12H on 2040 extrusion, 1200mm | $30-50 |
| Motor | NEMA 17 closed-loop stepper | $40-80 |
| Driver | TMC2209 (silent, 1/256 microstep interpolation) | $5-10 |
| Tensioner | Spring-loaded or adjustable end block | $5-10 |
| **Total drive system** | | **$90-165** |

**For focus stacking, add:**

| Component | Specification | Est. Cost |
|-----------|--------------|-----------|
| Linear encoder | Magnetic strip + read head, 5um resolution | $30-60 |

### Why Not Ball Screw?

A ball screw at 1200mm (SFU1605) maxes out at ~112 mm/sec. A fast cinematic push wants 300-500 mm/sec. You would need the speed of a belt for cinematic work and the precision of a screw for focus stacking -- and no single screw drive can do both. A belt with encoder feedback can cover the full range.

### Why Not Lead Screw?

Lead screws are speed-limited to ~43 mm/sec at 1500mm. Not viable for anything beyond time-lapse-only operation.

### Design Notes

1. **Belt width:** 9mm GT2 is the sweet spot -- stiffer than 6mm with minimal weight/size penalty. 6mm works if weight is paramount.
2. **Belt tension:** 8-12 N for camera slider loads. Use a spring-loaded tensioner for consistent tension.
3. **Extrusion choice:** 2040 provides a flat face for the MGN12 rail and room for a belt channel. 4040 is sturdier but heavier.
4. **Carriage design:** Two MGN12H carriages spaced 80-100mm apart provide excellent stability against moment loads.
5. **Homing:** Use limit switches + motor encoder for basic repeatability. Add a magnetic linear encoder strip only if sub-0.1mm focus stacking repeatability is required.
6. **Frame rigidity is critical** for long belt runs -- the extrusion must not flex or twist. Support the extrusion at its midpoint if using a tripod mount.

### Speed vs Precision Trade-off

| Mode | Speed | Position Control | Precision Needed |
|------|-------|-----------------|-----------------|
| Fast cinematic push | 300-500 mm/sec | Motor encoder only | +/-1mm is fine |
| Slow cinematic dolly | 5-50 mm/sec | Motor encoder only | +/-0.5mm is fine |
| Time-lapse interval | 0.1-5 mm/step | Motor encoder | +/-0.1mm |
| Focus stacking | 0.01-0.05 mm/step | Motor encoder + linear encoder | +/-0.01mm |

The belt drive handles all four modes. The linear encoder is only needed for the last mode.
