# Project Docs Index

| Path | Purpose | Key Details |
|------|---------|-------------|
| `initial-vision.md` | Original product concept and UX goals | Slider + gimbal, keyframe system, web app control; status: active |
| `architecture.md` | System architecture and design principles | Hybrid trajectory model, layer diagram, modularity boundaries; status: draft |
| `board-api.md` | Board API specification (REST + WebSocket) | Hardware-agnostic JSON protocol; capabilities discovery, trajectory upload, real-time jog/telemetry; status: draft |
| `bom.md` | Bill of materials with cost estimates | 4 build tiers from $105 (slider-only) to $680 (slider + DJI RS 4); GPIO pin budget; power architecture; optional add-ons; status: draft |
| `adr-001-microcontroller.md` | ADR: why ESP32-S3 over all alternatives | Compared 9 options; ESP32-S3 wins on Wi-Fi + CAN + USB-OTG + stepper HW + GPIO count in one chip; specific board: Espressif DevKitC-1 N8R8 or N32R16V; status: accepted |
| `adr-002-status-display.md` | ADR: optional on-board status display | SH1107 1.5" I2C OLED (default) or ST7789 SPI TFT (alternative); display is optional; status: accepted |
| `adr-003-linear-drive.md` | ADR: linear drive system (belt vs ball screw vs lead screw) | 4 options compared; belt wins on speed, ball screw wins on precision simplicity; hybrid belt+linear encoder covers both; status: exploring |
