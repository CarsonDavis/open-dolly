# Linear Guide and Rail System for DIY Camera Slider - Research Background

**Date:** 2026-03-20
**Topic:** Selecting the linear guide rail system for a motorized camera slider with 900-1200mm travel, 7kg payload capacity, all-orientation operation (including inverted). Comparing MGN rail sizes, aluminum extrusion profiles, carriage types, and mounting methods. Must support GT2 belt drive alongside the rail.

## Sources

[1]: https://www.dks-bearing.com/hiwin-mgn12h-linear-guideways/ "HIWIN MGN12H Linear Guideways - DKS Bearing"
[2]: https://www.dks-bearing.com/hiwin-mgn15h-linear-guideways/ "HIWIN MGN15H Linear Guideways - DKS Bearing"
[3]: https://www.circuitist.com/hiwin-linear-rails-guide/ "Guide to Hiwin Linear Rails - Circuitist"
[4]: https://builds.openbuilds.com/threads/how-to-calculate-v-slot%C2%AE-deflection.4881/ "How to Calculate V-Slot Deflection - OpenBuilds"
[5]: https://hts-alu.com/aluminum-t-slot-4040/ "T-slot 4040 Aluminum Extrusion - HTS"
[6]: https://www.automationwerks.com/products/tslots-40-4040-lite-40mmx40mm-extrusion-650033/2217522000016375960 "40 Series 4040 T-Slot Extrusion - AutomationWerks"
[7]: https://www.ifixit.com/Guide/MGN+Linear+Guide/143887 "MGN Linear Guide - iFixit"
[8]: https://hackaday.com/2024/08/25/v-slot-wheels-or-linear-rails/ "V-Slot Wheels or Linear Rails? - Hackaday"
[9]: https://builds.openbuilds.com/threads/v-slot-vs-linear-rail-mgn12.14411/ "V-Slot vs Linear Rail (MGN12) - OpenBuilds"
[10]: https://forum.duet3d.com/topic/31069/linear-rail-headaches "Linear Rail Headaches - Duet3D Forum"
[11]: https://www.printables.com/model/450861-mgn9-mgn12-linear-rail-2020-alignment-tools "MGN9 & MGN12 Linear Rail 2020 Alignment Tools - Printables"
[12]: https://www.printables.com/model/321212-mgn12-linear-rail-alignment-tool-for-2040-extrusio "MGN12 Linear Rail Alignment Tool for 2040 - Printables"
[13]: https://www.bhphotovideo.com/c/product/1536491-REG/syrp_sykit_0015h_magic_carpet_carbon_slider.html "Syrp Magic Carpet Carbon Slider Kit 48in - B&H"
[14]: https://www.bhphotovideo.com/c/product/1255826-REG/ifootage_s1s_shark_slider_s1.html "iFootage Carbon Fiber Shark Slider S1 - B&H"
[15]: https://rhinocameragear.com "Rhino Camera Gear"
[16]: https://digital-photography-school.com/rhino-slider-pro-4ft-review-camera-rail-system-for-time-lapse-and-video/ "Rhino Slider PRO 4ft Review"
[17]: https://zendamotion.com/linearailvslinearod/ "Linear Rail vs Linear Rod - Zenda Motion"
[18]: https://www.zyltech.com/zyltech-mgn12-h-type-linear-rail-with-single-or-double-carriage-block/ "Zyltech MGN12 H-TYPE Linear Rail - ZYLtech"
[19]: https://www.zyltech.com/2040-t-slot-aluminum-extrusion-pre-cut-lengths-300mm-2000mm/ "ZYLtech 2040 T-Slot Extrusion Pre-cut"
[20]: https://kb-3d.com/store/motion/377-kb3d-mgn12h-linear-rail-guide-with-carriage-multiple-lengths-1646161437836.html "KB3D MGN12H Linear Rail"
[21]: https://www.printables.com/model/983547-gt2-6mm-belt-idler-pulley-mount-for-v-slot-2020204 "GT2 6mm Belt Idler Pulley Mount for V-Slot 2020/2040 - Printables"

<!-- Add new sources here as you find them. Number sequentially. -->

## Research Log

---

### Search: "MGN12 vs MGN15 linear rail load rating static dynamic comparison specifications"

