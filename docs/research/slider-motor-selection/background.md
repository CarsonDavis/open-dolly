# Stepper Motor & Drive System Selection for Camera Slider - Research Background

**Date:** 2026-03-20
**Topic:** Selecting the right stepper motor (NEMA size, torque, model) and drive mechanism (belt, lead screw, ball screw) for a DIY motorized camera slider carrying ~15 lbs (7 kg), driven by TMC2209 + ESP32-S3.

## Sources

[1]: https://blog.hirnschall.net/diy-motorized-slider/ "Best DIY motorized DSLR slider (with WiFi) - Hirnschall"
[2]: https://www.smoothmotor.com/a-news-nema-23-vs-nema-17-choosing-the-right-stepper-motor-for-your-needs "NEMA 23 vs NEMA 17 - Smooth Motor"
[3]: http://blanch.org/belts-vs-screws-in-cnc-design/ "Belts vs Leadscrews and Ballscrews for CNC Design - Blanch.org"
[4]: https://emotimo.com/products/geared-stepper-motor "eMotimo Geared Stepper Motor"
[5]: https://www.bhphotovideo.com/c/product/1164219-REG/rhino_sku111_motorized_studio_slider_bundle.html "Rhino Motorized Studio Slider - B&H"
[6]: https://www.omc-stepperonline.com/nema-17-bipolar-59ncm-84oz-in-2a-42x48mm-4-wires-w-1m-cable-connector-17hs19-2004s1 "StepperOnline 17HS19-2004S1 59Ncm NEMA 17"
[7]: https://www.omc-stepperonline.com/nema-17-bipolar-1-8deg-65ncm-92oz-in-2-1a-3-36v-42x42x60mm-4-wires-17hs24-2104s "StepperOnline 17HS24-2104S 65Ncm NEMA 17"
[8]: https://www.linearmotiontips.com/how-to-calculate-motor-drive-torque-for-belt-and-pulley-systems/ "How to calculate motor drive torque for belt and pulley systems - Linear Motion Tips"
[9]: https://learn.watterott.com/silentstepstick/faq/ "SilentStepStick FAQ - Watterott"
[10]: https://forum.arduino.cc/t/driver-tmc-2209-v3-1-adapted-to-nema-23-stepper-motor/953574 "TMC2209 with NEMA 23 - Arduino Forum"
[11]: https://www.analog.com/media/en/technical-documentation/data-sheets/tmc2209_datasheet_rev1.09.pdf "TMC2209 Datasheet - Analog Devices"
[12]: https://hackaday.com/2016/08/29/how-accurate-is-microstepping-really/ "How Accurate Is Microstepping Really? - Hackaday"
[13]: https://www.wemacro.com/?product=micromate "WeMacro MicroMate Focus Stacking Rail"
[14]: https://github.com/gin66/FastAccelStepper "FastAccelStepper Library - GitHub"
[15]: https://www.omc-stepperonline.com/nema-17-bipolar-45ncm-64oz-in-2a-42x42x40mm-4-wires-w-1m-cable-connector-17hs16-2004s1 "StepperOnline 17HS16-2004S1 45Ncm NEMA 17"

<!-- Add new sources here as you find them. Number sequentially. -->

## Research Log

---

### Search: "DIY motorized camera slider stepper motor sizing NEMA 17 vs NEMA 23 heavy payload"

- **Hirnschall DIY slider** used a NEMA 17 pancake motor (only 0.13 Nm holding torque) with a **40:1 worm gear** to get enough torque. The worm gear's self-locking property means no power needed to hold position. However, this build was designed for only ~2 kg payload — far below our 7 kg requirement. ([Hirnschall][1])
- **The torque calculation from Hirnschall:** 2 kg load at 90 degrees requires at least 0.157 Nm. Rated holding torque is NOT running torque — motors deliver less torque when spinning. A standard NEMA 17 pancake at 0.13 Nm was insufficient even for 2 kg without gearing. ([Hirnschall][1])
- **Scaling to our 15 lb (7 kg) payload:** If 2 kg needs 0.157 Nm, 7 kg would need ~0.55 Nm minimum just for vertical operation, before accounting for acceleration and safety margin. This puts us solidly in the range of high-torque NEMA 17 (0.4-0.65 Nm) or low-end NEMA 23 territory.
- **General NEMA 23 vs 17 comparison:** NEMA 23 frame is 2.3" (58mm), NEMA 17 is 1.7" (42mm). NEMA 23 offers higher torque, better heat dissipation, but is larger/heavier/more expensive. NEMA 17 is recommended for cameras and consumer electronics; NEMA 23 for CNC and industrial. ([Smooth Motor][2])
- **Key insight from Hirnschall:** Using a worm gear adds complexity and cost, and limits maximum speed. For our use case needing both speed AND precision, direct belt drive with a higher-torque motor may be preferable.

