# DJI RSA/NATO Port Connector - 3D Printable Models

**Date:** 2026-03-25

## Executive Summary

After searching all eight major 3D model repositories (Thingiverse, Printables, Thangs, Cults3D, MyMiniFactory, GrabCAD, Yeggi, STLFinder) plus GitHub, using dozens of search term variations, **exactly one model exists that includes the electrical contact portion of the DJI RSA connector.** That model is on GitHub, not on any of the 3D printing sites. Several mechanical-only NATO rail dovetail models exist on Printables and Cults3D, but none include the 6-pin electrical interface.

---

## The One Model With Electrical Contacts

### Riley Harmon's CAN Connector (GitHub)

**URL:** https://github.com/rileyharmon/DJI-Ronin-RS2-Log-and-Replay/blob/main/3d-print-ronin-can-connector.stl

| Field | Details |
|-------|---------|
| **What it models** | Complete RSA/NATO port connector: mechanical dovetail + electrical contact housing |
| **Electrical contacts** | YES -- housing designed for a Precidip 813-S1-008-10-014101 SLC module (8-contact, double-row, 2.54mm pitch, spring-loaded) |
| **Assembly method** | Glue the Precidip SLC module into the 3D-printed housing, solder wires to the solder tails |
| **File format** | STL only (8.32 MB). No STEP or parametric CAD available |
| **License** | CC BY-NC 4.0 (Attribution-NonCommercial) |
| **Tested with** | DJI RS 2 (likely compatible with RS 3 Pro, RS 4, RS 4 Pro, RS 5 -- same RSA port) |
| **Project context** | Part of a motion-control record/replay system using CANable Pro + python-can on Ubuntu |

**Key component -- Precidip 813-S1-008-10-014101:**
- Spring-loaded contact (SLC) connector module -- not individual pogo pins
- 8 contacts in double-row, 2.54mm pitch, low-profile solder-tail
- Drops into the printed housing as a single unit
- Available from Precidip (Swiss manufacturer); also sold through distributors like Digi-Key and Mouser
- Product page: https://www.precidip.com/en/Products/Spring-Loaded-Connectors/pview/813-S1-NNN-10-014101.html

**Limitations:**
- STL only -- no parametric source, so modifying the design requires reverse-engineering the mesh
- 8.32 MB is unusually large for a connector housing, suggesting the mesh may not be optimized
- CC BY-NC license restricts commercial use
- No dimensional drawings or assembly guide beyond "glue in and solder"
- Not clear whether all 8 contacts are used or only the 6 needed for the RSA port

---

## Mechanical NATO Rail Models (No Electrical Contacts)

These models replicate the NATO rail dovetail shape that slides into the RSA port's mechanical slot but include **no provision for electrical pins**. Useful as reference geometry for the dovetail dimensions.

| Model | Platform | Format | Notes |
|-------|----------|--------|-------|
| [Folding Briefcase Handle for DJI NATO Adapter (RS 3 Mini)](https://www.printables.com/model/404063) | Printables | STL | By Jake Reeves. PETG recommended. NATO dovetail is "quite tight" initially. Most detailed mechanical-only reference |
| [DJI Ronin Handle for RS series](https://www.printables.com/model/426471) | Printables | STL | By Ammar Faridi. Handle that mounts to RS NATO rail |
| [Camera Backpack Mount (RS 3 mini)](https://www.printables.com/model/887562) | Printables | STL | By piet vangestel. Strap mount using NATO dovetail |
| [DJI Ronin RS and RSC2 Grip Handle](https://cults3d.com/en/3d-model/gadget/dji-ronin-rs-and-rsc2-grip-handle) | Cults3D | OBJ | By lumediajdoo. Two-piece design (mount + handle). PETG, 4 walls, 25% infill |
| [DJI RONIN-SC Riser Plate for NATO RAIL](https://www.printables.com/model/112638) | Printables | STEP, 3MF | By Kosut_Ondrej. **Ronin-SC** (older gimbal, may differ from RS series). Available in STEP format |

---

## Platforms With No Relevant Results

| Platform | Searched | Result |
|----------|----------|--------|
| Thingiverse | 4 query variations | Zero DJI RS NATO/RSA models |
| Thangs | 3 query variations | Zero relevant models; site returns 403 on direct fetch |
| GrabCAD | 2 query variations | Only general DJI drone models, no RS connector CAD |
| MyMiniFactory | 1 query | Zero results (platform focuses on miniatures) |
| Yeggi | 2 query variations | CAPTCHA wall blocked all results |
| STLFinder | 2 query variations | Generic category pages only, no specific connector models |

---

## Assessment

The pickings are extremely thin. The RSA/NATO connector is a proprietary DJI design and there is essentially no community ecosystem around replicating it. The Riley Harmon STL is the only known open-source attempt at the electrical connector, and it has significant limitations (STL-only, no parametric source, noncommercial license, sparse documentation).

For the OpenDolly project, the practical options are:

1. **Use the Riley Harmon STL as a starting point.** Download it, measure the Precidip SLC module cavity, and recreate the design in Fusion 360 or similar to get a parametric model. The critical dimensional information is the dovetail profile (measurable from the STL or from the mechanical-only Printables models) and the SLC module pocket.

2. **Buy a DJI Focus Wheel and harvest the connector.** The Focus Wheel has the complete RSA connector -- you could cut the cable and solder directly to the CAN bus wires. This avoids the 3D printing and pogo pin sourcing entirely. Used Focus Wheels can be found for $30-50.

3. **Measure the port directly with calipers and design from scratch.** You own an RS 5 -- the RSA port dimensions can be measured directly. The dovetail profile from the Printables briefcase handle model provides a cross-check. The electrical contact positions are documented in the DJI External Interface Diagram PDF (6-pin RSA port pinout).

4. **Use a commercial adapter.** The Middle Things APC-R Gimbal Adapter mounts to the RSA port and provides CAN bus access, though it is expensive (~$300) and overkill for just getting CAN bus connectivity.