**MGN12H carriage specifications** (HIWIN standard) ([DKS Bearing MGN12H][1]):
- Dynamic load rating (C): **3.72 kN** (379 kg / 836 lbs)
- Static load rating (C0): **5.88 kN** (599 kg / 1321 lbs)
- Moment ratings: MR = 38.22 N-m, MP = 36.26 N-m, MY = 36.26 N-m
- Block dimensions: 27mm wide x 45.4mm long
- Rail: 12mm wide x 8mm high, 0.65 kg/m
- Carriage weight: 54g

**MGN15H carriage specifications** (HIWIN standard) ([DKS Bearing MGN15H][2]):
- Dynamic load rating (C): **6.37 kN** (649 kg / 1432 lbs)
- Static load rating (C0): **9.11 kN** (929 kg / 2048 lbs)
- Moment ratings: MR = 73.5 N-m, MP = 57.82 N-m, MY = 57.82 N-m
- Block dimensions: 32mm wide x 58.8mm long
- Rail: 15mm wide x 10mm high, 1.06 kg/m
- Carriage weight: 92g

**Analysis for 7 kg (15 lbs) payload:**
- Our payload of 7 kg = 68.7 N of force. Even a single MGN12H carriage has a static rating of 5,880 N -- that's **85x our load**. Two carriages would share the load.
- The moment load is the more relevant concern. A 7 kg camera hanging 150mm below the rail centerline creates a moment of 7 * 9.81 * 0.15 = **10.3 N-m**. The MGN12H moment rating of 36-38 N-m provides a **3.5x safety factor** even on a single carriage. With two carriages the moment is shared.
- MGN15H is significant overkill for this application. Its moment ratings are ~2x higher than MGN12H, but we don't need them.
- **MGN12H is more than adequate for 7 kg at any orientation.** The 3.5x moment safety factor on a single carriage (7x with two carriages) is comfortable.

Rail weight comparison for 1200mm:
- MGN12: 1200mm rail = 780g, plus 2 carriages = 108g, total **888g**
- MGN15: 1200mm rail = 1272g, plus 2 carriages = 184g, total **1,456g**
- MGN15 adds ~570g (1.25 lbs) for capacity we don't need.

**Follow-up questions:**
- How do cheap Chinese MGN12 rails compare to genuine HIWIN? Load ratings may be derated.
- What about MGN12C (short carriage) -- lighter but lower moment capacity?

---

### Search: "aluminum extrusion 2020 2040 4040 beam deflection moment of inertia" (multiple searches consolidated)

**Moment of inertia values for V-slot aluminum extrusion profiles** ([OpenBuilds Deflection Guide][4], [HTS 4040][5], [AutomationWerks 4040][6]):

| Profile | Ix (cm^4) | Iy (cm^4) | Ix (mm^4) | Iy (mm^4) | Weight (kg/m) |
|---------|-----------|-----------|-----------|-----------|---------------|
| V-slot 2020 | 0.74 | 0.74 | 7,400 | 7,400 | ~0.5 |
| V-slot 2040 (short axis) | 1.21 | 4.75 | 12,100 | 47,500 | ~0.9 |
| V-slot 4040 (standard) | ~9.7 | ~9.7 | ~97,000 | ~97,000 | ~1.5 |

Note: V-slot 2040 has very different stiffness depending on orientation. Ix=12,100 mm^4 is bending around the 20mm (short) axis; Iy=47,500 mm^4 is bending around the 40mm (tall) axis. For a slider beam loaded vertically (gravity), you want the tall axis resisting deflection.

**Material: 6063-T5 aluminum, E = 68.9 GPa = 68,900 N/mm^2**

**Deflection calculations -- simply supported beam, center point load:**

Formula: delta = P * L^3 / (48 * E * I)

Where: P = 7 kg * 9.81 = 68.7 N, L = 1200 mm, E = 68,900 N/mm^2

| Profile | I (mm^4) | Deflection at center (mm) | Notes |
|---------|----------|---------------------------|-------|
| 2020 | 7,400 | **4.86 mm** | Way too much -- visible sag |
| 2040 (loaded on 20mm face, tall) | 47,500 | **0.76 mm** | Acceptable, borderline |
| 2040 (loaded on 40mm face, short) | 12,100 | **2.97 mm** | Too much -- wrong orientation |
| 4040 | 97,000 | **0.37 mm** | Excellent stiffness |

