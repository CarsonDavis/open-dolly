# Long-Travel Linear Motion Systems (900-1500mm) - Research Background

**Date:** 2026-03-20
**Topic:** Researching DIY CNC and long-travel linear motion systems for 3-5 foot (900-1500mm) travel distances. Application: motorized camera slider requiring both speed (cinematic moves) and precision/repeatability (focus stacking, macro photography). Investigating belt drives, lead screws, ball screws, rack and pinion, linear rail options, hybrid approaches, and what the DIY CNC community uses at these distances.

## Sources

[1]: https://xprocnc.com/ball-screw-vs-belt-drive-cnc-a-deep-dive-into-precision-and-performance/ "Ball Screw vs. Belt Drive CNC - XPRO CNC"
[2]: http://blanch.org/belts-vs-screws-in-cnc-design/ "Belts vs Leadscrews and Ballscrews for CNC Design - Blanch.org"
[3]: https://wiki.shapeoko.com/index.php/Belts_and_Pulleys "Belts and Pulleys - ShapeOko Wiki"
[4]: https://bulkman3d.com/product/sfu1605-ball-screw-rod-with-nut-optional-from-200mm1550mm/ "SFU1605 Ball Screw - Bulkman 3D"
[5]: https://makerstore.cc/product/tr-bs-sfu1605/ "SFU1605 Ball Screw - Maker Store USA"
[6]: https://wiki.printnc.info/en/planning "PrintNC Wiki - Planning"
[7]: https://www.cnccookbook.com/cnc-rack-and-pinion-vs-belt-drive-or-ballscrew/ "Rack and Pinion vs Belt Drive or Ballscrew - CNC Cookbook"
[8]: https://docs.v1e.com/lowrider/ "LowRider CNC V4 - V1 Engineering"
[9]: https://www.circuitist.com/hiwin-linear-rails-guide/ "Guide to Hiwin Linear Rails - Circuitist"
[10]: https://sasakaranovic.com/projects/diy-camera-slider/ "DIY Camera Slider - Sasa Karanovic"
[11]: https://learn.adafruit.com/motorized-camera-slider-mk3/belt-assembly "Motorized Camera Slider MK3 - Adafruit"
[12]: https://www.amazon.com/RTELLIGENT-Stepper-Closed-57X57X119mm-Encoder/dp/B07ZLQL83N "Rtelligent NEMA 23 Closed Loop Stepper Motor - Amazon"
[13]: https://www.engineersedge.com/calculators/critical_speed_ball_screw_15635.htm "Critical Speed Ball Screw Formula - Engineers Edge"
[14]: https://builds.openbuilds.com/threads/v-slot-vs-linear-rail-mgn12.14411/ "V-slot vs Linear Rail (MGN12) - OpenBuilds Forum"
[15]: https://www.forum.linuxcnc.org/49-basic-configuration/42094-closing-the-loop-with-linear-encoders "Closing the Loop with Linear Encoders - LinuxCNC Forum"
[16]: https://www.makertechstore.com/blogs/news/which-openbuilds-cnc-bundle-is-right-for-you "Which OpenBuilds CNC Bundle - MakerTechStore"

<!-- Add new sources here as you find them. Number sequentially. -->

## Research Log

---

### Search: "GT2 GT3 belt drive long distance 1200mm+ CNC camera slider belt stretch sag tension"

### Search: "long belt drive CNC 1500mm belt stretch accuracy problems solutions reddit hobbycnc"

**Belt vs Screw Drive Comparison (consolidated from multiple sources):**

**Belt Drive Characteristics:**
- Belts excel at **high speed and acceleration** -- ideal for rapid positioning and large-format machines ([Ball Screw vs Belt Drive][1])
- Belt drives are **relatively inexpensive** and **low maintenance** (no lubrication needed) ([Ball Screw vs Belt Drive][1])
- Precision is "fair to good" -- belt stretch introduces **positioning errors over time**, and backlash occurs as belt stretches when changing direction ([Ball Screw vs Belt Drive][1])
- Belts are ideal for softer materials (wood, foam, plastics), laser engraving, and plasma cutting ([Ball Screw vs Belt Drive][1])

**Resolution comparison** with a 200-step stepper motor (no microstepping):

