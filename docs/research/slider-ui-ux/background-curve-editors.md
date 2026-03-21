# Curve Editor & Timeline UI Patterns - Research Background

**Date:** 2026-03-21
**Topic:** Professional keyframe and curve editor UIs used in animation and motion graphics software, for informing OpenDolly V2 web UI design (per-axis curve editing, timeline view).

## Sources

[1]: https://helpx.adobe.com/after-effects/using/keyframe-interpolation.html "Adobe Help - Keyframe Interpolation in After Effects"
[2]: https://www.schoolofmotion.com/blog/after-effects-keyframe-types "School of Motion - After Effects Keyframe Types"
[3]: https://mtmograph.com/blogs/tools/how-to-use-the-graph-editor-in-after-effects "Mt. Mograph - How to Use the Graph Editor in After Effects"
[4]: https://www.provideocoalition.com/tool-tip-tuesday-for-adobe-premiere-pro-quickly-add-keyframes/ "ProVideo Coalition - Quickly Add Keyframes in Premiere Pro Timeline"
[5]: https://community.adobe.com/t5/premiere-pro-ideas/better-fine-control-for-manipulating-key-frame-rubber-bands/idi-p/14021597 "Adobe Community - Better fine control for manipulating key-frame rubber bands"
[6]: https://helpx.adobe.com/photoshop/using/curves-adjustment.html "Adobe Help - Photoshop Curves Adjustment"
[7]: https://jkost.com/blog/2019/10/working-with-the-curves-in-photoshop.html "Julieanne Kost - Working with Curves in Photoshop"
[8]: https://www.blenderbasecamp.com/how-does-interpolation-work-in-the-graph-editor/ "Blender Base Camp - Interpolation in the Graph Editor"
[9]: https://www.schoolofmotion.com/blog/graph-editor-in-cinema-4d "School of Motion - Graph Editor in Cinema 4D"
[10]: https://www.steakunderwater.com/VFXPedia/__man/Resolve18-6/DaVinciResolve18_Manual_files/part1805.htm "DaVinci Resolve Manual - Working with Keyframes and Splines"
[11]: https://www.theatrejs.com/docs/0.5/manual/sequences "Theatre.js - Working with Sequences"
[12]: https://www.theatrejs.com "Theatre.js - Animation Toolbox for the Web"
[13]: https://rive.app/docs/editor/animate-mode/interpolation-easing "Rive - Interpolation and Easing"
[14]: https://github.com/alexharri/animation-editor "alexharri/animation-editor - GitHub"
[15]: https://cubic-bezier.com/ "cubic-bezier.com - Lea Verou"
[16]: https://easings.net/ "Easing Functions Cheat Sheet"
[17]: https://facefx.github.io/documentation/doc/curve-editor "FaceFX Curve Editor Documentation"
[18]: https://link.springer.com/article/10.1007/s42979-021-00770-x "Conversion Between Cubic Bezier Curves and Catmull-Rom Splines"

## Research Log

---

### Search: "After Effects graph editor keyframe types bezier auto-bezier hold linear curve manipulation"

**Five keyframe interpolation types in After Effects:**

1. **Linear** -- diamond icon. Uniform rate of change, straight line in value graph. Mechanical-looking. Professional animators rarely use this on its own. ([School of Motion][2])
2. **Auto Bezier** -- circle icon. Automatically generates smooth curves; handle positions update automatically when you adjust keyframe values to maintain smooth transitions. Default spatial interpolation method. Convert from linear via Cmd/Ctrl+Click. ([Adobe Help][1], [School of Motion][2])
3. **Continuous Bezier** -- hourglass icon. Like Auto Bezier but with manually positioned handles. Direction handles change the curve on both sides of the keyframe, maintaining continuity (smooth join). Apply via Cmd+Shift+K or right-click Easy Ease. ([School of Motion][2])
4. **Bezier** -- diamond icon (same as linear visually). Full independent control of both handles on a keyframe. Handles operate independently in value graphs. Most precise control. ([Adobe Help][1])
5. **Hold** -- square icon. Temporal only. Property holds its value until the next keyframe, then jumps instantly. Useful for strobe/freeze effects. ([Adobe Help][1], [School of Motion][2])

