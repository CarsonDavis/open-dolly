# Gimbal Keyframe Interpolation and Smooth Path Generation

**Date:** 2026-03-20

## Executive Summary

Creating smooth camera motion between keyframes in a gimbal/slider system requires solving three distinct problems simultaneously: (1) interpolating rotational orientation (pan/tilt/roll), (2) interpolating linear position (slider axis), and (3) controlling the timing and velocity profile of the motion along those paths. The recommended approach for most implementations is to use **Catmull-Rom splines** (centripetal variant) for positional axes and **SQUAD interpolation** (built on quaternion SLERP) for rotational axes, with **S-curve acceleration profiles** or **easing functions** governing the speed of traversal. This combination provides C1-continuous paths that pass through every keyframe exactly, avoid gimbal lock, and produce physically smooth motion suitable for stepper/servo motor systems.

---

## 1. Rotation Representation: Euler Angles vs Quaternions

### The Gimbal Lock Problem

When orientation is stored as three Euler angles (yaw, pitch, roll), a well-known singularity called **gimbal lock** can occur. If the pitch axis reaches 90 degrees, the yaw and roll axes become parallel, collapsing 3 degrees of freedom into 2. This is not merely a software bug -- it is a **topological inevitability** of mapping the 3D rotation space SO(3) with only 3 parameters.

For a physical camera gimbal, this means that certain orientations become unreachable or cause erratic behavior when interpolating between keyframes near the singularity.

### Quaternion Representation

A unit quaternion q = (w, x, y, z) where w^2 + x^2 + y^2 + z^2 = 1 represents a rotation as a point on the 4D unit sphere S^3. This 4-parameter representation has **no singularities** anywhere in the rotation space.

**Euler-to-quaternion conversion** (ZYX convention, typical for pan/tilt/roll gimbals):

```
Given yaw(psi), pitch(theta), roll(phi):
w = cos(phi/2)*cos(theta/2)*cos(psi/2) + sin(phi/2)*sin(theta/2)*sin(psi/2)
x = sin(phi/2)*cos(theta/2)*cos(psi/2) - cos(phi/2)*sin(theta/2)*sin(psi/2)
y = cos(phi/2)*sin(theta/2)*cos(psi/2) + sin(phi/2)*cos(theta/2)*sin(psi/2)
z = cos(phi/2)*cos(theta/2)*sin(psi/2) - sin(phi/2)*sin(theta/2)*cos(psi/2)
```

**Important nuance:** The claim that "quaternions prevent gimbal lock" is partially misleading. The actual fix is the **method of accumulation** -- storing orientation as a quaternion and composing rotations multiplicatively (q_new = q_delta * q_current) rather than storing three independent angle values. Quaternions make this natural and numerically stable, but the same principle applies to rotation matrices.

**Practical tradeoff:** Operators think in degrees of pan/tilt/roll. The UI should present Euler angles for keyframe entry, but the interpolation engine should convert to quaternions internally, interpolate, then convert back for motor commands.

---

## 2. Interpolation Methods for Rotation (2 Keyframes)

### SLERP - Spherical Linear Interpolation

The foundational algorithm for interpolating between two orientations:

```
Slerp(q1, q2, t) = [sin((1-t)*theta) / sin(theta)] * q1 + [sin(t*theta) / sin(theta)] * q2
```

where theta is the angle between the two quaternions (computed via dot product) and t ranges from 0 to 1.

**Properties:**
- Follows the shortest rotation arc on the unit sphere
- Constant angular velocity throughout the interpolation
- No gimbal lock issues

**Implementation edge cases:**
- If dot(q1, q2) < 0, negate one quaternion to ensure shortest-path interpolation
- If dot(q1, q2) > 0.9995, fall back to normalized linear interpolation (NLERP) to avoid division-by-near-zero
- Near-180-degree rotations require special care (the "shortest path" becomes ambiguous)

**Limitation:** SLERP only handles two keyframes. Chaining SLERP segments between consecutive keyframe pairs produces **velocity discontinuities** at the joints -- the camera would appear to momentarily stop or jerk at each keyframe.

---

## 3. Interpolation Methods for Rotation (3+ Keyframes)

### SQUAD - Spherical Quadrangle Interpolation

SQUAD extends SLERP to produce **C1-continuous** rotation paths through multiple keyframes. It uses nested SLERP operations with auxiliary control quaternions:

```
squad(q_i, s_i, s_{i+1}, q_{i+1}, u) =
    slerp(slerp(q_i, q_{i+1}, u), slerp(s_i, s_{i+1}, u), 2u(1-u))
```

