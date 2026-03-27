# Firmware Tools

Standalone test sketches used during the DJI RS 5 CAN bus bring-up. Each tool is a self-contained `.cpp` file that temporarily replaces `main.cpp` for focused testing. See each tool's README for details on what it does and what we learned.

## How to use any tool

1. `mv src/main.cpp src/main.cpp.bak`
2. `cp tools/<tool-name>/<file>.cpp src/can_test.cpp`
3. `pio run -e esp32s3 --target upload`
4. `pio device monitor -b 115200`
5. When done: `rm src/can_test.cpp && mv src/main.cpp.bak src/main.cpp`

Tools that use `dji_protocol.h` / `dji_crc.h` depend on the `lib/dji_can/` library — they must be compiled from within the firmware project.

## The full story

This section documents how we brought up CAN bus communication with the DJI RS 5 gimbal from scratch, starting with zero documentation for the RS 5's hardware interface. The only reference was the [DJI R SDK v2.5](../docs/external/dji-r-sdk/docs/DJI_R_SDK_Protocol_and_User_Interface_EN_v2.5.md), written for the RS 2 in June 2021.

### Phase 1: Hardware — Mapping the RS 5 RSA connector

The RS 2 has a 6-pin RSA/NATO port. The RS 5 has a 10-pin RSA port with a different physical layout. No documentation exists for the RS 5 pinout.

**Process:**
1. Built a custom pogo-pin connector to access the RSA port contacts
2. Counted 10 pins: 4×2 grid + 2 above the right side
3. Used a multimeter to probe all pins against chassis ground (gimbal powered on, no pull-down)
4. Identified GND pins via continuity test (gimbal off) — pins 4 and 8 read 0Ω to chassis
5. Found the AD_COM pin by trial: connected a 100k pull-down resistor between each candidate pin and GND until VCC activated. Pin 3 triggered 9V on pins 1 and 5.
6. Confirmed CANH (pin 7) and CANL (pin 6) with an oscilloscope — both showed CAN bus traffic patterns
7. Identified SBUS_RX (pin 2) by elimination — 0V, no signal on scope
8. Pins 9 and 10 remain unknown — ~0V, no activity

**Result:** Full pinout documented in [wiring.md](../../docs/research/dji-rs5-ports-connectors/wiring.md).

See also: [rsa-pinout-test-procedure.md](../../docs/research/dji-rs5-ports-connectors/rsa-pinout-test-procedure.md) for the testing methodology.

### Phase 2: CAN bus — Confirming communication

**Tool:** [`can-listen/`](can-listen/) — Passive CAN listener

Connected ESP32-S3 + SN65HVD230 CAN transceiver to the RSA port (CANH, CANL, GND) with a 100k pull-down on AD_COM.

**Discovery:** The RS 5 broadcasts a heartbeat on CAN ID **0x426** (data: `55 45 04 DE E5 06 xx 54`). This is not documented anywhere — it's a new RS 5 feature that runs independently of the SDK protocol.

### Phase 3: SDK protocol — Enabling telemetry

**Tool:** [`telemetry-reader/`](telemetry-reader/) — Enable-push + telemetry decoder

Sent the enable-push command (CmdSet=0x0E, CmdID=0x07) on both RS 2 IDs (TX=0x223) and hypothesized RS 5 IDs (TX=0x427). **The RS 2 IDs worked.** The gimbal responded with SDK telemetry frames on 0x222.

**Key finding:** The R SDK protocol works unchanged on the RS 5 — same CAN IDs (0x222/0x223), same frame format, same commands. The 0x426 heartbeat is a separate layer.

### Phase 4: Position control — Moving the gimbal

**Tool:** [`position-control/`](position-control/) — Absolute position commands

First two attempts failed silently — the gimbal accepted enable-push but ignored position commands. The problem was **wrong CRC implementation**: we were using non-reflected CRC tables with init values from the PDF spec (CRC16=0xC55C, CRC32=0xC55C0000). The correct values are reflected tables with CRC16 init=0x3AA3, CRC32 init=0x00003AA3, matching the Python demo code (`SDKCRC.py`).

Once we switched to the existing `dji_crc` library, position commands worked immediately. Tested all three axes (pan ±45°, pitch ±30°, roll ±15°) — all responded correctly.

**Lesson:** The gimbal silently drops commands with bad CRC — no error response. CRC must be exactly right.

### Phase 5: Reverse-engineering — Undocumented settings

**Tool:** [`tlv-scanner/`](tlv-scanner/) — TLV parameter reader

To control features not in the R SDK (like push mode), we used a diff-based approach:
1. Scan all TLV parameter IDs with the setting OFF
2. Change the setting in the DJI Ronin app
3. Scan again and diff

This identified **push pan** (TLV 0x14) and **push tilt** (TLV 0x15) — undocumented parameters that can be read and likely written via CmdID 0x0B/0x0C.

## Debugging log

The detailed experiment-by-experiment log is at [gimbal-debugging.md](../../docs/research/dji-rs5-ports-connectors/gimbal-debugging.md).

## Tools index

| Tool | README | Purpose |
|------|--------|---------|
| [`can-listen/`](can-listen/) | [README](can-listen/README.md) | Passive CAN frame listener |
| [`telemetry-reader/`](telemetry-reader/) | [README](telemetry-reader/README.md) | Enable-push + decoded telemetry |
| [`position-control/`](position-control/) | [README](position-control/README.md) | Absolute position commands on all axes |
| [`tlv-scanner/`](tlv-scanner/) | [README](tlv-scanner/README.md) | TLV parameter reader for reverse-engineering |
