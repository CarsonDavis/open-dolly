# CAN Bus Listener

Passive CAN bus listener that prints all raw frames to serial. No commands are sent — TWAI runs in listen-only mode.

## What it does

Prints every CAN frame received with ID, DLC, and data bytes. Used to confirm wiring works and see what the gimbal broadcasts without any interaction.

## When we used it

First tool run during RS 5 bring-up (2026-03-25). Discovered that the RS 5 broadcasts a heartbeat on CAN ID **0x426** with data starting with `0x55`. This is separate from the SDK protocol frames (ID 0x222, starting with `0xAA`).

## What we learned

- RS 5 heartbeat: CAN ID 0x426, `55 45 04 DE E5 06 xx 54`, runs continuously
- No SDK frames appear without first sending an enable-push command
- CAN bus speed confirmed at 1 Mbps
