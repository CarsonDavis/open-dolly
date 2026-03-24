# AI Object Detection and Tracking for Gimbal Control

**Date:** 2026-03-23
**Description:** Research into real-time AI object detection, tracking, monocular depth estimation, and 3D tracking for driving a camera gimbal system. Focus on edge-deployable models, open-source repos, and existing gimbal tracking projects from 2023-2026.

## Sources

[1]: https://blog.roboflow.com/best-object-detection-models/ "Roboflow: Best Object Detection Models 2025"
[2]: https://www.ultralytics.com/blog/comparing-ultralytics-yolo11-vs-previous-yolo-models "Ultralytics: YOLO11 vs Previous YOLO Models"
[3]: https://docs.ultralytics.com/compare/ "Ultralytics: Model Comparisons"
[4]: https://docs.ultralytics.com/models/yolo11/ "Ultralytics YOLO11 Docs"
[5]: https://github.com/ultralytics/ultralytics "GitHub: ultralytics/ultralytics"
[6]: https://docs.ultralytics.com/models/yolov8/ "Ultralytics YOLOv8 Docs"
[7]: https://github.com/sunsmarterjie/yolov12 "GitHub: YOLOv12"
[8]: https://github.com/lyuwenyu/RT-DETR "GitHub: RT-DETR"
[9]: https://zhao-yian.github.io/RTDETR/ "RT-DETR Project Page"
[10]: https://medium.com/@aedelon/yolo-is-dead-welcome-rf-detr-the-transformer-that-just-shattered-the-60-map-barrier-e814475d9f8c "RF-DETR vs YOLO Medium article"
[11]: https://docs.ultralytics.com/modes/track/ "Ultralytics: Multi-Object Tracking Docs"
[12]: https://github.com/NirAharon/BoT-SORT "GitHub: BoT-SORT"
[13]: https://www.veroke.com/insights/how-top-ai-multi-object-trackers-perform-in-real-world-scenarios/ "Veroke: MOT Real-World Comparison"
[14]: https://github.com/mikel-brostrom/boxmot "GitHub: BoxMOT"
[15]: https://github.com/MIV-XJTU/ARTrack "GitHub: ARTrack"
[16]: https://github.com/Little-Podi/Transformer_Tracking "GitHub: Transformer Tracking Digest"
[17]: https://github.com/DepthAnything/Depth-Anything-V2 "GitHub: Depth Anything V2"
[18]: https://github.com/LiheYoung/Depth-Anything "GitHub: Depth Anything V1"
[19]: https://github.com/IRCVLab/Depth-Anything-for-Jetson-Orin "GitHub: Depth Anything for Jetson Orin"
[20]: https://github.com/lpiccinelli-eth/UniDepth "GitHub: UniDepth"
[21]: https://github.com/DepthAnything/Video-Depth-Anything "GitHub: Video Depth Anything"
[22]: https://arxiv.org/html/2510.04723v1 "Benchmark on Monocular Metric Depth in Wildlife"
[23]: https://learnopencv.com/depth-pro-monocular-metric-depth/ "LearnOpenCV: Depth Pro Explained"
[24]: https://github.com/google-ai-edge/mediapipe "GitHub: MediaPipe"
[25]: https://github.com/baptdes/YOLOx3D "GitHub: YOLOx3D"
[26]: https://github.com/BigTeacher-777/Awesome-Monocular-3D-Detection "GitHub: Awesome Monocular 3D Detection"
[27]: https://github.com/elvintanhust/MonoCD "GitHub: MonoCD (CVPR 2024)"
[28]: https://github.com/PuFanqi23/MonoDGP "GitHub: MonoDGP (CVPR 2025)"
[29]: https://github.com/lpiccinelli-eth/UniK3D "GitHub: UniK3D (CVPR 2025)"
[30]: https://github.com/maxboels/TrackingPanTiltCam "GitHub: TrackingPanTiltCam"
[31]: https://github.com/doxx/NOLO "GitHub: NOLO PTZ Camera"
[32]: https://github.com/JAYANTH-MOHAN/Object-Tracking-and-Servo-Control-with-Raspberry-Pi-using-Yolov5 "GitHub: RPi YOLO Servo Tracking"
[33]: https://github.com/rlew631/ObjectTrackingDrone "GitHub: Object Tracking Drone"
[34]: https://github.com/AutoPTZ/autoptz "GitHub: AutoPTZ"
[35]: https://support.dji.com/help/content?customId=en-us03400007634 "DJI ActiveTrack for Ronin"
[36]: https://dronedj.com/2024/05/24/dji-drone-follow-tracking-how/ "DroneDJ: DJI Tracking How-To"
[37]: https://github.com/blakeblackshear/frigate "GitHub: Frigate NVR"
[38]: https://christianjmills.com/posts/pytorch-train-object-detector-yolox-tutorial/jetson-object-tracking/ "YOLOX on Jetson Orin Nano"
[39]: https://wiki.seeedstudio.com/YOLOv8-TRT-Jetson/ "YOLOv8 TensorRT on Jetson"
[40]: https://docs.ultralytics.com/guides/deepstream-nvidia-jetson/ "Ultralytics YOLO on Jetson DeepStream"
[41]: https://forums.raspberrypi.com/viewtopic.php?t=373867 "RPi5 Hailo-8L YOLOv8 Benchmark"
[42]: https://community.hailo.ai/t/official-fps-benchmark-on-hailo-8-using-raspberry-pi-5/18873 "Hailo-8 Official FPS Benchmark"
[43]: https://www.jeffgeerling.com/blog/2024/testing-raspberry-pis-ai-kit-13-tops-70 "Jeff Geerling: RPi AI Kit Test"
[44]: https://github.com/AmirSa7/Drone-Tracking-Gimbal-Control "GitHub: Drone Tracking Gimbal Control"
[45]: https://github.com/phucttp/PID-Vision-Tracker "GitHub: PID Vision Tracker"

