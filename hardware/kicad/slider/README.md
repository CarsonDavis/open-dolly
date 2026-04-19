# Slider KiCad Project — Walkthrough

This is both the schematic for the Phase 1 breadboard bring-up and the starting point for the eventual custom PCB. The walkthrough below is designed to be followed with KiCad 10 open on your screen; each section ends with a **checkpoint** you should verify before continuing.

**Source of truth for the wiring:** [`docs/project/slider-wiring.md`](../../../docs/project/slider-wiring.md). If anything in this walkthrough disagrees with that guide, the guide wins — tell me and I'll fix the walkthrough.

---

## Vocabulary cheatsheet

You said you don't know what a symbol is, so here are the words you'll see in KiCad:

- **Symbol** — the picture of a part in the schematic. A resistor is a zigzag, an ESP32 is a labeled rectangle. Each symbol has **pins** with numbers and names.
- **Footprint** — the physical copper pads on the PCB for that same part. Not used until the PCB phase.
- **Wire** — a green line connecting two pins. An electrical connection.
- **Net** — a group of pins that are all wired together electrically. The `3V3` net, for example, is every pin that sits at 3.3 V.
- **Label** — a text tag attached to a wire. Two wires with the same label are the same net, even if they're on opposite sides of the page. This is how we avoid drawing long spaghetti wires.
- **Power port** — a special kind of label for power rails (`+24V`, `+5V`, `+3V3`, `GND`). Looks like a little triangle or T. Behaves like a label.
- **ERC** — Electrical Rules Check. A linter that catches unconnected pins, conflicting power nets, etc.
- **Library** — a collection of reusable symbols. KiCad ships with hundreds (`Device`, `Connector`, `Switch`, `Motor`, etc.).

The parts we don't have exact symbols for (ESP32-S3-DevKitC-1, BTT TMC2209 V1.3, LM2596 module) will be drawn as **generic connectors** — a row of numbered pins with each pin labeled with what's actually on the physical module. This is the right call for a breadboard reference: you're wiring to header pins, not to chip internals, so a generic connector matches reality better than a detailed IC symbol.

---

## §1. Open the project

1. Launch **KiCad** (the app with the gold-and-blue logo, not the individual editors).
2. **File → Open Project…** and pick `hardware/kicad/slider/slider.kicad_pro`.
3. The KiCad project window shows a file tree on the left and big buttons for **Schematic Editor**, **PCB Editor**, etc. on the right.
4. Double-click `slider.kicad_sch` in the file tree (or click the **Schematic Editor** button).
5. You should see an empty A4 sheet with a title block at the bottom-right.

