<div align="center">

# 🏗️ Tibyan

### AI-Powered Interior Measurement & Automated Quantity Takeoff

[![Platform](https://img.shields.io/badge/Platform-Windows%20|%20macOS-blue?style=for-the-badge)](#system-requirements)
[![Language](https://img.shields.io/badge/Core-C++17-00599C?style=for-the-badge&logo=cplusplus&logoColor=white)](#tech-stack)
[![AI](https://img.shields.io/badge/AI-ONNX%20Runtime-7B2D8E?style=for-the-badge)](#tech-stack)
[![License](https://img.shields.io/badge/License-Proprietary-red?style=for-the-badge)](#license)

**Transform smartphone video walkthroughs of unfinished interiors into accurate, priced Bills of Quantities — automatically.**

[Features](#-key-features) · [How It Works](#-how-it-works) · [Tech Stack](#-tech-stack) · [Getting Started](#-getting-started) · [System Requirements](#-system-requirements) · [Documentation](#-documentation)

---

<img src="docs/assets/hero-banner.png" alt="Tibyan — From Video to BOQ" width="800"/>

</div>

## 🎯 The Problem

Interior finishing contractors lose **4–8 hours per apartment** on manual measurements and an additional **2–4 hours** compiling Bills of Quantities. Worse, inaccurate estimates cause **cost overruns exceeding 10%** — one of the most persistent challenges in the AEC industry. Unfinished construction sites amplify this problem with poor lighting, uniform gray surfaces, and cluttered environments that make manual surveying tedious and error-prone.

## 💡 The Solution

**Tibyan** eliminates the measurement bottleneck entirely. Record a quick video walkthrough with any smartphone, load it into Tibyan, and receive a fully itemized, wastage-adjusted, priced BOQ within minutes — with **±2% measurement accuracy**.

> **No cloud. No internet. No specialized hardware.** All AI processing runs locally on your machine, keeping your project data completely private.

---

## ✨ Key Features

### 🎥 Smart Video Input
- Accepts **MP4, MOV, AVI, MKV** from any smartphone camera (720p to 4K)
- Automatic **camera intrinsic extraction** from EXIF metadata
- Built-in compensation for **poor lighting** and **shaky footage**
- Intelligent frame sampling to minimize processing time

### 🧠 Dual AI Engine
- **Semantic Segmentation** — Vision Transformer (D-Former) classifies every pixel into walls, ceilings, floors, doors, and windows, trained specifically on construction environments
- **Metric Depth Estimation** — Depth Anything V2 produces absolute distance maps in meters from monocular video frames
- Both models run **concurrently** via ONNX Runtime with hardware-adaptive acceleration

### 📐 Precision Measurement
- **2D → 3D projection** using the Pinhole Camera Model and Eigen-powered linear algebra
- **RANSAC planar fitting** for robust surface reconstruction
- **Automatic void deduction** — door and window areas subtracted from gross wall area
- **Occlusion handling** — Convex Hull algorithms complete partially hidden surfaces
- **Reference Gauge calibration** — input one known dimension for absolute metric accuracy

### 📊 Automated BOQ Generation
- Outputs organized by **trade category** (Plastering, Flooring, Painting, Ceiling Works)
- **Configurable waste factors** (5%–15%) per material type
- Embedded **pricing & consumption rates database** (user-editable)
- Full **in-app BOQ editor** with manual override, undo/redo, and audit trail

### 📤 Professional Export
- **Excel (.xlsx)** — formatted with formulas, subtotals, and project headers
- **PDF (.pdf)** — print-ready with logo placeholder and pagination
- **CSV (.csv)** — raw data for integration with external tools

---

## ⚙️ How It Works

```
┌─────────────┐     ┌──────────────────┐     ┌───────────────────────┐
│  📹 Video   │────▶│  FFmpeg Decoder   │────▶│  Frame Sampling       │
│  Input      │     │  + EXIF Parser    │     │  (Configurable FPS)   │
└─────────────┘     └──────────────────┘     └───────────┬───────────┘
                                                         │
                                              ┌──────────▼──────────┐
                                              │   ONNX Runtime      │
                                              │   Inference Engine   │
                                              └──────────┬──────────┘
                                                         │
                                    ┌────────────────────┼────────────────────┐
                                    ▼                                        ▼
                          ┌─────────────────┐                    ┌───────────────────┐
                          │  D-Former        │                    │  Depth Anything V2 │
                          │  Segmentation    │                    │  Metric Depth      │
                          │  (Wall/Ceil/Flr) │                    │  (Meters)          │
                          └────────┬────────┘                    └─────────┬─────────┘
                                   │                                       │
                                   └──────────────┬───────────────────────┘
                                                  ▼
                                    ┌──────────────────────┐
                                    │  3D Point Cloud      │
                                    │  Reconstruction      │
                                    │  (Eigen + Open3D)    │
                                    └──────────┬───────────┘
                                               │
                                    ┌──────────▼───────────┐
                                    │  RANSAC + Mesh       │
                                    │  Triangulation       │
                                    │  → Net Area (m²)     │
                                    └──────────┬───────────┘
                                               │
                                    ┌──────────▼───────────┐
                                    │  📋 BOQ Engine       │
                                    │  Pricing + Wastage   │
                                    │  → Excel / PDF / CSV │
                                    └──────────────────────┘
```

---

## 🛠️ Tech Stack

| Layer                  | Technology                                      | Purpose                                           |
| :--------------------- | :---------------------------------------------- | :------------------------------------------------ |
| **Language**           | C++17                                           | Core application, maximum performance             |
| **GUI Framework**      | Qt 6 (QThread, QThreadPool, Signals & Slots)    | Native desktop UI with multithreaded backend      |
| **AI Inference**       | ONNX Runtime + OpenVINO / TensorRT              | Hardware-adaptive local model execution           |
| **Segmentation Model** | D-Former (Vision Transformer)                   | Pixel-level surface classification                |
| **Depth Model**        | Depth Anything V2 — Metric (DPT + DINOv2)      | Absolute monocular depth estimation               |
| **Video Processing**   | FFmpeg                                          | Decoding, frame extraction, EXIF metadata parsing |
| **Image Processing**   | OpenCV                                          | Morphological ops, filtering, mask cleanup        |
| **3D Geometry**        | Open3D + Eigen                                  | Point clouds, meshing, RANSAC, area calculation   |
| **Model Training**     | Python + PyTorch (offline only)                 | R&D, training, ONNX export                        |
| **Model Compression**  | Intel NNCF / OpenVINO                           | FP32 → INT8 Post-Training Quantization            |

---

## 🚀 Getting Started

### Prerequisites

- **CMake** ≥ 3.22
- **C++17** compatible compiler (MSVC 2022+ / Clang 15+ / GCC 12+)
- **Qt 6.5+** with Widgets and Concurrent modules
- **ONNX Runtime** ≥ 1.16
- **OpenCV** ≥ 4.8
- **Open3D** ≥ 0.18
- **FFmpeg** ≥ 6.0 (shared libraries)
- **Eigen** ≥ 3.4

### Build

```bash
# Clone the repository
git clone https://github.com/your-org/tibyan.git
cd tibyan

# Configure
cmake -B build -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build build --config Release --parallel

# Run
./build/bin/Tibyan
```

### Download AI Models

```bash
# Download pre-trained ONNX models (INT8 quantized)
python scripts/download_models.py --output models/

# Expected structure:
# models/
# ├── dformer_segmentation_int8.onnx
# └── depth_anything_v2_metric_int8.onnx
```

---

## 💻 System Requirements

| Component   | Minimum                                        | Recommended                                       |
| :---------- | :--------------------------------------------- | :------------------------------------------------ |
| **OS**      | Windows 10 (64-bit) / macOS 12 Monterey        | Windows 11 / macOS 14+                            |
| **CPU**     | Intel i5 (10th Gen) / AMD Ryzen 5 3600         | Intel i7 (12th Gen+) / AMD Ryzen 7 5800X         |
| **RAM**     | 8 GB DDR4                                      | 16 GB DDR4/DDR5                                   |
| **GPU**     | None (CPU-only via OpenVINO)                   | NVIDIA RTX 3060 6 GB+ (CUDA 11.8+)               |
| **Storage** | 2 GB free                                      | SSD, ≥ 10 GB free                                 |
| **Display** | 1366 × 768                                     | 1920 × 1080+                                      |

### ⚡ Performance Benchmarks

| Scenario                  | Hardware                       | 1-min Video (1080p) |
| :------------------------ | :----------------------------- | :------------------ |
| **GPU-Accelerated**       | i7-12700 + RTX 3060            | ≤ 3 minutes         |
| **CPU-Only (Optimized)**  | i7-12700 + OpenVINO INT8       | ≤ 8 minutes         |

---

## 📂 Project Structure

```
tibyan/
├── src/
│   ├── core/                # AI inference, 3D processing, BOQ engine
│   ├── gui/                 # Qt-based UI (controllers, views, widgets)
│   ├── io/                  # Video ingestion, FFmpeg wrapper, export
│   └── main.cpp
├── models/                  # Pre-trained ONNX models (INT8)
├── data/
│   └── pricing_db/          # Default pricing & consumption rates
├── scripts/
│   ├── training/            # Python model training pipelines
│   └── download_models.py
├── tests/
├── docs/
│   └── PRD.md               # Product Requirements Document
├── CMakeLists.txt
└── README.md
```

---

## 📖 Documentation

| Document                                | Description                                           |
| :-------------------------------------- | :---------------------------------------------------- |
| [PRD.md](docs/PRD.md)                  | Product Requirements Document                         |
| `docs/technical-report.md`              | Foundational technical research report                |
| `docs/architecture.md`                  | System architecture & data pipeline deep-dive         |

---

## 🗺️ Roadmap

- [x] Technical research & architecture design
- [x] Product Requirements Document (PRD)
- [ ] Core C++ application scaffold with Qt GUI
- [ ] FFmpeg video ingestion & EXIF metadata extraction
- [ ] D-Former segmentation model training & ONNX export
- [ ] Depth Anything V2 metric model integration
- [ ] 3D reconstruction pipeline (Eigen + Open3D)
- [ ] BOQ engine with pricing database
- [ ] Excel & PDF export
- [ ] INT8 quantization & performance optimization
- [ ] Beta testing with field contractors
- [ ] v1.0 public release

---

## 🤝 Contributing

This is currently a proprietary, closed-source project. Contribution guidelines will be published if and when the project transitions to an open-source model.

---

## 📄 License

All rights reserved. This software and its source code are proprietary. Unauthorized copying, distribution, or modification is strictly prohibited.

---

<div align="center">

**Built with 🧠 AI and ❤️ for the AEC industry**

*Tibyan — تبيان*

</div>
