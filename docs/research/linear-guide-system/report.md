# Linear Guide System for Camera Slider (900-1200mm, 7 kg, All-Orientation)

**Date:** 2026-03-20

## Executive Summary

**Use a single MGN12H rail on 2040 V-slot aluminum extrusion oriented tall (40mm vertical).** This provides adequate stiffness (0.76mm center deflection under 7 kg), mechanically captured all-orientation operation, and a 3.5x moment load safety factor per carriage -- all for about $45-85 from budget suppliers or $100-170 from US vendors.

MGN12 is the right rail size. MGN15 is overkill -- its load ratings are 70% higher but our 7 kg payload uses less than 2% of even MGN12's static capacity. V-slot wheels are ruled out because they lack mechanical capture for inverted operation. Dual rods are ruled out because they deflect unacceptably at 1200mm span (8mm rods: 61mm, 16mm rods: 3.8mm). 2020 extrusion is too flexible (4.86mm deflection). 4040 is stiffer but heavier; 2040 oriented tall hits the right balance.

---

## 1. Rail Selection: MGN12 vs MGN15

| Spec | MGN12H | MGN15H | Our Load |
|------|--------|--------|----------|
| Static load C0 | 5,880 N (599 kg) | 9,110 N (929 kg) | 68.7 N (7 kg) |
| Dynamic load C | 3,720 N | 6,370 N | 68.7 N |
| Moment MR (roll) | 38.2 N-m | 73.5 N-m | ~10.3 N-m |
| Moment MP (pitch) | 36.3 N-m | 57.8 N-m | ~10.3 N-m |
| Rail width | 12 mm | 15 mm | -- |
| Rail weight/m | 0.65 kg/m | 1.06 kg/m | -- |
| Carriage weight | 54 g | 92 g | -- |
| System weight (1200mm + 2 carriages) | 888 g | 1,456 g | -- |

**Recommendation: MGN12H.** Our 7 kg payload creates a worst-case moment of ~10.3 N-m (7 kg hanging 150mm below the rail). A single MGN12H carriage handles 36-38 N-m -- a 3.5x safety factor. With two carriages sharing the load, the effective safety factor exceeds 7x. MGN15 adds 570g and $10-20 for capacity we will never approach.

### Carriage Type

Use **MGN12H-CC** (standard length, through-bolt holes):

- **H over C:** The H carriage is 45.4mm long vs ~29mm for C. It has 2.6x the moment capacity (38 N-m vs 14.7 N-m). With a heavy offset payload hanging below, we need the H carriage's moment resistance.
- **CC over CA:** CC carriages have through holes, allowing bolts from either direction. This matters for inverted operation -- the carriage plate can be secured from whichever side is accessible. CA (blind tapped) only accepts bolts from the top.

---

## 2. Extrusion Selection: Frame Beam

### Deflection Calculations

Simply supported beam, center point load of 7 kg (68.7 N), 1200mm span.

Formula: delta = P * L^3 / (48 * E * I), where E = 68,900 N/mm^2 (6063-T5 aluminum)

| Profile | I (mm^4) | Center Deflection | Weight (1200mm) | Verdict |
|---------|----------|-------------------|-----------------|---------|
| 2020 | 7,400 | 4.86 mm | 600 g | Too flexible, visible sag |
| 2040 (tall, 40mm vertical) | 47,500 | 0.76 mm | 1,080 g | Acceptable |
| 2040 (flat, 20mm vertical) | 12,100 | 2.97 mm | 1,080 g | Wrong orientation |
| 4040 | 97,000 | 0.37 mm | 1,800 g | Excellent, heavier |

### Recommendation: 2040 Oriented Tall

The 2040 extrusion oriented with its 40mm dimension vertical deflects 0.76mm under full load. This is acceptable for camera work -- it is not visible on screen and does not affect rail smoothness. The steel MGN12 rail bolted to the extrusion adds some composite stiffness (steel E = 200 GPa vs aluminum 69 GPa), so real-world deflection may be slightly less.

**4040 is the upgrade path** if 0.76mm deflection is not satisfactory, or if torsional stiffness is a concern (see below). It cuts deflection in half (0.37mm) at a cost of 720g more weight.

