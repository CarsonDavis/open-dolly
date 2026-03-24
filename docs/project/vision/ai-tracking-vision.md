# AI Object Tracking & Camera Control Vision

*Captured: 2026-03-23*

Status: **active**

---

## Motivation

The DJI RS gimbal has built-in AI tracking, but it uses the gimbal's own wide-angle camera — not the actual shooting camera. This creates real problems: at longer focal lengths there's significant parallax error between what the tracking camera sees and what the shooting lens frames. Users report consistent issues with DJI ActiveTrack on anything beyond a 35mm. Since we're designing around lenses like the Sigma 28-105mm f/2.8 and 100mm macro, relying on the gimbal's built-in tracking isn't viable.

Beyond the parallax problem, DJI's tracking is a black box and it may not actually be state of the art, since it has to run on limited hardware. For product photography, where you need to orbit around a random object and keep it centered, DJI's built-in tracking might simply not cut it.

The solution: take the image directly from the shooting camera, run our own detection and tracking, and drive the gimbal from that. This eliminates parallax entirely — what the model sees is exactly what the lens sees, and it let's us use the best possible model.

---

## Goals

1. **Use the actual camera feed for tracking.** Capture video from the Sony Alpha (via USB-C or HDMI) and run detection on the real image the lens produces — no parallax, no offset.

2. **Support arbitrary object tracking.** Not just faces and animals. Draw a bounding box around any object — a product, a shoe, a coffee cup — and the system tracks it through a full 180-degree orbit. 

3. **Bring your own model.** Use state-of-the-art models from Hugging Face or anywhere else. Swap in a better model when one comes along. The system should not be locked to one detection architecture.

4. **Sony camera integration without extra hardware.** Don't require users to buy additional tracking hardware. If you have a Sony Alpha camera, that's your tracking camera. The goal is to develop support for Sony cameras (starting with the A7IV) so users can use their existing gear.

5. **Control focus from software.** As the gimbal tracks and the camera moves, maintain focus on the subject — either by leveraging the camera's built-in autofocus (AF point control, Real-time Tracking AF) or by sending manual focus commands from the tracking system.

---

## Architecture: Two Phases

### Phase 1: Mac as Tracking Computer (Prototype)

Run everything on a connected Mac. The Mac captures video from the camera over USB-C, runs a powerful model (no size constraints — use the best available), computes gimbal corrections, and sends commands to the board.

```
Sony A7IV ──USB-C──► Mac (detection + tracking) ──WiFi──► Board ──► Gimbal
                                                           └──────► Slider
```

This is the fast path to a working prototype. No embedded hardware constraints. Can run large models. Good for validating the tracking-to-gimbal control loop.

### Phase 2: On-Board Tracking (Future)

Run tracking on a compute board mounted on the slider itself — no Mac needed. The ESP32-S3 almost certainly can't handle this; it would require a more capable board (Jetson, RPi 5 + accelerator, or similar). This is a V2 concern — figure out the right hardware after the tracking pipeline is proven on the Mac.

```
Sony A7IV ──USB-C──► Compute board (on slider) ──► Gimbal
                         └──► Slider motor
```

---

## Tracking Approach

There are two fundamental strategies for visual object tracking:

1. **Static template tracking.** Grab the object in frame 1 and track that initial template across all subsequent frames. Simple, but degrades as the object's appearance changes (rotation, lighting, occlusion).

2. **Continuously updating detection.** Detect the object at frame 1, track it for N frames, then re-detect at frame N to get a fresh template that reflects the object's current appearance. Feed the updated detection forward. Repeat. This handles appearance changes through rotation, which is critical for our use case — a 180-degree orbit means the object looks completely different halfway through.

The second approach is what we need. If existing trackers already do this (and many modern ones do — re-identification, template updating), we adopt them directly. If not, we build a detect-then-track loop with periodic re-detection.

The tracking output must be aggressive enough to handle large viewpoint changes. This isn't surveillance tracking where a person walks across a static frame — it's a camera orbiting an object where every frame has a different perspective.

---

## Integration with the Dolly

The tracking system doesn't replace the keyframe system — it augments it.

**Example workflow:** Program a slider move (linear slide from A to B over 10 seconds). The slide executes as programmed. But the gimbal is set to "track mode" — it keeps the subject centered throughout the move, adjusting pan/tilt in real time based on the tracking output. The slider follows the keyframe trajectory; the gimbal follows the subject.

This means:
- Slider axis: keyframe-driven (pre-programmed path)
- Gimbal axes: tracking-driven (real-time corrections)
- Focus: camera-controlled (AF tracking or software focus pull)

---

## Camera Control

For the Sony A7IV specifically:
- **Video feed:** Capture live video over USB-C (or HDMI capture as fallback) for the detection pipeline
- **Focus control:** Send AF commands, control AF point position, or send manual focus adjustments — keeping the subject sharp as the camera moves
- **Exposure (future):** Adjust aperture, ISO, shutter speed programmatically for consistent exposure during moves

Camera control is its own subsystem — see `camera-control/` for the implementation and `docs/research/sony-camera-control/` for the protocol research.

---

## Open Questions

- What's the right re-detection interval for the continuously updating tracker? Every N frames? Triggered by confidence drop?
- Can Sony's built-in Real-time Tracking AF handle our use case if we just keep the subject centered in frame, or do we need explicit focus distance commands?
- For Phase 2, what's the minimum viable compute board that can run detection + tracking at 30fps?
- How does tracking latency affect gimbal smoothness? What's the acceptable detection-to-command latency?
- Should the tracking system output velocity commands or position targets to the gimbal?
