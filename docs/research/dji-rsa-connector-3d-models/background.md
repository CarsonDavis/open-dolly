# DJI RSA/NATO Connector 3D Printable Models - Research Background

**Date:** 2026-03-25
**Topic:** Finding 3D printable models of the DJI RS RSA/NATO port connector — the proprietary connector that mates with the RSA port on DJI RS 2/3/4/5 gimbals. Critical distinction: models with electrical contact portion (6 pogo pins) vs. just the mechanical NATO rail dovetail.

## Sources

[1]: https://www.printables.com/model/404063-folding-briefcase-handle-for-dji-nato-adapter-rs-3 "Folding Briefcase Handle for DJI NATO Adapter (RS 3 Mini) - Printables"
[2]: https://www.printables.com/model/426471-dji-ronin-handle-for-rs-series "DJI Ronin Handle for RS series - Printables"
[3]: https://cults3d.com/en/3d-model/gadget/dji-ronin-rs-and-rsc2-grip-handle "DJI Ronin RS and RSC2 grip handle - Cults3D"
[4]: https://alvinscables.com/collections/for-dji-ronin "Alvin's Cables - For DJI Ronin"
[5]: https://github.com/rileyharmon/DJI-Ronin-RS2-Log-and-Replay "DJI-Ronin-RS2-Log-and-Replay - GitHub"
[6]: https://github.com/rileyharmon/DJI-Ronin-RS2-Log-and-Replay/blob/main/3d-print-ronin-can-connector.stl "3D Print Ronin CAN Connector STL - GitHub"
[7]: https://www.precidip.com/en/Products/Spring-Loaded-Connectors/pview/813-S1-NNN-10-014101.html "Precidip 813-S1 SLC Connector Series"
[8]: https://www.printables.com/model/112638-dji-ronin-sc-riser-plate-for-nato-rail "DJI RONIN-SC Riser Plate for NATO RAIL - Printables"
[9]: https://www.printables.com/model/887562-camera-backpack-mount-works-with-dji-rs-3-mini "Camera Backpack Mount (DJI RS 3 mini) - Printables"

## Research Log

---

### Search: "site:thingiverse.com DJI RS NATO connector" and "site:thingiverse.com DJI RS RSA connector accessory mount"

**No results on Thingiverse.** Multiple searches with variations ("NATO connector", "RSA connector", "NATO mount adapter") returned zero relevant results. Thingiverse results were all unrelated DJI drone accessories or gas mask NATO filter adapters.

---

### Search: "site:printables.com DJI RS NATO connector"

- **Folding Briefcase Handle for DJI NATO Adapter (RS 3 Mini)** by Jake Reeves ([Printables][1]) — A 3D-printable briefcase handle that connects via the NATO rail. The description notes the NATO connection is "quite tight during initial use" and recommends PETG. **Mechanical NATO rail only** — this is an accessory that mates with the dovetail, no electrical contacts. STL format.
- **DJI Ronin Handle for RS series** by Ammar Faridi ([Printables][2]) — A handle designed for DJI RS series gimbals. Likely mechanical NATO rail mount only. Could not fetch the page (403 error) to confirm details.
- **Camera Backpack Mount (works with DJI RS 3 mini)** ([Printables][9]) — another NATO rail accessory. Mechanical only.

**Note:** Printables blocks WebFetch (403), so details are from search snippets only.

---

### Search: Thangs, Cults3D, GrabCAD (site-restricted searches)

- **Thangs:** No results for "DJI RS NATO connector RSA" — zero relevant models found. Direct URL fetch also returned 403.
- **GrabCAD:** No results for "DJI RS NATO connector RSA" — only general DJI drone models and unrelated RS components.
- **Cults3D:** Found **DJI Ronin RS and RSC2 grip handle** by lumediajdoo ([Cults3D][3]) — OBJ format, 2 files (mount 100x197x53mm, handle 23x201x53mm). Printed in PETG, 4 walls, 25% infill. **Mechanical grip/handle only**, no electrical connector. No pogo pins.

---

### Search: Yeggi and STLFinder meta-searches

- **Yeggi:** Search pages for "dji ronin rs3" returned a CAPTCHA wall; could not access actual listings.
- **STLFinder:** "DJI RS NATO connector" returned generic category pages (nato adapter, dji gimbal) with no specific RSA electrical connector models found.