### Torsional Stiffness

A camera + gimbal hanging below and offset from the beam centerline creates a torque that wants to twist the extrusion. The 2040 oriented tall (20mm width, 40mm height) has limited torsional resistance due to its narrow 20mm cross-section. The 4040 (square, 40mm x 40mm) is significantly better for torsion.

For a centered load (gimbal mounted directly below the rail center), torsion is minimal. For an offset load, 4040 may be preferred. This depends on the carriage plate design.

---

## 3. Inverted and All-Orientation Operation

**MGN linear rails work in any orientation.** The carriage is mechanically captured on the rail by the ball recirculation channels and cannot come off without removing the end caps. The Gothic arch contact design of the ball raceways provides load capacity in all four directions. No adjustment is needed when changing orientation -- horizontal, vertical, inverted, or angled all work identically.

This is the primary advantage over V-slot wheels and rod-based bearings, both of which rely on clamping/preload force rather than mechanical capture.

### V-Slot Wheels: Ruled Out

V-slot wheels are not suitable for all-orientation operation:

- They rely on eccentric nut preload to prevent play -- no mechanical capture
- Preload degrades as Delrin wheels wear (flat spots after ~2 years)
- Wheel dust is cosmetically undesirable near camera equipment
- Preload adjustment is finicky and orientation-dependent

### Commercial Slider Comparison

Most commercial sliders derate for inverted/vertical use:

| Slider | Horizontal Capacity | Vertical/Inverted | Guide Type |
|--------|--------------------|--------------------|------------|
| Rhino PRO (stainless) | 50 lbs | Full (stainless rods) | Dual rods + linear bearings |
| iFootage Shark S1 | 15 lbs (7 kg) | 7.7 lbs (3.5 kg) | Carbon fiber rods + bearings |
| Syrp Magic Carpet | 11 lbs (5 kg) | Not rated | Carbon tubes + roller bearings |
| **Our design (MGN12H)** | **15 lbs (7 kg)** | **15 lbs (7 kg)** | **MGN12 rail on extrusion** |

Our MGN-based design maintains full load rating in all orientations because the carriage is captured. Commercial sliders using rod/roller bearings lose capacity when inverted because gravity works against the clamping preload.

---

## 4. Single Rail vs Dual Rail

**Single rail with two MGN12H carriages** is the correct configuration for this project.

- Two carriages spaced 80-100mm apart create a moment couple that resists rotation
- Each carriage has a 36-38 N-m moment rating -- far exceeding our ~10 N-m load
- A single rail is much simpler to align than dual parallel rails
- Dual rails on a 1200mm extrusion require ~0.05mm parallelism to avoid binding -- very difficult without machining
- Dual rails add ~780g and $15-25 with no meaningful benefit at 7 kg

**When dual rails would be needed:** loads above ~20 kg, or applications with high lateral cutting forces (CNC milling). Not applicable here.

---

## 5. Rail Alignment and Mounting

### How to Mount the Rail

1. Place MGN12 rail on the extrusion face with M3 bolts and T-nuts loosely inserted
2. Use 3D-printed alignment jigs (free STL files available for 2020 and 2040) to center the rail on the extrusion face -- place 2-3 jigs along the 1200mm length
3. Snug bolts with jigs in place, working from center outward
4. Remove jigs, final-tighten all bolts to **0.8 N-m** (per HIWIN spec -- do NOT over-torque)
5. Test by sliding carriage end-to-end. If smooth, alignment is good.

### Critical Rules

- **The rail gets its straightness from the mounting surface.** A single MGN rail has no straightness specification -- it conforms to whatever it's bolted to. Buy quality extrusion (Misumi, 80/20, OpenBuilds), not the cheapest AliExpress extrusion.
- **Over-torquing forces the rail to conform to extrusion imperfections.** The low 0.8 N-m torque spec is intentional -- it lets the rail maintain its own straightness even on an imperfect surface.
- **Aluminum extrusions are straight to ~0.5mm/m.** Over 1200mm, that's ~0.6mm potential deviation. For camera slider use, this is acceptable.
- **Rail mounting holes are every 25mm** (48 holes on a 1200mm rail). Using every other hole (24 screws) is sufficient.

