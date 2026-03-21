# Vertical Drive Motor Sizing & Brake Mechanisms - Research Background

**Date:** 2026-03-20
**Topic:** Motor sizing, self-locking mechanisms, and brake systems for vertical operation of a DIY camera slider carrying 3.5 kg real payload (7 kg design load with 2x safety margin). ESP32-S3 + TMC2209 stepper driver. Investigating torque requirements, self-locking drive options (lead screws, worm gears), electromagnetic brakes, counterbalance systems, and what commercial vertical sliders do.

## Sources

[1]: https://www.linearmotiontips.com/how-to-calculate-motor-drive-torque-for-belt-and-pulley-systems/ "How to calculate motor drive torque for belt and pulley systems - Linear Motion Tips"
[2]: https://www.linearmotiontips.com/calculate-motor-drive-torque-ball-screws/ "How to calculate motor drive torque for ball screws - Linear Motion Tips"
[3]: https://eng.libretexts.org/Bookshelves/Mechanical_Engineering/Mechanics_Map_(Moore_et_al.)/06:_Friction_and_Friction_Applications/6.04:_Power_Screws "Power Screws - Engineering LibreTexts"
[4]: https://www.autolinearmotion.com/how-to-know-whether-lead-screw-can-self-lock.html "How To Know Whether Lead Screw Can Self-lock - ALM"
[5]: https://www.iqsdirectory.com/articles/ball-screw/lead-screws.html "Types, Materials, and Benefits of Lead Screws - IQS Directory"
[6]: https://www.omc-stepperonline.com/nema-17-stepper-motor-0-52nm-73-64oz-in-with-24v-3-7w-electromagnetic-brake-17hs19-2004d-b070 "StepperOnline 17HS19-2004D-B070 NEMA 17 with Electromagnetic Brake"
[7]: https://www.omc-stepperonline.com/nema-17-stepper-motor-0-72nm-101-96oz-in-with-24v-3-7w-electromagnetic-brake-17hs24-2004d-b070 "StepperOnline 17HS24-2004D-B070 NEMA 17 72Ncm with Electromagnetic Brake"
[8]: https://www.omc-stepperonline.com/nema-17-stepper-0-59nm-84oz-in-w-brake-friction-torque-0-25nm-35-4oz-in-17hs19-2004d-b025 "StepperOnline 17HS19-2004D-B025 NEMA 17 59Ncm with 0.25Nm Brake"
[9]: https://www.omc-stepperonline.com/p-series-nema-17-closed-loop-stepper-motor-72ncm-101-98oz-in-with-electromagnetic-brake-17E1KBK05-07 "StepperOnline NEMA 17 Closed Loop with Encoder + Electromagnetic Brake"
[10]: https://edelkrone.com/products/vertical-module-v2-for-sliderplus "edelkrone Vertical Module v2 for SliderPLUS v6"
[11]: https://edelkrone.com/products/sliderone "edelkrone SliderONE v3"
[12]: https://www.diyphotography.net/the-syrp-genie-ii-is-the-perfect-motorized-slider-just-give-us-the-full-app-already/ "Syrp Genie II Review - DIY Photography"
[13]: https://www.cined.com/rhino-rov-an-affordable-portable-motorised-slider-for-smartphones-and-dslrs/ "Rhino ROV Review - CineD"
[14]: https://www.cnccookbook.com/improve-cnc-machine-performance-z-axis-counterbalance/ "Z-Axis Counterbalance - CNC Cookbook"
[15]: https://www.vulcanspring.com/counterbalance-with-constant-force-springs/ "Counterbalance with Constant Force Springs - Vulcan Spring"
[16]: https://creapunk.com/hardware/motion-and-position-control/ "CLN17 Closed-Loop Driver for NEMA 17 - Creapunk"
[17]: https://builds.openbuilds.com/threads/lead-ballscrew-max-speed-calculations.7910/ "Lead/Ballscrew Max Speed Calculations - OpenBuilds"
[18]: https://blog.hirnschall.net/diy-motorized-slider/ "Best DIY motorized DSLR slider (with WiFi) - Hirnschall"
[19]: https://www.omc-stepperonline.com/nema-17-stepper-motor-l-39mm-1-68a-gear-ratio-40-1-square-worm-geabox-17hs15-1684s-wg40 "StepperOnline NEMA 17 40:1 Worm Gearbox"
[20]: https://www.robotshop.com/products/aslong-motor-dual-shaft-worm-gear-nema-17-stepper-motor-reducer-18deg-24v-311 "Worm Gear NEMA 17 31:1 - RobotShop"
[21]: https://www.gearedsteppermotor.com/sale-43169399-42mm-nema-17-high-precision-hybrid-self-locking-worm-gear-stepper-motor-and-gearbox.html "NEMA 17 Self-locking Worm Gear Stepper Motor and Gearbox"
[22]: https://www.omc-stepperonline.com/nema-17-stepper-motor-bipolar-l-48mm-w-gear-ratio-5-1-planetary-gearbox-17hs19-1684s-pg5 "StepperOnline 17HS19-1684S-PG5 NEMA 17 with 5:1 Planetary Gearbox"
[23]: https://www.amazon.com/IFOOTAGE-Motorized-Compatible-Payload-15-4lbs/dp/B0DK6XZGYN "iFOOTAGE Shark Slider Nano II 660 - Amazon"
[24]: https://www.ifootagegear.com/products/shark-slider-mini-bundle "iFOOTAGE Shark Slider Mini"