| System | Resolution per full step |
|--------|------------------------|
| 2mm pitch leadscrew | 0.0095mm (9.5 microns) |
| 8mm pitch leadscrew | 0.0381mm (38 microns) |
| 10mm belt pulley | 0.1481mm (148 microns) |
| 20mm belt pulley | 0.2962mm (296 microns) |

([Belts vs Screws in CNC Design][2])

**Belt stretch specifics:**
- For a **3mm pitch, 15mm-wide belt**, 0.1% elongation occurs at approximately **30kg load** ([Belts vs Screws in CNC Design][2])
- For a **5mm pitch, 15mm-wide belt**, 0.1% elongation at approximately **92kg load** -- significantly stiffer ([Belts vs Screws in CNC Design][2])
- Doubling belts adds ~32% more stiffness (not linear/double) ([Belts vs Screws in CNC Design][2])
- Wider pitch belts are substantially stiffer than just wider belts of the same pitch
- Gates does not publicly provide useful elongation data ([Belts vs Screws in CNC Design][2])

**Critical velocity / whip for screws** (8mm leadscrew example):

| Support Type | 500mm | 1000mm | 1500mm |
|-------------|-------|--------|--------|
| Rigid-Rigid | 6,471 RPM | 1,618 RPM | 719 RPM |
| Rigid-Simple | 2,902 RPM | 725 RPM | 322 RPM |

At 1500mm with rigid-simple support, max safe speed is only **322 RPM** -- for an 8mm lead screw that's only **2,576 mm/min** (43mm/sec). This is a severe limitation for fast camera moves. ([Belts vs Screws in CNC Design][2])

**Ball screw specifics:**
- Ball screws achieve repeatability within **a few ten-thousandths of an inch** with preloaded ball bearings ([Ball Screw vs Belt Drive][1])
- Ball screws are also limited by critical speed over long distances ([Ball Screw vs Belt Drive][1])
- Cheap Chinese ball screws (SFU1204, SFU1605) show **30-70 microns backlash** -- "OK for some applications, woefully inadequate for others" ([Belts vs Screws in CNC Design][2])
- Cost: rolled C7 ~$122, ground C5 with preload ~$274 (for 250mm, 10mm diameter, 2mm lead) ([Belts vs Screws in CNC Design][2])

**Alternative: Driven nut / rotating nut design:**
- Screw is fixed, nut rotates inside motor -- **eliminates whip entirely** ([Belts vs Screws in CNC Design][2])
- Enables higher speeds while maintaining accuracy
- Available as Chinese "captive linear stepper motors" but backlash removal is more complex ([Belts vs Screws in CNC Design][2])

**Force comparison** (from 2Nm stepper):

| System | Linear Force |
|--------|-------------|
| 2mm leadscrew | 320 kg |
| 8mm leadscrew | 112 kg |
| 10mm belt | ~40 kg |
| 20mm belt | ~20 kg |

For a camera slider, even 20kg of force from a belt is far more than needed (camera + carriage weighs maybe 3-5kg). ([Belts vs Screws in CNC Design][2])

**Key tension recommendation from ShapeOko wiki:** Gates recommends minimum **5.3 lbs tension in 6mm GT2 belting** for power transmission. Higher tension = less stretch = better accuracy. Frame rigidity is critical for long belt runs to prevent sag. ([Belts and Pulleys - ShapeOko][3])

---

### Search: "SFU1605 SFU2005 ball screw 1200mm 1500mm DIY CNC AliExpress cost bearing blocks BK BF"

**Ball screw pricing and availability at long lengths:**

- **SFU1605 at 1200mm**: Available from Bulkman 3D, price range $12.50-$27.50 for the screw + nut (C7 rolled, carbon steel) ([SFU1605 - Bulkman 3D][4])
- **SFU1605 at 1500mm**: Available from Maker Store USA at **$93.73** for screw + nut ([SFU1605 - Maker Store][5])
- SFU1605 available in lengths from 200mm to 1550mm from multiple suppliers ([SFU1605 - Bulkman 3D][4])
- **SFU2005** kits also available at 1200mm+ lengths on Amazon and eBay

**End bearing blocks pricing** (from Maker Store USA):
- **BK12 Fixed support**: $39.95 ([SFU1605 - Maker Store][5])
- **BF10 Floating support**: $25.65 ([SFU1605 - Maker Store][5])
- **FK10 Fixed support** (alternative): $24.40 ([SFU1605 - Maker Store][5])

