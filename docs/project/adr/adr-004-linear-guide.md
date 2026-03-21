# ADR-004: Linear Guide System

*Status: accepted*
*Date: 2026-03-20*

## Context

The slider needs a linear guide system that:

1. Provides at least 3 feet (900mm) of travel, ideally 4 feet (1200mm)
2. Supports 15 lbs (7 kg) at the center of the span — cinema camera + DJI RS gimbal
3. Works in any orientation — horizontal, tilted, vertical, **and inverted** (camera hanging below for low-angle/overhead shots)
4. Mounts on a tripod
5. Is affordable and buildable without machining

## Decision

**Single MGN12H rail on 2040 aluminum extrusion, oriented tall (40mm vertical).** Two MGN12H-CC carriages spaced 80-100mm apart. Belt routes along a side T-slot channel.

## Why This Configuration

### Rods are ruled out at this length

We initially considered dual smooth rods with linear bearings — the classic DIY slider design. Deflection math killed it:

| Rod diameter (steel) | Deflection at 1200mm, 7 kg center | Verdict |
|---|---|---|
| 8 mm | 61 mm | Unusable |
| 12 mm | 12 mm | Bad |
| 16 mm | 3.8 mm | Marginal |
| 20 mm | 1.6 mm | Borderline, very heavy |

Rod deflection scales with L³ — doubling the span increases sag 8x. Rods work for sliders under ~600mm. At 1200mm, even 16mm steel rods sag nearly 4mm under our payload. Rod-based bearings (LM8UU etc.) also can't work inverted without retaining hardware.

### V-slot wheels are ruled out

V-slot wheels riding on the extrusion are simple and cheap, but:

- No mechanical capture — they rely on eccentric nut preload to stay on. Inverted operation is not reliable.
- Delrin wheels develop flat spots and generate dust after ~2 years.
- Preload adjustment is finicky and orientation-dependent.

### MGN rail on extrusion solves everything

An MGN rail bolted to the extrusion is supported continuously along its full length — there is no span to deflect. The carriage is mechanically captured on the rail by recirculating ball channels and cannot come off without removing end caps. Load capacity is identical in all orientations.

### Why MGN12, not MGN15

| Spec | MGN12H | MGN15H | Our load |
|---|---|---|---|
| Static load C0 | 5,880 N (599 kg) | 9,110 N (929 kg) | 68.7 N (7 kg) |
| Moment MR (roll) | 38.2 N-m | 73.5 N-m | ~10.3 N-m |
| System weight (1200mm + 2 carriages) | 888 g | 1,456 g | — |

Our 7 kg payload uses less than 2% of MGN12's static capacity. The worst-case moment (7 kg hanging 150mm below the rail) is 10.3 N-m — a 3.5x safety factor per carriage, 7x with two carriages. MGN15 adds 570g and $10-20 for capacity we'll never approach.

### Why MGN12H-CC carriages

- **H over C:** The H (standard) carriage is 45.4mm long with 2.6x the moment capacity of the C (short) carriage (38 N-m vs 14.7 N-m). With a heavy camera hanging below the rail, we need the moment resistance.
- **CC over CA:** CC has through-bolt holes — bolts can go in from either side. CA has blind tapped holes from the top only. CC allows flexible carriage plate mounting in any orientation.
- **Two carriages** spaced 80-100mm apart resist rotation and distribute the load.

### Why 2040 extrusion, oriented tall

| Profile | Deflection (7 kg, 1200mm) | Weight (1200mm) | Verdict |
|---|---|---|---|
| 2020 | 4.86 mm | 600 g | Too flexible |
| **2040 tall (40mm vertical)** | **0.76 mm** | **1,080 g** | Acceptable |
| 2040 flat (20mm vertical) | 2.97 mm | 1,080 g | Wrong orientation |
| 4040 | 0.37 mm | 1,800 g | Upgrade path |

0.76mm center deflection under full load is not visible on screen and doesn't affect rail smoothness. The MGN12 rail (steel, E = 200 GPa) bolted to the aluminum (E = 69 GPa) adds composite stiffness, so real-world deflection is likely less.

**Orientation matters enormously.** The same 2040 extrusion deflects 0.76mm oriented tall vs 2.97mm oriented flat. Always orient with the 40mm dimension vertical.

