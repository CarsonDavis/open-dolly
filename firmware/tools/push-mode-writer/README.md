# Push Mode Writer

Toggles push pan and push tilt on the DJI RS 5 via undocumented TLV parameters. Confirms that we can programmatically enable/disable push mode over CAN bus.

## What it does

1. Reads current push state (TLV 0x14 and 0x15)
2. Enables push pan (TLV 0x14 = 1) — 10 second window to test
3. Disables push pan
4. Enables push tilt (TLV 0x15 = 1) — 10 second window to test
5. Disables push tilt
6. Verifies both are off

## TLV IDs

| TLV ID | Function | Write via | Values |
|--------|----------|-----------|--------|
| 0x14 | Push pan | CmdSet=0x0E CmdID=0x0C | 0x00=off, 0x01=on |
| 0x15 | Push tilt | CmdSet=0x0E CmdID=0x0C | 0x00=off, 0x01=on |

Write payload format: `TLV_ID (1 byte) + Length (1 byte, always 0x01) + Value (1 byte)`

## When we used it

2026-03-26. Confirmed that writing TLV 0x14=1 enables push pan (gimbal follows hand push on yaw axis) and TLV 0x15=1 enables push tilt (gimbal follows hand push on pitch axis). Writing 0x00 disables each. Both return code 0x00 (success).

## What we learned

- Push mode is fully controllable via CAN bus using the undocumented TLV parameters
- These are standard R SDK user parameter TLV writes (CmdID 0x0C), just with IDs not listed in the v2.5 spec
- The gimbal responds immediately — no delay between write and behavior change
- Push mode and SDK position control can coexist: enable push to position by hand, disable push, then send position commands