**Follow-up questions:**
- What specific torque is needed to move 7 kg on a horizontal slider with GT2 belt? On an incline?
- What NEMA 17 models offer 40-65 Ncm and are compatible with TMC2209 (under 2A)?

---

### Search: "GT2 belt vs lead screw vs ball screw camera slider precision speed comparison"

**Critical data from blanch.org CNC drive comparison** ([Blanch.org][3]):

- **Resolution with a 200-step motor (no microstepping):**
  - 2mm lead screw: **9.5 microns/step**
  - 8mm lead screw (T8): **38 microns/step**
  - 10mm belt pulley (GT2 20-tooth): **148 microns/step**
  - 20mm belt pulley (GT2 40-tooth): **296 microns/step**
- **With 16x microstepping**, these divide by 16: GT2 20-tooth = ~9.3 microns, T8 lead screw = ~2.4 microns. Both sufficient for focus stacking.
- **Force output from a 2Nm stepper:**
  - 2mm lead screw: 320 kg actual (with 50% efficiency loss)
  - 8mm lead screw (T8): 112 kg actual (70% efficiency)
  - 10mm belt pulley: 40.8 kg (98% efficiency)
  - 20mm belt pulley: 20.4 kg
- **Speed:** Belt drives can do up to 5 m/s. Lead screws limited by whip — an 8mm lead screw at 500mm length has critical RPM of 725 (= 5800 mm/min = 96 mm/s). At 1000mm, only 322 RPM (= 2576 mm/min = 43 mm/s). **Belts are 10-50x faster than lead screws.**
- **Backlash:** Ball screws (C7 rolled) have ±52 microns per 300mm. Lead screws depend on anti-backlash nut. Belts have stretch rather than backlash — a 2mm GT2 belt needs ~1.72 kg force for 1% elongation, but with preload stiffness factor of 4x the effective tolerance is much better.
- **Cost hierarchy:** Belts cheapest ($5-15), then lead screws ($10-20), then ball screws ($120-275 for quality).
- **Belt stretch:** 3mm GT3 belt 15mm wide needs 7.5 kg for 1% elongation simple, ~30 kg real-world with preload. Dual belts double this.

**Key takeaway for our slider:** GT2 belt with 20-tooth pulley provides excellent speed (can do 2+ m/s) and with TMC2209 256x microstepping interpolation, resolution would be theoretical 148 microns / 256 = **0.58 microns per microstep** — far beyond what's needed for focus stacking. Real-world accuracy won't be that good due to belt elasticity, but even at 10x worse it's fine. Lead screw provides inherently better precision but severely limits speed.

**Follow-up questions:**
- Can we use belt for speed + accept its precision for focus stacking?
- What about a hybrid approach — belt for speed, and rely on microstepping for focus stacking precision?

---

### Search: "Rhino edelkrone eMotimo camera slider motor specifications"

**Commercial slider drive systems:**

- **Rhino EVO slider:** Uses a **belt-driven system**. The Rhino Motion motor pulls the rubber belt through gears to move the camera carriage. Claims **0.7 micron resolution**. Single compact motor with 7-hour built-in battery. ([Rhino - B&H][5])
- **eMotimo:** Offers both **NEMA 17 and NEMA 23** options. Their product line uses planetary gearboxes for torque multiplication: ([eMotimo][4])
  - **NEMA 17 5:1 geared** — fast enough for video interviews, can't do slopes
  - **NEMA 17 14:1 geared** — strong enough for video and timelapse, handles most slopes
  - **NEMA 17 27:1 geared** — strongest, half the speed but twice the torque of 14:1
  - **NEMA 17 100:1 geared** — very slow, monster torque (rare)
  - **NEMA 17 direct drive** — fastest motor, paired with shark slider for interviews
  - **NEMA 23 direct drive** — fastest big frame motor, paired with Dana Dolly (heavier loads)