**Key insight: orientation of 2040 matters enormously.** The 2040 loaded on its 20mm face (so the 40mm dimension is vertical, resisting deflection) gives 0.76mm -- acceptable but not great. This is the correct orientation for a slider beam. Loaded the other way (40mm face horizontal), it deflects nearly 3mm.

**Torsional stiffness consideration:** A camera hanging off one side of the rail creates a torque that wants to twist the extrusion. V-slot 2020 has very low torsional stiffness. V-slot 2040 is better because the 40mm height gives more torsional resistance, but still limited by the 20mm width. 4040 is best for torsion since it's square with more material.

**Weight comparison for 1200mm beam:**
- 2020: ~600g
- 2040: ~1,080g
- 4040: ~1,800g

**Follow-up questions:**
- Does adding the MGN12 rail to the extrusion improve stiffness? The rail is steel (E=200 GPa) and adds cross-section.
- What about 2060 extrusion as a middle ground?
- How much torsional deflection is acceptable?

---

### Search: "MGN carriage types CC through-bolt, H vs C, inverted operation" (multiple searches consolidated)

**Carriage types explained** ([Guide to Hiwin Linear Rails][3]):

- **H (standard length):** Longer carriage body with 4 mounting holes. Better moment load resistance due to longer bearing contact length. The standard choice for most applications.
- **C (compact/short):** Shorter carriage with 2 mounting holes. Lighter, fits in tighter spaces, but lower moment load capacity. For our application with a heavy offset payload, the H type is preferred.
- **CA (blind tapped holes):** Bolts thread into the carriage from the top only. Cannot be bolted from below.
- **CC (through holes):** Holes go all the way through the carriage. Can be bolted from either direction -- top or bottom. **This is what we want for inverted operation**, since the carriage plate bolts through from above when the slider is flipped.

**Inverted operation confirmed:** MGN linear rails work in **any orientation** -- horizontal, vertical, inverted, or angled. The carriage is mechanically captured on the rail by the ball recirculation channels and cannot fall off. The Gothic arch contact design of the ball raceways provides load capacity in all four directions (up, down, left, right relative to the rail). ([Guide to Hiwin Linear Rails][3])

**MGN12C vs MGN12H load comparison:**
- MGN12H: C0 = 5.88 kN, MR = 38.22 N-m, MP = 36.26 N-m ([DKS Bearing MGN12H][1])
- MGN12C: C0 = 3.43 kN, MR = 14.71 N-m, MP = 10.78 N-m (from HIWIN catalog -- roughly 40% of MGN12H moment capacity)

**Recommendation: MGN12H-CC** -- the standard-length carriage with through holes. It provides the best moment load capacity in the MGN12 family and allows bolting from either direction for maximum mounting flexibility across all orientations.

**Follow-up questions:**
- Are MGN12H-CC carriages readily available from budget suppliers (AliExpress, Amazon)?

---

### Search: "V-slot wheels vs MGN linear rail for inverted/all-orientation operation"

**V-slot wheels limitations for inverted operation** ([Hackaday][8], [OpenBuilds Forum][9]):

- V-slot wheels rely on **eccentric nut preload** to clamp the wheels against the V-groove. This preload prevents play but does not provide a positive mechanical capture -- the wheels ride on top of the V-groove profile.
- When inverted, V-slot wheels can technically work IF the eccentric nut preload is tight enough to hold the carriage against gravity. But this is **unreliable under dynamic loads** (acceleration, vibration, camera weight shifts). The preload adjustment is finicky and changes over time as Delrin wheels wear.
- **Delrin wheels degrade after ~2 years** of use, developing flat spots or becoming non-round. This would be unacceptable for a camera slider used intermittently over years. ([OpenBuilds Forum][9])
- V-slot wheels generate **rubber/plastic dust** from wear, which is cosmetically undesirable near camera equipment. ([Hackaday][8])

**MGN rail advantages for all-orientation operation:**
- The carriage is **mechanically captured** on the rail -- it physically cannot come off without removing the end caps. No adjustment needed for orientation changes.
- No wear-related looseness -- steel balls on hardened steel rail have much longer service life than Delrin on aluminum V-groove.
- Higher precision and rigidity under lateral loads. ([Hackaday][8])

> "Linear rail is good, but it's only as good and straight as what you fixture it to." -- Craig Cavanaugh, OpenBuilds forum ([OpenBuilds Forum][9])