where u is the local interpolation parameter in [0,1] for the segment between keyframes i and i+1.

**Auxiliary control quaternion computation:**

```
s_i = q_i * exp(-(ln(q_i^{-1} * q_{i+1}) + ln(q_i^{-1} * q_{i-1})) / 4)
```

This computation averages the "incoming" and "outgoing" rotation rates at each keyframe, analogous to how Catmull-Rom tangents average neighboring point differences.

**Properties:**
- C1 continuous: angular velocity is continuous at keyframe boundaries
- NOT C2 continuous: angular acceleration can still have discontinuities
- Passes through every keyframe exactly
- More complex than SLERP but still computationally tractable

**When to use SQUAD vs SLERP:**
- 2 keyframes: SLERP is sufficient
- 3+ keyframes: SQUAD provides smooth transitions; chained SLERP will jerk

---

## 4. Interpolation Methods for Position (Slider/Linear Axes)

### Catmull-Rom Splines (Recommended)

For linear position axes (slider travel, dolly position, focus motor), Catmull-Rom splines are the standard choice. Given four consecutive keyframe positions p0, p1, p2, p3, the curve between p1 and p2 is:

```
p(t) = 0.5 * ((2*p1) + (-p0 + p2)*t + (2*p0 - 5*p1 + 4*p2 - p3)*t^2
       + (-p0 + 3*p1 - 3*p2 + p3)*t^3)
```

**Advantages:**
- Passes through every control point exactly (unlike B-splines)
- C1 continuous (velocity continuity at keyframe boundaries)
- Computationally simple -- approximately 100-150 bytes of compiled code
- Tangent at each point is automatically derived from neighboring points

**Centripetal parameterization (alpha = 0.5)** is strongly recommended over uniform parameterization. It prevents loops and cusps that can occur with unevenly spaced keyframes -- critical for physical systems where the camera cannot teleport through impossible paths.

**Endpoint handling:** Catmull-Rom requires one point before the first keyframe and one after the last. Two approaches:
1. Duplicate the first/last keyframe position
2. Extrapolate using the tangent from the nearest two points

### Cubic Hermite Splines

A more general formulation where tangent vectors are explicitly specified at each keyframe rather than derived from neighbors. This gives more artistic control but requires the user (or algorithm) to define tangents.

Catmull-Rom is actually a special case of cubic Hermite where the tangents are computed as (p_{i+1} - p_{i-1}) / 2.

### B-Splines and NURBS

These do NOT pass through control points -- the curve is "attracted" to control points but generally does not touch them. This makes them less intuitive for cinematography (the operator expects the camera to actually reach the position they set) but they offer C2 continuity and guaranteed smoothness.

---

## 5. Comparison of Interpolation Methods

| Method | Continuity | Passes Through Points | Complexity | Best For |
|--------|-----------|----------------------|------------|----------|
| Linear (LERP) | C0 only | Yes | Trivial | Simple 2-point moves |
| SLERP | C0 between segments | Yes | Low | 2-keyframe rotation |
| SQUAD | C1 | Yes | Medium | Multi-keyframe rotation |
| Catmull-Rom | C1 | Yes | Low | Position interpolation |
| Cubic Hermite | C1 | Yes | Low | Position with manual tangents |
| B-spline | C2 | No | Medium | Smooth paths (approximation OK) |
| Quintic spline | C2 | Yes | High | Maximum smoothness |

---

## 6. Velocity and Timing Control

### The Two-Layer Architecture

Smooth gimbal motion separates **path shape** from **traversal speed**:

1. **Geometric layer:** Spline/SQUAD defines the shape of the path through space
2. **Timing layer:** An easing function or motion profile controls how fast the system moves along that path

This separation means you can define a beautiful curved path and independently choose whether the camera accelerates into the move, travels at constant speed, or decelerates at the end.

### Easing Functions

All easing functions map a normalized time t in [0,1] to an output progress value in [0,1], satisfying f(0)=0 and f(1)=1.

**Polynomial easing (Robert Penner formulas):**

| Type | Ease-In | Ease-Out | Ease-In-Out |
|------|---------|----------|-------------|
| Quadratic | t^2 | t(2-t) | piecewise |
| Cubic | t^3 | (t-1)^3 + 1 | piecewise |
| Quartic | t^4 | 1 - (t-1)^4 | piecewise |
| Quintic | t^5 | 1 + (t-1)^5 | piecewise |

**General pattern:** ease-in(t) = t^n, ease-out(t) = 1 - (1-t)^n