<!-- Add new sources here as you find them. Number sequentially. -->

## Research Log

---

### Search: "vertical linear motion torque calculation belt drive gravity load stepper motor holding torque"

**Belt drive torque equations for vertical operation** ([Linear Motion Tips][1]):

The total torque at constant velocity is:
- **Tc = (Fa x r1) / eta** where Fa = total axial force (N), r1 = drive pulley radius (m), eta = belt efficiency

For **vertical operation**, the axial force includes the full gravitational component:
- **Fa = m x g** (for pure vertical; friction force is negligible compared to gravity on vertical since it acts perpendicular)
- More precisely: **Fa = m x g x sin(theta) + m x g x cos(theta) x mu** where theta = angle from horizontal, mu = guide friction coefficient

For **acceleration**, additional torque:
- **Tacc = Jt x alpha** where Jt = total system inertia, alpha = angular acceleration
- The load's contribution to inertia reflected to the motor shaft: **Jl = m x (r1)^2** for belt drive (r1 = pulley radius)

Total torque during acceleration upward: **Ta = Tc + Tacc**
Total torque during deceleration (moving downward, braking): **Td = Tc - Tacc** (gravity assists, motor must resist)

**RMS torque** for duty cycle: TRMS = sqrt[(Ta^2 x ta + Tc^2 x tc + Td^2 x td) / t_total]

**Key insight for vertical belt drives:** The motor must continuously supply torque just to hold the load stationary against gravity. If power is cut, the load falls. Belt drives are NOT self-locking.

---

### Search: "ball screw lead screw self-locking lead angle back-driving vertical load"

**Self-locking condition for lead/power screws** ([Engineering LibreTexts][3], [ALM][4]):

The fundamental self-locking criterion:
- **Lead angle lambda < friction angle phi** where tan(phi) = mu_s (static friction coefficient)
- Equivalently: **mu_s >= tan(lambda)**
- Lead angle formula: **lambda = arctan(lead / (pi x d_mean))** where lead = linear travel per revolution, d_mean = mean thread diameter

**Efficiency and self-locking relationship:**
- Screw efficiency: **eta = tan(lambda) / tan(lambda + phi)**
- **Self-locking occurs when efficiency < 50%** (this is a derived consequence of lambda < phi)
- When efficiency > 50%, the screw is "overhauling" (will back-drive under load)

**Specific lead screw configurations known to be self-locking** ([ALM][4]):
- **T8x1 (1mm lead, 1 start)** -- definitely self-locking
- **T8x1.5 (1.5mm lead, 1 start)** -- self-locking
- **T8x2 (2mm lead, 1 start)** -- self-locking (lead angle ~2.5 degrees, well under typical friction angle of 5-8 degrees)

**Ball screws are NOT self-locking** because their rolling element contact has very low friction (eta typically 85-95%), so they always back-drive ([IQS Directory][5]).

**Common lead screw friction coefficients:**
- Brass nut on steel screw: mu ~ 0.08-0.15
- Polymer (POM/Delrin) nut on steel: mu ~ 0.10-0.20
- Bronze nut on steel: mu ~ 0.08-0.12

