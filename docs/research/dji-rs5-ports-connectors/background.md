# DJI RS 5 Ports, Connectors, and Pinouts - Research Background

**Date:** 2026-03-23
**Topic:** Physical ports, connector types, and pinouts on the DJI RS 5 gimbal stabilizer. Focus on RSS/NATO port, focus wheel port, CAN bus access, and compatibility with previous RS models.

## Sources

[1]: /Users/cdavis/github/slider/docs/external/dji-r-sdk/docs/DJI_R_SDK_Protocol_and_User_Interface_EN_v2.5.pdf "DJI R SDK Protocol and User Interface v2.5 (June 2021)"
[2]: https://terra-1-g.djicdn.com/851d20f7b9f64838a34cd02351370894/Ronin%E7%B3%BB%E5%88%97/External%20interface%20diagram.pdf "DJI R SDK External Interface Diagram PDF"
[3]: https://www.dji.com/support/product/rs-5 "DJI RS 5 Support Page"
[4]: https://www.dji.com/rs-5/specs "DJI RS 5 Specs Page"
[5]: https://support.dji.com/help/content?customId=en-us03400007633 "Ronin Series Accessories Compatibility List"
[6]: https://dl.djicdn.com/downloads/DJI_RS_5/20260115/UM_2/DJI_RS_5_User_Manual_en.pdf "DJI RS 5 User Manual v1.0"
[7]: https://www.dji.com/rs-sdk "DJI RS SDK Page"
[8]: https://www.middlethings.co/ronin-s-setup-guide/ "Middle Things APC Setup Guide"
[9]: https://www.middlethings.co/apcr-support/ "Middle Things APC-R Support"
[10]: https://www.newsshooter.com/2026/01/14/turn-dji-rs-3-pro-and-rs-4-gimbals-into-real-ptz-cameras-with-skaarhoj-controllers/ "SKAARHOJ DJI RS PTZ Control - Newsshooter"
[11]: https://dl.djicdn.com/downloads/DJI_RS_5/20260115/DJI_RS_Series_Universal_Accessories_List_en_202601.pdf "DJI RS Series Universal Accessories List (Jan 2026)"
[12]: https://store.middlethings.co/products/gimbal-adapter "Middle Things APC-R Gimbal Adapter"
[13]: https://emotimo.com/pages/dji-rs-gimbals "eMotimo DJI RS Gimbals Page"
[14]: https://store.dji.com/product/dji-rs-5 "DJI RS 5 Store Page"

<!-- Add new sources here as you find them. Number sequentially. -->

## Research Log

---

### Search: DJI R SDK Protocol v2.5 PDF (local file)

The R SDK v2.5 document (written for DJI RS 2, dated June 2021) contains detailed hardware interface information in Section 3:

- **CAN bus parameters:** 1 Mbps baud rate, standard frames. CAN Tx = 0x222, CAN Rx = 0x223 (from gimbal's perspective). When configuring a PC CAN adapter, use CAN Tx = 0x223, CAN Rx = 0x222 (reversed). ([DJI R SDK v2.5][1], p.18)

- **Device Connection Diagram (Figure, p.18):** Shows a DJI Focus Wheel connected to the gimbal's NATO port, with the Focus Wheel's cable carrying VCC_5V, GND, CANH, CANL to a CAN converter connected to a PC. The Focus Wheel acts as the physical access point for the CAN bus. ([DJI R SDK v2.5][1], p.18)

- **RSA/NATO Port Pinout (Figure 39, p.19):** The port is a **6-pin connector** with this pinout:
  - Pin 1: **VCC** - Power output. 8V +/- 0.4V, rated 0.8A, peak 1.2A
  - Pin 2: **CANL** - CAN Low
  - Pin 3: **SBUS_RX** - SBUS input
  - Pin 4: **CANH** - CAN High
  - Pin 5: **AD_COM** - Accessory detect port. DJI RS 2 has built-in 100k pull-up; recommended to use 10-100k pull-down resistor. NATO port will not output power unless an accessory is mounted.
  - Pin 6: **GND** - Ground
  ([DJI R SDK v2.5][1], p.19)

- **Key note about power:** The NATO port will NOT output power unless an accessory is detected via the AD_COM pin (pin 5). A pull-down resistor on AD_COM is required to trigger power output. ([DJI R SDK v2.5][1], p.19)

- **Symmetry note:** "The expansion ports on the right and left side are rotational symmetric. They are not mirror symmetric." ([DJI R SDK v2.5][1], p.19)

- The document refers to these as "RSA/NATO Ports" -- RSA = Ronin Series Accessories.

---

### Search: "DJI RS 5 ports connectors RSS NATO pinout specifications"

- **DJI RS 5 Support Page confirms these ports:** RSA/NATO Ports, 1/4"-20 Mounting Hole, Cold Shoe, Camera Control Port (USB-C), Multifunctional Port (USB-C). ([DJI RS 5 Support Page][3])

- **DJI R SDK External Interface Diagram PDF** ([External Interface Diagram][2]) is the same content as the R SDK v2.5 document Section 3 -- shows the RS 2's NATO port 6-pin pinout and the Focus Wheel connection diagram. ([DJI R SDK v2.5][1])

- **RS 5 has at least 3 USB-C ports:** Camera Control Port, Multifunctional Port, and a Charging Port (supports 5V/3A through 20V/3A). ([DJI RS 5 Support Page][3])

---

### Search: "DJI RS 5 RSA NATO port connector type focus wheel compatibility RS 4 RS 3 Pro"

- **DJI RS 5 specs page** lists the same accessory port format as previous models: RSA/NATO Ports, 1/4"-20 Mounting Hole, Cold Shoe, Camera Control Port (USB-C), Multifunctional Port (USB-C). ([DJI RS 5 Specs Page][4])

- **Accessories compatibility list** confirms that the **Ronin Focus Wheel** is compatible with RS 5, RS 4, RS 4 Pro, RS 3, RS 3 Pro, RS 2. This strongly implies the RS 5 uses the same RSA/NATO port connector. ([Accessories Compatibility List][5])

- **DJI RS Tethered Control Handle** is compatible with RS 5, RS 4, RS 4 Pro, RS 3, RS 3 Pro, RS 2. ([Accessories Compatibility List][5])

---

### Search: "DJI RS NATO port connector type pogo pin proprietary 6-pin physical description" + DJI RS SDK page

- **DJI RS SDK page** confirms supported gimbals: RS 5, RS 4, RS 4 Pro, RS 3 Pro. ([DJI RS SDK Page][7])

- **RS 4 Mini** explicitly distinguishes between "NATO Port" and "RSS Camera Control Port (USB-C)" and a separate "Pogo Pin" connector. **"RSS" refers to camera control cables (USB-C), NOT the NATO/RSA port.**

---

### Search: "DJI RS 2 CAN bus access focus wheel cable splice breakout DIY"

- **DJI R Focus Wheel** has a physical switch to select between SBUS and CANBUS modes. The NATO port carries both SBUS (pin 3) and CAN (pins 2, 4). ([Middle Things APC Setup Guide][8])

- **DJI R SDK connection diagram** shows the canonical CAN access method: mount Focus Wheel on NATO port, tap into Focus Wheel cable (VCC_5V, GND, CANH, CANL) to connect a CAN adapter. ([DJI R SDK v2.5][1], p.18)

---

### Search: SKAARHOJ and Middle Things APC-R CAN bus connection methods

- **Middle Things APC-R** supports RS 2, RS 3 Pro, RS 4, and RS 5 via CANBUS. **Requires "DJI Focus Wheel or APC-R Gimbal Adapter" to connect.** ([Middle Things APC-R Support][9])

- **APC-R Gimbal Adapter** is a third-party adapter that mates with the DJI RS NATO port and provides CAN bus access without a Focus Wheel. Dimensions: 7x11x3cm, 80g. ([Middle Things APC-R Gimbal Adapter][12])

---

### Search: DJI RS Series Universal Accessories List (PDF)

**DJI RS Series Universal Accessories List (Jan 2026)** -- full compatibility matrix. ([Accessories List][11])

Key RS 5 findings:
- **DJI RS Focus Wheel:** Compatible with RS 5. ([Accessories List][11])
- **DJI RS Focus Motor (2022):** Compatible with RS 5. ([Accessories List][11])
- **DJI RS RSS Control Cables (RSS-F, RSS-P):** **NOT compatible with RS 5.** ([Accessories List][11])
- **DJI RS Electronic Briefcase Handle:** **RS 5 ONLY.** ([Accessories List][11])
- **DJI RS L-Shaped Cable (USB-C, 13 cm):** **RS 5 ONLY.** ([Accessories List][11])
- **DJI RS Enhanced Intelligent Tracking Module:** RS 5 native; RS 4 Pro/RS 4 with adapter. ([Accessories List][11])
- **DJI RS Mini-HDMI cables:** All NOT compatible with RS 5. ([Accessories List][11])
- **DJI Video Transmitter, Transmission combos, LiDAR Range Finder:** NOT compatible with RS 5. ([Accessories List][11])

---

### Search: eMotimo RSI connection to DJI RS gimbals

- **eMotimo Conductor SA2.6** includes an **"RSI + 6Pin RSI Cable (DJI RS2/RS3/RS4 integration dongle and cable)"** for CAN bus control. The "6-pin" matches the NATO port's 6-pin pinout. ([eMotimo DJI RS Gimbals Page][13])

- eMotimo supports RS 2, RS 3, RS 3 Pro, RS 4 for CAN bus control but does not explicitly list RS 5 yet.

---

### Search: DJI RS 5 port reviews + DJI Store product comparison

**Critical finding from DJI Store comparison table:** The RS 5 has: ([DJI RS 5 Store Page][14])
- **RSA Port x 1** -- This is the electrical/communication port (the 6-pin connector with CAN bus, SBUS, power, etc.)
- **NATO Port x 2** -- These are purely mechanical NATO rail slots for mounting accessories (no electrical contacts)
- **Charging Adapter Port x 1**

**This clarifies the RSA vs NATO distinction:**
- **RSA port** = the 6-pin electrical/data connector documented in the R SDK. It carries VCC, CAN_H, CAN_L, SBUS_RX, AD_COM, GND. This is where the Focus Wheel makes electrical contact. There is only ONE of these on the RS 5.
- **NATO ports** = standard NATO rail mechanical mounting slots (the ~20mm dovetail channel). These are purely for physical attachment of accessories like monitors, handles, lights. There are TWO of these.

On previous models (RS 2, RS 3 Pro), the RSA connector was embedded within the NATO rail slot (same physical location -- the NATO rail had both mechanical mount AND electrical contacts). The DJI documentation called them "RSA/NATO Ports" because they served both functions.

**The RS 5 appears to separate these:** 1 RSA (electrical) + 2 NATO (mechanical only). But the Focus Wheel still works with RS 5, so the RSA port must still be in a NATO-style rail slot (the Focus Wheel slides onto the rail and its contacts mate with the RSA connector pins).

**Follow-up questions:**
- Is the 1 RSA port on the left or right side? Or is it at the bottom?
- Do the 2 NATO ports still have electrical contacts, or are they truly mechanical-only?
- Need to verify this RSA/NATO count against the user manual.

---

<!-- Continue appending search entries below. One entry per search, every time. -->
