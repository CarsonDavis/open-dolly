# Telemetry Reader

Sends enable-push command and decodes live gimbal telemetry. Prints attitude angles, joint angles, axis limits, and motor stiffness values in real-time.

## What it does

1. Sends enable-push (CmdSet=0x0E, CmdID=0x07) to start telemetry streaming
2. Re-sends every 5 seconds to keep the stream alive
3. Parses push data frames (CmdSet=0x0E, CmdID=0x08) and prints decoded values
4. Move the gimbal by hand to see angles change

## When we used it

Second test during RS 5 bring-up (2026-03-25). Used to confirm that the R SDK telemetry push works identically to the RS 2, and that all three axes report correct angles.

## What we learned

- Telemetry push rate is ~1 Hz on the RS 5
- All three axes report attitude and joint angles correctly
- Gimbal actively stabilizes — attitude returns to ~0 when released
- RS 5 limits: pitch 55-155°, yaw ±180°, roll ±30°
- Stiffness values: pitch=46, yaw=65, roll=54 (user-configured)