## Research Log

---

### Search: "YOLOv8 YOLOv9 YOLOv10 YOLOv11 real-time object detection comparison 2024 2025 edge deployment"

- **YOLOv8** (Jan 2023): Anchor-free, CSPNet. Most adopted. Ultralytics. ([Roboflow][1])
- **YOLOv9** (Feb 2024): PGI + GELAN. ([Roboflow][1])
- **YOLOv10** (May 2024): NMS-free. 1.8x faster than RT-DETR-R18. ([Roboflow][1])
- **YOLO11** (Oct 2024): 22% fewer params than YOLOv8m, higher mAP. ([Ultralytics][2])
- Nano variants >30 FPS Jetson Nano. Export: ONNX, TensorRT, CoreML, OpenVINO, TFLite. ([Roboflow][1], [Ultralytics][3])

---

### Search: "ultralytics YOLO11 YOLOv8 mAP COCO benchmark"

- **YOLO11n**: 39.5% mAP, ~56ms CPU ([YOLO11 Docs][4])
- **YOLO11s**: 47.0% mAP, ~90ms CPU ([YOLO11 Docs][4])
- **YOLO11m**: 50.3% mAP, ~171ms CPU ([YOLO11 Docs][4])
- **YOLOv8n**: 37.3% mAP, 0.99ms A100 TensorRT ([YOLOv8 Docs][6])
- Sub-15ms TensorRT latencies. 5 sizes: n/s/m/l/x. ([Ultralytics][2])

---

### Search: "RT-DETR RF-DETR YOLO-NAS YOLOv12 GitHub benchmark"

- **YOLOv12-N**: 40.6% mAP, 1.64ms T4. github.com/sunsmarterjie/yolov12 ([GitHub][7])
- **RF-DETR**: 60.5 mAP (large), RF-DETR-S: 53.0%, 3.52ms. ([RF-DETR article][10])
- **RT-DETR-R50**: 53.1% mAP, 108 FPS T4. github.com/lyuwenyu/RT-DETR ([RT-DETR][9], [GitHub][8])
- **YOLO-NAS**: Overtaken. ([Roboflow][1])

---

### Search: "ByteTrack BoT-SORT StrongSORT OC-SORT multi-object tracking"

Ultralytics built-in: ByteTrack + BoT-SORT via `model.track()`. ([Ultralytics Tracking][11])
- **ByteTrack**: Fastest, static cameras. ([Veroke][13])
- **BoT-SORT**: +ReID +GMC, moving cameras. ([BoT-SORT][12])
- **StrongSORT**: Best identity accuracy. ([Veroke][13])

---

### Search: "ultralytics YOLO built-in tracking"

Single `pip install ultralytics` = detection + tracking. ([Ultralytics Tracking][11])

---

### Search: "single object tracking MixFormer OSTrack ARTrack"

- **ARTrack** (CVPR 2023): 72.6% AUC. **ARTrackV2** (CVPR 2024). ([GitHub: ARTrack][15])
- MOT with target selection more practical for gimbal than pure SOT.

---

### Search: "BoxMOT GitHub"

**BoxMOT** (github.com/mikel-brostrom/boxmot): DeepOCSORT, StrongSORT, BoostTrack, HybridSORT. YOLO11 + RF-DETR. ReID export. ([GitHub][14])

---

### Search: "Depth Anything V2 ZoeDepth Metric3D UniDepth"

- **Depth Anything V2** (NeurIPS 2024): 25M-1.3B params. MAE 0.454m. ([GitHub][17], [Benchmark][22])
- **Video Depth Anything** (CVPR 2025): 30 FPS. ([GitHub][21])
- **UniDepth** (CVPR 2024): Metric depth, no intrinsics. ([GitHub][20])
- **ZoeDepth**: MAE 3.087m outdoors. ([Benchmark][22])
- **Depth Pro** (Apple 2024): Sharp metric depth. ([LearnOpenCV][23])

