# AI Object Detection and Tracking for Gimbal Control

**Date:** 2026-03-23

## Executive Summary

The technology for AI-driven gimbal tracking is mature, well-supported by open source, and deployable on edge hardware today. The recommended starting stack is **Ultralytics YOLO11 (nano or small) for detection, BoT-SORT for tracking (built into Ultralytics), and Depth Anything V2 Small for monocular distance estimation**, all running on an **NVIDIA Jetson Orin Nano/NX**. This combination gives real-time (>30 FPS) detection + tracking in a single Python package, with depth estimation running concurrently on the same GPU. The core control loop is straightforward: detect subject, compute pixel offset from frame center, run through a PID controller, and output pan/tilt/slide commands to the gimbal and slider.

---

## 1. Object Detection Models

The detection landscape has evolved rapidly from 2023-2025. The YOLO family remains dominant for real-time edge deployment, but transformer-based detectors (RT-DETR, RF-DETR) now compete on both speed and accuracy.

### YOLO Family (Ultralytics)

**Repository:** [github.com/ultralytics/ultralytics](https://github.com/ultralytics/ultralytics)
**Framework:** Python / PyTorch
**Export formats:** ONNX, TensorRT, CoreML, OpenVINO, TFLite
**Activity:** Extremely active, maintained by Ultralytics Inc.

| Model | Release | mAP (COCO) | Latency (CPU) | TensorRT (A100) | Key Innovation |
|-------|---------|------------|---------------|-----------------|----------------|
| YOLOv8n | Jan 2023 | 37.3% | - | 0.99ms | Anchor-free, CSPNet backbone |
| YOLOv8s | Jan 2023 | ~44% | - | ~1.5ms | Balanced accuracy/speed |
| YOLO11n | Oct 2024 | 39.5% | ~56ms | <15ms | C3k2 bottlenecks, C2PSA attention |
| YOLO11s | Oct 2024 | 47.0% | ~90ms | <15ms | 22% fewer params vs YOLOv8 equiv |
| YOLO11m | Oct 2024 | 50.3% | ~171ms | <15ms | Best accuracy/size tradeoff |

All 5 sizes available: nano (n), small (s), medium (m), large (l), x-large (x).

**Why it matters for us:** Ultralytics is the single most practical choice. One `pip install ultralytics` gives you detection, tracking (BoT-SORT/ByteTrack), segmentation, pose estimation, and export to every edge runtime. The ecosystem is unmatched.

### YOLOv12

**Repository:** [github.com/sunsmarterjie/yolov12](https://github.com/sunsmarterjie/yolov12)
**Paper:** NeurIPS 2025
**Framework:** Python / PyTorch

| Model | mAP (COCO) | Latency (T4) |
|-------|------------|-------------|
| YOLOv12-N | 40.6% | 1.64ms |

First attention-centric YOLO. Outperforms YOLO11-N by 1.2% mAP at comparable speed. Uses Area Attention Module (A2) and R-ELAN. Newer and less battle-tested than Ultralytics, but worth watching.

### RT-DETR (Real-Time Detection Transformer)

**Repository:** [github.com/lyuwenyu/RT-DETR](https://github.com/lyuwenyu/RT-DETR)
**Paper:** CVPR 2024
**Framework:** PaddlePaddle + PyTorch

| Model | mAP (COCO) | FPS (T4) |
|-------|------------|----------|
| RT-DETR-R50 | 53.1% | 108 |
| RT-DETR-R101 | 54.3% | 74 |

First real-time end-to-end transformer detector. NMS-free (no post-processing bottleneck). Very high accuracy at real-time speeds. Also available through Ultralytics.

### RF-DETR

**Paper:** 2025 (Roboflow)
**Framework:** Python / PyTorch

| Model | mAP (COCO) | Latency (T4) |
|-------|------------|-------------|
| RF-DETR-S | 53.0% | 3.52ms |
| RF-DETR-L | 60.5% | 40ms (25 FPS) |

First detector to exceed 60 mAP on COCO. Uses DINOv2 backbone. The small variant beats YOLO11-X in both accuracy (53.0 vs 51.2) and speed (3.52ms vs 11.92ms). NMS-free. Impressive but newer, less community-tested for edge deployment.

### Others

- **YOLO-NAS** (Deci AI, May 2023): Used NAS for architecture search. Largely overtaken by YOLOv10+, RT-DETR, and RF-DETR in 2024-2025 benchmarks. Not recommended as a starting point.
- **YOLO26** (Sep 2025): Latest Ultralytics model. Designed for edge with FP16/INT8 quantization stability. Available via Ultralytics.
- **NanoDet / PicoDet**: Lightweight detectors for mobile. Less relevant since YOLO nano variants are now equally small and better supported.

---

## 2. Object Tracking

### Multi-Object Tracking (MOT) -- Recommended Approach

For gimbal tracking, **MOT with target selection** (detect all objects, let the user click one to follow) is more practical than pure single-object tracking. MOT handles re-detection automatically when the target re-appears after occlusion.

#### Built into Ultralytics

| Tracker | Speed | Camera Motion | Occlusion Handling | Best For |
|---------|-------|---------------|-------------------|----------|
| **ByteTrack** | Fastest | Poor | Basic | Static cameras, simple scenes |
| **BoT-SORT** | Fast | Good (GMC) | Good (ReID) | **Moving cameras (recommended)** |

Usage: `model.track(source, tracker="botsort.yaml")` -- one line of code.

**BoT-SORT is the recommended default for gimbal tracking** because:
1. Global Motion Compensation (GMC) handles the fact that the gimbal is constantly moving the camera
2. Re-identification (ReID) features re-lock onto the target after occlusion
3. Built into Ultralytics -- zero integration work

#### BoxMOT (Extended Tracker Library)

**Repository:** [github.com/mikel-brostrom/boxmot](https://github.com/mikel-brostrom/boxmot)
**Framework:** Python

Pluggable tracker library with more options than Ultralytics built-in:
- DeepOCSORT, StrongSORT, BoT-SORT, BoostTrack, HybridSORT
- Works with YOLO11, YOLOv8, RF-DETR
- ReID model export to ONNX/TensorRT
- Genetic algorithm for hyperparameter tuning

Use BoxMOT if you need StrongSORT (best identity accuracy in crowded scenes) or want to experiment with multiple tracker algorithms.

### Single Object Tracking (SOT)

| Tracker | Paper | AUC | Notes |
|---------|-------|-----|-------|
| ARTrack | CVPR 2023 | 72.6% | Autoregressive trajectory modeling |
| ARTrackV2 | CVPR 2024 | Higher | Pre-trained ViT backbone |
| MixFormer | CVPR 2022 | - | End-to-end transformer |
| OSTrack | ECCV 2022 | - | ViT-based, lightweight |

**Repositories:**
- ARTrack: [github.com/MIV-XJTU/ARTrack](https://github.com/MIV-XJTU/ARTrack)
- Transformer Tracking survey: [github.com/Little-Podi/Transformer_Tracking](https://github.com/Little-Podi/Transformer_Tracking)

SOT trackers are academically strong but less practical for our use case than MOT with target selection. SOT requires a manual initialization bounding box and does not handle target loss + re-detection as gracefully as MOT.

---

## 3. Monocular Depth Estimation

For estimating subject distance from a single camera (no stereo, no LiDAR).

### Depth Anything V2 -- Recommended

**Repository:** [github.com/DepthAnything/Depth-Anything-V2](https://github.com/DepthAnything/Depth-Anything-V2)
**Paper:** NeurIPS 2024

| Variant | Parameters | MAE (outdoor) | Speed |
|---------|-----------|--------------|-------|
| Small | 25M | 0.454m | Real-time on Jetson |
| Base | ~100M | Better | Near real-time |
| Large | 1.3B | Best | Too slow for edge |

Produces per-pixel depth maps from a single image. The Small variant achieves sub-0.5m mean absolute error in outdoor benchmarks -- adequate for estimating subject distance in the 2-20m range. Dramatically better than ZoeDepth (3.087m MAE outdoors).

**Important distinction:** Standard Depth Anything V2 produces **relative depth** (good for ordering but not absolute meters). For metric depth (actual distance in meters), use the metric fine-tuned variants or combine with camera intrinsics.

### Video Depth Anything

**Repository:** [github.com/DepthAnything/Video-Depth-Anything](https://github.com/DepthAnything/Video-Depth-Anything)
**Paper:** CVPR 2025 Highlight

Temporally consistent depth for video (no flickering between frames). Smallest model runs at 30 FPS. Online variant (oVDA) achieves 20 FPS on Jetson with FP16.

### UniDepth / UniDepthV2

**Repository:** [github.com/lpiccinelli-eth/UniDepth](https://github.com/lpiccinelli-eth/UniDepth)
**Paper:** CVPR 2024

Predicts **metric** 3D depth from a single image without camera intrinsics at inference time. This is particularly useful if you want actual distance in meters without calibrating the camera. Heavier than Depth Anything Small but more accurate for metric estimation.

### Other Models

| Model | Notes |
|-------|-------|
| **Depth Pro** (Apple 2024) | Sharp metric depth, fast inference |
| **Metric3D v2** | Zero-shot metric depth + surface normals |
| **ZoeDepth** | MAE 3.087m outdoors -- not recommended |
| **UniK3D** (CVPR 2025) | Universal camera monocular 3D estimation, by UniDepth team |

### Edge Deployment for Depth

- **Depth Anything for Jetson Orin:** [github.com/IRCVLab/Depth-Anything-for-Jetson-Orin](https://github.com/IRCVLab/Depth-Anything-for-Jetson-Orin) -- optimized for real-time on Jetson
- Video Depth Anything achieves 20 FPS on Jetson with FP16 precision

---

## 4. 3D Object Tracking / Pose Estimation

### Practical Approach: Detection + Depth = 3D

The most practical way to get 3D position from monocular video for gimbal control:

1. **YOLO detection** gives a 2D bounding box (pixel coordinates)
2. **Depth Anything** gives per-pixel depth
3. **Sample depth within the bounding box** (median of center region) for subject distance
4. **Camera intrinsics** convert pixel coordinates + depth to 3D world coordinates

**YOLOx3D** ([github.com/baptdes/YOLOx3D](https://github.com/baptdes/YOLOx3D)) demonstrates exactly this pipeline: YOLOv11 + Depth Anything V2 combined to produce 3D bounding boxes from monocular video.

### Academic Monocular 3D Detection

These are primarily for autonomous driving (cars, pedestrians) but demonstrate the approach:

| Model | Paper | Repo |
|-------|-------|------|
| MonoCD | CVPR 2024 | [github.com/elvintanhust/MonoCD](https://github.com/elvintanhust/MonoCD) |
| MonoDGP | CVPR 2025 | [github.com/PuFanqi23/MonoDGP](https://github.com/PuFanqi23/MonoDGP) |
| UniK3D | CVPR 2025 | [github.com/lpiccinelli-eth/UniK3D](https://github.com/lpiccinelli-eth/UniK3D) |

Curated list: [github.com/BigTeacher-777/Awesome-Monocular-3D-Detection](https://github.com/BigTeacher-777/Awesome-Monocular-3D-Detection)

### 3D Human Pose Estimation

**MediaPipe Pose** ([github.com/google-ai-edge/mediapipe](https://github.com/google-ai-edge/mediapipe)):
- 33 3D body landmarks with world coordinates in meters (origin at hip center)
- Real-time on mobile phones, desktops, and web
- Useful for human-specific tracking (e.g., following a person's torso)
- **Limitation:** The 3D coordinates from a monocular camera are approximate, not metrically accurate

**MoveNet:** 17 2D keypoints only. No depth information. Not useful for 3D tracking.

---

## 5. Gimbal/Camera Tracking Systems

### Open Source Projects

| Project | Repo | Detection | Control | Platform |
|---------|------|-----------|---------|----------|
| **TrackingPanTiltCam** | [github.com/maxboels/TrackingPanTiltCam](https://github.com/maxboels/TrackingPanTiltCam) | YOLOv8 + Kalman filter | Arduino pan-tilt servos | PC + Arduino |
| **NOLO** | [github.com/doxx/NOLO](https://github.com/doxx/NOLO) | YOLO | PTZ camera ONVIF | Go-based, spatial awareness, predictive motion |
| **Drone Tracking Gimbal** | [github.com/AmirSa7/Drone-Tracking-Gimbal-Control](https://github.com/AmirSa7/Drone-Tracking-Gimbal-Control) | YOLOv5 + Kalman | PID + Arduino stepper motors | Jetson + Arduino |
| **RPi YOLO Servo** | [github.com/JAYANTH-MOHAN/...](https://github.com/JAYANTH-MOHAN/Object-Tracking-and-Servo-Control-with-Raspberry-Pi-using-Yolov5) | YOLO + DeepSORT | Servo via TCP | RPi + laptop |
| **AutoPTZ** | [github.com/AutoPTZ/autoptz](https://github.com/AutoPTZ/autoptz) | Face recognition | Any PTZ camera | Python |
| **Object Tracking Drone** | [github.com/rlew631/ObjectTrackingDrone](https://github.com/rlew631/ObjectTrackingDrone) | OpenCV | MavLink gimbal control | Pixhawk + RPi |
| **PID Vision Tracker** | [github.com/phucttp/PID-Vision-Tracker](https://github.com/phucttp/PID-Vision-Tracker) | Smartphone camera | PID + UART to MCU | Mobile + Arduino |
| **Frigate NVR** | [github.com/blakeblackshear/frigate](https://github.com/blakeblackshear/frigate) | AI detection | PTZ auto-tracking (discussion) | Linux NVR |

### Common Architecture Pattern

Every project follows the same fundamental loop:

```
Camera Frame
    |
    v
Object Detection (YOLO)
    |
    v
Tracking (assign persistent ID)
    |
    v
Compute Error (target center - frame center)
    |
    v
PID Controller (proportional-integral-derivative)
    |
    v
Gimbal Command (pan/tilt velocity or position)
```

### Commercial Reference: DJI ActiveTrack

DJI's ActiveTrack (used in drones and the Ronin series) demonstrates the end goal:
- Creates a digital model of the subject (shape, color patterns, movement)
- **Motion prediction** estimates direction and speed to maintain tracking through occlusions
- Two modes: **Trace** (constant distance follow) and **Parallel** (alongside at angle)
- Combines GPS + vision recognition for robustness
- ActiveTrack 5.0: tracks from greater distances, handles rapid direction changes

The fundamental approach is the same as the open-source projects: detect target, compute offset, drive gimbal to minimize offset. DJI's edge is in the reliability and polish of their perception pipeline and the integration with flight/stabilization controls.

---

## 6. Edge Deployment

### Hardware Comparison

| Platform | Compute | YOLO11n FPS | Multi-Model | Cost | Notes |
|----------|---------|-------------|-------------|------|-------|
| **Jetson Orin Nano** | GPU (40 TOPS) | 200+ (TensorRT) | Yes | ~$200-300 | Best for multi-model pipeline |
| **Jetson Orin NX** | GPU (70-100 TOPS) | 300+ (TensorRT) | Yes | ~$400-500 | More headroom |
| **RPi5 + Hailo-8L** | NPU (13 TOPS) | 80-120 (YOLOv8s) | Limited | ~$105 | Detection only; PCIe bottleneck |
| **RPi5 + Hailo-8** | NPU (26 TOPS) | 431 (YOLOv8n) | Possible | ~$150 | Better but still PCIe limited |
| **Coral USB TPU** | TPU (4 TOPS) | Low | No | ~$60 | Too limited for this use case |

### Recommended: NVIDIA Jetson Orin Nano/NX

The Jetson Orin is the clear choice for a gimbal tracking system because:

1. **Real GPU** -- can run detection, tracking, and depth estimation concurrently
2. **TensorRT** -- optimizes models to sub-3ms inference (up to 2.6ms per image for YOLO)
3. **CUDA ecosystem** -- all major models (Ultralytics, Depth Anything, MediaPipe) have first-class support
4. **DeepStream SDK** -- Ultralytics provides an official integration for video analytics pipelines on Jetson
5. **FP16/INT8 quantization** -- YOLO models maintain accuracy under quantization

### Deployment Stack

| Layer | Tool | Purpose |
|-------|------|---------|
| Runtime | TensorRT | Model optimization and inference |
| Detection | Ultralytics YOLO11n/s (exported to TensorRT) | Object detection |
| Tracking | BoT-SORT (built into Ultralytics) | ID persistence, re-identification |
| Depth | Depth Anything V2 Small (ONNX/TensorRT) | Distance estimation |
| Video | DeepStream SDK or OpenCV | Camera input, frame management |
| Control | Python + serial/CAN | PID loop, gimbal commands |

### Alternative: RPi5 + Hailo-8L

Viable as a **budget option** for detection-only (no depth). YOLOv8n runs at hundreds of FPS. But:
- PCIe Gen3 x1 bottleneck limits larger models (YOLO11m drops to ~24 FPS)
- Running depth estimation concurrently would need either time-interleaving (alternating frames) or a second accelerator
- Less ecosystem support than Jetson

---

## 7. Recommendations

### Recommended Starting Stack

| Component | Choice | Reason |
|-----------|--------|--------|
| **Detection** | YOLO11s via Ultralytics | Best accuracy/size tradeoff (47% mAP). YOLO11n for max speed (39.5% mAP). |
| **Tracking** | BoT-SORT (Ultralytics built-in) | GMC for moving camera, ReID for occlusion recovery. Zero extra code. |
| **Depth** | Depth Anything V2 Small | 25M params, sub-0.5m MAE, real-time on Jetson. Add only when 3D tracking needed. |
| **Hardware** | NVIDIA Jetson Orin Nano | Runs all three models concurrently. TensorRT for optimization. |
| **Control Loop** | PID controller | Simple, proven. Compute pixel offset from center, output gimbal velocity. |
| **Framework** | Python + Ultralytics + OpenCV | Maximum ecosystem support, fastest prototyping. |

### Implementation Phases

**Phase 1: 2D Tracking (MVP)**
- YOLO11s + BoT-SORT on Jetson Orin Nano
- PID controller: target center pixel -> gimbal pan/tilt velocity
- Camera feed via OpenCV or GStreamer
- Gimbal control via DJI R SDK (CAN bus from ESP32)

**Phase 2: Depth + Distance**
- Add Depth Anything V2 Small running on alternate frames or at lower frequency
- Sample depth within tracked bounding box for subject distance
- Use distance for: focal length control, subject-size-aware framing, speed-dependent tracking gain

**Phase 3: 3D Tracking + Prediction**
- Convert 2D pixel + depth to 3D world coordinates using camera intrinsics
- Kalman filter on 3D position for motion prediction
- Predictive gimbal commands (anticipate movement, not just react to it)
- Potentially add MediaPipe Pose for human-specific body landmark tracking

### Key GitHub Repos to Start With

1. **[ultralytics/ultralytics](https://github.com/ultralytics/ultralytics)** -- Detection + tracking. This is the foundation.
2. **[DepthAnything/Depth-Anything-V2](https://github.com/DepthAnything/Depth-Anything-V2)** -- Depth estimation.
3. **[IRCVLab/Depth-Anything-for-Jetson-Orin](https://github.com/IRCVLab/Depth-Anything-for-Jetson-Orin)** -- Depth on Jetson.
4. **[maxboels/TrackingPanTiltCam](https://github.com/maxboels/TrackingPanTiltCam)** -- Reference for detection->servo control loop.
5. **[AmirSa7/Drone-Tracking-Gimbal-Control](https://github.com/AmirSa7/Drone-Tracking-Gimbal-Control)** -- Reference for PID + gimbal + Jetson.
6. **[mikel-brostrom/boxmot](https://github.com/mikel-brostrom/boxmot)** -- If you need more tracker options.
7. **[baptdes/YOLOx3D](https://github.com/baptdes/YOLOx3D)** -- Reference for YOLO + depth fusion.
