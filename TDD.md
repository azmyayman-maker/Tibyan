# Technical Design Document (TDD)

## **Tibyan** — AI-Powered Interior Measurement & Automated Quantity Takeoff

| Field               | Detail                                                    |
| :------------------ | :-------------------------------------------------------- |
| **Document Version** | 1.0                                                       |
| **Date**            | 2026-03-26                                                |
| **Author**          | Engineering Team                                          |
| **Parent Document** | [PRD v1.0](./PRD.md)                                     |
| **Status**          | Draft — Pending Architecture Review                       |
| **Confidentiality** | Internal — Engineering Distribution Only                  |

---

## Table of Contents

1. [Design Goals & Constraints](#1-design-goals--constraints)
2. [High-Level System Architecture](#2-high-level-system-architecture)
3. [Module Decomposition](#3-module-decomposition)
4. [Data Flow & Pipeline Architecture](#4-data-flow--pipeline-architecture)
5. [AI Inference Subsystem](#5-ai-inference-subsystem)
6. [3D Reconstruction & Measurement Engine](#6-3d-reconstruction--measurement-engine)
7. [BOQ Engine & Pricing Logic](#7-boq-engine--pricing-logic)
8. [GUI Architecture (Qt)](#8-gui-architecture-qt)
9. [Data Models & Storage](#9-data-models--storage)
10. [Concurrency Model & Thread Architecture](#10-concurrency-model--thread-architecture)
11. [Hardware Abstraction & Execution Providers](#11-hardware-abstraction--execution-providers)
12. [Error Handling & Recovery Strategy](#12-error-handling--recovery-strategy)
13. [Build System, Packaging & Distribution](#13-build-system-packaging--distribution)
14. [Testing Strategy](#14-testing-strategy)
15. [Risk Register & Mitigation Matrix](#15-risk-register--mitigation-matrix)
16. [Appendices](#16-appendices)

---

## 1. Design Goals & Constraints

### 1.1 Primary Design Goals

| ID    | Goal                          | Description                                                                                          |
| :---- | :---------------------------- | :--------------------------------------------------------------------------------------------------- |
| DG-01 | **Offline-First**             | Zero network dependency. All AI inference, geometric computation, and BOQ generation execute locally. |
| DG-02 | **Performance Ceiling**       | Process a 1-min 1080p video in ≤3 min (GPU) / ≤8 min (CPU-only). GUI never blocks.                   |
| DG-03 | **Measurement Accuracy**      | ±2% surface area error with Reference Gauge calibration; ±8% without.                                |
| DG-04 | **Memory Discipline**         | Peak processing overhead ≤6 GB RAM via INT8 quantization and streaming frame processing.              |
| DG-05 | **Cross-Platform**            | Single C++ codebase targeting Windows 10+ (primary) and macOS 12+ (secondary).                       |
| DG-06 | **Extensibility**             | Modular architecture allowing future model upgrades, new export formats, and plugin trades.           |

### 1.2 Architectural Constraints

- **Language:** C++17 for the entire production application. Python restricted to offline model training and ONNX export.
- **No Virtual Machine:** JVM/CLR-based languages are excluded to avoid memory overhead and native binding complexity.
- **No Electron/CEF:** The GUI must use native Qt widgets — no embedded browser engines consuming memory during 3D processing.
- **Model Format:** All production models must be ONNX. No PyTorch/TensorFlow runtime in the shipped binary.
- **Thread Safety:** The main (UI) thread shall never execute inference, I/O, or geometric computation. All heavy work runs on `QThreadPool` worker threads.

---

## 2. High-Level System Architecture

### 2.1 Layered Architecture Overview

```
┌──────────────────────────────────────────────────────────────────────────┐
│                        PRESENTATION LAYER                               │
│   Qt 6 Widgets  |  QML Views  |  Wizard Controller  |  BOQ Editor      │
│   ─────────────────── Signals & Slots ──────────────────────────────    │
├──────────────────────────────────────────────────────────────────────────┤
│                        ORCHESTRATION LAYER                              │
│   PipelineOrchestrator  |  QThreadPool  |  FrameScheduler               │
│   ProgressTracker       |  ProjectManager  |  AutoSaveManager           │
├──────────────────────────────────────────────────────────────────────────┤
│                        PROCESSING LAYER                                 │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐  ┌─────────────┐ │
│  │ VideoIngestion│  │ AIInference  │  │ Geometry3D   │  │ BOQEngine   │ │
│  │ Module       │  │ Module       │  │ Module       │  │ Module      │ │
│  └──────────────┘  └──────────────┘  └──────────────┘  └─────────────┘ │
├──────────────────────────────────────────────────────────────────────────┤
│                        INFRASTRUCTURE LAYER                             │
│   FFmpeg  |  ONNX Runtime  |  OpenCV  |  Open3D  |  Eigen  |  libxlsx  │
├──────────────────────────────────────────────────────────────────────────┤
│                        PLATFORM LAYER                                   │
│   OS APIs  |  CUDA/TensorRT  |  OpenVINO  |  File System  |  Threads   │
└──────────────────────────────────────────────────────────────────────────┘
```

### 2.2 Layer Responsibilities

| Layer              | Responsibility                                                                                   | Key Constraint                                    |
| :----------------- | :----------------------------------------------------------------------------------------------- | :------------------------------------------------ |
| **Presentation**   | Render UI, capture user input, display progress bars, BOQ tables, 3D preview.                    | Runs exclusively on the main thread.              |
| **Orchestration**  | Coordinate the multi-stage pipeline, manage thread pools, track progress, auto-save state.        | Owns the `QThreadPool`; emits progress signals.   |
| **Processing**     | Execute domain logic: decode video, run AI, compute geometry, generate BOQ.                       | All methods are thread-safe and reentrant.         |
| **Infrastructure** | Wrap third-party C/C++ libraries behind stable internal APIs.                                     | Abstracted via facades; never leaked to upper layers. |
| **Platform**       | OS-specific APIs, GPU drivers, file system, native threading primitives.                          | Accessed only through Qt/Infrastructure wrappers. |

### 2.3 Dependency Graph

```
Presentation ──▶ Orchestration ──▶ Processing ──▶ Infrastructure ──▶ Platform
     │                                   │
     └───────── (Signals/Slots) ─────────┘
```

> **Rule:** Dependencies flow strictly downward. No lower layer may reference a higher layer. Inter-layer communication from Processing → Presentation is achieved exclusively via Qt Signals emitted upward.

---

## 3. Module Decomposition

### 3.1 Module Registry

| Module ID | Module Name           | Namespace                | Primary Class                | Dependencies                       |
| :-------- | :-------------------- | :----------------------- | :--------------------------- | :--------------------------------- |
| M-01      | Video Ingestion       | `tibyan::ingest`         | `VideoIngestor`              | FFmpeg, OpenCV                     |
| M-02      | Camera Intrinsics     | `tibyan::ingest`         | `IntrinsicsResolver`         | FFmpeg, CameraDB (JSON)            |
| M-03      | AI Inference          | `tibyan::ai`             | `InferenceEngine`            | ONNX Runtime, OpenVINO, TensorRT   |
| M-04      | Segmentation          | `tibyan::ai`             | `SegmentationPipeline`       | InferenceEngine, OpenCV            |
| M-05      | Depth Estimation      | `tibyan::ai`             | `DepthPipeline`              | InferenceEngine                    |
| M-06      | Mask Post-Processing  | `tibyan::ai`             | `MaskRefiner`                | OpenCV                             |
| M-07      | 3D Projection         | `tibyan::geometry`       | `PointCloudProjector`        | Eigen                              |
| M-08      | Surface Reconstruction| `tibyan::geometry`       | `SurfaceReconstructor`       | Open3D, Eigen                      |
| M-09      | Area Calculator       | `tibyan::geometry`       | `AreaCalculator`             | Open3D                             |
| M-10      | Scale Calibration     | `tibyan::geometry`       | `ScaleCalibrator`            | Eigen                              |
| M-11      | BOQ Engine            | `tibyan::boq`            | `BOQGenerator`               | PricingDB, AreaCalculator          |
| M-12      | Pricing Database      | `tibyan::boq`            | `PricingDatabase`            | SQLite / JSON                      |
| M-13      | Export Engine          | `tibyan::export`         | `ExportManager`              | libxlsxwriter, libharu (PDF)      |
| M-14      | Project Manager       | `tibyan::project`        | `ProjectManager`             | Qt Core (QFile, QJsonDocument)     |
| M-15      | Pipeline Orchestrator | `tibyan::orchestration`  | `PipelineOrchestrator`       | All processing modules, QThreadPool|
| M-16      | GUI Shell             | `tibyan::ui`             | `MainWindow`                 | Qt Widgets, Orchestrator           |

### 3.2 Module Interaction Matrix

```
              M-01  M-02  M-03  M-04  M-05  M-06  M-07  M-08  M-09  M-10  M-11  M-12  M-13  M-14  M-15  M-16
M-01 Ingest    —     W     ·     ·     ·     ·     ·     ·     ·     ·     ·     ·     ·     ·     R     ·
M-02 Intrins   ·     —     ·     ·     ·     ·     W     ·     ·     W     ·     ·     ·     ·     R     ·
M-03 AIEngine  ·     ·     —     W     W     ·     ·     ·     ·     ·     ·     ·     ·     ·     R     ·
M-04 Segment   ·     ·     R     —     ·     W     ·     ·     ·     ·     ·     ·     ·     ·     R     ·
M-05 Depth     ·     ·     R     ·     —     ·     W     ·     ·     ·     ·     ·     ·     ·     R     ·
M-06 MaskRef   ·     ·     ·     R     ·     —     W     ·     ·     ·     ·     ·     ·     ·     R     ·
M-07 Project   R     R     ·     ·     R     R     —     W     ·     ·     ·     ·     ·     ·     R     ·
M-08 Surface   ·     ·     ·     ·     ·     ·     R     —     W     ·     ·     ·     ·     ·     R     ·
M-09 Area      ·     ·     ·     ·     ·     ·     ·     R     —     R     W     ·     ·     ·     R     ·
M-10 Scale     ·     R     ·     ·     ·     ·     ·     ·     R     —     ·     ·     ·     ·     R     R
M-11 BOQ       ·     ·     ·     ·     ·     ·     ·     ·     R     ·     —     R     W     ·     R     ·
M-12 Pricing   ·     ·     ·     ·     ·     ·     ·     ·     ·     ·     R     —     ·     ·     ·     R
M-13 Export    ·     ·     ·     ·     ·     ·     ·     ·     ·     ·     R     ·     —     W     R     ·
M-14 Project   ·     ·     ·     ·     ·     ·     ·     ·     ·     ·     ·     ·     R     —     R     R
M-15 Orch      W     W     W     W     W     W     W     W     W     W     W     ·     W     W     —     ·
M-16 GUI       ·     ·     ·     ·     ·     ·     ·     ·     ·     R     ·     R     ·     R     R     —

Legend: R = Reads/Calls  |  W = Writes/Produces for  |  · = No direct interaction
```

---

## 4. Data Flow & Pipeline Architecture

### 4.1 End-to-End Pipeline Sequence

```
Phase 1              Phase 2                Phase 3              Phase 4              Phase 5
DATA INGESTION       CONCURRENT AI          3D PROJECTION        GEOMETRIC PROC.      BOQ GENERATION
                     INFERENCE
┌─────────┐     ┌─────────────────┐     ┌──────────────┐     ┌──────────────┐     ┌──────────────┐
│ Load    │     │SegmentationPipe │     │ PointCloud   │     │ RANSAC Plane │     │ Apply Waste  │
│ Video   │────▶│ (D-Former)      │────▶│ Projector    │────▶│ Fitting      │────▶│ Factors      │
│ File    │     │                 │     │ (Pinhole     │     │              │     │              │
│         │     │ DepthPipeline   │     │  Model +     │     │ Mesh Triang. │     │ Material     │
│ Extract │     │ (DepthAnythV2)  │     │  Eigen)      │     │              │     │ Quantities   │
│ Frames  │     │                 │     │              │     │ Net Area     │     │              │
│ + EXIF  │     │ [Concurrent]    │     │              │     │ Calculation  │     │ Export       │
│ + K     │     │                 │     │              │     │              │     │ XLSX/PDF/CSV │
└─────────┘     └─────────────────┘     └──────────────┘     └──────────────┘     └──────────────┘
```

### 4.2 Per-Frame Data Flow (Detailed)

```
Input: RGB Frame (cv::Mat, H×W×3, uint8)
       Camera Intrinsic Matrix K (Eigen::Matrix3d)

Step 1 ─ Preprocessing
  │  CLAHE equalization → Enhanced Frame
  │  Luminance check → Accept / Reject frame
  ▼
Step 2 ─ Concurrent AI Inference (via QThreadPool)
  │
  ├──▶ Thread A: SegmentationPipeline
  │    Input:  Enhanced Frame (resized to model input, e.g. 512×512)
  │    Model:  D-Former (ONNX, INT8)
  │    Output: Semantic Mask (H×W, uint8) — class IDs:
  │            0=Background, 1=Wall, 2=Ceiling, 3=Floor,
  │            4=Window, 5=Door, 6=Clutter
  │
  └──▶ Thread B: DepthPipeline
       Input:  Enhanced Frame (resized to model input, e.g. 518×518)
       Model:  Depth Anything V2 Metric (ONNX, INT8)
       Output: Depth Map (H×W, float32) — meters from sensor
  │
  ▼ [Synchronization Barrier — both threads complete]
Step 3 ─ Mask Post-Processing (MaskRefiner)
  │  Morphological Opening (3×3 kernel, 2 iterations) → remove noise
  │  Morphological Closing (5×5 kernel, 2 iterations) → fill gaps
  │  Elevation-based Filtering → refine wall extents
  │  Output: Cleaned Semantic Mask
  ▼
Step 4 ─ Temporal Smoothing
  │  Exponential Moving Average across depth maps of frames [t-2, t-1, t]
  │  Alpha = 0.6 (current frame weight)
  │  Output: Temporally Smoothed Depth Map
  ▼
Step 5 ─ 3D Projection (PointCloudProjector)
  │  For each pixel (u,v) with class ∈ {Wall, Ceiling, Floor, Window, Door}:
  │    [X, Y, Z]ᵀ = Z_depth(u,v) · K⁻¹ · [u, v, 1]ᵀ
  │  Output: Per-class Point Clouds (std::unordered_map<ClassID, open3d::PointCloud>)
  ▼
Step 6 ─ Multi-Frame Registration & Merging
  │  ICP (Iterative Closest Point) alignment between consecutive frames
  │  Voxel downsampling (voxel_size = 0.01 m) to manage density
  │  Output: Unified per-surface Point Clouds
```

### 4.3 Inter-Module Data Contracts

| Producer              | Consumer                | Data Structure                         | Format              | Threading   |
| :-------------------- | :---------------------- | :------------------------------------- | :------------------ | :---------- |
| VideoIngestor         | SegmentationPipeline    | `FramePacket`                          | `cv::Mat` (BGR, u8) | Queue-based |
| VideoIngestor         | DepthPipeline           | `FramePacket`                          | `cv::Mat` (BGR, u8) | Queue-based |
| IntrinsicsResolver    | PointCloudProjector     | `CameraIntrinsics`                     | `Eigen::Matrix3d`   | Shared ptr  |
| SegmentationPipeline  | MaskRefiner             | `RawSegmentationMask`                  | `cv::Mat` (u8)      | Direct call |
| MaskRefiner           | PointCloudProjector     | `CleanedMask`                          | `cv::Mat` (u8)      | Direct call |
| DepthPipeline         | PointCloudProjector     | `MetricDepthMap`                       | `cv::Mat` (f32)     | Direct call |
| PointCloudProjector   | SurfaceReconstructor    | `ClassifiedPointClouds`                | `open3d::PointCloud` | Move semantics |
| SurfaceReconstructor  | AreaCalculator          | `TriangulatedMeshes`                   | `open3d::TriangleMesh` | Move semantics |
| AreaCalculator        | BOQGenerator            | `SurfaceAreaReport`                    | Custom struct       | Signal/Slot |
| BOQGenerator          | ExportManager           | `BOQTable`                             | Custom struct       | Signal/Slot |
| ScaleCalibrator       | PointCloudProjector     | `GlobalScaleFactor`                    | `double`            | Atomic      |

---

## 5. AI Inference Subsystem

### 5.1 ONNX Runtime Integration Architecture

```cpp
// Simplified class hierarchy
namespace tibyan::ai {

class InferenceEngine {
public:
    InferenceEngine();
    ~InferenceEngine();

    // Detects GPU/CPU and selects optimal Execution Provider
    void initialize(const HardwareProfile& hw);

    // Loads an ONNX model with the selected EP
    ModelHandle loadModel(const std::filesystem::path& onnx_path,
                          const ModelConfig& config);

    // Synchronous inference on a single frame
    InferenceResult run(ModelHandle handle,
                        const cv::Mat& input_frame);

private:
    Ort::Env                     env_;
    Ort::SessionOptions          session_opts_;
    std::unique_ptr<Ort::Session> session_;
    ExecutionProviderType         active_ep_;   // TENSORRT | OPENVINO | CPU
};

} // namespace tibyan::ai
```

### 5.2 Execution Provider Selection Logic

```
START
  │
  ▼
  Detect NVIDIA GPU via CUDA API?
  ├── YES ──▶ Load TensorRT EP
  │           ├── Success ──▶ USE TENSORRT
  │           └── Fail ──▶ Fallback to CUDA EP
  │                         ├── Success ──▶ USE CUDA
  │                         └── Fail ──▶ goto CPU check
  │
  └── NO ──▶ Detect Intel CPU/iGPU?
              ├── YES ──▶ Load OpenVINO EP
              │           ├── Success ──▶ USE OPENVINO
              │           └── Fail ──▶ USE CPU (MLAS)
              │
              └── NO ──▶ USE CPU (MLAS)
```

### 5.3 Model Specifications

| Property                 | D-Former (Segmentation)     | Depth Anything V2 (Metric)  |
| :----------------------- | :-------------------------- | :-------------------------- |
| **Architecture**         | Vision Transformer (ViT)    | DPT + DINOv2 encoder       |
| **Input Tensor**         | `[1, 3, 512, 512]` float32  | `[1, 3, 518, 518]` float32  |
| **Output Tensor**        | `[1, 6, 512, 512]` float32 (logits) | `[1, 1, 518, 518]` float32 (meters) |
| **Output Classes**       | 6 (Wall, Ceiling, Floor, Window, Door, Clutter) | N/A (continuous depth)   |
| **FP32 Size (est.)**     | ~180 MB                     | ~350 MB                     |
| **INT8 Size (est.)**     | ~45 MB                      | ~90 MB                      |
| **Target Latency (GPU)** | ≤150 ms / frame             | ≤220 ms / frame             |
| **Target Latency (CPU)** | ≤800 ms / frame             | ≤1200 ms / frame            |
| **Quantization**         | PTQ via NNCF (INT8)         | PTQ via NNCF (INT8)         |
| **Training Dataset**     | StructScan3D + Rohbau3D     | Synthetic + Metric fine-tune |

### 5.4 Pre- and Post-Processing Specifications

**Segmentation Pre-Processing:**
1. Resize input frame to `512×512` using bilinear interpolation.
2. Normalize pixel values: `pixel = (pixel / 255.0 - mean) / std` where `mean = [0.485, 0.456, 0.406]`, `std = [0.229, 0.224, 0.225]` (ImageNet normalization).
3. Transpose from HWC to CHW layout.
4. Cast to `float32` tensor.

**Segmentation Post-Processing:**
1. Apply `argmax` across the class dimension to obtain per-pixel class IDs.
2. Resize mask back to original frame resolution using nearest-neighbor interpolation.
3. Pass to `MaskRefiner` for morphological cleanup.

**Depth Pre-Processing:**
1. Resize input frame to `518×518` using bilinear interpolation.
2. Apply same ImageNet normalization as segmentation.
3. Transpose HWC → CHW, cast to `float32`.

**Depth Post-Processing:**
1. Squeeze output tensor to `[518, 518]`.
2. Resize depth map back to original frame resolution using bilinear interpolation.
3. Multiply by `GlobalScaleFactor` from `ScaleCalibrator`.
4. Apply temporal smoothing (EMA, alpha=0.6).

---

## 6. 3D Reconstruction & Measurement Engine

### 6.1 Pinhole Camera Model — Inverse Projection

The core mathematical operation transforming 2D pixels into 3D metric space:

```
Given:
  (u, v)    = pixel coordinates in the image
  Z         = metric depth value at (u, v) from the depth map (meters)
  K         = Camera Intrinsic Matrix:
              ┌ fx   0   cx ┐
              │  0  fy   cy │
              └  0   0    1 ┘

Inverse Projection:
  X = Z * (u - cx) / fx
  Y = Z * (v - cy) / fy
  Z = Z (depth value)

Result: Point (X, Y, Z) in camera coordinate frame (meters)
```

**Implementation:** Vectorized using Eigen for batch processing of all classified pixels in a single matrix operation:

```cpp
// Pseudocode — PointCloudProjector::project()
Eigen::MatrixXd pixels(3, N);    // [u; v; 1] for N classified pixels
Eigen::VectorXd depths(N);       // Z_depth for each pixel

Eigen::Matrix3d K_inv = K.inverse();
Eigen::MatrixXd points_3d = K_inv * pixels;  // [3 x N]

// Scale by depth
for (int i = 0; i < 3; ++i)
    points_3d.row(i) = points_3d.row(i).cwiseProduct(depths.transpose());
```

### 6.2 Surface Reconstruction Pipeline

```
Per-Class Point Cloud (e.g., "Wall-North")
  │
  ▼
Step 1: Statistical Outlier Removal
  │  nb_neighbors = 20, std_ratio = 2.0
  │  Removes isolated noise points
  ▼
Step 2: Voxel Downsampling
  │  voxel_size = 0.01 m (1 cm resolution)
  │  Reduces density for faster meshing
  ▼
Step 3: RANSAC Planar Fitting
  │  distance_threshold = 0.02 m
  │  ransac_n = 3, num_iterations = 1000
  │  Output: Plane equation (ax + by + cz + d = 0), inlier indices
  │  Points not fitting any plane are discarded
  ▼
Step 4: Convex Hull / OBB Construction
  │  For occluded surfaces: compute the Convex Hull of inlier points
  │  This "completes" partially visible walls
  ▼
Step 5: Mesh Triangulation
  │  Delaunay triangulation of the planar inlier points
  │  Output: open3d::TriangleMesh
  ▼
Step 6: Gross Area Calculation
  │  Sum of all triangle areas: A = Σ (0.5 * |e1 × e2|)
  │  Output: Gross area in m²
  ▼
Step 7: Void Deduction
  │  Project Window/Door point clouds onto parent wall plane
  │  Compute void areas via separate Convex Hulls
  │  Net Area = Gross Area - Σ(Void Areas)
  │  Output: Net area in m²
```

### 6.3 Scale Calibration Algorithm

```
Input:
  user_reference_distance_m  : double  (e.g., door width = 0.90 m)
  pixel_coords_start         : (u1, v1)
  pixel_coords_end           : (u2, v2)

Process:
  1. depth_start = depth_map[v1][u1]
  2. depth_end   = depth_map[v2][u2]
  3. point3d_start = inverse_project(u1, v1, depth_start, K)
  4. point3d_end   = inverse_project(u2, v2, depth_end, K)
  5. estimated_distance = euclidean_distance(point3d_start, point3d_end)
  6. GlobalScaleFactor = user_reference_distance_m / estimated_distance
  7. Apply: all_depths *= GlobalScaleFactor

Confidence:
  If |GlobalScaleFactor - 1.0| < 0.05  → HIGH confidence
  If |GlobalScaleFactor - 1.0| < 0.15  → MEDIUM confidence
  Else                                  → LOW confidence (warn user)
```

---

## 7. BOQ Engine & Pricing Logic

### 7.1 Core Data Structures

```cpp
namespace tibyan::boq {

struct SurfaceAreaReport {
    std::string room_id;
    std::unordered_map<SurfaceType, double> gross_areas;  // m²
    std::unordered_map<SurfaceType, double> net_areas;    // m²
    std::unordered_map<SurfaceType, double> void_areas;   // m²
};

struct MaterialEntry {
    std::string material_id;
    std::string trade;           // "Plastering", "Painting", etc.
    std::string description;     // "Interior wall putty"
    std::string unit;            // "kg", "liter", "m²"
    double consumption_rate;     // units per m²
    double unit_price;           // currency per unit
    double default_waste_pct;    // 0.05 to 0.15
};

struct BOQLineItem {
    int    item_no;
    std::string trade;
    std::string description;
    std::string unit;
    double net_quantity;
    double waste_factor_pct;
    double gross_quantity;       // net_quantity * (1 + waste_factor_pct)
    double unit_rate;
    double total_cost;           // gross_quantity * unit_rate
    bool   is_user_overridden;   // true if manually edited
};

struct BOQTable {
    std::string project_name;
    std::string date;
    std::string prepared_by;
    std::vector<BOQLineItem> line_items;
    double grand_total;
};

} // namespace tibyan::boq
```

### 7.2 Quantity Calculation Formula

```
For each MaterialEntry M linked to a SurfaceType S:

  net_quantity   = net_areas[S] × M.consumption_rate
  gross_quantity = net_quantity × (1 + M.default_waste_pct)
  total_cost     = gross_quantity × M.unit_price
```

### 7.3 Pricing Database Schema (SQLite)

```sql
CREATE TABLE trades (
    trade_id    TEXT PRIMARY KEY,
    trade_name  TEXT NOT NULL          -- "Plastering", "Painting", etc.
);

CREATE TABLE materials (
    material_id       TEXT PRIMARY KEY,
    trade_id          TEXT NOT NULL REFERENCES trades(trade_id),
    description       TEXT NOT NULL,
    unit              TEXT NOT NULL,    -- "kg", "liter", "m²", "piece"
    consumption_rate  REAL NOT NULL,    -- units per m²
    unit_price        REAL NOT NULL,    -- currency per unit
    waste_factor_pct  REAL NOT NULL DEFAULT 0.10,
    surface_type      TEXT NOT NULL,    -- "WALL", "CEILING", "FLOOR"
    is_active         INTEGER NOT NULL DEFAULT 1,
    updated_at        TEXT NOT NULL DEFAULT (datetime('now'))
);

CREATE TABLE material_overrides (
    override_id   INTEGER PRIMARY KEY AUTOINCREMENT,
    project_id    TEXT NOT NULL,
    material_id   TEXT NOT NULL REFERENCES materials(material_id),
    unit_price    REAL,
    waste_factor  REAL,
    created_at    TEXT NOT NULL DEFAULT (datetime('now'))
);
```

---

## 8. GUI Architecture (Qt)

### 8.1 View Hierarchy

```
MainWindow (QMainWindow)
├── ProjectDashboardView       ── Recent projects grid, New/Open/Import
├── VideoImportView            ── Drag-drop zone, file browser, metadata display
├── CalibrationView            ── Reference Gauge input, camera intrinsics display
├── ProcessingView             ── Progress bars (per-phase), frame preview, cancel button
├── ResultsView
│   ├── MeasurementSummaryPanel ── Per-surface area breakdown table
│   ├── PointCloudPreviewPanel  ── 3D viewport (Open3D + Qt widget)
│   └── BOQEditorPanel          ── Editable table (QTableView + custom model)
├── ExportView                 ── Format selection, path picker, template config
└── SettingsView               ── Language, pricing DB, default waste factors, GPU toggle
```

### 8.2 Wizard-Style Navigation

```
[Dashboard] ──▶ [Import Video] ──▶ [Calibration] ──▶ [Processing] ──▶ [Results] ──▶ [Export]
                                         │                                │
                                         └── Skip (use EXIF only) ◀──────┘ (Edit & Re-export)
```

### 8.3 Signal/Slot Communication Map

| Signal Emitter          | Signal                          | Slot Receiver            | Slot                              |
| :---------------------- | :------------------------------ | :----------------------- | :-------------------------------- |
| `VideoImportView`       | `videoSelected(QString path)`   | `PipelineOrchestrator`   | `onVideoSelected()`              |
| `CalibrationView`       | `gaugeProvided(double meters)`  | `ScaleCalibrator`        | `setReferenceDistance()`         |
| `PipelineOrchestrator`  | `progressUpdated(int pct, QString phase)` | `ProcessingView` | `updateProgressBar()`            |
| `PipelineOrchestrator`  | `frameProcessed(cv::Mat preview)` | `ProcessingView`       | `displayFramePreview()`         |
| `PipelineOrchestrator`  | `pipelineComplete(SurfaceAreaReport)` | `ResultsView`      | `displayResults()`              |
| `BOQEditorPanel`        | `lineItemEdited(int row, BOQLineItem)` | `BOQGenerator`    | `overrideLineItem()`            |
| `ExportView`            | `exportRequested(ExportConfig)` | `ExportManager`          | `exportBOQ()`                   |
| `AutoSaveManager`       | `autoSaveTriggered()`           | `ProjectManager`         | `saveProjectState()`            |

---

## 10. Concurrency Model & Thread Architecture

### 10.1 Thread Pool Design

```
Main Thread (GUI)
  │  ── Renders UI, handles user input
  │  ── NEVER performs I/O, inference, or geometry
  │
  ├── QThreadPool (global, maxThreadCount = CPU cores)
  │   ├── VideoDecoderWorker      (QRunnable) ── Decodes frames via FFmpeg
  │   ├── SegmentationWorker      (QRunnable) ── Runs D-Former inference
  │   ├── DepthWorker             (QRunnable) ── Runs Depth Anything V2 inference
  │   ├── GeometryWorker          (QRunnable) ── 3D projection + meshing
  │   └── ExportWorker            (QRunnable) ── Excel/PDF generation
  │
  └── AutoSaveTimer (QTimer, 60s interval)
      └── Fires on main thread, dispatches save to QThreadPool
```

### 10.2 Frame Processing Concurrency

```
Frame Queue (Thread-Safe, capacity = 10)
  │
  ▼
VideoDecoderWorker ──[produces frames]──▶ Queue
  │
  └──▶ For each frame dequeued:
        ├── SegmentationWorker ──▶ [mask]  ─┐
        └── DepthWorker        ──▶ [depth] ─┤
                                             ▼
                               QFutureSynchronizer (barrier)
                                             │
                                             ▼
                               GeometryWorker ──▶ [point cloud]
```

### 10.3 Thread Safety Rules

| Rule ID | Rule                                                                                          |
| :------ | :-------------------------------------------------------------------------------------------- |
| TS-01   | All GUI widget access occurs exclusively on the main thread.                                  |
| TS-02   | Worker threads communicate results to the main thread only via `Qt::QueuedConnection` signals.|
| TS-03   | `GlobalScaleFactor` is stored as `std::atomic<double>`.                                       |
| TS-04   | The frame queue uses `QMutex` + `QWaitCondition` for producer-consumer synchronization.       |
| TS-05   | ONNX Runtime sessions are not shared between threads; each worker holds its own `Ort::Session`.|
| TS-06   | Open3D point clouds are passed between stages via `std::move` (transfer of ownership).        |

---

## 11. Hardware Abstraction & Execution Providers

### 11.1 Hardware Detection at Startup

```cpp
struct HardwareProfile {
    bool   has_nvidia_gpu;
    std::string nvidia_gpu_name;     // e.g. "NVIDIA GeForce RTX 3060"
    int    cuda_compute_capability;  // e.g. 86
    size_t gpu_vram_mb;              // e.g. 6144
    bool   has_intel_cpu;
    bool   has_intel_igpu;
    int    cpu_core_count;
    size_t total_ram_mb;
};
```

### 11.2 Execution Provider Priority Table

| Priority | Condition                               | EP Selected    | Expected Speedup vs. CPU |
| :------- | :-------------------------------------- | :------------- | :----------------------- |
| 1        | NVIDIA GPU + CUDA ≥ 11.8 + VRAM ≥ 4 GB | TensorRT       | ~8–12×                   |
| 2        | NVIDIA GPU + CUDA (fallback)            | CUDA EP        | ~5–8×                    |
| 3        | Intel CPU (10th Gen+) or Intel iGPU     | OpenVINO       | ~2–3.6× (with INT8)     |
| 4        | Any CPU (fallback)                      | CPU (MLAS)     | 1× (baseline)           |

---

## 12. Error Handling & Recovery Strategy

### 12.1 Error Classification

| Category        | Examples                                            | Severity | Recovery Strategy                                         |
| :-------------- | :-------------------------------------------------- | :------- | :-------------------------------------------------------- |
| **Input Error** | Unsupported codec, corrupt file, missing EXIF        | Warning  | Notify user, prompt for manual intrinsics or different file |
| **AI Error**    | Model load failure, NaN in output tensor             | Critical | Abort pipeline, log diagnostics, prompt user to reinstall  |
| **Geometry Error** | RANSAC yields 0 inliers, degenerate triangulation | Warning  | Skip frame, log + continue; warn in final report          |
| **IO Error**    | Disk full during export, write permissions denied    | Error    | Retry with user-selected path; auto-save to temp directory |
| **OOM Error**   | RAM exhaustion during inference                      | Critical | Reduce batch size to 1, retry; if still fails, abort with advice |

### 12.2 Auto-Save & Crash Recovery

```
Normal Operation:
  AutoSaveTimer (60s) ──▶ ProjectManager::saveCheckpoint()
    Writes: processed_frames[], point_clouds[], partial_boq, pipeline_state
    To:     <project_dir>/.tibyan_autosave/

Crash Recovery:
  On next launch ──▶ Detect .tibyan_autosave/ directory
    ──▶ Prompt: "A previous session was interrupted. Resume from frame 47/300?"
    ──▶ YES: Reload checkpoint, skip already-processed frames
    ──▶ NO:  Delete checkpoint, start fresh
```

---

## 13. Build System, Packaging & Distribution

### 13.1 Build Configuration (CMake)

```cmake
cmake_minimum_required(VERSION 3.22)
project(Tibyan VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Dependencies
find_package(Qt6 REQUIRED COMPONENTS Widgets Concurrent LinguistTools)
find_package(OpenCV REQUIRED COMPONENTS core imgproc videoio)
find_package(Eigen3 REQUIRED)
find_package(onnxruntime REQUIRED)
find_package(Open3D REQUIRED)

# FFmpeg via pkg-config
find_package(PkgConfig REQUIRED)
pkg_check_modules(FFMPEG REQUIRED libavformat libavcodec libavutil libswscale)

# Application target
add_executable(Tibyan
    src/main.cpp
    src/ui/MainWindow.cpp
    src/ingest/VideoIngestor.cpp
    src/ingest/IntrinsicsResolver.cpp
    src/ai/InferenceEngine.cpp
    src/ai/SegmentationPipeline.cpp
    src/ai/DepthPipeline.cpp
    src/ai/MaskRefiner.cpp
    src/geometry/PointCloudProjector.cpp
    src/geometry/SurfaceReconstructor.cpp
    src/geometry/AreaCalculator.cpp
    src/geometry/ScaleCalibrator.cpp
    src/boq/BOQGenerator.cpp
    src/boq/PricingDatabase.cpp
    src/export/ExportManager.cpp
    src/project/ProjectManager.cpp
    src/orchestration/PipelineOrchestrator.cpp
)

target_link_libraries(Tibyan PRIVATE
    Qt6::Widgets Qt6::Concurrent
    ${OpenCV_LIBS} Eigen3::Eigen
    onnxruntime::onnxruntime Open3D::Open3D
    ${FFMPEG_LIBRARIES}
)
```

### 13.2 Distribution Packaging

| Platform     | Packaging Method          | Installer Tool        | Size Estimate |
| :----------- | :------------------------ | :-------------------- | :------------ |
| **Windows**  | NSIS / WiX Toolset        | `.msi` or `.exe`      | ~250–350 MB   |
| **macOS**    | CMake + `macdeployqt`     | `.dmg` bundle         | ~280–380 MB   |

Bundled artifacts: Qt runtime, ONNX Runtime shared libs, OpenCV/Open3D shared libs, FFmpeg shared libs, INT8 ONNX models, pricing database (SQLite), camera intrinsics database (JSON).

---

## 14. Testing Strategy

### 14.1 Test Pyramid

| Level                    | Framework          | Scope                                                     | Target Coverage |
| :----------------------- | :----------------- | :-------------------------------------------------------- | :-------------- |
| **Unit Tests**           | Google Test (gtest)| Individual classes: `AreaCalculator`, `ScaleCalibrator`, `BOQGenerator`, `MaskRefiner` | ≥ 80%           |
| **Integration Tests**    | Google Test + CTest| Module interactions: Ingestion→AI→Geometry, BOQ→Export     | ≥ 70%           |
| **Accuracy Benchmarks**  | Custom harness     | 50+ rooms: compare computed areas vs. laser ground truth   | mIoU ≥ 85%, ±2% area |
| **Performance Benchmarks**| Custom harness    | 1-min video processing: GPU ≤3 min, CPU ≤8 min, RAM ≤6 GB | All thresholds met |
| **UI Tests**             | Qt Test            | Wizard flow, BOQ editor, export dialogs                    | Critical paths  |
| **Crash/Stress Tests**   | Custom + ASAN      | 4K 60fps video, corrupt files, OOM simulation              | 0 crashes       |

### 14.2 Accuracy Validation Dataset

| Dataset Component      | Volume             | Source                                        |
| :--------------------- | :----------------- | :-------------------------------------------- |
| Ground truth rooms     | 50+ rooms          | Laser-measured by professional surveyors      |
| Annotated frames       | 500+ images        | Manual pixel-level annotation (6 classes)     |
| Diverse conditions     | 5 categories       | Good light / Poor light / Cluttered / Occluded / Multi-room |

---

## 15. Risk Register & Mitigation Matrix

| Risk ID | Risk Description                                     | Probability | Impact   | Mitigation Strategy                                                                            |
| :------ | :--------------------------------------------------- | :---------- | :------- | :--------------------------------------------------------------------------------------------- |
| R-01    | D-Former accuracy degrades on unseen construction styles | Medium   | High     | Continuous training dataset expansion; user feedback loop for misclassified regions.            |
| R-02    | Depth Anything V2 scale drift in long corridors       | Medium      | High     | Mandatory Reference Gauge for rooms >5 m depth; multi-gauge support in v1.1.                   |
| R-03    | TensorRT version incompatibility across GPU drivers   | High        | Medium   | Ship multiple TRT versions; fallback to CUDA EP; clear driver requirement docs.                |
| R-04    | Qt licensing cost for commercial distribution         | Low         | High     | Use Qt LGPLv3 modules only; legal review of dynamic linking compliance.                        |
| R-05    | INT8 quantization causes accuracy regression >1%      | Medium      | Medium   | Maintain FP16 fallback models; A/B benchmark INT8 vs FP32 on every release.                    |
| R-06    | Contractor laptops lack AVX2 instructions (old CPUs)  | Low         | Medium   | Compile with SSE4.2 baseline; detect AVX2 at runtime for optimized code paths.                 |
| R-07    | macOS Metal API incompatibilities with ONNX Runtime   | Medium      | Medium   | Use CoreML EP on macOS; extensive beta testing on Apple Silicon and Intel Macs.                 |
| R-08    | FFmpeg EXIF parsing fails for certain smartphone vendors | Medium   | Low      | Expand camera intrinsics database; user manual input fallback always available.                 |

---

## 16. Appendices

### Appendix A: Directory Structure

```
tibyan/
├── CMakeLists.txt
├── README.md
├── PRD.md
├── TDD.md
├── src/
│   ├── main.cpp
│   ├── ui/
│   │   ├── MainWindow.h / .cpp
│   │   ├── ProjectDashboardView.h / .cpp
│   │   ├── VideoImportView.h / .cpp
│   │   ├── CalibrationView.h / .cpp
│   │   ├── ProcessingView.h / .cpp
│   │   ├── ResultsView.h / .cpp
│   │   ├── BOQEditorPanel.h / .cpp
│   │   ├── ExportView.h / .cpp
│   │   └── SettingsView.h / .cpp
│   ├── ingest/
│   │   ├── VideoIngestor.h / .cpp
│   │   └── IntrinsicsResolver.h / .cpp
│   ├── ai/
│   │   ├── InferenceEngine.h / .cpp
│   │   ├── SegmentationPipeline.h / .cpp
│   │   ├── DepthPipeline.h / .cpp
│   │   └── MaskRefiner.h / .cpp
│   ├── geometry/
│   │   ├── PointCloudProjector.h / .cpp
│   │   ├── SurfaceReconstructor.h / .cpp
│   │   ├── AreaCalculator.h / .cpp
│   │   └── ScaleCalibrator.h / .cpp
│   ├── boq/
│   │   ├── BOQGenerator.h / .cpp
│   │   └── PricingDatabase.h / .cpp
│   ├── export/
│   │   └── ExportManager.h / .cpp
│   ├── project/
│   │   └── ProjectManager.h / .cpp
│   └── orchestration/
│       └── PipelineOrchestrator.h / .cpp
├── models/
│   ├── dformer_segmentation_int8.onnx
│   └── depth_anything_v2_metric_int8.onnx
├── data/
│   ├── pricing.db                    (SQLite)
│   └── camera_intrinsics.json
├── tests/
│   ├── unit/
│   ├── integration/
│   ├── benchmarks/
│   └── test_data/
├── scripts/
│   ├── training/
│   ├── quantization/
│   └── download_models.py
├── resources/
│   ├── icons/
│   ├── translations/
│   │   ├── tibyan_en.ts
│   │   └── tibyan_ar.ts
│   └── sample_video/
└── docs/
    ├── architecture_diagrams/
    └── api_reference/
```

### Appendix B: Technology Version Matrix

| Technology     | Required Version | License        | Notes                            |
| :------------- | :--------------- | :------------- | :------------------------------- |
| C++ Standard   | C++17            | N/A            | MSVC 2022+ / Clang 15+ / GCC 12+|
| Qt             | 6.5+             | LGPLv3         | Dynamic linking required         |
| ONNX Runtime   | 1.16+            | MIT            |                                  |
| OpenCV         | 4.8+             | Apache 2.0     | core, imgproc, videoio modules   |
| Open3D         | 0.18+            | MIT            | C++ build                        |
| Eigen          | 3.4+             | MPL 2.0        | Header-only                      |
| FFmpeg         | 6.0+             | LGPL 2.1+      | Dynamic linking for LGPL compliance |
| TensorRT       | 8.6+             | NVIDIA EULA    | Optional (GPU only)              |
| OpenVINO       | 2023.2+          | Apache 2.0     | Optional (Intel optimization)    |
| SQLite         | 3.40+            | Public Domain  | Embedded pricing database        |
| libxlsxwriter  | 1.1+             | BSD 2-Clause   | Excel export                     |
| libharu        | 2.4+             | zlib License   | PDF export                       |
| Google Test     | 1.14+            | BSD 3-Clause   | Unit/integration testing         |

---

> **Document End** — This TDD shall be reviewed by the Engineering Lead and Architecture Board before development sprints commence. All module interfaces described herein are subject to refinement during Sprint 0 (Technical Spike).
