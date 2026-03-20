# Gimbal Keyframe Interpolation and Smooth Path Generation - Research Background

**Date:** 2026-03-20
**Topic:** Comprehensive research on methods for setting keyframes in gimbal software and creating smooth paths between keyframes, covering interpolation algorithms, quaternion math, open-source implementations, commercial systems, and multi-axis synchronization.

## Sources

[1]: https://en.wikipedia.org/wiki/Slerp "Slerp - Wikipedia"
[2]: https://palospublishing.com/using-slerp-for-smooth-rotations/ "Using Slerp for Smooth Rotations - Palos Publishing"
[3]: https://theory.org/software/qfa/writeup/node12.html "Quaternion Interpolation - QFA Writeup"
[4]: https://splines.readthedocs.io/en/latest/rotation/squad.html "SQUAD - Splines Documentation"
[5]: https://www.sciencedirect.com/science/article/abs/pii/S0094114X21000999 "Practical algorithm for smooth interpolation between angular positions"
[6]: https://iquilezles.org/articles/minispline/ "Catmull-Rom Splines - Inigo Quilez"
[7]: https://en.wikipedia.org/wiki/Centripetal_Catmull%E2%80%93Rom_spline "Centripetal Catmull-Rom Spline - Wikipedia"
[8]: https://www.litchiutilities.com/docs/splines.php "Litchi Missions vs DJI Fly Missions - Splines"
[9]: https://forum.dji.com/thread-196261-1-1.html "No motion control waypoint smoothing - DJI Forum"
[10]: https://edelkrone.com/pages/edelkrone-system-wide-features-guide "edelkrone System-Wide Features Guide"
[11]: https://www.pmdcorp.com/resources/type/articles/get/mathematics-of-motion-control-profiles-article "Mathematics of Motion Control Profiles - PMD Corp"
[12]: https://www.zaber.com/articles/jerk-control "S-Curve Motion Profiles for Jerk Control - Zaber"
[13]: https://www.et.byu.edu/~ered/ME537/Notes/Ch5.pdf "S-Curve Equations for Trajectory Generator - BYU"
[14]: https://github.com/olliw42/storm32bgc "STorM32 BGC - GitHub"
[15]: https://github.com/EvvGC/Firmware "EvvGC Open Source Gimbal Controller - GitHub"
[16]: https://github.com/LivelyLiz/OptFlowCam "OptFlowCam - Blender smooth camera paths - GitHub"
[17]: https://github.com/maumatus/blenderDragon "blenderDragon - Blender to Dragonframe converter - GitHub"
[18]: https://github.com/yomotsu/camera-controls "camera-controls for three.js - GitHub"
[19]: https://github.com/nytimes/three-story-controls "three-story-controls - NYT - GitHub"
[20]: https://control.ros.org/humble/doc/ros2_controllers/joint_trajectory_controller/doc/trajectory.html "ROS2 Joint Trajectory Controller - Trajectory Representation"
[21]: https://index.ros.org/p/joint_trajectory_controller/ "joint_trajectory_controller - ROS Package"
[22]: https://easings.net/ "Easing Functions Cheat Sheet"
[23]: https://gist.github.com/gre/1650294 "Simple Easing Functions in Javascript - GitHub Gist"
[24]: https://developer.mozilla.org/en-US/docs/Web/CSS/easing-function "easing-function - CSS MDN"
[25]: https://notes.yvt.jp/Graphics/Easing-Functions/ "Easing Functions - yvt"
[26]: https://www.geometrictools.com/Documentation/MovingAlongCurveSpecifiedSpeed.pdf "Moving Along a Curve with Specified Speed - Geometric Tools"
[27]: https://www.dragonframe.com/dragonframe-software/ "Dragonframe Software Features"
[28]: https://en.wikipedia.org/wiki/Gimbal_lock "Gimbal Lock - Wikipedia"
[29]: https://news.ycombinator.com/item?id=27355270 "Gimbal Lock Discussion - Hacker News"
[30]: https://www.cined.com/rhino-arc-ii-4-axis-motorized-head-and-slider-for-automated-camera-movement/ "Rhino Arc II - CineD"
[31]: https://github.com/gre/bezier-easing "bezier-easing - GitHub"

