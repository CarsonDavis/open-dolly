# DJI RS 5 CAN Bus Debugging Log

Tracking each experiment and results while bringing up R SDK communication with the RS 5.

## Experiment 1: Raw CAN Listen (listen-only mode)

**Goal:** Confirm CAN bus wiring works and gimbal is transmitting.
**Setup:** ESP32-S3 TWAI in listen-only mode, 1 Mbps, accept all IDs.
**Result:** SUCCESS
- Received continuous frames on CAN ID **0x426**
- Data: `55 45 04 DE E5 06 xx 54` — heartbeat, ~fast rate
- No SDK frames (0xAA) — heartbeat only
- **Finding:** RS 5 broadcasts a heartbeat on 0x426 (not documented in R SDK)

## Experiment 2: Enable-Push Command

**Goal:** Send enable-push (CmdSet=0x0E, CmdID=0x07) to start SDK telemetry.
**Setup:** ESP32-S3 TWAI in normal mode, alternating between RS 2 IDs (0x223) and RS 5 IDs (0x427).
**Result:** SUCCESS on RS 2 IDs
- Sent on **TX ID 0x223** → gimbal replied with SDK frames on **RX ID 0x222**
- SDK frame starts with `0xAA 0x28` (40 bytes)
- CmdSet=0x0E, CmdID=0x08 = Gimbal Parameter Push (telemetry)
- **Finding:** RS 5 uses the same CAN IDs as RS 2 (0x222/0x223) for SDK protocol. The 0x426 heartbeat runs independently.

## Experiment 3: Live Telemetry Parsing

**Goal:** Decode telemetry push data and verify angles respond to physical movement.
**Setup:** Enable-push active, parsing attitude and joint angles from push frames.
**Result:** SUCCESS
- All three axes responding correctly
- Attitude angles change when gimbal is physically moved
- Joint angles show motor compensation
- Gimbal actively stabilizes (attitude returns to ~0 when released)
- Limits: pitch 55-155°, yaw ±180°, roll ±30°
- Stiffness: pitch=46, yaw=65, roll=54
- Push rate: ~1 Hz

## Experiment 4: Position Control — Pan (attempt 1)

**Goal:** Send position command to pan gimbal 45° right.
**Setup:** CmdSet=0x0E, CmdID=0x00, yaw=450 (45.0°), ctrl_byte=0x0D (absolute, roll+pitch invalid), time_for_action=20 (2.0s). CRC32 zeroed out.
**Result:** FAILED — gimbal did not move
- Telemetry continued flowing (connection still active)
- Yaw stayed at ~-0.4° throughout
- No error response observed
- **Hypothesis:** CRC32 validation required for position commands (was zeroed)

## Experiment 5: Position Control — Pan (attempt 2, with CRC32)

**Goal:** Same as experiment 4 but with CRC32 computed.
**Setup:** Same command, CRC32 computed with non-reflected tables and init=0xC55C0000.
**Result:** FAILED — gimbal did not move
- CRC implementation was WRONG — used non-reflected tables and wrong init values
- Correct values (from existing dji_crc lib matched to Python demo): CRC16 init=0x3AA3, CRC32 init=0x00003AA3, reflected tables
- This also means the enable-push command worked despite bad CRC — the gimbal may not validate CRC on push-enable but does on position commands

## Experiment 6: Position Control — Pan (attempt 3, correct CRC)

**Goal:** Same command with correct CRC from the existing dji_crc library.
**Setup:** Using dji::buildFrame from dji_protocol.h, CmdType=0x03, CmdSet=0x0E, CmdID=0x00, yaw=450 (45.0°), roll=0, pitch=0, ctrl_byte=0x01 (absolute, all axes valid), time_for_action=20 (2.0s).
**Result:** SUCCESS — gimbal moved to yaw=45.0°
- Position command reply: `CmdType=0x20 CmdSet=0x0E CmdID=0x00 payload_len=1 data: 00` (return code 0x00 = success)
- Telemetry confirmed: `yaw=45.0 roll=0.0 pitch=0.0`
- **Root cause of earlier failures:** CRC implementation was wrong. The test was using non-reflected CRC tables with init values from the PDF spec (CRC16 init=0xC55C, CRC32 init=0xC55C0000). The correct implementation uses reflected tables with init values CRC16=0x3AA3, CRC32=0x00003AA3, matching the Python demo code (SDKCRC.py).
- **Key frame bytes:** `AA 1A 00 03 00 00 00 00 0D 00 7E 7E 0E 00 C2 01 00 00 00 00 01 14 84 24 DC CF`

## Experiment 7: Confirm position control — Pan to -30°

**Goal:** Verify position control is repeatable by commanding a different angle.
**Setup:** Same as experiment 6, yaw=-300 (-30.0°).
**Result:** SUCCESS — gimbal moved from 45° to -30°

## Experiment 8: Pitch and roll control

**Goal:** Verify pitch and roll axes respond to position commands.
**Setup:** Cycle through pitch up 30°, pitch down 20°, roll right 15°, roll left 15°, return to center.
**Result:** SUCCESS — all axes moved correctly and returned to center

## Summary

All R SDK commands tested and working on the DJI RS 5:

| Feature | Status | Notes |
|---------|--------|-------|
| CAN bus communication | Working | Same IDs as RS 2 (TX=0x223, RX=0x222) |
| Enable telemetry push | Working | CmdSet=0x0E CmdID=0x07, returns 0x00 |
| Telemetry parsing | Working | Attitude + joint angles, limits, stiffness |
| Position control (yaw) | Working | Tested 45° and -30° |
| Position control (pitch) | Working | Tested 30° and -20° |
| Position control (roll) | Working | Tested ±15° |
| Camera control | Untested | |

The existing `dji_can` library in `firmware/lib/dji_can/` works with the RS 5 with zero code changes.

## Notes — RS 5 Angle Limits (physically tested)

| Axis | Min | Max | Unit |
|------|-----|-----|------|
| Pitch | -64° | 169° | degrees |
| Yaw | -180° | 180° | degrees (from telemetry limits) |
| Roll | -30° | 30° | degrees (from telemetry limits) |
