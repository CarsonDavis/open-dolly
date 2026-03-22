# V2 Wireframes

Status: **active**

ASCII wireframes for all V2 views and states. These define layout and information hierarchy, not visual design (colors, fonts, spacing are handled in implementation).

---

## 1. Keyframe View — Phone Portrait (<640px)

### Empty State
```
┌─────────────────────────┐
│  ● Connected    ▪ Idle  │ ← status bar
├─────────────────────────┤
│                         │
│     No keyframes yet.   │
│     Use the controls    │
│     below to position   │
│     the camera, then    │
│     capture.            │
│                         │
│                         │
│                         │
│                         │
├─────────────────────────┤
│ ──── Control Panel ──── │
│                         │
│ Pan    [◄-1°][◄-15°]   │
│  45.0°  ═══●═══════    │
│        [+15°►][+1°►]   │
│                         │
│ Slide  [◄-1][◄-10]     │
│  245mm  ════●══════    │
│        [+10►][+1►]     │
│                         │
│ [  ⚙ Jog  |  Position] │ ← mode toggle
│                         │
│ [ ★  Capture Keyframe ] │
│ [     ▼ minimize      ] │
└─────────────────────────┘
```

### With Keyframes and Transitions
```
┌─────────────────────────┐
│  ● Connected    ▪ Idle  │
├─────────────────────────┤
│ ┌─────────────────────┐ │
│ │ Keyframe 1          │ │
│ │ Pan 0° Tilt 0°      │ │
│ │ Slide 0mm           │ │
│ └─────────────────────┘ │
│         │ 8.0s │        │ ← duration bracket (tap to edit)
│ ┌─────────────────────┐ │
│ │ Keyframe 2          │ │
│ │ Pan 45° Tilt -10°   │ │
│ │ Slide 500mm         │ │
│ └─────────────────────┘ │
│        │ 12.0s │        │
│ ┌─────────────────────┐ │
│ │ Keyframe 3          │ │
│ │ Pan 90° Tilt 5°     │ │
│ │ Slide 800mm         │ │
│ └─────────────────────┘ │
│                         │
├─────────────────────────┤
│ [  ★ Capture Keyframe ] │ ← minimized control panel
│ [     ▲ expand        ] │
├─────────────────────────┤
│ [ ▶ Upload & Play     ] │ ← transport bar
└─────────────────────────┘
```

### Keyframe Selected (editing mode)
```
┌─────────────────────────┐
│  ● Connected    ▪ Idle  │
├─────────────────────────┤
│ ┌─────────────────────┐ │
│ │ Keyframe 1          │ │
│ └─────────────────────┘ │
│         │ 8.0s │        │
│ ┌═════════════════════┐ │ ← selected (highlighted border)
│ ║ ✎ Keyframe 2        ║ │
│ ║ Pan 45° Tilt -10°   ║ │
│ ║ Slide 500mm         ║ │
│ ║ [Deselect]          ║ │
│ └═════════════════════┘ │
│        │ 12.0s │        │
│ ┌─────────────────────┐ │
│ │ Keyframe 3          │ │
│ └─────────────────────┘ │
├─────────────────────────┤
│ ──── Control Panel ──── │
│ Editing Keyframe 2      │ ← indicator replaces capture button
│ Pan    45.0°  [jog ═══] │   jogging updates KF2's stored values
│ Slide  500mm  [jog ═══] │
└─────────────────────────┘
```

---

## 2. Transition Simple View — Phone Portrait

Appears when tapping a duration bracket.

```
┌─────────────────────────┐
│  ● Connected    ▪ Idle  │
├─────────────────────────┤
│ ┌─────────────────────┐ │
│ │ Keyframe 1          │ │
│ └─────────────────────┘ │
│         │ ▼ │           │ ← bracket expanded
├─────────────────────────┤
│ Transition: KF1 → KF2  │
│                         │
│ Duration  [ 8.0    ] s  │ ← primary input
│                         │
│ Slide   31.9 mm/s  ◁   │ ← tap to switch to speed-primary
│ Pan      5.6  °/s  ◁   │
│                         │
│         [Advanced ▸]    │ ← opens curve editor
│                         │
├─────────────────────────┤
│ ┌─────────────────────┐ │
│ │ Keyframe 2          │ │
│ └─────────────────────┘ │
└─────────────────────────┘
```

### Speed-Primary Mode (after tapping Slide speed)
```
│ Duration    10.0     s  │ ← auto-calculated, greyed out
│                         │
│ Slide  [ 25.5 ] mm/s ● │ ← now editable, active indicator
│ Pan      4.5   °/s  ◁  │ ← recalculated
│                         │
│ [Back to Duration]      │
```