<!-- Add new sources here as you find them. Number sequentially. -->

## Research Log

---

### Search: "SLERP quaternion interpolation camera gimbal keyframe smooth rotation"

**SLERP formula:** Slerp(q1, q2, t) = [sin((1-t)*theta)/sin(theta)] * q1 + [sin(t*theta)/sin(theta)] * q2, where theta is the angle between two quaternions and t ranges from 0.0 to 1.0 ([Using Slerp for Smooth Rotations][2])

**Key properties of SLERP for gimbal systems:**
- **Shortest path guaranteed** - always follows shortest rotation arc ([Using Slerp for Smooth Rotations][2])
- **Constant angular velocity** - even motion throughout transitions ([Using Slerp for Smooth Rotations][2])
- **Avoids gimbal lock** - operates on quaternions not Euler angles ([Using Slerp for Smooth Rotations][2])

**Implementation details:**
- Normalize both quaternions before interpolation
- If dot product negative, negate one quaternion (shortest path)
- Dot product > 0.9995: fall back to linear interpolation (numerical stability)
- Near-180-degree rotations need special handling
([Using Slerp for Smooth Rotations][2])

**SQUAD** recommended for multi-keyframe sequences needing C1 continuity ([Using Slerp for Smooth Rotations][2])

---

### Search: "SQUAD spherical quadrangle interpolation quaternion multiple keyframes C1 continuity camera path"

**SQUAD formula:**
squad(b0, S1, S2, b3, u) = slerp(slerp(b0, b3, 2u(1-u)), slerp(S1, S2, 2u(1-u)), 2u(1-u))
([Quaternion Interpolation - QFA][3])

**Auxiliary point calculation:**
a_i = q_i * exp(-(ln(q_i^-1 * q_{i+1}) + ln(q_i^-1 * q_{i-1})) / 4)
([Quaternion Interpolation - QFA][3])

**Algorithm:** Extract integer/fractional from t -> convert 4 adjacent rotations to quaternions -> calculate inner quadrangle points -> nested SLERPs -> convert back ([Quaternion Interpolation - QFA][3])

**Provides C1 continuity but NOT C2** ([SQUAD - Splines Documentation][4])

---

### Search: "Catmull-Rom spline camera path keyframe interpolation"

**Formula:** p(t) = 0.5 * ((2*p1) + (-p0+p2)*t + (2*p0-5*p1+4*p2-p3)*t^2 + (-p0+3*p1-3*p2+p3)*t^3)
([Catmull-Rom Splines - Inigo Quilez][6])

- Passes through all control points exactly
- C1 continuous, simple to compute (~100-150 bytes)
- Centripetal variant (alpha=0.5) avoids loops/cusps ([Centripetal Catmull-Rom Spline - Wikipedia][7])
- Three parameterizations: Uniform (0), Centripetal (0.5), Chordal (1)
- Endpoints need extra ghost points (duplicate or extrapolate)

---

### Search: "DJI Ronin motion path interpolation" and "edelkrone keyframe motion control"

- **DJI Fly uses centripetal Catmull-Rom splines** with endpoint mirroring ([Litchi vs DJI Fly Splines][8])
- **Litchi uses hybrid Bezier/B-spline** - does NOT pass through interior waypoints ([Litchi vs DJI Fly Splines][8])
- Early **DJI Ronin SC lacked easing/smoothing** - users complained of "jarring linear travel" ([DJI Forum - No smoothing][9])
- **edelkrone**: Keyposes for all axes, separate speeds per transition, up to 3-axis sync, no published algorithm ([edelkrone Features][10])

---

### Search: "minimum jerk trajectory S-curve acceleration profile motor control"

**Trapezoidal:** 3 phases, infinite jerk at transitions, causes vibration ([Mathematics of Motion Control Profiles][11])

**S-curve (7-phase):** Bounded jerk, dramatically less vibration. Phases: ramp-accel, const-accel, ramp-decel-to-zero, coast, ramp-decel, const-decel, ramp-to-zero ([Mathematics of Motion Control Profiles][11])

| Aspect | Trapezoidal | S-Curve |
|--------|-------------|---------|
| Order | 2nd | 3rd |
| Phases | 3 | 7 |
| Jerk | Infinite | Bounded |
| Vibration | Higher | Much lower |