---

### Search: "Depth Anything V2 Jetson edge inference"

- Jetson Orin repo: github.com/IRCVLab/Depth-Anything-for-Jetson-Orin ([GitHub][19])
- Online Video Depth Anything: 20 FPS Jetson FP16. ([GitHub][21])

---

### Search: "MediaPipe MoveNet 3D pose estimation"

- **MediaPipe Pose**: 33 3D landmarks, meters, real-time. Approximate 3D from mono. ([MediaPipe][24])
- **MoveNet**: 2D only.

---

### Search: "monocular 3D object detection"

- **YOLOx3D**: YOLOv11 + Depth Anything V2 -> 3D bboxes. ([GitHub][25])
- **MonoCD** (CVPR 2024), **MonoDGP** (CVPR 2025), **UniK3D** (CVPR 2025). ([GitHub][27], [28], [29])
- Practical: YOLO 2D + Depth Anything -> distance.

---

### Search: "AI gimbal tracking auto follow camera GitHub"

- **TrackingPanTiltCam**: YOLOv8 + Kalman + Arduino servos. ([GitHub][30])
- **NOLO**: Go, PTZ, YOLO, predictive motion vectors. ([GitHub][31])
- **RPi YOLO Servo**: YOLO + DeepSORT + TCP + RPi. ([GitHub][32])
- **Object Tracking Drone**: MavLink + OpenCV + Pixhawk. ([GitHub][33])

---

### Search: "PTZ camera auto tracking AI"

- **AutoPTZ** (github.com/AutoPTZ/autoptz): Any PTZ camera, face recognition. ([GitHub][34])
- **Frigate NVR**: PTZ auto-tracking discussion. ([GitHub][37])

---

### Search: "DJI ActiveTrack gimbal AI tracking"

- CV + AI + subject recognition. Digital model (shape, color, movement). ([DJI Support][35])
- Motion prediction, Trace + Parallel modes, GPS + vision. ([DroneDJ][36])
- **Core loop: detect -> compute offset from center -> drive gimbal to reduce offset.**

---

### Search: "NVIDIA Jetson Orin YOLO TensorRT Hailo-8 RPi5 Coral TPU edge inference"

**Jetson Orin + TensorRT:**
- Up to 2.6ms per image inference. ([Seeed Jetson][39])
- DeepStream SDK support for video pipelines. ([Ultralytics DeepStream][40])
- YOLO26 optimized for FP16/INT8. ([Ultralytics DeepStream][40])

**RPi5 + Hailo-8L (13 TOPS):**
- YOLOv8n on Hailo-8 (26 TOPS): 431 FPS. ([Hailo Community][42])
- YOLOv8s on Hailo-8L: 80-120 FPS. ([RPi Forum][41])
- YOLO11m on AI HAT+: ~24 FPS (PCIe bottleneck). ([RPi Forum][41])
- RPi AI Kit = Hailo-8L @ 13 TOPS for $70. ([Jeff Geerling][43])

**Hailo-8 vs Coral TPU:** Hailo 26 TOPS, Coral 4 TOPS.

**Verdict:** Jetson Orin for multi-model pipeline. RPi5 + Hailo for detection-only.

---

### Search: "Hailo-8L RPi5 YOLOv8 YOLO11 FPS benchmark 2024"

- YOLOv8s: 80-120 FPS on Hailo-8L. ([RPi Forum][41])
- YOLOv8n: 431 FPS on Hailo-8. ([Hailo Community][42])
- YOLO11m: ~24 FPS on Hailo-8 AI HAT+. ([RPi Forum][41])

---

### Search: "camera detection tracking gimbal control loop PID servo Python full pipeline GitHub 2024"

**Full pipeline examples:**

- **Drone-Tracking-Gimbal-Control** (github.com/AmirSa7/Drone-Tracking-Gimbal-Control): YOLOv5 + Kalman filter + **PID controller** on Jetson, 2-axis stepper motors controlled via Arduino C++. ([GitHub][44])
- **PID-Vision-Tracker** (github.com/phucttp/PID-Vision-Tracker): Real-time tracking with PID, smartphone camera, UART to microcontroller for servo control. ([GitHub][45])
- **Common architecture pattern** across all projects: Camera -> Detection -> Error Calculation (offset from center) -> PID Controller -> Servo/Motor Commands

**Key insight:** The control loop is simple: compute (x,y) pixel offset of target from frame center, run through PID, output pan/tilt velocity or position commands. The hard part is the detection/tracking being fast and reliable enough. The PID tuning is straightforward once the perception pipeline works.