**Two graph types in the Graph Editor:**

- **Value Graph**: Shows property values over time. Vertical axis = property value (pixels, degrees, percentage). Steep sections = large changes, flat = minimal change. A straight diagonal = constant speed. S-curve = ease in/out. ([Mt. Mograph][3])
- **Speed Graph**: Shows velocity over time. Flat line = no movement. Peaks = faster change. Can dip below zero for overshoot. ([Mt. Mograph][3])
- Toggle between them via a button at the bottom of the Graph Editor. Both visualize the same animation from different perspectives. ([Mt. Mograph][3])

**Handle manipulation conventions:**

- Hover over a keyframe to see Bezier handles. Handles have two properties: **direction** (angle -- up = quick change, down = gradual/reverse) and **length** (longer = more weight/influence on curve shape). ([Mt. Mograph][3])
- **Alt/Option-drag** splits incoming/outgoing handles so they operate independently (asymmetric curves). ([Adobe Help][1], [Mt. Mograph][3])
- **Shift-drag** snaps to horizontal/vertical angles. ([Mt. Mograph][3])
- **Ctrl/Cmd-drag** locks handle length while adjusting angle. ([Mt. Mograph][3])

**Key shortcuts:**

- **Shift+F3**: Open Graph Editor
- **F9**: Apply Easy Ease (both in and out)
- **Shift+F9**: Easy Ease In only (deceleration)
- **Ctrl/Cmd+Shift+F9**: Easy Ease Out only (acceleration)
- Cmd/Ctrl+Click: toggle between Linear and Auto Bezier in layer bar mode

**Spatial vs. Temporal interpolation**: AE distinguishes these. Spatial = motion path shape in the comp. Temporal = timing/speed along that path. For position, you can "Separate Dimensions" to edit X and Y curves independently -- this is analogous to OpenDolly's per-axis editing. ([Mt. Mograph][3])

**Key insight for OpenDolly**: AE's "Separate Dimensions" concept maps directly to our per-axis curve editors. Each axis gets its own value graph. The value graph is more intuitive than the speed graph for our use case (users think in terms of "where is the slider at time T" not "how fast is it moving"). But the speed graph could be useful for verifying that acceleration stays within hardware limits.

**Follow-up questions:**
- How does Premiere Pro handle keyframe curves differently (inline on clips vs. separate editor)?
- What about Photoshop curves interaction model?

---

### Search: "Premiere Pro rubber band keyframes inline clip timeline audio volume line drag bezier interpolation types"

**Premiere Pro's "rubber band" model -- inline keyframe editing directly on clips:**

- Premiere Pro shows a horizontal line (the "rubber band") overlaid on each clip in the timeline. For audio clips, this line represents volume level; for video, it can show opacity or other properties. ([ProVideo Coalition][4])
- **Adding keyframes**: Hold Cmd (Mac) / Ctrl (PC) and click on the rubber band line to add a keyframe at that point. The cursor changes to show an animation icon with a plus sign. ([ProVideo Coalition][4])
- **Bezier handles**: Once a keyframe exists, Cmd/Ctrl+click it again to convert it to a Bezier keyframe with handles, allowing curved (eased) transitions instead of linear. ([ProVideo Coalition][4])
- **Moving keyframes**: Drag keyframes up/down to change the value (e.g., volume level), or left/right to change the timing.
- The rubber band approach works for both audio and video keyframes across all parameter types. ([ProVideo Coalition][4])

**Effect Controls Panel -- the "other" keyframe editor:**