---

## 3. Curve Editor — Phone Portrait

Replaces control panel area when "Advanced" is tapped.

```
┌─────────────────────────┐
│  ● Connected    ▪ Idle  │
├─────────────────────────┤
│ KF1 → KF2  (8.0s)      │
│ [Done]     [Presets ▾]  │
├─────────────────────────┤
│ □ Slide                 │ ← checkbox for linking
│ 1│         ╱‾‾‾‾‾‾‾    │
│  │       ╱·             │ ← · = control point (draggable)
│  │     ╱                │
│  │   ╱                  │
│ 0│_╱__________________  │
│  0s       4s       8s   │
├─────────────────────────┤
│ □ Pan                   │
│ 1│              ╱‾      │
│  │            ╱         │
│  │          ╱           │
│  │        ·             │ ← point at 5s, progress near 0
│ 0│______╱___________    │   (pan delayed for first 5s)
│  0s       4s       8s   │
├─────────────────────────┤
│ ☑ Tilt  (linked w/ Pan) │ ← checked = linked
│ 1│              ╱‾      │ ← same curve shape as Pan
│  │            ╱         │
│  │          ╱           │
│  │        ·             │
│ 0│______╱___________    │
│  0s       4s       8s   │
└─────────────────────────┘
```

---

## 4. Control Panel Detail — Jog Mode vs. Position Mode

### Jog Mode (default)
```
┌──────────────────────────────────┐
│ [Interactive ● | ○ Static]       │ ← mode toggle
├──────────────────────────────────┤
│ Slide  245.3 mm                  │ ← tap to type exact value
│ [-10][-1] ◄═══════●═══════► [+1][+10] │ ← jog: drag from center
│                                  │      increment buttons at edges
├──────────────────────────────────┤
│ Pan  45.0°                       │
│ [-15][-1] ◄═══●═══════════► [+1][+15] │
├──────────────────────────────────┤
│ Tilt  0.0°                       │
│ [-15][-1] ◄══════════●════► [+1][+15] │
├──────────────────────────────────┤
│ [ ★ Capture Keyframe ]           │
│ [        ▼ minimize  ]           │
└──────────────────────────────────┘
```

### Position Mode
```
│ Slide  245.3 mm                  │
│ [-10][-1]  ░░░░░░▓░░░░░░░  [+1][+10] │
│           0mm  ↑  ↑       1000mm │
│                │  └─ tap here to jump to ~600mm
│                └─ current position marker
```

---

## 5. Keyframe View — Landscape Phone / Tablet (640-1024px)

```
┌──────────────────┬───────────────────────────┐
│  ● Connected     │                           │
├──────────────────┤  ┌───────────────────────┐ │
│                  │  │ Keyframe 1            │ │
│  Control Panel   │  │ Pan 0° Slide 0mm      │ │
│                  │  └───────────────────────┘ │
│  Slide  245mm    │          │ 8.0s │          │
│  [jog ══●══════] │  ┌───────────────────────┐ │
│  [-10][-1][+1][+10]│ │ Keyframe 2            │ │
│                  │  │ Pan 45° Slide 500mm   │ │
│  Pan  45.0°      │  └───────────────────────┘ │
│  [jog ════●════] │         │ 12.0s │          │
│                  │  ┌───────────────────────┐ │
│  Tilt  0.0°      │  │ Keyframe 3            │ │
│  [jog ══════●══] │  │ Pan 90° Slide 800mm   │ │
│                  │  └───────────────────────┘ │
│                  │                           │
│ [★ Capture]      │                           │
├──────────────────┴───────────────────────────┤
│ [ ▶ Upload & Play ]                          │
└──────────────────────────────────────────────┘
```

---

## 6. Desktop Layout (>1024px) — Keyframe View + Timeline

```
┌──────────────────┬───────────────────────────────────────┐
│  ● Connected     │                                       │
├──────────────────┤  ┌─────────────┐   ┌─────────────┐   │
│  Control Panel   │  │ Keyframe 1  │ 8s│ Keyframe 2  │12s│...
│                  │  └─────────────┘   └─────────────┘   │
│  [axis controls] │                                       │
│  [★ Capture]     │  [ ▶ Upload & Play ] [⏸] [⏹]        │
├──────────────────┴───────────────────────────────────────┤
│ Timeline                                    [View: Both ▾]│
├──────────────────────────────────────────────────────────┤
│ KF1          KF2                    KF3                  │
│  ▼            ▼                      ▼     ← markers     │
│  ├── 8.0s ───┤────── 12.0s ────────┤                    │
├──────────────────────────────────────────────────────────┤
│ Slide │╱‾‾‾‾‾‾‾‾│╱‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾│     ← axis lanes  │
│ Pan   │_____╱‾‾‾│╱‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾│                    │
│ Tilt  │_____╱‾‾‾│─────────────────│     (flat = no Δ)  │
├──────────────────────────────────────────────────────────┤
│  |                  ▼                                    │
│  0s     4s     8s    12s    16s    20s    ← time scale   │
│                     ↑ playhead (draggable for scrub)     │
└──────────────────────────────────────────────────────────┘
```