- **edelkrone SliderPLUS v6:** Uses stepper motor with "CNC-level precision." Minimum speed of **0.6 mm/sec** for macro shots. No published motor specs — proprietary integrated design.

**Key insight:** Commercial sliders overwhelmingly use **belt drives** (Rhino, most DIY builds). eMotimo is notable for offering geared NEMA 17 options — the 14:1 planetary gear is their sweet spot for balancing speed vs torque. For heavy loads (Dana Dolly), they step up to NEMA 23 direct drive.

**Implication for our build:** Belt drive is the industry standard. A high-torque NEMA 17 with direct belt drive should work for most scenarios, but the option to add a planetary gear (5:1 or 14:1) gives us a path to handle slopes if direct drive isn't enough.

**Follow-up questions:**
- What specific NEMA 17 high-torque models are available that work with TMC2209?
- What is the actual torque needed for a 7 kg load on a belt-driven horizontal slider?

---

### Search: "NEMA 17 stepper motor torque speed curve 59Ncm TMC2209 24V performance"

**Specific motor models identified:**

- **StepperOnline 17HS19-2004S1** — The gold standard NEMA 17 for this application: ([StepperOnline][6])
  - Holding torque: **59 Ncm** (84 oz-in)
  - Rated current: **2.0A per phase** (perfect TMC2209 match at 2A RMS)
  - Voltage: 2.8V (rated), but driven at 12-24V with chopper driver
  - Phase resistance: **1.4 ohms**
  - Inductance: **3.0 mH** (decent — not too high for speed, not too low for holding)
  - Step angle: 1.8 degrees (200 steps/rev)
  - Body: 42x42x48mm, weight 390g
  - Price: ~$10-12

