# TLV Parameter Scanner

Reads all user parameter TLV IDs (0x00–0x3F) from the gimbal using CmdSet=0x0E, CmdID=0x0B (Obtain Handheld Gimbal User Parameters). Used for reverse-engineering undocumented gimbal settings.

## What it does

1. Sends enable-push to establish connection
2. Waits for first telemetry frame to confirm the gimbal is responding
3. Iterates through TLV IDs 0x00–0x3F, sending a read request for each
4. Prints the return code and raw data bytes for each ID
5. Stops after one full scan — reset the ESP to scan again

## How to use for reverse-engineering

1. Set the gimbal to a known baseline state (e.g., all push modes OFF)
2. Flash and run the scanner, copy the output
3. Change ONE setting in the DJI Ronin app (e.g., enable push pan)
4. Reset the ESP, run the scanner again, copy the output
5. Diff the two outputs to find which TLV ID changed

The response format is: `TLV ID (1 byte) + Length (1 byte) + Value (N bytes)`. The TLV ID in the response data may not match the request ID due to response timing — always use the ID in the data field as the ground truth.

## When we used it

Used on 2026-03-26 to reverse-engineer the push mode settings on the RS 5. Ran three scans:

1. Both push OFF (baseline)
2. Both push pan + push tilt ON
3. Push pan ON, push tilt OFF

## What we learned

Comparing the three scans identified two undocumented TLV IDs:

| TLV ID | Function | Values |
|--------|----------|--------|
| 0x14 | Push pan | 0x00=off, 0x01=on |
| 0x15 | Push tilt | 0x00=off, 0x01=on |

These are not documented in the R SDK v2.5 spec. They can likely be written using CmdSet=0x0E, CmdID=0x0C (Set Handheld Gimbal User Parameters) with the same TLV format.

## Caveats

- Some TLV IDs return `rc=7` (undefined error) or no response — these may be write-only, require a specific parameter table selection, or not exist on the RS 5
- Response timing can cause occasional misalignment between requested ID and received response — the actual TLV ID is always in the first byte of the data field
- The scan range 0x00–0x3F may not cover all IDs — extend if needed