### Checkpoint 1
- [ ] KiCad opens the project without a migration prompt. (If it asks to upgrade, accept — but tell me, because we already upgraded the file so it shouldn't ask.)
- [ ] Schematic Editor shows a blank A4 sheet with grid dots.

**If something went wrong**, stop and tell me what error you saw — don't click through.

---

## §2. Place the power labels

Before placing any parts, we'll drop the four power rails as labels. This teaches the click-to-place mechanic and also gets the "hard" step out of the way — once the rails exist, every part just connects to them.

Keyboard shortcuts you'll use a lot:
- `M` — move the selected item
- `R` — rotate
- `Esc` — cancel current tool
- `G` — drag (keeps wires attached)
- `Del` — delete
- `Ctrl+Z` — undo

### 2.1 Place the +24V rail

1. In the right-hand toolbar, click the **Power Port** icon (looks like a ground symbol — an upside-down T). Or press `P`.
2. The **Choose a symbol** dialog opens. Type `+24V` in the filter box.
3. Pick `+24V` from the `power` library. Click **OK**.
4. A red "+24V" symbol follows your cursor. Click near the top-left of the sheet to drop it. Press `Esc` to stop placing.

### 2.2 Place +5V, +3V3, GND

Repeat `P` for each:
- `+5V` — drop it below +24V
- `+3V3` — below +5V
- `GND` — below +3V3. The `GND` symbol looks like a ground triangle (that's the symbol for electrical ground).

Don't worry about exact placement — we'll tidy up later. For now just get one of each on the sheet.

### 2.3 Save

Press `Ctrl+S`. You should see the title bar change from `*slider [unsaved]` to `slider`.

### Checkpoint 2
- [ ] Four power port symbols visible on the sheet: `+24V`, `+5V`, `+3.3V`, `GND`.
- [ ] File saved without errors.

> **Naming consistency:** KiCad's power library ships both `+3V3` and `+3.3V` as separate nets. Pick one and use it everywhere. This walkthrough uses `+3.3V`.

---

## §3. Place the power tree: PSU + LM2596 buck

The slider-wiring guide's power tree is:

```
24 V wall PSU ──► 24 V rail ──┬──► TMC2209 VM (motor power)
                              └──► LM2596 IN+ ──► LM2596 OUT+ (tuned to 5.0 V) ──► ESP32 5V
```

We'll capture this on the schematic as: a **barrel jack** symbol for the wall PSU, and a **4-pin generic connector** representing the LM2596 module (its four screw terminals: IN+, IN−, OUT+, OUT−). Each gets wired to the power labels you just placed.

### 3.1 Place the barrel jack

1. Press `A` (add symbol) — or click the AND-gate-looking icon in the right toolbar.
2. In the **Choose a symbol** dialog, filter for `Barrel_Jack`. Pick `Connector:Barrel_Jack`.
3. Click **OK**, drop the symbol to the left of your power labels (say, below `GND` and to the left).
4. Press `Esc`.

The symbol shows three pins: `1` (center/tip, `+`), `2` (ring, `−`), and `3` (switch — unused on most jacks). We'll only wire pins 1 and 2.

### 3.2 Draw the LM2596 symbol yourself (Symbol Editor detour)

The LM2596 module isn't a stock symbol, and grabbing an unverified community upload for a generic Chinese board is worse than drawing it ourselves — it's a rectangle with 4 pins. The workflow you're about to learn (make a project library, draw a symbol, use it on the sheet) is the one you'll use for every module that isn't in stock, forever. Good investment.

#### 3.2.1 Create a project-local symbol library

A "library" is a `.kicad_sym` file that holds your custom symbols. We'll put it next to the project so it travels with the repo.

1. Back in the KiCad project window (not the Schematic Editor), click the **Symbol Editor** button (the op-amp-looking icon). A new window opens.
2. In the Symbol Editor: **File → New Library…**
3. When prompted **Global or Project?**, pick **Project**.
4. Save it as **`slider-modules.kicad_sym`** inside the project folder `hardware/kicad/slider/`.
5. You'll see a new library entry appear in the left panel (tree view).

#### 3.2.2 Create the LM2596 symbol

1. In the left panel, **right-click on `slider-modules`** → **New Symbol…**
2. Dialog fields:
   - **Symbol name:** `LM2596_Module`
   - **Default reference designator:** `U`
   - Leave the rest at defaults.
3. Click **OK**.

You're now on an empty edit canvas with crosshairs at the origin (center).

#### 3.2.3 Draw the body

1. In the right toolbar, click the **Add a rectangle** icon (filled square outline). Or **Place → Add Rectangle**.
2. Click once about 4 grid units left and 3 up from the center — that's the top-left corner.
3. Click again about 4 grid units right and 3 down — bottom-right corner.
4. Press `Esc`. You should have a yellow-filled rectangle.

Don't fuss over exact size; you can drag-resize the corners later.

#### 3.2.4 Add the four pins

In KiCad, each pin has: **name** (the label you see), **number** (the physical pin #, 1-4 for our module), **electrical type** (affects ERC), and an **orientation** (which way it sticks out of the body).

1. Click the **Add a pin** tool in the right toolbar (looks like a single line with a dot on one end). Or press `P`.
2. A **Pin Properties** dialog opens. Fill in:

   | Field | Value |
   |---|---|
   | Pin name | `IN+` |
   | Pin number | `1` |
   | Electrical type | **Power input** |
   | Orientation | **Right** (pin sticks out to the right of its attach point — so you'll attach it to the LEFT edge of the body, with the line extending leftward away from the body; rotate with `R` if it looks wrong) |
   | Length | `2.54 mm` (default) |

3. Click **OK**. The pin follows your cursor.
4. Click on the **left edge** of the rectangle, upper portion, to drop it.
5. Press `P` again for the next pin:

   | Pin 2 | |
   |---|---|
   | Name | `IN-` |
   | Number | `2` |
   | Type | **Power input** |
   | Orientation | Right (same as pin 1) |

6. Drop it on the left edge below pin 1.
7. Press `P` for pin 3:

   | Pin 3 | |
   |---|---|
   | Name | `OUT+` |
   | Number | `3` |
   | Type | **Power output** |
   | Orientation | **Left** (pin stuck out to the left of attach point — i.e., sticks out of the RIGHT edge of the body) |

8. Drop it on the **right edge** of the rectangle, upper portion.
9. Press `P` for pin 4:

   | Pin 4 | |
   |---|---|
   | Name | `OUT-` |
   | Number | `4` |
   | Type | **Power output** |
   | Orientation | Left |

10. Drop it on the right edge below pin 3. Press `Esc`.

If a pin ends up pointing the wrong way, select it and press `R` to rotate.

#### 3.2.5 Save the library

`Ctrl+S`. The title should lose its asterisk.

You can close the Symbol Editor window now — back to the Schematic Editor.

### 3.3 Place the LM2596 on the schematic

1. In the Schematic Editor, press `A`.
2. In the Choose Symbol dialog, type `LM2596` — your `LM2596_Module` should appear under the `slider-modules` library.
3. Click **OK**, click to drop it in the middle of the sheet, right of the barrel jack. Press `Esc`.
4. **Double-click the symbol** to set **Reference = `U1`**. (The Value already says `LM2596_Module`.) Click **OK**.

### 3.3 Wire the barrel jack to the 24V rail

Time for your first wires.

1. Press `W` (wire tool) — or click the green diagonal line icon in the right toolbar.
2. Click the **small circle on pin 1 of the barrel jack** (the `+` tip contact).
3. Move the cursor up and left — KiCad draws a green wire.
4. Click at a clear spot, then move up to your `+24V` power port.
5. Click on the **small circle at the base of the `+24V` arrow** to end the wire. It snaps in.
6. Press `Esc`.

Do the same for the GND pin:
7. Press `W`, click pin 2 of the barrel jack (`−` ring), run a wire to the small circle at the top of the `GND` symbol.
8. Press `Esc`.

Leave pin 3 unconnected. It'll cause an ERC warning later — we'll silence it with a **no-connect flag** (`Q` shortcut) then, not now.

### 3.4 Wire the LM2596 to the rails

We need:
- LM2596 pin 1 (`IN+`) → +24V
- LM2596 pin 2 (`IN−`) → GND
- LM2596 pin 3 (`OUT+`) → +5V
- LM2596 pin 4 (`OUT−`) → GND

Rather than drawing four long spaghetti wires back to the power labels, we'll use **local labels** on short stub wires at each pin. Two wires with the same label are electrically the same net.

For each of the four pins:

1. Press `W`, click on the pin, drag the wire 3–4 grid squares to the side, click to end, `Esc`.
2. With nothing selected, press `L` (local label).
3. Type the label name (e.g. `+24V`), press **Enter**.
4. The label follows your cursor — click on the **end of the stub wire** you just drew.

Labels to apply, pin-by-pin:
- Pin 1 (`IN+`) → label `+24V`
- Pin 2 (`IN−`) → label `GND`
- Pin 3 (`OUT+`) → label `+5V`
- Pin 4 (`OUT−`) → label `GND`

> **Important:** for `+24V`, `+5V`, `GND`, `+3.3V` — use **power labels** (the `P` shortcut, same as §2) rather than local labels (`L`). Power labels and local labels with matching text behave as the same net in KiCad, but power labels have a visible arrow and are the convention for power. So: `P` → type `+24V` → drop on the wire end. Do the same for `+5V` and `GND`.
>
> Use `L` (local labels) for signal names later like `STEP`, `DIR`, etc. Save `P` for rails.

Save: `Ctrl+S`.

### Checkpoint 3

- [ ] Barrel jack pin 1 wired to `+24V` power port; pin 2 wired to `GND`.
- [ ] LM2596 (U1) has four stub wires, each tagged with a `+24V`, `GND`, `+5V`, or `GND` power port.
- [ ] You can click any wire and it highlights; the whole "+24V net" lights up together when you click any `+24V`-labelled wire or port.

**Ping me at Checkpoint 3.** Known-ugly things you can ignore for now: ERC isn't clean yet (barrel jack pin 3 is unconnected, and we haven't put a `PWR_FLAG` on any net — both will be fixed after §4). The schematic doesn't need to be pretty-laid-out yet either.
