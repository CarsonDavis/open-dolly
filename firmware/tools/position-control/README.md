# Position Control Test

Sends absolute position commands to move the gimbal through a sequence of poses, confirming all three axes respond to SDK commands.

## What it does

1. Sends enable-push to establish connection
2. Waits for first telemetry frame
3. Cycles through 7 moves (4 seconds each): pan right 45°, pan left 45°, pitch up 30°, pitch down 20°, roll right 15°, roll left 15°, return to center
4. Prints telemetry between moves so you can verify the gimbal reached each target

## When we used it

Third test during RS 5 bring-up (2026-03-25). Initially failed because the CRC implementation was wrong — used non-reflected tables from the PDF spec instead of the reflected tables from the Python demo code. Once we switched to using the `dji_crc` library (CRC16 init=0x3AA3, CRC32 init=0x00003AA3, reflected I/O), all position commands worked.

## What we learned

- Position control works on all three axes (pan, pitch, roll)
- The gimbal replies with return code 0x00 (success) for valid commands
- CRC must be correct — the gimbal silently drops commands with bad CRC (no error response)
- `time_for_action` field sets how fast the gimbal moves (in 0.1s units)
- The `ctrl_byte` bit layout: bit 0 = absolute(1)/incremental(0), bits 1-3 = axis invalid flags