**Verdict: V-slot wheels are NOT suitable for all-orientation operation.** They can work horizontal-only, but for a slider that must work inverted, vertical, and at arbitrary angles, MGN linear rails are the only viable option. The mechanical capture of the MGN carriage is the key differentiator.

**Follow-up questions:**
- None -- this question is decisively answered. MGN rail wins for all-orientation.

---

### Search: "mounting MGN12 linear rail on aluminum extrusion alignment methods"

**Alignment methods for MGN12 on extrusion** ([Duet3D Forum][10], [Printables][11], [Printables 2040][12]):

**Method 1: 3D-printed alignment jigs (most common for DIY)**
- Print small clips that straddle both the rail and extrusion edge, centering the rail on the extrusion face.
- Place 2-3 jigs along the 1200mm length, snug bolts with jigs in place, then remove jigs and final-tighten.
- Free STL files available for both 2020 and 2040 extrusion profiles. ([Printables][11], [Printables 2040][12])
- Accuracy is limited by 3D print tolerance (~0.1-0.2mm) and extrusion edge straightness.

**Method 2: Precision straight edge / ground parallel**
- Clamp a precision ground parallel bar alongside the rail as a reference edge.
- Tighten bolts sequentially while the rail is pushed against the straight edge.
- More accurate than 3D-printed jigs, but requires owning a precision straight edge. ([Duet3D Forum][10])

**Method 3: Machined reference groove**
- Machine a shallow groove or shoulder into the mounting surface that the rail sits in.
- The groove constrains the rail laterally. This is the industrial method.
- Not practical for T-slot extrusion without custom machining. ([Duet3D Forum][10])

**Critical insights about rail straightness:**
- **The rail gets its straightness from the mounting surface.** A single MGN rail has no straightness specification -- it conforms to whatever it's bolted to. ([Duet3D Forum][10])
- **Do NOT over-torque rail screws.** HIWIN specifies 0.8 N-m for MGN rail mounting screws. Over-tightening forces the rail to conform to surface imperfections in the extrusion, introducing kinks. ([Duet3D Forum][10])
- **Aluminum extrusions are typically straight to ~0.5mm/m.** Over 1200mm, that's ~0.6mm potential deviation. For a camera slider this is acceptable -- the carriage smooths over small deviations due to its bearing compliance.
- **The extrusion quality matters more than the rail quality** for final straightness. Buy quality extrusion from a reputable supplier (Misumi, 80/20, OpenBuilds).

**Practical recommendation for this project:**
1. Use quality 2040 or 4040 extrusion (not the cheapest AliExpress extrusion)
2. Start with 3D-printed alignment jigs -- they're free and get you to ~0.2mm alignment
3. Hand-tighten rail screws to ~0.8 N-m, working from center outward
4. Test by sliding carriage end-to-end -- if it's smooth, alignment is good enough for camera work