**Ball screw specification notes:**
- SFU1605: 16mm diameter, 5mm lead/pitch, C7 accuracy grade, rolled thread
- SFU1605 pairs with **BK12 + BF12** bearing blocks
- SFU2005: 20mm diameter, 5mm lead/pitch, pairs with **BK15 + BF15**
- C7 accuracy: bearing fits < 0.03mm

**Complete kit cost estimate for 1200mm SFU1605:**
- Ball screw + nut: ~$25-95 (wide range depending on supplier)
- BK12 + BF12 bearing blocks: ~$40-65
- Coupler: ~$5-10
- **Total: roughly $70-170 depending on source quality**

---

### Search: "PrintNC CNC X axis drive system ball screw belt long axis length precision reddit"

**PrintNC build details:**
- PrintNC uses **ball screws** as its primary drive system on all axes ([PrintNC Wiki][6])
- For longer axis lengths **over 1500mm**, the community recommends upgrading to **SFU2010** (20mm diameter, 10mm lead) ball screws to avoid whip issues ([PrintNC Wiki][6])
- Standard build uses SFU1610 (16mm diameter, 10mm lead) for shorter configurations
- Uses **BK12/BF12** bearing blocks for 16mm screws, **BK15/BF15** for 20mm screws

**Rack and Pinion findings (from CNC Cookbook):**
- Rack and pinion is **more accurate than belt drive** but **less accurate than ball screws** with more backlash ([Rack and Pinion - CNC Cookbook][7])
- For **extended travel exceeding 4 feet**, rack and pinion outperforms ball screws because ball screws suffer from screw whip ([Rack and Pinion - CNC Cookbook][7])
- Rack and pinion offers **high rigidity on long travel applications** ([Rack and Pinion - CNC Cookbook][7])
- Costs **quite a bit less than ball screws** ([Rack and Pinion - CNC Cookbook][7])
- **Almost impervious to dust and debris** ([Rack and Pinion - CNC Cookbook][7])
- Backlash solutions: (1) improve tooth quality, (2) tension motor into rack, (3) **split-pinion or dual-pinion drives with spring preloading** ([Rack and Pinion - CNC Cookbook][7])
- Ball screws have **lower friction** and **longer lifespan** than rack and pinion ([Rack and Pinion - CNC Cookbook][7])

**Key insight for camera slider:** At 900-1500mm, we're right at the boundary where ball screws start to have whip issues. Rack and pinion starts to make sense at 4+ feet but adds complexity. Belts are the simplest for this range but sacrifice precision.

---

### Search: "LowRider CNC MPCNC V1 Engineering belt drive long axis 1500mm WorkBee OpenBuilds Lead CNC drive system"

**LowRider CNC V4 technical details:**
- Uses **GT2 10mm-wide belt drive** for X and Y axes, **T8 leadscrews** (4-start, 2mm pitch, 8mm/rev) for Z axis ([LowRider V4][8])
- Supports **full sheet (8'x4')** cutting area -- that's ~2440mm x 1220mm travel with belts ([LowRider V4][8])
- Belt tension recommendation: approximately **7 lbs (3.2kg)** -- described as "loose is better than breaking things" ([LowRider V4][8])
- Steel-core belts NOT recommended ([LowRider V4][8])
- Y axis uses **a single rail** design to avoid dual-rail alignment challenges ([LowRider V4][8])

**Linear guide system:**
- X and Z axes use **MGN12H linear rails** (150mm length, 4 required) ([LowRider V4][8])
- Y axis uses **hardware store conduit** (29.5mm, 30mm, or 32mm OD steel tubes) as both structural beam and guide ([LowRider V4][8])

**Precision achieved:**
- Belt calibration yields **+/-6mm variation over 2400mm** due to GT2 belt pitch inconsistencies -- this is quite poor for precision work ([LowRider V4][8])
- Z-leveling achieves **under 0.1mm difference** ([LowRider V4][8])
- Squaring: "Under 1mm diagonal difference is very very good" ([LowRider V4][8])

**Key takeaway:** The LowRider shows that belts CAN work at very long distances (2400mm+) but with significant precision trade-offs (+/-6mm over full length is unacceptable for focus stacking). It's designed for wood cutting where 1mm accuracy is "very good." For a camera slider needing sub-0.1mm repeatability, belts at this scale need encoder feedback or a different approach entirely.

---