### Belt routing

The 20mm top face of the 2040 holds the MGN12 rail (12mm wide + 4mm clearance per side). The GT2 belt routes along a side T-slot channel on the 40mm face. 3D-printed idler brackets mount in the T-slots at each end. Belt ends attach to the carriage plate via belt clamps.

## Alternatives Considered

### Dual rods + linear bearings

The classic DIY slider approach. Works well under 600mm but deflection at 1200mm is unacceptable (see table above). Also cannot work inverted without retaining hardware on the bearings.

### V-slot wheels on extrusion

Cheap and simple but no mechanical capture for inverted operation. Delrin wheels degrade and generate dust. Ruled out by the all-orientation requirement.

### Dual parallel MGN12 rails

Two parallel rails on a wider extrusion (4040 or dual 2020). Provides more lateral stability but requires ~0.05mm parallelism over 1200mm to avoid binding — very difficult without machining. A single rail with two H carriages provides sufficient stability for our loads. Dual rails add ~780g and $15-25 with no benefit at 7 kg.

### MGN15

Higher load ratings (70% more than MGN12) but 570g heavier. Our load is less than 2% of MGN12's capacity — MGN15 provides no practical benefit.

## Inverted Operation

MGN carriages are mechanically captured — they cannot come off the rail. Load capacity is identical in all orientations. This is confirmed by the Gothic arch contact geometry of the ball raceways, which provides load capacity in all four directions.

Commercial sliders typically derate for inverted use (iFootage Shark drops from 15 lbs to 7.7 lbs inverted). Our MGN-based design maintains full 7 kg at any angle because the carriage retention is mechanical, not gravity-dependent.

## Cost

| Component | Budget | Mid-Range |
|---|---|---|
| MGN12 rail 1200mm + 2x MGN12H-CC | $15-25 | $40-60 |
| 2040 extrusion 1200mm | $8-12 | $15-25 |
| M3 T-nuts + screws (x24) | $3-5 | $5-8 |
| End brackets + motor mount | $5-10 | $10-20 |
| **Guide system total** | **$31-52** | **$70-113** |

## Risks

1. **2040 torsional stiffness.** The 20mm-wide cross section has limited torsional resistance. A camera + gimbal hanging offset from the beam center creates a torque. If the carriage plate is designed to load directly below the rail, torsion is minimal. If not, 4040 may be needed. **Mitigation:** prototype with 2040 first — it's lighter and cheaper, and the extrusion can be swapped without changing the rail or belt.

2. **Rail quality from budget suppliers.** Cheap Chinese MGN12 rails may have tight spots or poor grease. **Mitigation:** clean, regrease with quality lubricant, and test for smoothness before assembly. Lapping sticky spots with a fine abrasive compound is common practice.

3. **Extrusion straightness.** The rail conforms to whatever it's bolted to. Aluminum extrusion is typically straight to ~0.5mm/m (~0.6mm over 1200mm). For camera slider use this is acceptable, but buying from a quality supplier (Misumi, 80/20, OpenBuilds) reduces the risk of a badly bowed extrusion.

## Upgrade Path

If prototype testing reveals problems:

1. **Swap to 4040 extrusion** — halves deflection (0.37mm), much better torsional stiffness, adds 720g. Same rail, same belt, same brackets with minor resizing.
2. **Add center support** — a tripod mount or leg at the midpoint eliminates span deflection entirely.
3. **Upgrade to MGN15H** — only if moment loads are higher than expected (unlikely).

## Consequences

- The frame is a single 2040 aluminum extrusion (1200mm), not a dual-beam or rod design
- One MGN12 rail bolted to the top face, two MGN12H-CC carriages
- GT2 belt in the side T-slot channel
- Motor and idler mount at each end via 3D-printed or machined brackets
- All-orientation operation is a baseline capability, not a special mode
- Carriage plate design should minimize load offset from the rail centerline to reduce torsion on the 2040

## References

- [Research: Linear Guide System](../research/linear-guide-system/report.md) — full analysis with deflection calculations, load ratings, commercial comparison
- [Research: Long-Travel Linear Motion](../research/long-travel-linear-motion/report.md) — belt drive, extrusion, and CNC community practices
- [ADR-003: Linear Drive System](adr-003-linear-drive.md) — GT2 belt drive decision