"Only a small amount of S can substantially reduce vibration" ([Mathematics of Motion Control Profiles][11])
Tuned S-curve can reduce transfer time by **25%+** ([Mathematics of Motion Control Profiles][11])

---

### Search: "github open source gimbal controller keyframe" and "github camera motion control"

Open-source gimbal FW focuses on **stabilization not keyframe paths**: STorM32 ([storm32bgc][14]), EvvGC ([EvvGC Firmware][15]). SimpleBGC is commercial.

Camera path projects: OptFlowCam ([OptFlowCam][16]) - Blender add-on with Bezier/splines; blenderDragon ([blenderDragon][17]) - Blender to Dragonframe; camera-controls ([camera-controls][18]) - three.js; three-story-controls ([three-story-controls][19]) - NYT three.js.

---

### Search: "ROS trajectory planning pan tilt gimbal joint trajectory multi-axis synchronization"

**ROS2 joint_trajectory_controller** three interpolation levels ([ROS2 Trajectory Representation][20]):
- **Linear spline** (position only): C0, discontinuous velocities - discouraged
- **Cubic spline** (pos + vel): C1, velocity-continuous
- **Quintic spline** (pos + vel + accel): C2, acceleration-continuous - best for smooth motion

**Multi-joint sync:** `JointTrajectory` message specifies all joints at each time stamp ([ROS2 Trajectory Representation][20])
**Trajectory splicing:** New trajectories merge smoothly with current ([ROS2 Trajectory Representation][20])
**Integration:** Heun's method can derive missing pos/vel/accel values ([ROS2 Trajectory Representation][20])

---

### Search: "easing functions mathematical definition ease-in ease-out cubic bezier timing curve"

**All easing functions map t in [0,1] to output in [0,1]** where f(0)=0 and f(1)=1.

**Polynomial easing functions** ([Simple Easing Functions JS][23]):

| Function | Ease-In | Ease-Out | Ease-In-Out |
|----------|---------|----------|-------------|
| Quadratic | t^2 | t*(2-t) | t<0.5 ? 2t^2 : -1+(4-2t)*t |
| Cubic | t^3 | (t-1)^3+1 | t<0.5 ? 4t^3 : (t-1)(2t-2)^2+1 |
| Quartic | t^4 | 1-(t-1)^4 | t<0.5 ? 8t^4 : 1-8(t-1)^4 |
| Quintic | t^5 | 1+(t-1)^5 | t<0.5 ? 16t^5 : 1+16(t-1)^5 |

**General pattern:** Ease-in = t^n; Ease-out = 1-(1-t)^n; Ease-in-out = piecewise combination ([Simple Easing Functions JS][23])

**CSS cubic-bezier easing** ([easing-function - MDN][24]):
- ease: cubic-bezier(0.25, 0.1, 0.25, 1.0)
- ease-in: cubic-bezier(0.42, 0, 1.0, 1.0)
- ease-out: cubic-bezier(0, 0, 0.58, 1.0)
- ease-in-out: cubic-bezier(0.42, 0, 0.58, 1.0)

**Sinusoidal easing:** easeInSine = -cos(t * pi/2) + 1 ([Easing Functions - yvt][25])

**Application to gimbal motion:** Easing functions apply **timing remapping** - feed easing(t) into spline instead of linear t. Changes speed profile without altering geometric path.

---

### Search: "Dragonframe motion control keyframe" and "Freefly MoVI controller keyframe"

**Dragonframe** uses **Bezier spline keyframe interface** for motion control ([Dragonframe Software][27]):
- Multi-axis bezier spline keyframe system integrated since Dragonframe 3.0
- Simplified "bezier handle" per axis to adjust smoothness
- Users mark positions then adjust keyframes to define paths
- Supports 4+ axes via controllers like eMotimo spectrum ST4 (pan, tilt, slider, focus)
- Primarily for stop-motion animation where camera moves frame-by-frame

**Freefly MoVI Controller:** Primarily a **real-time stabilization and manual control** system, not a keyframe motion path system. The MoVI Controller provides pan/tilt/roll control for MoVI gimbals but no apparent keyframe waypoint interpolation features were found in public documentation. The Freefly ecosystem is oriented toward live camera operation rather than pre-programmed paths.