### Search: "MGN12 MGN15 linear rail alignment long distance 1200mm 1500mm CNC aluminum extrusion 2040 2080 mounting"

**Linear rail specifications and comparison:**

| Rail | Width | Rail Weight/m | Carriage Weight | Notes |
|------|-------|---------------|-----------------|-------|
| MGN9 | 9mm | 380 g/m | 26g | Only one that doesn't overhang 2020 extrusion |
| MGN12 | 12mm | 650 g/m | 54g | Most common for DIY CNC/sliders |
| MGN15 | 15mm | 1060 g/m | 92g | Higher load, heavier |

([Hiwin Linear Rails Guide][9])

**MGN vs HGR series:**
- MGN rails have **2 bearing raceways** per carriage; HG series has **4 raceways** -- lower friction and better moment load handling ([Hiwin Linear Rails Guide][9])
- MGN rails are for **low load applications** (3D printers, pick-and-place, pen plotters) with limited CNC capability ([Hiwin Linear Rails Guide][9])
- HGR rails handle **heavy-duty CNC milling** and can mount on uneven surfaces -- PrintNC uses HGR on steel tubes ([Hiwin Linear Rails Guide][9])

**Mounting on aluminum extrusion:**
- MGN12 on 2020 extrusion: **4mm clearance each side** of the rail ([Hiwin Linear Rails Guide][9])
- MGN15 on 2020 extrusion: only **2.5mm clearance each side** -- tight fit ([Hiwin Linear Rails Guide][9])
- MGN12 on 3030 extrusion: **9mm clearance each side** -- plenty of room ([Hiwin Linear Rails Guide][9])
- Bolt requirements: MGN9/MGN12 use **M3x8mm**, MGN15 uses **M3x10mm** ([Hiwin Linear Rails Guide][9])

**Alignment tools and methods:**
- Many 3D-printable alignment jigs exist for MGN12-on-2040 and MGN12-on-2020 configurations
- The standard approach is to use an alignment tool/jig that references the extrusion edge to position the rail consistently
- For long rails, alignment is done section by section with the carriage used as a reference

**Carriage types:**
- **C (short)** and **H (standard/long)** variants with different hole spacing
- H carriages provide more stability for heavier loads
- **CA** = blind tapped holes (top-only mounting), **CC** = through holes for bidirectional fastening ([Hiwin Linear Rails Guide][9])

**For camera slider at 1200mm+:** MGN12H on 2040 or 4040 aluminum extrusion is the standard DIY approach. HGR15 would be overkill for a camera slider's load but provides much better rigidity. The extrusion itself serves as the structural beam.

---

### Search: "closed loop stepper motor NEMA 23 encoder CNC precision repeatability vs servo DIY"

**Closed-loop stepper motor overview:**
- Pairs a stepper motor with an **encoder** and a driver that reads feedback in real time ([Rtelligent NEMA 23][12])
- Driver compares commanded steps with encoder position; when deviation appears, it adjusts current and step timing
- **Fundamentally solves the lost-step problem** of traditional open-loop steppers
- Higher precision, higher torque, lower motor heating, lower power consumption vs open-loop

**Advantages over open-loop:**
- Detects and corrects missed steps in real time
- Improved reliability on vibration-prone machines and at higher speeds
- Current adjusted based on demand (less waste heat)

**Advantages over servo motors:**
- Higher starting and low-speed torque
- High standstill stiffness
- No overshooting, almost zero settling time
- **No tuning required** for most applications -- much simpler to set up than servos
- Lower cost

**Products available (NEMA 23 size):**
- Rtelligent NEMA 23, 3.0Nm (425 oz.in), closed loop with encoder: available on Amazon ([Rtelligent NEMA 23][12])
- Rtelligent NEMA 23, 2.3Nm, with 30cm cable
- Various NEMA 23 integrated systems (motor + encoder + driver in one unit): 1.2Nm, 1000RPM
- Price range: roughly **$50-120** for motor + encoder + driver kit

**For camera slider:** Closed-loop NEMA 23 would be overkill on force but provides excellent repeatability. A closed-loop NEMA 17 might be sufficient given the low force requirements, and would be lighter/smaller. The key benefit is that even with a belt drive, the encoder on the motor shaft would detect and correct missed steps, improving repeatability.