**Working through T8 examples:**
- T8x2 (2mm lead, 8mm nominal, ~6.5mm mean diameter): lambda = arctan(2 / (pi x 6.5)) = arctan(0.098) = 5.6 degrees. With polymer nut mu=0.15, phi = arctan(0.15) = 8.5 degrees. Since 5.6 < 8.5, **self-locking confirmed**.
- T8x8 (8mm lead, 4 starts): lambda = arctan(8 / (pi x 6.5)) = arctan(0.392) = 21.4 degrees. phi = 8.5 degrees. Since 21.4 >> 8.5, **NOT self-locking** -- will back-drive.
- SFU1605 ball screw (5mm lead, 16mm diameter): Not self-locking regardless of lead angle due to ball bearing efficiency.

**Torque to drive a lead screw with load:**
- **T_raise = (F x d_mean / 2) x (lead + pi x mu x d_mean) / (pi x d_mean - mu x lead)** (for square threads; acme/trapezoidal have a cos(thread_angle) correction)
- **T_lower = (F x d_mean / 2) x (pi x mu x d_mean - lead) / (pi x d_mean + mu x lead)**
- When T_lower > 0, screw is self-locking (needs torque to lower). When T_lower < 0, load will back-drive.

**Follow-up questions:**
- What is the speed penalty of using T8x2 vs belt drive?
- Can we use a T8x2 lead screw for vertical safety but still achieve reasonable video speeds?
- What about electromagnetic brakes on belt-drive systems?

---

### Search: "electromagnetic brake stepper motor NEMA 17 spring-engaged power-off brake vertical axis CNC"

**StepperOnline NEMA 17 motors with electromagnetic brakes** -- multiple models available:

**17HS19-2004D-B070** ([StepperOnline][6]):
- Motor holding torque: 52 Ncm (0.52 Nm)
- **Brake torque: 70 Ncm (0.7 Nm)** -- power-off (spring-engaged) brake
- Brake voltage: 24V, brake power: 3.7W
- Rated current: 2.0A per phase
- Body length: 79mm (motor 48mm + brake ~31mm)
- **Price: $29.26** (vs ~$10-12 for the same motor without brake)
- This is the same 17HS19-2004S motor but with a brake module bolted to the rear shaft

**17HS24-2004D-B070** ([StepperOnline][7]):
- Motor holding torque: 72 Ncm (0.72 Nm) -- higher torque, longer body
- **Brake torque: 70 Ncm (0.7 Nm)** -- same brake module
- Rated current: 2.0A per phase
- Body length: 91mm (motor 60mm + brake ~31mm)

**17HS19-2004D-B025** ([StepperOnline][8]):
- Motor holding torque: 59 Ncm (0.59 Nm)
- **Brake torque: 25 Ncm (0.25 Nm)** -- smaller/cheaper brake
- Rated current: 2.0A

**Closed-loop + brake combo: 17E1KBK05-07** ([StepperOnline][9]):
- Motor holding torque: 72 Ncm
- Built-in **1000-line encoder** for closed-loop operation
- **Electromagnetic brake** included
- This is the ultimate safety solution: closed-loop prevents missed steps AND brake holds on power loss

**How power-off brakes work:** Spring-engaged, electrically released. When 24V is applied to the brake coil, the electromagnetic force overcomes the spring and releases the friction disc, allowing rotation. When power is cut, the spring mechanically forces the armature plate to clamp the friction disc, locking the shaft. This is **fail-safe** -- power loss = brake engaged = load held.

**Key consideration for our project:** The brake adds ~31mm to motor length and requires a separate 24V brake control wire. The ESP32 can control the brake via a MOSFET -- energize to release before moving, de-energize to lock when stopped or on power loss.

**Follow-up questions:**
- Is 70 Ncm brake torque sufficient for our 7 kg vertical load?
- How fast does the brake engage after power cut? (response time)

---

### Search: "edelkrone SliderONE vertical operation mechanism Rhino Arc vertical slider motor drive system teardown"

**edelkrone approach to vertical operation:**

**SliderONE v3** ([edelkrone][11]):
- Motorized backpack slider, very compact
- **Automatically disables hand-control when set up at an incline** to enhance vertical carrying capacity
- Payload capacity: **up to 4 lbs (1.8 kg)** -- this is far below our 3.5 kg real payload
- Uses proprietary compact motor, no published motor specs