---

### Search: "DJI Ronin RSA port reverse engineer connector custom DIY 3D print"

**MAJOR FIND:** The GitHub repository **DJI-Ronin-RS2-Log-and-Replay** by Riley Harmon ([GitHub][5]) contains a **3D-printable CAN connector STL file** ([STL file][6]).

Key details:
- **File:** `3d-print-ronin-can-connector.stl` — 8.32 MB STL file
- **What it models:** A custom connector that plugs into the RSA/NATO port as an alternative to the DJI R Focus Wheel for CAN bus access
- **Includes electrical contacts:** YES — the design includes **slots for pogo pins** that make electrical contact with the RSA port's CAN bus pins. The README says to "glue in and solder" pogo pins.
- **Specific pogo pin part:** **Precidip 813-S1-008-10-014101** — a "Straight SLC connector, low-profile solder-tail, double-row, 8-contacts" ([Precidip][7]). This is a spring-loaded contact (SLC) connector module with 2.54mm pitch, 8 contacts in double row. The connector module drops into the 3D-printed housing.
- **Format:** STL only (no STEP or parametric CAD)
- **License:** CC BY-NC 4.0 (Attribution-NonCommercial)
- **Hardware context:** Used with CANable Pro adapter (candlelight firmware) for CAN bus communication with the gimbal
- **Project purpose:** Record and replay gimbal movements (yaw, roll, pitch, focus motor position)

This is **the only model found across all platforms that includes the electrical contact portion** of the RSA connector. All other models only replicate the mechanical NATO rail dovetail.

---

### Search: MyMiniFactory

**No results** for "DJI RS NATO connector ronin" on MyMiniFactory. The platform is primarily miniatures/figurines and had no relevant gimbal accessories.

---

### Search: "DJI RS RSA NATO connector STL STEP F3D" (broad format search)

No additional models found beyond the ones already cataloged. The same Printables briefcase handle ([Printables][1]) and Ronin handle ([Printables][2]) keep appearing. Also found:
- **DJI RONIN-SC Riser Plate for NATO RAIL** by Kosut_Ondrej ([Printables][8]) — available in STEP and 3MF formats. This is for the older Ronin-SC (different NATO rail than RS series). Mechanical mount only.

---

### Search: "DJI Ronin focus wheel teardown disassembly connector inside pogo spring pin"

- DJI forum thread "Ronin S: Pinning focus wheel connector" — could not fetch content (empty page returned). Thread likely discusses the internal connector of the original Ronin-S focus wheel.
- FCC internal photos of Ronin-S teardown exist at fccid.io but were not fetched.
- CFECONN article on gimbal pogo pins mentions DJI Ronin 4D uses pogo pins for LiDAR data but provides no specific technical details about RSA port connector dimensions or part numbers.

---

### Search: Precidip 813-S1-008-10-014101 details

- The Precidip 813-S1 series is a **spring-loaded connector (SLC)** product line ([Precidip][7])
- The 813 series features **low-profile solder-tail** termination
- **8 contacts in double-row configuration**
- **2.54mm pitch** (standard 0.1" spacing)
- This is **not individual pogo pins** but a **modular connector block** with spring-loaded contacts built in — designed to drop into a housing
- Available in variants with different heights (6mm to 7.5mm body height)
- The 6-pin variant (811-S1-006) in the same family has a single-row configuration

**This confirms the Riley Harmon design uses a commercial SLC module rather than individual pogo pins, making the assembly much simpler — just glue the module into the 3D-printed housing and solder wires to the solder tails.**

---

### Search: Final round — broader terms across all remaining platforms

Additional searches with terms "DJI RS NATO dovetail 3D print STL mount clamp", "DJI RS SBUS CAN custom adapter open source hardware", "DJI ronin NATO connector CAN adapter" on Printables, and "DJI ronin RS gimbal connector adapter mount" on Thangs all returned **no new models** beyond those already cataloged. The Riley Harmon GitHub STL remains the only electrical connector model found anywhere.

Also checked: DJI forum thread about DJI RS 2 CAD model — returned empty page, likely a request post rather than a shared model.

---