- Premiere also has a dedicated Effect Controls panel (similar to AE's approach) where you can see keyframes on a mini-timeline per property, with diamond keyframe icons.
- The Stopwatch icon enables keyframing for a property; moving the playhead and changing a value auto-creates keyframes.
- This is more like AE's layer-based approach, while the rubber band is Premiere-specific inline editing.

**Key difference from After Effects:**

- **AE**: Dedicated Graph Editor (value graph / speed graph) as a separate panel. Rich curve editing with full Bezier handles. Designed for animation where curves are the primary tool.
- **Premiere**: Inline "rubber band" editing on clips. Simpler -- you see the value overlaid on the clip itself. Less precise curve control but more contextual (you see the curve in the context of the clip content). The Effect Controls panel provides more detailed keyframe editing if needed.

**Key insight for OpenDolly**: The Premiere rubber band model is very close to what the vision doc describes for per-axis curve editing. A line overlaid on a channel that you click to add points and drag to reshape. The simplicity is appealing -- no separate "graph editor" mode, the curve IS the channel display. But Premiere's rubber band is limited to one curve per clip view. OpenDolly needs multiple per-axis curves visible simultaneously, which is more like AE's multi-property graph editor.

**Follow-up questions:**
- How does Photoshop's curves dialog work as an interaction model?
- What about web-based curve editors (Theatre.js, etc.)?

---

### Search: "Photoshop curves adjustment dialog interaction model click add points drag reshape UX design"

**Photoshop Curves dialog -- the interaction model the vision doc calls out:**

- **Graph layout**: Square grid with a diagonal baseline representing the identity (no change). Horizontal axis = input values, vertical axis = output values. The upper-right area = highlights, lower-left = shadows. Users can toggle between 10% and 25% grid increments. ([Adobe Help][6])
- **Adding points**: Click directly on the curve line to add a control point. Up to **14 control points** are permitted. An alternative: use the "On-image adjustment tool" to click on the actual image, which places a corresponding point on the curve. ([Adobe Help][6], [Julieanne Kost][7])
- **Dragging behavior**: Drag up/down to change output value. Drag left/right to shift along the input axis. The curve's steepness indicates contrast level; flatter = reduced contrast. ([Adobe Help][6])
- **Deleting points**: Three methods -- drag the point off the graph, select it and press Delete, or Cmd/Ctrl+click the point. ([Adobe Help][6], [Julieanne Kost][7])
- **Precise positioning**: Click a point and use arrow keys to nudge it. Shift+arrow for larger increments. Input/Output number fields show exact values for the selected point. ([Adobe Help][6])
- **Navigating between points**: Shift+"+" and Shift+"-" to move between points on the curve. Cmd/Ctrl+D to deselect all. ([Julieanne Kost][7])
- **Multi-select**: Shift+click to select multiple points for simultaneous manipulation. ([Julieanne Kost][7])
- **Pencil tool**: Switch to freehand mode to draw the curve directly (no control points). A "Smooth" button can then smooth the freehand drawing. ([Adobe Help][6], [Julieanne Kost][7])
- **Visual feedback**: Intersection lines appear when dragging a point, showing its position relative to the grid. ([Adobe Help][6])
- **Channel selection**: Dropdown to switch between composite (RGB) and individual color channels. Keyboard shortcuts: Option/Alt+2 (composite), Option/Alt+3-5 (R, G, B). ([Julieanne Kost][7])

**Key insight for OpenDolly**: The Photoshop Curves model is fundamentally different from AE/Premiere keyframe curves:

- **PS Curves** maps input-to-output (a transfer function). It's a static function, not time-based.
- **AE/Premiere curves** map time-to-value (a temporal curve). They're animations.
- For OpenDolly's per-axis curve editors, we're mapping **normalized time [0,1] to normalized progress [0,1]** -- which is actually closer to the PS Curves model (input/output mapping) than to AE's value graph (which maps time to actual property values).
- The PS interaction model (click to add points, drag to reshape, delete by dragging off) is directly applicable and simpler than AE's Bezier handle model.
- **14-point limit** is worth noting -- our vision doc says "you can add a whole bunch but not infinite." 14 is a reasonable upper bound.
- The pencil tool (freehand drawing) is an interesting option but likely overkill for OpenDolly.

**The hybrid model for OpenDolly**: Use the PS Curves click-to-add/drag-to-reshape interaction, but with AE-style per-axis channels displayed in a stacked timeline layout (like Premiere's inline rubber bands). This gives users the familiar curve manipulation of PS, the multi-axis visibility of AE, and the contextual inline display of Premiere.

**Follow-up questions:**
- How do Blender and other 3D apps handle curve editing?
- What web-based implementations exist (Theatre.js, Rive)?

---

### Search: "Blender graph editor f-curves keyframe editing interpolation modes handles UX design" + "DaVinci Resolve Fusion keyframe curve editor spline editor" + "Cinema 4D timeline F-curve editor"

**Blender's Graph Editor:**

- **Three core interpolation modes**: Bezier (default -- auto-eased, smooth acceleration/deceleration), Linear (constant speed, straight line), Constant (hold -- no change until next keyframe, staircase shape). ([Blender Base Camp][8])
- **Handle types** -- five varieties:
  - **Auto**: Automatically interpolated, smooth curves
  - **Auto Clamped**: Like auto but prevents overshoot (clamps values)
  - **Vector**: Maintains rotation when moved, straight tangent lines
  - **Aligned**: Handle maintains rotation when moved (continuous tangent)
  - **Free**: Independent handles, breaks tangent continuity
- **Easing presets**: Beyond basic interpolation, Blender includes Robert Penner easing equations (sinusoidal, quadratic, cubic, quartic, quintic). Also dynamic effects: Back (overshoot), Bounce (bouncing), Elastic (sine wave oscillation). ([Blender Base Camp][8])
- **Key shortcut**: Press **T** with keyframe selected to open interpolation mode menu. **V** for handle type menu.
- **Unique feature**: Built-in easing equations reduce need for manual curve shaping in common cases.

**Cinema 4D's Timeline / F-Curve Editor:**

- **Two modes in one panel**: Dope Sheet (keyframes as squares -- for retiming) and F-Curve mode (interpolation curves -- for shaping). Toggle with **Tab** key. ([School of Motion - C4D][9])
- **Shortcuts**: **Shift+F3** to open graph editor (same as AE), **H** to frame all keyframes, **1+drag** to pan, **2+drag** to zoom.
- **Key Mute**: Right-click a keyframe to mute it non-destructively -- useful for A/B testing animation variants. ([School of Motion - C4D][9])
- **Velocity overlay**: C4D lacks a dedicated speed graph like AE, but has a velocity overlay mode (F-Curve > Show Velocity) as a workaround. ([School of Motion - C4D][9])
- **Breakdown keys**: C4D's equivalent of AE's "roving keyframes" -- keyframes that maintain their relative position on the curve when other keyframes move. ([School of Motion - C4D][9])

**DaVinci Resolve Fusion Spline Editor:**

- **Spline panel**: Graphical keyframe editor showing parameter values over time as spline curves. X-axis = time, Y-axis = parameter value. ([DaVinci Resolve Manual][10])
- **Interpolation options**: Linear (fixed consistent rate), Smooth (auto-extends direction handles for gentle transitions), Custom (opens mini spline editor for full control).
- **Terminology**: Resolve calls them "splines" (not F-curves or value graphs). Keyframes are "control points" on the spline.
- Users can move, copy, and change interpolation of control points. The spline editor includes toolbar buttons for switching interpolation modes.

**Cross-tool patterns emerging:**

| Pattern | AE | Premiere | Blender | C4D | Resolve |
|---------|-----|----------|---------|-----|---------|
| Graph editor as separate panel | Yes | No (inline) | Yes | Yes | Yes |
| Value graph | Yes | N/A | Yes (F-curve) | Yes (F-curve) | Yes (spline) |
| Speed/velocity graph | Yes | No | Via overlay | Via overlay | No |
| Dope sheet mode | Yes | No | Yes | Yes | Yes |
| Bezier handles | Yes | Limited | Yes | Yes | Yes |
| Easing presets | Easy Ease | No | Penner equations | Limited | Smooth preset |
| Handle splitting | Alt-drag | N/A | Yes | Yes | Yes |

**Key insight for OpenDolly**: Every professional tool except Premiere uses a separate graph editor panel. Premiere's inline approach is simpler but less powerful. OpenDolly's vision doc describes something in between: per-axis curves displayed inline (like Premiere's rubber bands) but with the control depth of a graph editor (click to add points, drag to reshape). This hybrid is actually novel -- most tools make you choose between inline simplicity and separate-panel power.

Blender's **Auto Clamped** handles are worth considering -- they prevent overshoot, which matters for physical hardware (you don't want the slider to temporarily reverse direction just because of a curve overshoot).

**Follow-up questions:**
- Theatre.js and other web-based curve editors?
- CSS cubic-bezier editors as a simpler model?

---

### Search: "Theatre.js keyframe curve editor web browser animation timeline JavaScript" + "web based curve editor keyframe animation JavaScript canvas HTML open source library"

**Theatre.js -- the leading open-source web-based animation editor:**

- **Architecture**: Split into `@theatre/core` (runtime, plays animations) and `@theatre/studio` (visual editor GUI for creating animations). MIT licensed, fully open source. ([Theatre.js][12])
- **Sequence editor**: Horizontal timeline with multi-track keyframes. Add keyframes by inserting into the timeline; remove via right-click context menu. Left-click a keyframe to open an inline editor popup for value editing. ([Theatre.js Sequences][11])
- **Tween/curve editor**: Controls speed curves between keyframes. Users can fuzzy-search easing function names ("linear", "quad-in-out") or define custom cubic Bezier curves. ([Theatre.js Sequences][11])
- **Multi-track curve editor**: Accessible via an icon next to track names. Shows speed curves in colors matching track icons. Users edit curves by moving handles. ([Theatre.js Sequences][11])
- **Aggregate keyframes**: Compound properties automatically get parent keyframes that contain child keyframes. Moving an aggregate connector moves all children. Deleting an aggregate removes all children. ([Theatre.js Sequences][11])
- **Selection**: Shift+drag to create a selection rectangle for batch movement/deletion. Focus range feature (Shift+drag in top bar) isolates editing to a specific time range. ([Theatre.js Sequences][11])

**Rive -- browser-based animation editor with curve editing:**

- **Three interpolation types**: Linear (constant rate, default), Cubic (curve-based with two draggable handles, auto-eased S-curve by default), Hold (step/discrete). ([Rive Interpolation][13])
- **Curve editor**: Interpolation panel appears alongside the timeline when keys are selected. Shows a visual graph (x = time, y = value change). ([Rive Interpolation][13])
- **Manual easing input**: Users can type four comma-separated numbers (cubic-bezier control point coordinates, typically 0-1 range) for precise control and brand consistency across files. ([Rive Interpolation][13])
- **Graph editor**: Replaces standard timeline view. Shows property changes over time with editable cubic curves via handle manipulation. ([Rive Interpolation][13])
- **Handles can extend beyond graph bounds**, and the view auto-adjusts to keep them visible. ([Rive Interpolation][13])

**alexharri/animation-editor -- open-source TypeScript web animation editor:**

- Built with React, Redux, PIXI.js, HTML Canvas. 87% TypeScript codebase. ([animation-editor GitHub][14])
- Features a Graph Editor for fine-grained curve control, composition/layer system, pen tool, SVG import, and a Blender-inspired multi-window layout. ([animation-editor GitHub][14])
- Demonstrates that a full graph editor is feasible in a web browser using Canvas rendering.

**Other notable web-based tools:**

- **Motion Canvas**: TypeScript library for creating animated videos, web-based editor synced with audio.
- **Bezier Editor** (123hurray/bezier-editor): HTML5 Canvas tool for creating Photoshop-style bezier curves, exports as JS code for animation paths.
- **Mojs**: Open-source motion library with a curve/timeline motion editor.

**Key insight for OpenDolly**: Theatre.js is the closest existing web implementation to what we need. Its multi-track sequence editor with per-property curves is conceptually similar to OpenDolly's per-axis curve channels. Key differences: Theatre.js uses cubic-bezier easing between keyframes (like CSS transitions), while OpenDolly needs multi-point curves (like PS Curves). Theatre.js is designed for DOM/WebGL animation, not physical hardware control.

Rive's approach of allowing manual cubic-bezier input (four numbers) is worth adopting -- it enables precise sharing of "brand" easing values and is a good complement to the visual curve editor.

**Follow-up questions:**
- What are the universal UX patterns across all these tools?
- How do CSS cubic-bezier editors work as a simpler interaction model?

---

### Search: "Rive animation editor curve keyframe web browser editor timeline" + "cubic-bezier.com CSS easing editor interaction design"

**CSS cubic-bezier editors -- the simplest curve editing interaction:**

- **cubic-bezier.com** (Lea Verou): Visual editor for CSS `cubic-bezier()` functions. Two control handles on a unit square graph. Drag handles to reshape the curve. Duration parameter (default 1s) for real-time preview animation. Can compare curves side-by-side. Save/import/export with permalinks for sharing. ([cubic-bezier.com][15])
- **Interaction model**: Extremely simple -- just two handles to drag. The curve is defined by exactly 4 numbers (two control points). No adding/removing points. The simplicity is the feature. ([cubic-bezier.com][15])
- **Limitation**: Only a single cubic bezier segment. Cannot create multi-segment curves, delays, or complex shapes. Good enough for simple ease-in/out, not enough for "delay pan until 5 seconds in" scenarios.

**Standard easing function taxonomy (easings.net):**

- **30 standard easing functions** organized into 10 families, each with In/Out/InOut variants: ([easings.net][16])
  - **Sine, Quad, Cubic, Quart, Quint** -- polynomial easings of increasing aggressiveness
  - **Expo** -- exponential
  - **Circ** -- circular
  - **Back** -- overshoot
  - **Elastic** -- oscillating
  - **Bounce** -- bouncing
- Most can be expressed as cubic-bezier() values. Back, Elastic, and Bounce require custom functions.
- These are the Robert Penner easing equations used in Blender and many other tools.

**Key insight for OpenDolly**: The CSS cubic-bezier model (exactly 2 control points, 4 numbers) is too limited for our use case. We need multi-point curves to support scenarios like:
- Delay an axis start (flat line then ramp)
- Complex S-curves with multiple inflection points
- Different acceleration profiles for different phases of a move

But the cubic-bezier model should be a **building block** -- each segment between two control points on our multi-point curve could be a cubic bezier segment. This is how Photoshop and most spline editors work: piecewise cubic curves joined at control points.

---

### Search: "curve editor UX design patterns common conventions keyboard shortcuts snapping grid animation software"

**Universal UX conventions across professional curve editors:**

Based on analysis of FaceFX documentation and cross-referencing with AE, Blender, C4D, and Resolve: ([FaceFX Curve Editor][17])

**Point management:**
- **Add point**: Click/double-click on curve line, or Insert key + click, or Cmd/Ctrl+click. Nearly universal.
- **Select point**: Click individual points. Marquee/box select for multiple. Ctrl+click to toggle selection. Shift+click to add to selection. ([FaceFX Curve Editor][17], [Adobe Help][6])
- **Move point**: Click and drag. Some tools support constrained movement (horizontal-only or vertical-only via modifier key or middle+left mouse). ([FaceFX Curve Editor][17])
- **Delete point**: Delete key for selected points. Drag off graph (Photoshop convention). Right-click > Delete.
- **Arrow keys**: Nudge selected points. Shift+arrow for larger increments. ([Adobe Help][6])

**Tangent/handle conventions:**
- **Split handles**: Alt/Option+drag (AE, Blender). This is nearly universal.
- **Lock/unlock tangents**: Dedicated buttons or shortcuts.
- **Reset tangents**: Ctrl+D or similar shortcut.

**Navigation:**
- **Pan**: Middle mouse drag, or Space+drag, or number key+drag.
- **Zoom**: Scroll wheel (time axis). Shift+scroll (value axis). Pinch on trackpad.
- **Frame all**: H key (Blender, C4D) or Home key. "Fit to view" buttons.
- **Frame selection**: Similar shortcut, fits view to selected points only.

**Grid and snapping:**
- **Snap to grid**: Toggle buttons for time snap and value snap independently.
- **Grid density**: Adjustable or auto-scaling based on zoom level.

**Curve operations:**
- **Copy/paste**: Ctrl+C/V for keyframes and curve segments.
- **Interpolation switching**: Right-click or dedicated shortcut (T in Blender, context menu in AE).
- **Undo/redo**: Ctrl+Z / Ctrl+Shift+Z. Universal.

**Visual conventions:**
- **Selected points**: Highlighted color (usually yellow or white on dark background).
- **Handles**: Thin lines extending from points, with small circles or squares at the ends.
- **Curve color**: Different colors per channel/property (AE uses R/G/B for X/Y/Z; Theatre.js uses track-specific colors).
- **Grid lines**: Subtle, receding background. Time markers on horizontal axis, value markers on vertical.
- **Playhead**: Vertical line showing current time position, draggable.

**Key insight for OpenDolly**: The interaction conventions are remarkably consistent across tools. For our web-based curve editor, we should adopt:
1. Click on curve to add point (PS model, universal)
2. Drag to move points (universal)
3. Delete key or drag-off-graph to remove (PS model)
4. Arrow keys for nudging (PS model, universal)
5. Scroll wheel for zoom (universal)
6. Different colors per axis channel (AE/Theatre.js model)
7. Grid with auto-scaling density based on zoom
8. Undo/redo with Ctrl+Z/Shift+Ctrl+Z

For touch (phone): long-press to add point, drag to move, swipe off to delete. Pinch to zoom. This adapts the desktop conventions to mobile in a natural way.

---

### Search: "piecewise cubic bezier vs catmull-rom spline curve editor animation control points comparison"

**Math model considerations for OpenDolly's curve editor:**

- **Catmull-Rom splines** are interpolating -- the curve passes through all control points. This is the behavior Photoshop Curves uses and what OpenDolly needs (user places points, curve goes through them). The motion-math library already implements Catmull-Rom splines. ([Conversion paper][18])
- **Cubic Bezier curves** are approximating -- the curve does not pass through the intermediate control points, only the endpoints. This is what CSS `cubic-bezier()` and AE's Bezier handles use. ([Conversion paper][18])
- **Piecewise cubic Bezier** chains multiple bezier segments end-to-end. Each segment has 4 control points (2 endpoints + 2 handles). This is how AE, Blender, and most professional curve editors work internally.
- **Conversion is possible**: Catmull-Rom and cubic Bezier are mathematically convertible via linear transformation of control points. ([Conversion paper][18])
- **For OpenDolly's use case**: Since users will click to place points and expect the curve to pass through those points (PS Curves model), Catmull-Rom or a similar interpolating spline is the right choice for the user-facing interaction. Internally, it can be converted to piecewise bezier for rendering and evaluation if needed.

**Key insight for OpenDolly**: The existing `motion-math` library already has Catmull-Rom spline support. The curve editor should use an interpolating spline (Catmull-Rom or monotone cubic) so that user-placed control points are actual points on the curve, matching the Photoshop Curves mental model. Monotone cubic interpolation is worth considering because it guarantees the curve never overshoots between control points -- critical for physical hardware safety.

---