**Rhino Arc II** ([Rhino Arc II - CineD][30]):
- Supports up to **5 keyframes** via the Rhino Arc app
- 4-axis motion control (pan, tilt, slide, focus)
- Keyframes played back at adjustable speeds
- Integrates with Rhino sliders for synchronized multi-axis movement
- **No published technical details on interpolation algorithm**

---

### Search: "arc length parameterization constant speed spline curve traversal"

**The problem:** Standard parametric curves (Bezier, Catmull-Rom) do NOT traverse at constant speed when t increases linearly. Speed varies with curvature. ([Moving Along a Curve - Geometric Tools][26])

**Arc-length function:** s(t) = integral from 0 to t of ||C'(u)|| du, where ||C'(u)|| is the speed at parameter u ([Moving Along a Curve - Geometric Tools][26])

**To achieve constant-speed traversal:** Must invert s(t) to find t(s) - the parameter value that corresponds to a desired distance along the curve. This inversion generally requires numerical methods. ([Moving Along a Curve - Geometric Tools][26])

**Two numerical approaches:**
1. **Bisection method:** Build lookup table of (t, s) pairs, then binary search for desired s value. Robust but slower. ([Moving Along a Curve - Geometric Tools][26])
2. **Newton's method:** Use derivative information for faster convergence when inverting arc-length function. ([Moving Along a Curve - Geometric Tools][26])

**Variable-speed traversal:** Instead of constant speed, define a custom speed function sigma(t) and reparameterize accordingly. This enables acceleration/deceleration profiles along the curve. ([Moving Along a Curve - Geometric Tools][26])

**Practical implementation for gimbal systems:**
1. Pre-compute arc-length lookup table at initialization (sample many t values, compute cumulative arc length)
2. At runtime, given desired distance, binary search the table for corresponding t
3. Interpolate between table entries for sub-sample accuracy
4. Apply easing function to the distance parameter for acceleration/deceleration

---

### Search: "gimbal lock problem euler angles quaternion representation"

**Gimbal lock defined:** Loss of one degree of freedom when two gimbal axes align in parallel. With Euler angles, rotating 90 degrees in one axis can cause two other axes to become equivalent, leaving only 2 DOF instead of 3. ([Gimbal Lock - Wikipedia][28])

**The topological root cause:** Rotations form the space SO(3), while three Euler angles form a torus T^3. These are topologically distinct - no continuous mapping from T^3 to SO(3) can exist without singularities. Gimbal lock is therefore **mathematically inevitable** with any 3-parameter representation. ([Gimbal Lock Discussion - HN][29])

**Nuanced view on quaternions:** Common claim that "quaternions prevent gimbal lock" is **partially misleading**. The actual fix is not the representation (quaternion vs matrix vs Euler) but the **method of accumulation**. Gimbal lock occurs when you store orientation as three fixed angles and compose rotations by modifying those angles independently. It can be avoided with ANY representation if you accumulate rotations multiplicatively (q_new = q_delta * q_current). Quaternions make this natural and numerically stable. ([Gimbal Lock Discussion - HN][29])

**Quaternion advantages for gimbal systems:**
- 4 parameters to represent 3 DOF - no singularities on the unit sphere S^3
- Easy to compose: multiplication instead of matrix building from angles
- Easy to normalize (just normalize 4-vector)
- SLERP provides natural interpolation
- But: less intuitive for operators who think in pan/tilt/roll degrees

**Euler-to-quaternion conversion** (ZYX convention common for gimbals):
Given yaw(psi), pitch(theta), roll(phi):
- w = cos(phi/2)*cos(theta/2)*cos(psi/2) + sin(phi/2)*sin(theta/2)*sin(psi/2)
- x = sin(phi/2)*cos(theta/2)*cos(psi/2) - cos(phi/2)*sin(theta/2)*sin(psi/2)
- y = cos(phi/2)*sin(theta/2)*cos(psi/2) + sin(phi/2)*cos(theta/2)*sin(psi/2)
- z = cos(phi/2)*cos(theta/2)*sin(psi/2) - sin(phi/2)*sin(theta/2)*cos(psi/2)

---

<!-- Continue appending search entries below. One entry per search, every time. -->