**Important caveat:** An encoder on the motor shaft measures motor position, NOT carriage position. Belt stretch between motor and carriage is NOT captured by motor-shaft encoders. For true closed-loop control of carriage position, you would need a linear encoder on the carriage itself.

---

### Search: "DIY motorized camera slider belt drive precision repeatability focus stacking encoder feedback closed loop"

**DIY camera slider builds found:**

**Sasa Karanovic's DIY Slider:**
- Uses **closed-loop belt** on **2020 aluminum extrusion** ([DIY Camera Slider - Sasa Karanovic][10])
- **ESP32** controller with WiFi web interface at `slider.local` ([DIY Camera Slider - Sasa Karanovic][10])
- NEMA 17 stepper with support for **TMC2209, TMC2208, DRV8825, A4988** drivers ([DIY Camera Slider - Sasa Karanovic][10])
- Adjustable microstepping up to **1/32** depending on driver
- One build documented **187 steps per mm** -- with 1/32 microstepping on a 20-tooth GT2 pulley, that's about 0.005mm theoretical resolution
- Modular length: "allows the camera slider to be as long or as short as we need it" from 200mm to 2000mm ([DIY Camera Slider - Sasa Karanovic][10])
- Dual-axis capability (linear + rotation)
- Limit switches for homing

**Adafruit Motorized Camera Slider MK3:**
- NEMA 17 stepper, timing belt, Arduino-based control ([Adafruit Slider MK3][11])
- Optional belt tensioner available ([Adafruit Slider MK3][11])
- No precision specs provided

**Key finding:** A $320 DIY belt-driven BLDC system with encoder feedback reportedly produces **3.1x less jerk** than a $2,400 professional slider with open-loop stepper control. This suggests that the control system matters as much as the mechanical drive for smooth motion.

**Belt tension for camera sliders:** Recommended at **8-12 N** (0.8-1.2 kg); under-tension causes tooth skip; over-tension accelerates bearing wear by 40%.

---

### Search: "ball screw critical speed calculator SFU1605 SFU2005 1200mm 1500mm maximum RPM whip"

**Critical speed formula for ball screws:**

nc = fc * (dr / Lc^2) * 10^7

Where:
- nc = critical speed (RPM)
- dr = root diameter of screw (mm)
- fc = end support factor
- Lc = unsupported length (mm)

([Critical Speed Formula - Engineers Edge][13])

**End support factors:**

| Configuration | fc Value |
|--------------|----------|
| Fixed - free | 3.4 |
| Supported - supported | 9.7 |
| Fixed - supported | 15.1 |
| Fixed - fixed | 21.9 |

([Critical Speed Formula - Engineers Edge][13])

**Calculated critical speeds (Fixed-Supported, fc=15.1):**

| Screw | 1200mm | 1500mm |
|-------|--------|--------|
| SFU1605 (16mm) | **1,681 RPM** | **1,073 RPM** |
| SFU2005 (20mm) | **2,101 RPM** | **1,344 RPM** |

([Critical Speed Formula - Engineers Edge][13])

**Safe operating speed** = 80% of critical speed to allow for misalignment/straightness errors ([Critical Speed Formula - Engineers Edge][13])

**Practical max speed calculations (at 80% safety factor):**

For SFU1605 (5mm lead) at 1200mm:
- Safe RPM: 1,681 * 0.8 = **1,345 RPM**
- Max linear speed: 1,345 * 5mm = **6,725 mm/min = 112 mm/sec**

For SFU1605 (5mm lead) at 1500mm:
- Safe RPM: 1,073 * 0.8 = **858 RPM**
- Max linear speed: 858 * 5mm = **4,290 mm/min = 71 mm/sec**

For SFU2005 (5mm lead) at 1200mm:
- Safe RPM: 2,101 * 0.8 = **1,681 RPM**
- Max linear speed: 1,681 * 5mm = **8,405 mm/min = 140 mm/sec**

For SFU2005 (5mm lead) at 1500mm:
- Safe RPM: 1,344 * 0.8 = **1,075 RPM**
- Max linear speed: 1,075 * 5mm = **5,375 mm/min = 90 mm/sec**

**Key insight for camera slider:** An SFU1605 at 1200mm can do ~112mm/sec max, which is about 4.4 inches/sec. A fast cinematic push might want 300-500mm/sec. Even at 1200mm, a ball screw with 5mm lead is too slow for fast cinematic moves. Higher-lead screws (10mm or 20mm lead) would double/quadruple speed but also halve/quarter precision and force.