---

## 7. Timeline View — Keyframe Retiming

Dragging a keyframe marker on the timeline redistributes duration.

### Before drag:
```
KF1          KF2                    KF3
 ▼            ▼                      ▼
 ├── 8.0s ───┤────── 12.0s ────────┤
```

### During drag (KF2 dragged left to 5s):
```
KF1     KF2                         KF3
 ▼       ▼                           ▼
 ├─5.0s─┤──────── 15.0s ────────────┤
         ↑ dragging
```

Total unchanged: 8 + 12 = 20s → 5 + 15 = 20s.

---

## 8. Curve Editor States

### Empty (linear — default)
```
│ Slide                   │
│ 1│                  ╱   │
│  │              ╱       │
│  │          ╱           │  ← straight line = linear
│  │      ╱               │
│ 0│__╱_______________    │
│  0s              8s     │
```

### Single point (ease out)
```
│ Slide                   │
│ 1│         ╱‾‾‾‾‾‾‾    │
│  │       ╱·             │  ← one control point dragged up
│  │     ╱                │     fast start, gentle stop
│  │   ╱                  │
│ 0│_╱________________    │
│  0s              8s     │
```

### Delayed start
```
│ Pan                     │
│ 1│              ╱‾      │
│  │            ╱         │
│  │          ╱           │  ← flat for first 5s, then ramp
│  │        ·             │     point near (0.625, 0.0)
│ 0│______╱___________    │
│  0s    5s        8s     │
```

### Complex multi-point
```
│ Slide                   │
│ 1│          ·╱‾‾‾‾‾    │
│  │        ╱             │  ← three control points
│  │     ·╱               │     S-curve shape
│  │   ╱                  │
│ 0│_╱·_______________    │
│  0s              8s     │
```

---

## 9. Buffer Indicators

### In Keyframe View (duration bracket with buffers)
```
│ ┌─────────────────────┐ │
│ │ Keyframe 1          │ │
│ └─────────────────────┘ │
│     │1.0s│ 8.0s │1.0s│  │ ← pre/motion/post
│ ┌─────────────────────┐ │
│ │ Keyframe 2          │ │
│ └─────────────────────┘ │
```

### Buffer Warning
```
┌─────────────────────────────┐
│ ⚠ Buffer Warning            │
│                             │
│ Slide axis at Keyframe 3:   │
│ Only 5mm of travel remain-  │
│ ing. Buffer needs ~25mm.    │
│                             │
│ [Skip Buffer] [Adjust KF3]  │
└─────────────────────────────┘
```

---

## 10. Transport Bar States

### Disabled — durations not set
```
│ [ Set transition durations  ] │  ← greyed out
```

### Ready to play
```
│ [ ▶ Upload & Play          ] │
```

### Computing / Uploading
```
│ [ ◌ Computing...            ] │
│ [ ◌ Uploading...            ] │
```

### Playing
```
│ [⏸ Pause] [⏹ Stop]  ░░░▓░░░░░░  12.3s / 20.0s │
```

### Paused
```
│ [▶ Resume] [⏹ Stop]  ░░░▓░░░░░░  12.3s / 20.0s │
```

---

## 11. Phone Portrait — Minimized Control Panel

When the user needs more screen space for keyframes:

```
┌─────────────────────────┐
│  ● Connected    ▪ Idle  │
├─────────────────────────┤
│ ┌─────────────────────┐ │
│ │ Keyframe 1          │ │
│ └─────────────────────┘ │
│         │ 8.0s │        │
│ ┌─────────────────────┐ │
│ │ Keyframe 2          │ │
│ └─────────────────────┘ │
│        │ 12.0s │        │
│ ┌─────────────────────┐ │
│ │ Keyframe 3          │ │
│ └─────────────────────┘ │
│                         │
│                         │ ← more room for keyframes
│                         │
├─────────────────────────┤
│ [★ Capture]  [▲ expand] │ ← thin tray
├─────────────────────────┤
│ [ ▶ Upload & Play     ] │
└─────────────────────────┘
```
