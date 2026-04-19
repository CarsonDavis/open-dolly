# Slider Project Devlog

Reverse-chronological record of *confirmed, verified* work across all subsystems. See the work-journal protocol in `/CLAUDE.md` for what belongs here.

---

## 2026-04-19 — Bring-up toolbar + firmware commands shipped and verified

Added the control surface needed to actually run §6.3 motor tests: new `clear_error` / `disable` / `restart` WebSocket commands end-to-end (shared types, firmware WS parser, `CommandType::DISABLE` → `stepperDisable()`, direct `ESP.restart()` on "restart"), plus a `BringupToolbar.svelte` always-visible above the main UI with E-STOP / Home / Disable / Restart and a conditional Clear Error (pulsing yellow when `state === 'error'`). Flashed `slider_only` firmware + LittleFS with the rebuilt web bundle; smoke-tested all four toolbar actions on the bench (limit-press → state=error → Clear Error returns to idle → Restart reboots with WS auto-reconnect). Mock-server was also extended to keep `npm run dev` green. Next: real §6.3 steps 4–6 with 24 V PSU plugged in.

## 2026-04-18 — Slider Phase 1 wiring and first power-up: PASS

Completed `docs/project/slider-wiring.md` §3 end-to-end on the breadboard: pre-power continuity checks (§3.12) clean, then first power-up with all four §3.13 voltage readings in spec — 5 V rail within 4.9–5.1 V, 3.3 V rail within 3.2–3.4 V, TMC2209 VM ≈ 24 V (no VM/VIO swap), VREF ≈ 1.1 V after trimpot calibration. Motor and driver not loaded yet; bench powered down cleanly. Next: §4 firmware polarity flip (`INPUT_PULLUP` → `INPUT_PULLDOWN` on `PIN_LIMIT_MIN`/`PIN_LIMIT_MAX`), flash `slider_only` build (§6.1), and run the §6.3 smoke test sequence (boot log → manual limit actuation → jog ±10 mm → limit stop → home repeatability).

## 2026-04-18 — KiCad project scaffolded for schematic + eventual PCB

Bootstrapped `hardware/kicad/slider/` on KiCad 10.0.1 with a minimum-viable project (`slider.kicad_pro`, `slider.kicad_sch`) verified by `kicad-cli sch erc` (0 violations). Added a beginner walkthrough (`README.md`) that will double as the printable breadboard wiring reference as it's filled in — §1–§3 drafted (open project, place `+24V`/`+5V`/`+3.3V`/`GND` power ports, barrel jack wiring, LM2596 via Symbol Editor). Committed as `959fc20`.