**Sinusoidal easing:** easeInSine(t) = 1 - cos(t * pi / 2)

**Cubic Bezier easing** (CSS-style, most flexible):
- ease-in: cubic-bezier(0.42, 0, 1.0, 1.0)
- ease-out: cubic-bezier(0, 0, 0.58, 1.0)
- ease-in-out: cubic-bezier(0.42, 0, 0.58, 1.0)

For gimbal/slider systems, **cubic or quintic ease-in-out** is most appropriate -- it provides gentle acceleration and deceleration that real motors can follow smoothly.

### Arc-Length Parameterization

Standard spline parameterization does NOT produce constant speed. The parameter t and actual distance along the curve are not linearly related -- the camera speeds up on straight sections and slows on tight curves.

To achieve constant speed (or any desired speed profile), the curve must be **reparameterized by arc length**:

1. Compute arc-length function: s(t) = integral of ||C'(u)|| du from 0 to t
2. Build a lookup table of (t, s) pairs at many sample points
3. At runtime, to find the parameter t for a desired distance s, binary search the table
4. Interpolate between table entries for sub-sample accuracy

This is computationally feasible even on embedded systems if the lookup table is built at path planning time, not during motion execution.

### Motor-Level Velocity Profiles

At the motor driver level, the most common profiles are:

**Trapezoidal velocity profile (3-phase):**
- Phase 1: Constant acceleration to target velocity
- Phase 2: Constant velocity (coast)
- Phase 3: Constant deceleration to zero
- Problem: Infinite jerk at phase boundaries causes vibration

**S-curve velocity profile (7-phase):**
- Phases 1-3: Smooth ramp-up (increasing acceleration, constant acceleration, decreasing acceleration)
- Phase 4: Constant velocity
- Phases 5-7: Mirror of 1-3 for deceleration
- Advantage: Bounded jerk dramatically reduces vibration

Key findings on S-curves:
- Even a small amount of S-curve rounding "can substantially reduce induced vibration"
- A tuned S-curve can reduce effective transfer time by 25%+ compared to trapezoidal, because lower vibration allows higher peak speeds
- For cinematography gimbals, S-curves are essential -- trapezoidal profiles produce visible camera shake at phase transitions

**Minimum-jerk trajectory** (5th-order polynomial):
- Position is a 5th-degree polynomial of time
- Zero velocity, acceleration, AND jerk at both start and end
- Produces the smoothest possible point-to-point motion
- Computationally more expensive but provides the gold standard for smooth mechanical motion

---

## 7. Multi-Axis Synchronization

### The Synchronization Problem

A typical cinematography rig has 4+ axes: pan, tilt, roll, and slide. Each axis may need to travel a different distance between keyframes. The challenge is ensuring all axes start together, move proportionally, and arrive at the next keyframe simultaneously.

### Time-Based Synchronization (Standard Approach)

The most common approach, used by ROS2 and most commercial systems:

1. Define keyframes as complete system states: {time, pan, tilt, roll, slide, focus, ...}
2. Each waypoint specifies positions for ALL axes at a given timestamp
3. Interpolate each axis independently using the same time parameter
4. The time parameter naturally synchronizes all axes

This is the approach used by the ROS2 `joint_trajectory_controller`, which accepts `JointTrajectory` messages specifying all joint positions (and optionally velocities/accelerations) at each time point.

### Scaling to Slowest Axis

When using velocity-limited profiles (trapezoidal or S-curve), a practical approach:

1. For each axis, compute the minimum time needed to travel the required distance given max velocity and acceleration limits
2. Set the segment duration to the longest of these times
3. Scale all other axes' profiles to match this duration
4. Result: all axes reach the target simultaneously, no axis exceeds its limits

### ROS2 Trajectory Controller Interpolation Levels

The ROS2 joint_trajectory_controller demonstrates a well-tested approach:

- **Linear (position only):** C0 continuity. Velocities discontinuous at waypoints. Discouraged for smooth motion.
- **Cubic spline (position + velocity):** C1 continuity. Smooth velocity. Good default.
- **Quintic spline (position + velocity + acceleration):** C2 continuity. Smooth acceleration. Best for vibration-sensitive systems.

Each level requires providing more data at each waypoint. Missing derivatives can be computed via Heun's integration method.

---

## 8. Commercial System Survey

### DJI Ronin / DJI Fly

