# AI Object Tracking

AI-powered object detection and tracking for automated gimbal control. Detects subjects in a live camera feed, tracks them in 3D space, and drives the gimbal to keep them in frame — with focus commands sent to the camera to maintain sharpness on the subject.

## What it does

1. **Ingest** a live video feed from the camera (Sony Alpha via USB or WiFi)
2. **Detect** objects/subjects using a lightweight ML model — draw bounding boxes, classify
3. **Track** selected subjects across frames, maintaining identity through occlusion and fast motion
4. **Estimate 3D position** of the tracked subject relative to the camera/gimbal
5. **Drive the gimbal** — compute pan/tilt/roll corrections and send them to the DJI RS gimbal via the existing Board API
6. **Control focus** — send continuous AF or manual focus pull commands to the camera to keep the subject sharp

## Architecture

```
Camera (Sony A7IV)
  │
  ├─ Live video feed ──► AI Detection/Tracking ──► Gimbal commands
  │   (USB or WiFi)       (bounding boxes,          (pan/tilt via
  │                        3D position est.)          Board API)
  │
  └─ Focus control ◄──── Focus commands ◄────────── Tracking output
      (USB PTP or           (distance, AF point)
       WiFi API)
```

The tracking system sits between the camera and the gimbal controller. It consumes video, produces gimbal motion commands and focus commands. It talks to the existing OpenDolly board (or directly to the gimbal) via the Board API.

## Key requirements

- **Real-time**: Will need to do testing to see what performance we can get and how much actually matters. Is it a problem if we only detect at 5fps? Etc
- **Small models preferred**: This may run on an edge device (Jetson, RPi 5, or even the phone doing the UI). Accuracy matters but so does latency. However, for version 1, we will be running on an m3 max with 36gb of shared ram, so we can prototype with actually good models.
- **Focus tracking**: I think we can drive the location of the autofocus point and have that at the center of the tracked object, and the camera will handle the rest as long as it is in AF-C
- **Multiple subject types**: People, faces, animals, vehicles, custom objects
- **Smooth output**: Research suggsets that raw detections might noisy. The gimbal commands might need filtering/smoothing (Kalman filter, etc.) to produce cinematic motion, not jittery corrections.

## Status

**Research phase.** Investigating:
- Object detection models suitable for real-time edge deployment (YOLO variants, RT-DETR, etc.)
- Single-object tracking (SOT) and multi-object tracking (MOT) approaches
- Monocular depth estimation for 3D position from a single camera
- How to combine detection + tracking + depth into a smooth gimbal control loop

See `docs/research/ai-object-tracking/` for the full research report.