**Follow-up questions:**
- How many M3 screws does a 1200mm MGN12 rail need? (Rail has mounting holes every 25mm, so 48 holes, but you don't need to use all of them.)

---

### Search: "commercial camera slider guide systems - Rhino, edelkrone, Syrp, iFootage"

**Survey of commercial slider guide mechanisms:**

**Rhino Slider PRO (4ft)** ([Rhino Camera Gear][15], [Rhino Review][16]):
- Uses **precision-machined stainless steel rods** (dual round rods) with linear bearings
- Stainless steel rails support up to **50 lbs** (studio/cinema configuration)
- Carbon fiber rail option: lighter, supports up to **15 lbs** -- sufficient for DSLR but not cinema
- Drive: rubber belt pulled through gears by motor, 0.7 micron resolution
- The rods work at this length because they are **continuously supported** by the slider body -- the bearing carriage rides along them, not spanning unsupported. However, this means the slider body must be rigid enough to keep the rods straight.
- Interchangeable rail system -- carbon fiber for field, stainless for studio

**Syrp Magic Carpet Carbon** ([Syrp Magic Carpet][13]):
- Uses **carbon fiber tube tracks** with **8 ball bearing roller carriage**
- Roller bearings ride on the outside of the carbon fiber tubes
- Supports up to **11 lbs** (5 kg) -- lighter-duty, not suitable for our 7 kg cinema setup
- Modular track system: 24", 48", 72" configurations via extension pieces
- 48" version weighs just 5 lbs total -- extremely portable
- The bearings are so smooth that users actually want more damping/resistance

**iFootage Shark Slider S1** ([iFootage Shark S1][14]):
- Uses **carbon fiber rails** with **silent Japanese ball bearings**
- Has a **flywheel** mechanism for inertia/damping -- unique feature
- Supports up to **15 lbs** (7 kg) horizontal, **7.7 lbs** (3.5 kg) vertical
- Note: vertical/inverted capacity is **half** the horizontal capacity -- this is a common limitation of rod/bearing systems
- Synchronous belt drive for motorized option

**MYT Works (professional cinema sliders)**:
- Uses **patented sleeved bearing blocks** on extruded aluminum rails
- Hybrid rolling-sliding mechanism: rolling reduces friction, sliding prevents overshoot/backlash
- Double-extruded one-piece aluminum rails for rigidity at all lengths
- Supports very heavy cinema payloads (50-150 lbs depending on model)
- The rigid, monolithic rail design is what allows long travel without center support

**Key observations:**
- **No commercial slider uses MGN-style linear rails.** They all use either round rods with bearings, roller bearings on tracks, or custom profiled rails. MGN rails are a CNC/3D-printer component repurposed for DIY slider builds.
- **Commercial sliders that claim "any orientation" often derate for vertical/inverted use.** The iFootage drops from 15 lbs to 7.7 lbs for vertical. This is because roller/rod bearings have less capture force than MGN rails.
- **None of the commercial sliders use rods spanning unsupported at 1200mm.** When rods are used (Rhino), they're supported by the slider body. The rods don't span -- the carriage rides along them.
- **The heaviest-duty commercial sliders use custom extruded aluminum rails** (MYT Works), not standard round rods or MGN rails.
- **For DIY, MGN rail on extrusion is actually superior to the commercial rod approach** for all-orientation operation, because the MGN carriage is mechanically captured while rod-based bearings rely on clamping force.

**Follow-up questions:**
- What do DIY all-orientation slider builds use? Are there examples with MGN rails that work inverted?

---

### Search: "single MGN12 rail vs dual rail for camera slider stability"

**Single rail with two carriages (recommended for this project):**

A single MGN12 rail with two MGN12H carriages spaced 80-100mm apart is the standard configuration for DIY camera sliders and most 3D printer axes. This works because:

- Two carriages on the same rail create a **moment couple** that resists rotation around the rail axis. The wider the spacing, the better the anti-rotation resistance.
- The MGN12H carriage itself has high moment ratings (36-38 N-m) that resist pitch and yaw individually.
- A single rail is **much simpler to align** than dual parallel rails. Dual rails require precise parallelism -- any misalignment causes binding. On a 1200mm extrusion, maintaining parallelism to the required tolerance (~0.05mm) is very difficult without machining. ([Duet3D Forum][10])

**When you would need dual rails:**
- Very heavy loads (>20 kg) where single-rail moment capacity is exceeded
- Very wide carriages that need lateral support over a wide footprint
- CNC milling machines where cutting forces create large lateral loads
- Our 7 kg payload does NOT require dual rails.

**Dual rail drawbacks for this project:**
- Requires a wider extrusion (4040 minimum, or custom plate)
- Alignment is critical and difficult -- misalignment causes binding
- Adds cost (second 1200mm rail = ~$15-25 more)
- Adds weight (~780g more)
- Adds complexity with no real benefit at 7 kg

**Verdict: single MGN12 rail with two MGN12H carriages.** This is adequate for 7 kg at any orientation. The moment safety factor is comfortable. Dual rails add cost, weight, and alignment complexity without meaningful benefit.

---

### Search: "GT2 belt routing alongside MGN12 rail on aluminum extrusion"

**Belt routing on 2040 extrusion** ([GT2 Belt Idler Mount for V-Slot][21]):

The standard approach for routing a GT2 belt alongside an MGN rail on aluminum extrusion:

**On V-slot 2020:**
- The MGN12 rail (12mm wide) sits centered on the 20mm face, leaving **4mm clearance on each side**. The belt runs in the V-slot groove on an adjacent face (2020 has 4 faces).
- The V-slot groove opening is approximately 6.2mm wide -- a 6mm GT2 belt fits with minimal clearance. The belt's smooth back side can ride in the groove.
- Idler pulleys mount at each end using 3D-printed brackets that clip into the T-slots.
- Problem: with the rail on top, there's no room for the belt on the same face. The belt must route on a side face or below.

**On V-slot 2040 (recommended):**
- The 2040 provides **two 20mm faces side by side** when oriented with the 40mm dimension horizontal (as a flat beam).
- The MGN12 rail mounts on **one 20mm face**, and the GT2 belt routes through the **T-slot channel of the adjacent 20mm face** or along the side.
- However, if the 2040 is oriented **tall** (20mm face up, 40mm dimension vertical for stiffness), then the top face is only 20mm -- same constraint as 2020.
- **Best approach with tall 2040:** mount the MGN12 rail on the 20mm top face, route the GT2 belt along one of the 40mm side faces using 3D-printed belt guides. The side T-slot channels on the 40mm faces can anchor the belt ends and idler pulleys.

**On 4040:**
- Plenty of room. The 40mm top face can accommodate both the 12mm MGN12 rail and a belt channel side by side (12mm rail + 6mm belt + spacing = ~22mm, well within 40mm).
- Alternatively, route the belt in a side T-slot channel.

**Belt attachment to carriage:**
- The GT2 belt forms a loop with both ends attached to the carriage plate (the part that bolts to the MGN12H carriages).
- The belt runs along the length of the extrusion, around idler pulleys at each end, and wraps around the drive pulley on the motor.
- The carriage plate has belt clamps -- simple 3D-printed or machined parts that grip the belt teeth.

**Key consideration:** The belt path should NOT interfere with the rail screws or the carriage body. On a 2040 oriented tall, this means the belt needs to route below or beside the rail, typically using the side T-slot channels of the extrusion.

---

### Search: "MGN12 1200mm rail cost and full guide system pricing"

**Estimated costs for the complete linear guide system** ([ZYLtech MGN12H][18], [ZYLtech 2040][19], [KB3D MGN12H][20]):

| Component | Budget (AliExpress/eBay) | Mid-range (US suppliers) | Notes |
|-----------|--------------------------|--------------------------|-------|
| MGN12 rail, 1200mm | $15-25 | $40-60 | Budget rails from Chinese sellers; US suppliers like ZYLtech, KB3D charge more but better QC |
| MGN12H carriages x2 | Included with rail or $6-10 ea | $13 ea (ZYLtech) | Most budget rails include 1-2 carriages |
| 2040 extrusion, 1200mm | $8-12 | $15-25 | V-slot or T-slot, widely available |
| 4040 extrusion, 1200mm | $15-25 | $25-40 | Heavier, stiffer option |
| M3 T-nuts + screws (x24) | $3-5 | $5-8 | For rail mounting, every other hole |
| End brackets / motor mount | $5-10 (3D printed) | $10-20 (machined) | 3D printing recommended for prototyping |
| GT2 belt + pulleys | $8-15 | $15-25 | Covered in ADR-003, included here for total |
| **Total (2040 beam)** | **$45-85** | **$100-170** | |
| **Total (4040 beam)** | **$55-95** | **$115-195** | |

**Pricing notes:**
- ZYLtech sells MGN12H rails up to 1000mm ($30-87 range depending on length and carriage count). For 1200mm, Amazon or AliExpress are the main sources. ([ZYLtech MGN12H][18])
- KB3D sells MGN12H rails up to 600mm ($29-40). Longer lengths not available from this supplier. ([KB3D MGN12H][20])
- ZYLtech 2040 extrusion: $8-42 range for 300-2000mm lengths. ([ZYLtech 2040][19])
- Budget Chinese MGN12 rails (AliExpress) at 1200mm with 2 carriages: typically $15-30 shipped. Quality varies -- worth cleaning, regreasing, and testing before use.
- Genuine HIWIN MGN12H at 1200mm: $80-150+, available from industrial distributors. Overkill for a camera slider.

**Cost comparison with commercial sliders:**
- Rhino Slider PRO 4ft: ~$500-700
- iFootage Shark S1 4ft: ~$250-400
- Syrp Magic Carpet 4ft: ~$400-600
- Our DIY linear guide system: **$45-170** (guide system only, not including motor/electronics)

The DIY approach is 3-10x cheaper than commercial sliders for the guide system alone.

---

<!-- Continue appending search entries below. One entry per search, every time. -->