**edelkrone Vertical Module v2** ([edelkrone][10]):
- Add-on for SliderPLUS v6
- Uses **edelkrone's patented tension-wire counterbalance system** -- constant-tension springs counterbalance the camera weight so the motor doesn't have to fight gravity
- Price: **$690** just for the vertical module (not including the slider itself)
- Weight: 2,885g (6.4 lbs) -- significant additional weight
- The idea: tension wire + springs create a "weightless" feel, so the motor only needs to provide acceleration force, not gravitational holding force
- **Patented approach** -- uses constant-force springs (like tape measure springs) that provide equal force throughout travel, unlike regular springs that change force with extension

**Key insight from edelkrone:** They explicitly state that "other solutions typically require stronger motors, larger batteries, and result in noisier operation" for vertical use. Their spring counterbalance is specifically designed to avoid needing a more powerful motor.

---

### Search: "Syrp Genie vertical slider payload derating" + "stepper motor holding torque thermal heating continuous hold vertical load"

**Commercial slider vertical capabilities:**

**Syrp Genie II Linear** ([DIY Photography][12]):
- **Powerful enough for vertical operations** -- explicitly stated as a differentiator
- No published payload derating for vertical vs horizontal
- Uses belt drive with internal motor
- Battery powered, compact unit

**Rhino ROV** ([CineD][13]):
- Uses a **coreless DC motor** (not a stepper!) -- interesting design choice
- Payload: up to **5 lbs (2.5 kg)**
- No specific vertical derating found

**Thermal implications of continuous holding** (general NEMA 17 data):
- Stepper motors draw rated current even when holding position (unlike servos that only draw current proportional to load)
- **Power dissipation when holding:** P = I^2 x R x 2 (two phases). For 17HS19-2004S at 2.0A, R=1.4 ohm: P = 4 x 1.4 x 2 = **11.2W** continuous heat in the motor
- Motor temperature rise: NEMA 17 typically rated for **80-100C internal winding temperature**, max ambient usually 50C
- Thermal resistance of NEMA 17 body: approximately 10-15 C/W to ambient (depends on mounting)
- At 11.2W: temperature rise = 11.2 x 12 = **~134C above ambient** -- this will overheat without heat sinking!
- **With TMC2209 CoolStep:** When holding a static load, the driver can reduce current automatically. CoolStep detects the load and adjusts current -- for a vertical hold with belt drive, the load is constant but the motor isn't moving, so CoolStep may not help (it's designed for moving loads)
- **StealthChop at standstill:** TMC2209 uses a different current regulation at standstill that is more efficient. In practice, motors running at holding current do get warm (40-60C case temperature) but don't typically overheat if mounted to aluminum structure as a heatsink

**Key insight:** Continuous holding at full rated current generates significant heat. This is another argument for either (a) a counterbalance to reduce the required holding current, (b) a brake to hold position without motor current, or (c) a self-locking lead screw that holds without any power.

---

### Search: "closed loop stepper motor NEMA 17 holding vertical load encoder correction" + "constant force spring counterbalance vertical axis DIY"

**Closed-loop stepper for vertical operation:**

**CLN17 closed-loop driver** ([Creapunk][16]):
- Uses **TMC2209** as the motor driver -- compatible with our existing driver choice
- Uses **Infineon TLE5012B magnetic angle sensor** with 15-bit resolution (0.01 degrees)
- Effective practical resolution: **12,800 positions per revolution** (1/64 interpolation recommended)
- Includes integrated **6-axis IMU** (LSM6DSO) for vibration monitoring and collision detection
- **Key limitation:** No specific data on power consumption for holding vertical loads. However, closed-loop systems can reduce holding current by monitoring the encoder and only applying as much current as needed to maintain position -- in theory more efficient than open-loop for static holding.

**Closed-loop advantages for vertical operation:**
- Detects missed steps in real-time and corrects them
- Can **automatically adjust current based on load** -- under low load, current drops to minimum, reducing motor noise and heat
- For a vertical holding scenario: the encoder detects if the shaft starts to rotate under gravity load and applies corrective current. This should be more efficient than open-loop because it only uses the current needed to resist the actual gravitational torque, not full rated current.

**Z-axis counterbalance methods from CNC world** ([CNC Cookbook][14]):

Three main approaches:
1. **Counterweights** -- simple cables/chains over pulleys. Drawback: doubles the moving mass (bad for acceleration), needs space for the weight travel path
2. **Gas springs** -- compact, good for small machines. Drawback: force changes with compression (not constant), and gas springs weaken over time
3. **Constant-force springs** -- provide uniform force throughout travel regardless of extension. This is what edelkrone uses. ([Vulcan Spring][15])