---

## 6. Belt Routing

The GT2 belt must route alongside the MGN12 rail on the extrusion. The approach depends on extrusion orientation:

### 2040 Oriented Tall (recommended configuration)

- The 20mm top face holds the MGN12 rail (12mm wide, centered, with 4mm clearance per side)
- The GT2 belt routes along one of the 40mm side faces, inside a T-slot channel
- 3D-printed idler pulley brackets mount in the T-slots at each end
- Belt ends attach to the carriage plate via belt clamps
- The drive motor mounts at one end with its pulley aligned to the belt path

### 4040

- The 40mm top face has room for both the 12mm rail and a 6mm belt side by side (22mm total, well within 40mm)
- Or route the belt in a side T-slot channel for cleaner packaging

### Belt Dimensions

A 6mm or 9mm GT2 belt fits in the T-slot channel of standard V-slot extrusion (the V-groove opening is ~6.2mm). For a 9mm belt, it rides along the flat side face rather than inside the slot. Either width works -- 9mm provides more stiffness per the drive system research (ADR-003).

---

## 7. Cost Estimate

| Component | Budget | Mid-Range | Notes |
|-----------|--------|-----------|-------|
| MGN12 rail 1200mm + 2x MGN12H | $15-25 | $40-60 | Budget: AliExpress. Mid: US suppliers |
| 2040 extrusion 1200mm | $8-12 | $15-25 | V-slot or T-slot |
| M3 T-nuts + screws (x24) | $3-5 | $5-8 | For rail mounting |
| End brackets + motor mount | $5-10 | $10-20 | 3D printed for prototype |
| Alignment jigs | $0 (3D print) | $0 | Free STL files |
| **Guide system total** | **$31-52** | **$70-113** | Excludes motor/belt/electronics |

Adding the GT2 belt + pulleys ($8-25) and NEMA 17 motor + TMC2209 ($15-45) from ADR-003, the complete slide axis mechanical system costs **$55-180** depending on component sourcing.

For comparison, commercial motorized 4ft sliders cost $250-700 (iFootage, Rhino, Syrp).

---

## 8. Recommended Bill of Materials

| Component | Specification | Qty |
|-----------|--------------|-----|
| Aluminum extrusion | 2040 V-slot, 1200mm, quality supplier | 1 |
| Linear rail | MGN12, 1200mm | 1 |
| Linear carriage | MGN12H-CC (through-bolt) | 2 |
| Rail mounting hardware | M3x8 SHCS + M3 T-nuts (for 6mm slot) | 24 |
| Alignment jigs | 3D printed, MGN12-to-2040 design | 3 |
| End brackets | 3D printed or machined aluminum | 2 |
| Carriage plate | 3D printed or machined aluminum | 1 |

### Upgrade Path

If testing reveals excessive deflection or torsion:

1. **Upgrade to 4040 extrusion** -- cuts deflection from 0.76mm to 0.37mm, better torsional stiffness, adds 720g
2. **Add center support** -- a tripod mount or support leg at the beam midpoint eliminates span deflection entirely
3. **Upgrade to MGN15H** -- only if moment loads are higher than expected (unlikely at 7 kg)

---

## 9. Open Questions for ADR-004

1. **2040 vs 4040:** The deflection math says 2040 works (0.76mm). Prototype testing will determine if the torsional stiffness of 2040 is adequate or if 4040 is needed. Start with 2040 -- it's lighter and cheaper, and can be swapped.
2. **Center support:** A tripod mount at the beam midpoint would eliminate deflection concerns entirely. Worth designing the end brackets to allow both end-supported and center-supported configurations.
3. **Carriage spacing:** 80-100mm between carriages is typical. Wider is more stable against rotation but reduces usable travel length.
4. **Rail quality:** Budget Chinese MGN12 rails should be cleaned, regreased with quality lubricant, and tested for smoothness before assembly. Lapping may be needed for sticky spots.