- **StepperOnline 17HS24-2104S** — Longest NEMA 17 body for max torque: ([StepperOnline][7])
  - Holding torque: **65 Ncm** (92 oz-in)
  - Rated current: **2.1A per phase** (slightly over TMC2209's 2A RMS — would need to run at ~95% or use TMC2226)
  - Voltage: 3.36V
  - Body: 42x42x**60mm** (12mm longer than 17HS19)
  - Heaviest NEMA 17 option

**Torque at speed (general NEMA 17 behavior):** At 24V supply, a typical NEMA 17 retains about 50% of holding torque at 500 RPM and about 20-30% at 1000 RPM. Higher supply voltage helps maintain torque at speed — this is why 24V is strongly preferred over 12V. At 12V, torque drops off much faster above 300 RPM.

**TMC2209 compatibility note:** The 17HS19-2004S1 at 2.0A is an exact match. The 17HS24-2104S at 2.1A is very close but technically slightly over spec — running it at 2.0A would sacrifice only ~5% of holding torque (from 65 to ~62 Ncm), which is negligible and still above the 17HS19's 59 Ncm.

**Follow-up questions:**
- What is the actual speed achievable with a GT2 belt + 20-tooth pulley + 200-step motor at 200k steps/sec?
- Need to calculate torque requirements for our specific payload.

---

### Search: "stepper motor torque calculation linear motion belt drive payload force friction"

**Torque calculation formulas** ([Linear Motion Tips][8]):

The key formulas for belt-drive torque:

1. **Constant velocity torque:** `Tc = (Fa × r1) / η`
   - Fa = total axial force (N), r1 = drive pulley radius (m), η = belt efficiency (~0.95-0.98)
2. **Axial force:** `Fa = m × g × μ` (horizontal) or `Fa = m × g × (μ × cos θ + sin θ)` (inclined)
   - μ = coefficient of friction of the linear guide
3. **Acceleration torque:** `Ta = Tc + (Jt × α)`
   - Jt = total system inertia, α = angular acceleration

**Our specific calculation (GT2 belt, 20-tooth pulley):**

Pulley: 20-tooth GT2 = 20 × 2mm pitch / π = **12.73mm diameter**, radius r1 = **6.37mm = 0.00637m**

**Case 1: Horizontal sliding (worst-case friction)**
- Mass m = 7 kg, μ = 0.01 (ball bearing linear guide), g = 9.81
- Fa = 7 × 9.81 × 0.01 = **0.69 N**
- Tc = 0.69 × 0.00637 / 0.95 = **0.0046 Nm = 0.46 Ncm**
- This is trivially easy — any NEMA 17 has 100x this torque

**Case 2: 45-degree incline**
- Fa = 7 × 9.81 × (0.01 × cos 45° + sin 45°) = 7 × 9.81 × (0.007 + 0.707) = **48.9 N**
- Tc = 48.9 × 0.00637 / 0.95 = **0.328 Nm = 32.8 Ncm**
- A 59 Ncm motor has 1.8x safety factor — adequate but not generous

**Case 3: Vertical (90 degrees)**
- Fa = 7 × 9.81 × 1.0 = **68.7 N**
- Tc = 68.7 × 0.00637 / 0.95 = **0.460 Nm = 46.0 Ncm**
- A 59 Ncm motor has only 1.28x safety margin at holding torque. AT SPEED, torque drops — this would likely stall above 200-300 RPM.

**Case 4: Acceleration (horizontal, 1 m/s² target)**
- Additional force = m × a = 7 × 1.0 = 7 N
- Additional torque = 7 × 0.00637 / 0.95 = 0.047 Nm = 4.7 Ncm
- Total for horizontal acceleration: 0.46 + 4.7 = **5.2 Ncm** — still trivial

**Key conclusion:** On a horizontal slider, torque requirements are extremely low. **The motor is massively overkill for flat slides.** The constraint only appears on inclines: at 45 degrees, a 59 Ncm motor is adequate; at 90 degrees (vertical), it's marginal. For a slider that may be tilted to 30-45 degrees, a 59 Ncm NEMA 17 is sufficient with a small safety margin.

**Follow-up questions:**
- What about using a larger pulley (e.g., 16-tooth instead of 20-tooth) to reduce torque requirement on inclines?
- Actually, a SMALLER pulley gives more mechanical advantage but less speed. Might want to check.

---

### Search: "TMC2209 StealthChop noise" + "TMC2209 NEMA 23 compatibility TMC5160"

**TMC2209 noise characteristics:**

- **StealthChop mode** produces "almost silent" operation. SpreadCycle is "more powerful and louder." ([SilentStepStick FAQ][9])
- Sound gets quieter with motor supply voltage above 18V in SpreadCycle mode. ([SilentStepStick FAQ][9])
- For quietest operation, motors should have **phase voltage <=4V and inductance <=4mH**. The 17HS19-2004S1 (2.8V, 3.0mH) fits perfectly. ([SilentStepStick FAQ][9])
- StealthChop works best at low-to-medium speeds — perfect for cinematic slides. At higher speeds, the driver can auto-transition to SpreadCycle (configurable).
- **256 microstep interpolation** is always active in TMC2209, even when receiving lower microstep commands. This smooths out motion and reduces resonance noise.

**TMC2209 with NEMA 23:**

- TMC2209 rated for **2.0A RMS, 2.8A peak**. ([TMC2209 Datasheet][11])
- Many NEMA 23 motors need **2.8-4.2A per phase**, making TMC2209 insufficient. ([Arduino Forum][10])
- Running TMC2209 at its absolute max with a 2.8A NEMA 23 leaves "no margin" — not recommended. ([Arduino Forum][10])
- **Low-current NEMA 23 motors exist** (some are rated 1.0-2.0A) and would work with TMC2209, but these have lower torque (typically 80-130 Ncm) — not much more than a big NEMA 17.
- **TMC5160 upgrade:** Handles up to 3.2A RMS with external MOSFETs. Recommended for full-size NEMA 23 (2.8A+). Also supports StealthChop. More expensive ($15-25 vs $5-8 for TMC2209).
- **TMC2226:** Drop-in replacement for TMC2209 with slightly higher current (2.0A RMS same, but better thermal handling). Not a significant upgrade for NEMA 23.

**Key conclusion for our project:** If we stick with **NEMA 17 (59 Ncm, 2.0A)**, the TMC2209 is a perfect match. Upgrading to NEMA 23 only makes sense for vertical operation with heavy loads, but it would require upgrading to TMC5160, adding cost and complexity. Given our torque calculations show NEMA 17 is sufficient for horizontal and moderate inclines, **TMC2209 + NEMA 17 is the right choice.**

---

### Search: "focus stacking rail precision stepper motor microstep accuracy belt drive 10 micron"

**Microstepping accuracy — the reality check** ([Hackaday][12]):

- **Critical finding:** Microstepping provides smooth motion and vibration reduction, but **NOT proportional positioning accuracy**. Tested on a NEMA 17, the actual shaft position error was **+/- 1 to 2 full steps** regardless of microstep level.
- At 256 microsteps, the incremental holding torque per microstep is only **0.61% of full-step torque**. Any friction or load can deflect the shaft by multiple microsteps.
- Under 25% load (1000 g-cm on a 4000 g-cm motor), deflection exceeded **half a full step**.
- **Implication for belt-driven focus stacking:** With a 20-tooth GT2 pulley, one full step = 0.2mm (200 microns). If real-world error is +/- 1-2 full steps, that's +/- 200-400 microns — **NOT sufficient for focus stacking** which needs 10-50 micron precision.
- **However:** With a belt-driven slider, the load on the motor during focus stacking (horizontal, very slow movement) is extremely low — under 1% of holding torque. At such low loads, microstepping accuracy should be much better than the Hackaday test (which used 25% load).
- **Dedicated focus stacking rails** (WeMacro, Novoflex CASTEL-MICRO) use **lead screws with gear reduction** — NOT belts. The WeMacro achieves 1 micron minimum step size. Novoflex uses a 50:1 gear ratio. ([WeMacro][13])

**Focus stacking with belt drive — the honest assessment:**
- Belt stretch and elasticity are the real enemy for focus stacking precision. Even if the motor can microstep accurately, the belt acts as a spring — small movements get absorbed by belt compliance rather than moving the carriage.
- For focus stacking specifically, a lead screw or ball screw would provide dramatically better results due to rigid mechanical coupling.
- **Possible compromise:** Use belt drive for general slider motion and accept that focus stacking precision will be limited to ~50-100 microns realistically (sufficient for moderate macro, but not extreme). For serious focus stacking, users would need a separate dedicated macro rail anyway.

**Follow-up questions:**
- What is the real-world belt precision for slow, low-load movements?
- Should we recommend belt for v1 and note lead screw as a possible future option?

---

### Search: "stepper motor maximum speed GT2 belt FastAccelStepper ESP32" + "StepperOnline NEMA 17 45Ncm 17HS16"

**Speed calculations** ([FastAccelStepper][14]):

- FastAccelStepper on ESP32-S3 achieves up to **200,000 steps/sec** using RMT or MCPWM/PCNT drivers.
- With I2S MUX mode: limited to 40 kHz (40,000 steps/sec).

**Speed with GT2 20-tooth pulley (direct drive, no microstepping):**
- Linear travel per step = (20 teeth × 2mm pitch) / 200 steps = **0.2 mm/step**
- At 200,000 steps/sec = 200,000 × 0.2 mm = **40,000 mm/s = 40 m/s** — impossibly fast, limited by motor physics
- Motor RPM at 200k steps/sec: 200,000 / 200 = **1000 RPM** — this is achievable for an unloaded NEMA 17 at 24V, but torque is very low at this speed

**Speed with 16x microstepping (typical operation):**
- 200,000 microsteps/sec / 16 = 12,500 full steps/sec
- Linear: 12,500 × 0.2mm = **2,500 mm/s = 2.5 m/s** — excellent fast repositioning speed
- Motor RPM: 12,500 / 200 × 60 = **3,750 RPM** — too fast for any useful torque. Realistically limited to ~1000-1500 RPM at 24V = ~0.5-1.0 m/s with 16x microstepping.

**Realistic maximum speed for our system:**
- At 24V with 16x microstepping, practical max ~**500-800 mm/s** (0.5-0.8 m/s) with meaningful torque
- At full-step mode (for fast repositioning only), could reach **1.5-2.0 m/s** with very light horizontal load
- This is competitive with commercial sliders (Rhino does ~1 m/s max)

**Third motor option — StepperOnline 17HS16-2004S1:** ([StepperOnline][15])
- Holding torque: **45 Ncm** (64 oz-in)
- Rated current: **2.0A per phase**
- Body: 42x42x**40mm** — shortest of the three options (8mm shorter than 17HS19)
- **Trade-off:** 24% less torque than the 17HS19 (45 vs 59 Ncm), but more compact. Still sufficient for horizontal and mild inclines (45 Ncm vs our calculated 33 Ncm needed at 45 degrees).

---

<!-- Continue appending search entries below. One entry per search, every time. -->
