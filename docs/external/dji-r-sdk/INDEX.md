# DJI R SDK Index

## Overview

Official DJI R SDK for controlling RS-series handheld gimbals over CAN bus. Includes DJI's demo application (Python/PyQt5, Windows-only) and protocol specification documents.

| Path | Purpose | Key Details |
|------|---------|-------------|
| `demo-software/` | DJI's official demo app source code | Python 3 + PyQt5; Windows-only due to CAN adapter DLLs (ZLG/GC USBCAN); protocol layer (`protocol/sdk/`) is pure Python and portable |
| `demo-software/main.py` | App entry point | Launches PyQt5 GUI |
| `demo-software/sys_infor.py` | Global state store and status display | Stores gimbal attitude/joint angles; defines CAN/UART exception types |
| `demo-software/protocol/sdk/CmdCombine.py` | Packet builder | Constructs DJI R SDK frames: header + CRC16 + payload + CRC32; manages sequence numbers |
| `demo-software/protocol/sdk/SDKCRC.py` | CRC16/CRC32 in Python | CRC16: poly=0x8005, init=0x3AA3; CRC32: poly=0x04C11DB7, init=0x3AA3; both reflected I/O |
| `demo-software/protocol/sdk/SDKHandle.py` | Packet parser and response dispatcher | Reassembles multi-frame CAN data; validates CRC; dispatches by CmdSet:CmdID; parses push data (attitude + joint angles) |
| `demo-software/protocol/sdk/CmdHandler.py` | Standalone command callbacks (unused) | Duplicate of handlers in SDKHandle.py; appears to be an earlier version |
| `demo-software/protocol/connection/CANConnection.py` | CAN hardware abstraction | Wraps ZLG_USBCAN and GC_USBCAN DLLs via ctypes; CAN Tx=0x223, Rx=0x222, 1Mbps; Windows-only — needs replacement for macOS/Linux |
| `demo-software/ui/gimbal_control.py` | PyQt5 UI layout | Auto-generated from .ui file; directional buttons, speed/position spinboxes, camera controls, attitude/joint angle displays |
| `demo-software/ui_init/gimbal_window_init.py` | Main controller logic | Connects CAN device; sends position/speed/camera commands; enables gimbal push telemetry; 50Hz timer for joystick-style speed control |
| `docs/DJI_R_SDK_Protocol_v2.2_EN.pdf` | Protocol spec v2.2 (Oct 2020) | 21 pages; bundled with demo software; angle ranges ±180° all axes |
| `docs/DJI_R_SDK_Protocol_and_User_Interface_EN_v2.5.pdf` | Protocol spec v2.5 (June 2021) | 22 pages; adds Focus Motor Control (CmdID 0x12), narrows roll to ±30° and pitch to -56°/+146°, adds CmdSet/CmdID to reply frames |
| `docs/custom_crc16.c`, `docs/custom_crc16.h` | CRC16 reference implementation in C | Table-driven; includes test main() with sample packet bytes |
| `docs/custom_crc32.c`, `docs/custom_crc32.h` | CRC32 reference implementation in C | Table-driven; includes test main() with sample packet bytes |