**Constant-force springs for DIY:**
- Available from industrial suppliers (Vulcan Spring, McMaster-Carr, various on Amazon)
- Shaped like a coiled strip of spring steel (like a tape measure)
- **Force is nearly constant** throughout the entire extension range
- Can be stacked or paralleled to increase force
- For our 7 kg design load: need a constant force spring providing ~68.7N (7 kg x 9.81 m/s^2)
- Amazon/AliExpress have constant force springs in the 10-50N range for $5-15 each
- Would likely need **2-3 springs in parallel** to counterbalance the full 7 kg design load, or 1-2 for the 3.5 kg real load

**Benefits of counterbalancing for our project:**
- Reduces motor torque requirement to near-zero for holding (only friction + acceleration needed)
- Eliminates thermal concerns for continuous holding
- Reduces power consumption dramatically for vertical operation
- Allows the same motor (17HS19-2004S, 59 Ncm) to work for both horizontal and vertical without upgrade
- **Drawback:** Additional mechanical complexity, spring needs to be adjustable for different payload weights, adds weight to the slider assembly

---

### Search: "T8x2 lead screw maximum speed RPM critical speed" + "reddit DIY vertical camera slider"

**T8x2 lead screw speed limits:**

**Critical speed (whip limit) for T8 lead screws** ([OpenBuilds][17]):
- For an 8mm diameter screw with "Rigid/Free" bearing support:
  - 250mm length: **426 RPM safe** (80% of critical)
  - 500mm length: **106 RPM safe**
  - 1000mm length: **26 RPM safe**
- These are extremely conservative figures for the worst bearing configuration. With "Fixed/Fixed" support (both ends constrained), critical speed roughly quadruples.

**Speed calculation for T8x2 (2mm lead):**
- At 500mm length, 106 RPM safe: linear speed = 106 x 2mm = **212 mm/min = 3.5 mm/s** -- extremely slow!
- At 1000mm length, 26 RPM safe: linear speed = 26 x 2mm = **52 mm/min = 0.87 mm/s** -- virtually useless for video
- Even with fixed/fixed bearings (4x improvement): 500mm -> 424 RPM -> 848 mm/min = **14 mm/s** -- still very slow compared to belt drive's 500+ mm/s

**Comparison: T8x8 (8mm lead, 4 starts) -- NOT self-locking:**
- Same RPM limits but 4x the linear speed
- At 500mm, 106 RPM: 106 x 8 = 848 mm/min = **14 mm/s** -- still slow but more usable
- At 1000mm, 26 RPM: 26 x 8 = 208 mm/min = **3.5 mm/s** -- still bad

**Key conclusion: T8x2 lead screw is impractically slow for camera slider video work.** It would be fine for timelapse (where you want very slow movement) but completely inadequate for repositioning or real-time video slides.

**DIY vertical slider experiences from forums** ([Hirnschall][18]):
- The Hirnschall build used a **40:1 worm gear** specifically to solve the vertical/self-locking problem
- Worm gears are inherently self-locking at high ratios (>20:1)
- 40:1 worm gear with NEMA 17 pancake (0.13 Nm): output torque = 0.13 x 40 x ~0.5 (worm efficiency) = ~2.6 Nm -- massively more than needed
- Speed: motor at 1000 RPM / 40 = 25 RPM output. With GT2 20-tooth pulley: 25 x 40mm = 1000 mm/min = **16.7 mm/s** -- slow but usable for video slides
- Main complaint: **speed limitation.** Worm gear dramatically reduces maximum speed.
- Other forum users report similar challenges: vertical operation is the #1 reason builds fail or need redesign

---

### Search: "worm gear NEMA 17 self-locking specifications" + "StepperOnline NEMA 17 brake motor price"

**Worm gear NEMA 17 options:**

**StepperOnline 17HS15-1684S-WG40** ([StepperOnline][19]):
- 40:1 worm gearbox integrated with NEMA 17
- Input motor: 42 Ncm, 1.68A
- Output torque: ~17 Nm (theoretical) but actual limited by gearbox ratings
- **Self-locking: YES** -- worm gears with ratios >15:1 are typically self-locking
- 90-degree output shaft configuration
- Backlash: typically 1-2 degrees at output