- DJI Fly's waypoint system uses **centripetal Catmull-Rom splines** for path generation, with endpoint mirroring
- Early Ronin SC firmware used simple **linear interpolation** between keyframes with no smoothing -- users reported "jarring linear travel between key-framed positions"
- Supports up to 10 waypoints per path
- Duration parameter controls time between waypoints
- "Smoothing" value affects joystick response but historically did not affect waypoint path interpolation

### edelkrone

- "Keyposes" as keyframes across all axes (pan, tilt, slide, focus)
- Separate transition speeds settable between each pose pair
- Supports back-and-forth looping
- Up to 3-axis synchronized motion (HeadPLUS for pan+tilt, wirelessly paired with slider/dolly)
- No public documentation on internal interpolation algorithm

### Dragonframe

- Professional stop-motion software with integrated motion control
- Uses **Bezier spline keyframe interface** since version 3.0
- Simplified per-axis "bezier handle" for adjusting smoothness
- Supports 4+ axes via controllers like eMotimo spectrum ST4
- Designed for frame-by-frame motion where the camera moves to exact positions

### Rhino Arc II

- Up to **5 keyframes** via the Rhino Arc app
- 4-axis motion control (pan, tilt, slide, focus)
- Adjustable playback speeds
- No published technical details on interpolation algorithm

### Freefly MoVI

- Primarily a **real-time stabilization and manual control** system
- No apparent keyframe waypoint interpolation features in public documentation
- Oriented toward live camera operation rather than pre-programmed paths

---

## 9. Open-Source Implementations and Resources

### Gimbal Controller Firmware

Open-source gimbal controller projects focus almost entirely on **real-time stabilization** (PID control loops) rather than keyframe motion paths:

- **STorM32 BGC** (github.com/olliw42/storm32bgc): 3-axis brushless gimbal controller on STM32, open hardware. Extensive documentation at olliw.eu wiki.
- **EvvGC** (github.com/EvvGC/Firmware): Open-source 3-axis gimbal controller firmware.
- **SimpleBGC** (Basecam Electronics): Commercial, not open source. Communicates via MAVLink or custom serial protocol.

Keyframe motion path features are typically implemented in **companion apps or higher-level control software**, not in the gimbal firmware itself.

### Camera Path Planning Software

- **OptFlowCam** (github.com/LivelyLiz/OptFlowCam): Blender add-on for smooth camera paths using 3DImageFlow metric, Bezier curves, and spline variants (centripetal, chordal, uniform parameterizations)
- **blenderDragon** (github.com/maumatus/blenderDragon): Converts Blender camera keyframe data to Dragonframe motion control format for Emotimo TB3 robot
- **camera-controls** (github.com/yomotsu/camera-controls): Three.js camera control with smooth state transitions
- **three-story-controls** (github.com/nytimes/three-story-controls): NYT's three.js toolkit for interactive camera stories
- **bezier-easing** (github.com/gre/bezier-easing): JavaScript cubic-bezier easing implementation

### Robotics Frameworks

- **ROS2 joint_trajectory_controller**: Production-tested multi-joint trajectory controller with linear/cubic/quintic spline interpolation and multi-axis synchronization
- **MoveIt**: ROS motion planning framework with jerk-limited trajectory generation using optimized S-curve algorithms

---

## 10. Recommended Architecture for a Gimbal Keyframe System

Based on this research, a well-designed keyframe system for a camera gimbal/slider should have the following layers:

### Layer 1: Keyframe Storage
- Store each keyframe as: {time, pan_degrees, tilt_degrees, roll_degrees, slide_mm, focus_position, ...}
- Operator enters values in intuitive units (degrees, millimeters)
- Minimum 2 keyframes, ideally support 10+

### Layer 2: Path Computation (done once when path is defined/edited)
- Convert pan/tilt/roll Euler angles to quaternions
- Compute SQUAD auxiliary quaternions for rotation axes
- Compute Catmull-Rom spline coefficients for position axes (slider, focus)
- Use centripetal parameterization for Catmull-Rom
- Build arc-length lookup table if constant-speed mode is needed

### Layer 3: Timing/Easing (configurable per segment)
- Apply easing function to time parameter before feeding into interpolation
- Default: cubic ease-in-out for natural-feeling motion
- Options: linear (constant speed), ease-in only, ease-out only, custom cubic-bezier

### Layer 4: Motor Command Generation (real-time loop)
- At each control cycle, compute current time parameter
- Apply easing to get progress parameter
- Evaluate SQUAD for rotation, Catmull-Rom for position
- Convert quaternion back to Euler angles for motor commands
- Apply S-curve or trapezoidal velocity limiting at motor driver level as safety bounds
- Send synchronized commands to all axes simultaneously