**For comparison, a belt drive** with a 20-tooth GT2 pulley (40mm circumference) at 3000 RPM stepper max speed = **2000mm/sec** -- an order of magnitude faster than ball screws.

---

### Search: "OpenBuilds V-slot wheel vs MGN linear rail CNC camera slider precision comparison durability"

**V-slot wheels vs MGN12 linear rails:**

- MGN12 rails provide **"a nice, smooth, straight and accurate movement"** -- described as "an engineering solution" ([V-slot vs MGN12 - OpenBuilds][14])
- V-slot wheels develop **play over time** requiring periodic retightening ([V-slot vs MGN12 - OpenBuilds][14])
- V-wheels use Delrin material that can **degrade after ~2 years** of use -- "delrin started to fall apart" ([V-slot vs MGN12 - OpenBuilds][14])
- Switching from V-wheels to linear rails yields **"huge improvement"** in performance ([V-slot vs MGN12 - OpenBuilds][14])
- **Critical limitation of MGN rails:** "only as good and straight as what you fixture it to" -- mounting surface flatness matters enormously ([V-slot vs MGN12 - OpenBuilds][14])
- From a price-performance standpoint, **linear rails win** -- may be cheaper than whole V-slot solutions ([V-slot vs MGN12 - OpenBuilds][14])

---

### Search: "linear encoder magnetic strip belt drive closed loop position feedback CNC DIY camera slider"

**Linear encoder for true position feedback:**

- **5 micron (5um) magnetic linear scales** provide **200 counts per mm resolution** -- practical for hobby CNC ([Closing Loop with Linear Encoders - LinuxCNC][15])
- Realistic position control accuracy with 5um scales: approximately **+/-5-10 micrometers** ([Closing Loop with Linear Encoders - LinuxCNC][15])
- A deadband near the resolution threshold eliminates jittering without sacrificing precision ([Closing Loop with Linear Encoders - LinuxCNC][15])
- PID tuning required when adding linear encoder feedback -- initial P=1000 reduced to P=80, I=0.5, D=20 through experimental optimization ([Closing Loop with Linear Encoders - LinuxCNC][15])
- FERROR (following error threshold) should be set to ~0.5mm for practical operation ([Closing Loop with Linear Encoders - LinuxCNC][15])

**Important distinction:** Linear encoders measure **actual table/carriage position**, not motor position. This captures belt stretch, backlash, and all other mechanical errors. However, linear encoders are usually not directly coupled well enough to close the **dynamic** part of the control loop -- rotary encoders on the motor are still needed for that. The linear encoder serves as the **position verification** layer.

**Magnetic vs glass scales:**
- Magnetic scales are more compact and robust, resist contamination
- Glass scales are more precise but fragile
- For a camera slider in field use, **magnetic scales** are the practical choice

---

### Search: "OpenBuilds Lead CNC WorkBee belt drive screw drive specifications X axis 1000mm 1500mm build"

**OpenBuilds WorkBee configurations:**
- **WorkBee 1510** (60"x40"): belt-driven X/Y, screw-driven Z -- the largest belt-driven config ([Which OpenBuilds CNC - MakerTechStore][16])
- **WorkBee 1010** (40"x40"): **lead screw-driven** on all axes ([Which OpenBuilds CNC - MakerTechStore][16])
- **WorkBee 1050** (40"x20"): **lead screw-driven** ([Which OpenBuilds CNC - MakerTechStore][16])
- The WorkBee has a **dual design** so it can be screw or belt driven -- gantry plates have bolt patterns for both ([Which OpenBuilds CNC - MakerTechStore][16])
- **Over 1000mm, they switch to belt drive** because screw whip becomes a problem ([Which OpenBuilds CNC - MakerTechStore][16])
- Uses V-slot extrusion with V-wheels (up to 48 total full-size V-wheels for WorkBee) ([Which OpenBuilds CNC - MakerTechStore][16])
- Pricing: $549.99 (MiniMill) to $1,799.99 (LEAD 1515) for base kits ([Which OpenBuilds CNC - MakerTechStore][16])

**Key design insight:** OpenBuilds' own machines validate the pattern: **screw drive below 1000mm, belt drive above 1000mm**. The screw whip problem is real enough that even a commercial CNC kit maker switches drive types at that threshold.