**ASLONG worm gear NEMA 17 models** ([RobotShop][20]):
- Available in 17:1, 31:1, 50:1, 100:1 ratios
- 24V rated, dual-shaft options available
- Typical output torques by ratio:
  - 17:1 = ~1.0 Nm output
  - 31:1 = ~1.8 Nm output
  - 50:1 = ~2.9 Nm output
- Self-locking at all ratios above ~15:1

**Generic NEMA 17 worm gear specs** ([GearStepperMotor][21]):
- Self-locking by design -- "the gearing system inside a worm gearbox is self-locking by nature"
- Backlash: 15-30 arcmin (0.25-0.5 degrees) at output
- Efficiency: typically 40-60% for single-stage worm gears (energy lost to friction is what creates self-locking)
- Price range: $25-50 on Amazon/AliExpress for integrated motor+gearbox units

**Electromagnetic brake motor pricing:**
- **17HS19-2004D-B070** (52 Ncm motor + 70 Ncm brake): **$29.26** ([StepperOnline][6])
- **17HS19-2004D-B025** (59 Ncm motor + 25 Ncm brake): likely ~$20-25
- **SWB-01 standalone brake** (0.25 Nm, fits NEMA 17): ~$15 separate
- **17E1KBK05-07** (closed-loop + encoder + brake): likely ~$50-60

**Speed implications of worm gear approach for our slider:**
- With a 31:1 worm gear + GT2 20-tooth pulley (40mm per rev):
  - Motor at 500 RPM: output = 16 RPM -> 640 mm/min = **10.7 mm/s**
  - Motor at 1000 RPM: output = 32 RPM -> 1280 mm/min = **21.3 mm/s**
  - Motor at 2000 RPM: output = 64 RPM -> 2560 mm/min = **42.7 mm/s**
- With a 10:1 planetary gear (NOT self-locking but faster):
  - Motor at 1000 RPM: output = 100 RPM -> 4000 mm/min = **66.7 mm/s**
- For comparison, direct belt drive at 1000 RPM: **667 mm/s** -- 30x faster

---

### Search: "NEMA 17 planetary gearbox 5:1 stepper motor belt drive" + "iFOOTAGE Shark slider vertical payload capacity"

**Planetary gearbox options for NEMA 17** ([StepperOnline][22]):
- **17HS19-1684S-PG5** -- 5:1 planetary gearbox with NEMA 17 (48mm body)
- Input motor: 45 Ncm (1.68A), output after 5:1: **~200 Ncm effective** (accounting for ~90% gear efficiency)
- Total length: ~80mm (motor + gearbox)
- **NOT self-locking** -- planetary gears are back-drivable at all ratios
- Backlash: typically <1 degree (much better than worm gears)
- This would need a brake for vertical safety

**Speed with 5:1 planetary + GT2 20T belt:**
- Motor at 1000 RPM: output = 200 RPM -> 8000 mm/min = **133 mm/s** -- usable for video!
- Motor at 500 RPM: output = 100 RPM -> 4000 mm/min = **67 mm/s** -- decent for cinematic
- Torque at output: 45 Ncm x 5 x 0.9 = **~200 Ncm** -- massive overkill for 7 kg vertical
- **This is the best of both worlds:** enough speed for video, enough torque for vertical, just needs a brake

**iFOOTAGE commercial slider vertical payload data -- critical real-world derating data:**

**iFOOTAGE Shark Slider Nano II 660** ([Amazon][23]):
- Horizontal payload: **15.4 lbs (7 kg)**
- **Vertical payload: 7.7 lbs (3.5 kg)** -- exactly 50% derating!
- Uses brushless motors (not steppers)
- Compatible with DJI RS 2/RS3 Pro/RS 4 gimbals -- exactly our use case

**iFOOTAGE Shark Slider Mini** ([iFOOTAGE][24]):
- Standard payload: ~6.6 lbs with L-plate for vertical
- Uses "flywheel and beltless track" -- proprietary drive system
- "Original flywheel" provides momentum for smooth movement

**iFOOTAGE Shark Slider Nano (original):**
- Horizontal: 7.7 lbs (3.5 kg)
- **Vertical: 4.4 lbs (2.0 kg)** -- 57% derating
- Tripod-mounted: 5.5 lbs (2.5 kg) -- intermediate derating

**Key finding: Commercial sliders typically derate vertical payload to 50-57% of horizontal capacity.** This confirms that vertical operation requires roughly double the motor capability compared to horizontal.

---