### Layer 5: Motor Driver (hardware level)
- Receive position/velocity targets from Layer 4
- Apply S-curve velocity profile for smooth motor motion
- PID or stepper pulse control for accurate positioning
- Emergency stop and limit switch handling

---

## 11. Key Mathematical Reference

### SLERP
```
Slerp(q1, q2, t) = sin((1-t)*theta)/sin(theta) * q1 + sin(t*theta)/sin(theta) * q2
theta = acos(dot(q1, q2))
```

### SQUAD
```
squad(q_i, s_i, s_{i+1}, q_{i+1}, u) =
    slerp(slerp(q_i, q_{i+1}, u), slerp(s_i, s_{i+1}, u), 2u(1-u))

s_i = q_i * exp(-(log(q_i^{-1} * q_{i+1}) + log(q_i^{-1} * q_{i-1})) / 4)
```

### Catmull-Rom (centripetal)
```
p(t) = 0.5 * ((2*p1) + (-p0+p2)*t + (2*p0-5*p1+4*p2-p3)*t^2 + (-p0+3*p1-3*p2+p3)*t^3)
```
(For uniform parameterization. Centripetal requires adjusting knot spacing by distance^0.5)

### Cubic Ease-In-Out
```
f(t) = t < 0.5 ? 4*t^3 : (t-1)*(2*t-2)^2 + 1
```

### S-Curve Velocity Profile (7-phase)
```
Phase 1: v(t) = J_max * t^2 / 2                    (jerk = +J_max)
Phase 2: v(t) = v1 + a_max * (t - t1)               (jerk = 0)
Phase 3: v(t) = v2 + a_max*(t-t2) - J_max*(t-t2)^2/2  (jerk = -J_max)
Phase 4: v(t) = v_max                                (coast)
Phases 5-7: mirror of 1-3 for deceleration
```

---

## Sources

- [Slerp - Wikipedia](https://en.wikipedia.org/wiki/Slerp)
- [Using Slerp for Smooth Rotations - Palos Publishing](https://palospublishing.com/using-slerp-for-smooth-rotations/)
- [Quaternion Interpolation - QFA Writeup](https://theory.org/software/qfa/writeup/node12.html)
- [SQUAD - Splines Documentation](https://splines.readthedocs.io/en/latest/rotation/squad.html)
- [Catmull-Rom Splines - Inigo Quilez](https://iquilezles.org/articles/minispline/)
- [Centripetal Catmull-Rom Spline - Wikipedia](https://en.wikipedia.org/wiki/Centripetal_Catmull%E2%80%93Rom_spline)
- [Litchi Missions vs DJI Fly Missions - Splines](https://www.litchiutilities.com/docs/splines.php)
- [DJI Forum - No motion control waypoint smoothing](https://forum.dji.com/thread-196261-1-1.html)
- [Mathematics of Motion Control Profiles - PMD Corp](https://www.pmdcorp.com/resources/type/articles/get/mathematics-of-motion-control-profiles-article)
- [Moving Along a Curve with Specified Speed - Geometric Tools](https://www.geometrictools.com/Documentation/MovingAlongCurveSpecifiedSpeed.pdf)
- [ROS2 Joint Trajectory Controller - Trajectory Representation](https://control.ros.org/humble/doc/ros2_controllers/joint_trajectory_controller/doc/trajectory.html)
- [Simple Easing Functions in Javascript - GitHub Gist](https://gist.github.com/gre/1650294)
- [easing-function - CSS MDN](https://developer.mozilla.org/en-US/docs/Web/CSS/easing-function)
- [Easing Functions - yvt](https://notes.yvt.jp/Graphics/Easing-Functions/)
- [Dragonframe Software Features](https://www.dragonframe.com/dragonframe-software/)
- [Gimbal Lock - Wikipedia](https://en.wikipedia.org/wiki/Gimbal_lock)
- [Gimbal Lock Discussion - Hacker News](https://news.ycombinator.com/item?id=27355270)
- [STorM32 BGC - GitHub](https://github.com/olliw42/storm32bgc)
- [EvvGC Open Source Gimbal Controller - GitHub](https://github.com/EvvGC/Firmware)
- [OptFlowCam - Blender smooth camera paths - GitHub](https://github.com/LivelyLiz/OptFlowCam)
- [blenderDragon - Blender to Dragonframe converter - GitHub](https://github.com/maumatus/blenderDragon)
- [camera-controls for three.js - GitHub](https://github.com/yomotsu/camera-controls)
- [bezier-easing - GitHub](https://github.com/gre/bezier-easing)
