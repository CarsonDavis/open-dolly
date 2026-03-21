# ADR-002: Status Display

*Status: accepted*
*Date: 2026-03-20*

## Context

The slider board runs headless — all primary interaction happens through the web UI on your phone or laptop. But there are situations where glancing at the board itself is useful: checking battery voltage, confirming Wi-Fi AP is running, seeing the current IP/mDNS address, or knowing whether a move is in progress. A small on-board display makes the system more self-contained without duplicating the web UI's role.

## Decision

**Support an optional status display.** Recommend the **SH1107 1.5" 128x128 monochrome OLED** (I2C) as the default, with the **ST7789 240x240 color TFT** (SPI) as an alternative for users who want more pixels and color.

The display is optional — the system must work fully without one.

## Why SH1107 (I2C) as default

| Factor | SH1107 1.5" I2C | ST7789 1.5-2.0" SPI |
|--------|------------------|----------------------|
| Resolution | 128x128 | 240x240 to 320x240 |
| Color | Monochrome | 65K color |
| GPIO pins | 0 extra (shares I2C bus) | 5-6 dedicated |
| Price | $6-10 | $5-12 |
| Readability outdoors | Excellent (OLED, self-emitting) | Good (needs backlight) |
| Library | U8g2, Adafruit SH110X | TFT_eSPI |
| Firmware complexity | Low (text + simple graphics) | Medium (framebuffer, color mgmt) |

The I2C OLED wins as the default because:

- **Zero additional GPIO.** It shares the I2C bus (SDA/SCL) already allocated for expansion. The SPI display costs 5-6 pins that could go to other peripherals.
- **Better outdoor visibility.** OLED pixels emit their own light — readable in direct sunlight without a backlight. Camera sliders are often used outdoors.
- **Simpler firmware.** Text rendering on a mono OLED is straightforward. Color TFT needs a framebuffer and more complex rendering.
- **128x128 is enough for status.** Battery voltage, IP address, playback progress, axis positions, and error messages fit comfortably. This is not a UI — it's an instrument panel.

The ST7789 is a fine alternative for users who want it. The firmware should abstract the display interface so either can be used.

## What to show

- Wi-Fi AP status and IP / mDNS name
- Battery voltage (with low-battery warning)
- Current axis positions
- Playback state (idle / playing / paused) and progress
- Error messages (CAN timeout, limit switch hit, etc.)

## Alternatives Considered

### SSD1306 0.96" 128x64

The classic default. Works, but 0.96" is small and hard to read at arm's length on a slider rig. Half the pixels of the SH1107 for only $2-3 less. Not worth the savings.

### SSD1327 1.5" 128x128 grayscale

Same resolution and size as SH1107 but with 16-level grayscale. Slightly nicer for icons/graphs but more expensive ($8-12) and less library support. Grayscale isn't useful enough for status text to justify the premium.

### SSD1351 1.5" 128x128 color OLED (SPI)

Color OLED — great display quality, but requires SPI (5-6 GPIO) and costs $10-15. If you're going to spend SPI pins on a display, the ST7789 gives you more pixels (240x240) for less money.

### No display

Valid choice. The web UI shows everything. But checking your phone just to see battery voltage or confirm the board is running gets tedious during a shoot. A $6-10 OLED is worth it.

## Consequences

- Firmware must abstract display output behind an interface (I2C OLED vs SPI TFT)
- Display is optional — all code paths must work with no display connected
- I2C bus (SDA/SCL) is allocated in the GPIO budget for shared use (display + future I2C peripherals)
- The BOM lists both display options with pin counts so builders can choose
