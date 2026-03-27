# Product Requirements Document (PRD)

## **Tibyan** — AI-Powered Interior Measurement & Automated Quantity Takeoff

| Field               | Detail                                                    |
| :------------------ | :-------------------------------------------------------- |
| **Document Version** | 1.0                                                       |
| **Date**            | 2026-03-26                                                |
| **Author**          | Product Management                                        |
| **Status**          | Draft — Pending Engineering & Stakeholder Review          |
| **Confidentiality** | Internal — Engineering & QA Distribution Only             |

---

## Table of Contents

1. [Executive Summary & Product Vision](#1-executive-summary--product-vision)
2. [Target Audience & User Personas](#2-target-audience--user-personas)
3. [Comprehensive Functional Requirements](#3-comprehensive-functional-requirements)
4. [Non-Functional Requirements](#4-non-functional-requirements)
5. [Out of Scope](#5-out-of-scope)
6. [Success Metrics (KPIs)](#6-success-metrics-kpis)
7. [Glossary](#7-glossary)

---

## 1. Executive Summary & Product Vision

### 1.1 Problem Statement

The interior finishing phase of construction projects is plagued by a systemic cost-estimation problem. Industry data consistently shows that **cost overruns exceeding 10%** are commonplace, primarily rooted in **inaccurate Bill of Quantities (BOQ)** generated through manual measurement processes. Today, quantity surveyors and finishing contractors rely on tape measures, laser distance meters, and hand-drawn sketches to quantify surfaces — a workflow that is:

- **Error-prone:** Human transcription mistakes, missed surfaces, and incorrect deductions for openings (doors, windows) cascade into material over- or under-ordering.
- **Time-consuming:** A single residential apartment (≈120 m²) can require 4–8 hours of on-site measurement and an additional 2–4 hours of office-based BOQ compilation.
- **Non-reproducible:** Results vary between surveyors; there is no auditable digital trail linking a measurement to its source.

Unfinished interior spaces (bare-shell / plaster stage) compound these challenges: surfaces lack color contrast, lighting is poor and inconsistent, and the environment is cluttered with scaffolding, wiring, and scattered materials.

### 1.2 Product Vision

**Tibyan** transforms a simple smartphone video walkthrough of an unfinished interior space into a fully itemized, priced Bill of Quantities — automatically. By fusing state-of-the-art semantic segmentation, monocular metric depth estimation, and 3D geometric reconstruction, Tibyan eliminates the manual measurement bottleneck and delivers:

- **Speed:** A 1-minute video processed in under 5 minutes on standard desktop hardware.
- **Accuracy:** Surface measurements within a **±2%** margin of error relative to physical ground truth.
- **Completeness:** Automatic detection and classification of walls, ceilings, floors, and openings (doors/windows), with net area calculations and wastage-adjusted material quantities.

### 1.3 Value Proposition

| Stakeholder              | Value Delivered                                                                                                  |
| :----------------------- | :--------------------------------------------------------------------------------------------------------------- |
| **Finishing Contractors** | Reduce estimation time by **≥80%**. Minimize material waste and costly re-orders. Win more bids with faster, more accurate quotes. |
| **Quantity Surveyors**    | Replace manual takeoff with an auditable, repeatable digital workflow. Increase throughput (projects per week).    |
| **Interior Design Firms** | Provide clients with instant, data-backed cost breakdowns during design consultations. Improve project budgeting. |
| **Project Owners**        | Greater transparency in cost estimation, reducing disputes and change-order frequency.                           |

---

## 2. Target Audience & User Personas

### 2.1 Persona 1: The Finishing Engineer (Primary)

| Attribute               | Detail                                                                                                       |
| :----------------------- | :----------------------------------------------------------------------------------------------------------- |
| **Role**                | Site engineer or foreman responsible for executing interior finishing works (plastering, tiling, painting).     |
| **Age Range**           | 28–45                                                                                                         |
| **Technical Proficiency** | **Moderate.** Comfortable with smartphones and basic office software (Excel, PDF viewers). No CAD or 3D modeling experience. Limited patience for complex software workflows. |
| **Pain Points**         | Manually measuring dozens of rooms per project; errors discovered only after material procurement; lack of a standard template for BOQ.  |
| **Goals**               | Record a quick video, get accurate measurements and a ready-to-use BOQ within minutes. Edit quantities if needed before sending to procurement. |
| **Environment**         | On-site with a smartphone; back at the office with a mid-range Windows laptop or desktop.                      |

> **UI/UX Implication:** The interface must follow a linear, wizard-style workflow (Import → Process → Review → Export). Minimize cognitive load. All labels must be clear and jargon-free. Provide real-time progress feedback during processing.

### 2.2 Persona 2: The Quantity Surveyor (Secondary)

| Attribute               | Detail                                                                                                       |
| :----------------------- | :----------------------------------------------------------------------------------------------------------- |
| **Role**                | Professional quantity surveyor preparing cost estimates for contractors or project owners.                      |
| **Age Range**           | 30–50                                                                                                         |
| **Technical Proficiency** | **High.** Proficient in Excel, familiar with CAD software and estimation tools. Expects precise control over outputs. |
| **Pain Points**         | Time pressure when handling multiple projects simultaneously; inconsistent measurement data from field teams; manual cross-checking of areas against drawings. |
| **Goals**               | Validate AI-generated measurements against known references; override individual line items; export BOQ in a format compatible with existing financial workflows. |
| **Environment**         | Office-based, dual-monitor setup, Windows workstation.                                                         |

> **UI/UX Implication:** Provide a detailed review mode with per-surface measurement breakdown, manual override inputs, and export customization. Support keyboard shortcuts for power users.

### 2.3 Persona 3: The Independent Contractor (Tertiary)

| Attribute               | Detail                                                                                                       |
| :----------------------- | :----------------------------------------------------------------------------------------------------------- |
| **Role**                | Small-scale independent contractor handling residential finishing projects (painting, flooring, gypsum board).  |
| **Age Range**           | 25–55                                                                                                         |
| **Technical Proficiency** | **Low to Moderate.** Primarily smartphone-oriented. May have limited experience with desktop applications.     |
| **Pain Points**         | No dedicated estimation staff; relies on rough guesses for material quantities; frequently over-purchases or under-purchases materials. |
| **Goals**               | A simple, fast tool that gives a material list with quantities and approximate costs, requiring minimal technical knowledge. |
| **Environment**         | Primarily on-site; uses a budget laptop or shared desktop computer.                                            |

> **UI/UX Implication:** The default "Quick Mode" must produce a usable BOQ with as few clicks as possible (ideally ≤3 steps: load video → process → export). Tooltips and contextual help must be available at every stage.

---

## 3. Comprehensive Functional Requirements

### 3.1 Input Module (FR-INP)

The Input Module is responsible for ingesting video files, validating their suitability, and extracting the metadata required by downstream AI and geometric processing stages.

#### 3.1.1 Video Ingestion

| Req. ID   | Requirement                                                                                                                                                  | Priority |
| :-------- | :----------------------------------------------------------------------------------------------------------------------------------------------------------- | :------- |
| FR-INP-01 | The system **SHALL** accept video files in the following container/codec combinations: **MP4 (H.264/H.265), MOV (H.264/H.265/ProRes), AVI (H.264), MKV (H.264/H.265)**. | P0       |
| FR-INP-02 | The system **SHALL** support video resolutions from **720p (1280×720)** up to **4K (3840×2160)**.                                                             | P0       |
| FR-INP-03 | The system **SHALL** support video frame rates from **24 FPS to 60 FPS**.                                                                                     | P0       |
| FR-INP-04 | The system **SHALL** impose a maximum single video file size limit of **4 GB** and a maximum duration of **10 minutes** per clip.                              | P1       |
| FR-INP-05 | The system **SHALL** provide a drag-and-drop zone and a file-browser dialog for video import.                                                                 | P0       |
| FR-INP-06 | Upon import, the system **SHALL** display a video preview thumbnail and key metadata (resolution, duration, frame rate, file size) in the UI.                  | P1       |

#### 3.1.2 Metadata Extraction (Camera Intrinsics)

| Req. ID   | Requirement                                                                                                                                                  | Priority |
| :-------- | :----------------------------------------------------------------------------------------------------------------------------------------------------------- | :------- |
| FR-INP-07 | The system **SHALL** use **FFmpeg** to parse EXIF/metadata embedded in the video container, extracting: **Camera Lens Model, Focal Length (fx, fy), Sensor Size, and Optical Center Coordinates (cx, cy)**. | P0       |
| FR-INP-08 | Using extracted focal length and sensor dimensions, the system **SHALL** automatically construct the **Camera Intrinsic Matrix (K)** as defined by the Pinhole Camera Model. | P0       |
| FR-INP-09 | If EXIF metadata is absent or incomplete, the system **SHALL** prompt the user to manually input the smartphone model from a pre-populated database of common smartphone camera intrinsics, or enter focal length values directly. | P0       |
| FR-INP-10 | The system **SHALL** maintain an updatable local database of camera intrinsic parameters for the **top 50 most common smartphone models** (by global market share). | P1       |

#### 3.1.3 Handling Adverse Capture Conditions

| Req. ID   | Requirement                                                                                                                                                  | Priority |
| :-------- | :----------------------------------------------------------------------------------------------------------------------------------------------------------- | :------- |
| FR-INP-11 | The system **SHALL** apply **adaptive histogram equalization (CLAHE)** to compensate for poor or uneven lighting in unfinished interiors.                      | P0       |
| FR-INP-12 | The system **SHALL** detect and flag severely under-exposed or over-exposed frames (mean luminance < 30 or > 225 on a 0–255 scale) and exclude them from processing with a user notification. | P1       |
| FR-INP-13 | For shaky footage, the system **SHALL** apply **Spatio-Temporal Filtering** to smooth depth estimates across sequential frames, ensuring temporal consistency and reducing jitter. | P0       |
| FR-INP-14 | The system **SHALL** sample frames at a configurable rate (default: **5 FPS**) to reduce computational load while maintaining sufficient spatial coverage, with an advanced option to adjust sampling rate (1–15 FPS). | P0       |

---

### 3.2 AI Processing Engine (FR-AI)

The AI Processing Engine is the computational core of the application. It executes two concurrent deep-learning inference pipelines — semantic segmentation and metric depth estimation — locally on the user's machine, with no cloud dependency.

#### 3.2.1 Runtime Architecture

| Req. ID   | Requirement                                                                                                                                                  | Priority |
| :-------- | :----------------------------------------------------------------------------------------------------------------------------------------------------------- | :------- |
| FR-AI-01  | All AI inference **SHALL** execute locally via **ONNX Runtime** embedded within the C++ application binary. No data shall be transmitted to external servers.   | P0       |
| FR-AI-02  | ONNX Runtime **SHALL** dynamically detect available hardware acceleration and route inference accordingly: **Intel CPU/iGPU → OpenVINO Execution Provider**; **NVIDIA GPU → TensorRT Execution Provider**; **Fallback → CPU (MLAS)**. | P0       |
| FR-AI-03  | All production models **SHALL** be exported to the **ONNX format** from their PyTorch training environment.                                                   | P0       |
| FR-AI-04  | Model weights **SHALL** be compressed from FP32 to **INT8** via **Post-Training Quantization (PTQ)** using Intel's NNCF/OpenVINO toolkit, targeting a **≈4× memory reduction** and **≈3.6× inference speedup** on CPU. | P0       |

#### 3.2.2 Semantic Segmentation Pipeline

| Req. ID   | Requirement                                                                                                                                                  | Priority |
| :-------- | :----------------------------------------------------------------------------------------------------------------------------------------------------------- | :------- |
| FR-AI-05  | The system **SHALL** employ a **Vision Transformer-based segmentation model (D-Former architecture)**, fine-tuned on construction-specific datasets (e.g., StructScan3D, Rohbau3D). | P0       |
| FR-AI-06  | The segmentation model **SHALL** classify every pixel in each processed frame into one of the following semantic categories: **Wall, Ceiling, Floor, Window Opening, Door Opening, Clutter/Ignore**. | P0       |
| FR-AI-07  | Raw segmentation masks **SHALL** be post-processed using **OpenCV morphological operations** (Opening to remove noise; Closing to fill micro-gaps) and **Dilation/Erosion** to smooth surface edges. | P0       |
| FR-AI-08  | The system **SHALL** apply **Elevation-based Filtering**: ceiling-wall intersection lines shall be detected as reliable, clutter-free geometric references and projected downward to infer true wall extents, bypassing floor-level clutter (scaffolding, materials, equipment). | P1       |

#### 3.2.3 Metric Depth Estimation Pipeline

| Req. ID   | Requirement                                                                                                                                                  | Priority |
| :-------- | :----------------------------------------------------------------------------------------------------------------------------------------------------------- | :------- |
| FR-AI-09  | The system **SHALL** employ the **Depth Anything V2 (Metric)** model, based on DPT architecture with DINOv2 encoder, fine-tuned for **metric (absolute) depth output** using techniques analogous to ZoeDepth's Adaptive Metric Binning. | P0       |
| FR-AI-10  | For each processed frame, the model **SHALL** output a dense depth map where every pixel holds an absolute distance value in **meters** from the camera sensor plane. | P0       |
| FR-AI-11  | Depth maps **SHALL** be temporally smoothed across sequential frames to mitigate frame-to-frame jitter (see FR-INP-13).                                        | P0       |
| FR-AI-12  | Target inference latency per frame for the depth model **SHALL** be **≤0.25 seconds** on a mid-range NVIDIA GPU (e.g., RTX 3060) at 1080p input resolution.    | P1       |

#### 3.2.4 Concurrent Execution

| Req. ID   | Requirement                                                                                                                                                  | Priority |
| :-------- | :----------------------------------------------------------------------------------------------------------------------------------------------------------- | :------- |
| FR-AI-13  | The Semantic Segmentation and Metric Depth Estimation pipelines **SHALL** execute **concurrently** on separate threads for each sampled frame, managed by `QThreadPool`. | P0       |
| FR-AI-14  | Pipeline results (segmentation mask + depth map) for each frame **SHALL** be synchronized before passing to the 3D Projection stage.                           | P0       |

---

### 3.3 Measurement & Quantification Module (FR-MES)

This module transforms 2D AI inference outputs into 3D geometric reconstructions and calculates accurate surface areas in metric units.

#### 3.3.1 2D-to-3D Projection

| Req. ID   | Requirement                                                                                                                                                  | Priority |
| :-------- | :----------------------------------------------------------------------------------------------------------------------------------------------------------- | :------- |
| FR-MES-01 | For each pixel classified by the segmentation mask, the system **SHALL** compute its real-world 3D coordinate `(X, Y, Z)` using the **Pinhole Camera Model inverse projection**: `[X, Y, Z]ᵀ = Z_depth · K⁻¹ · [u, v, 1]ᵀ`, executed via the **Eigen** linear algebra library in C++. | P0       |
| FR-MES-02 | The system **SHALL** generate **independent 3D Point Clouds** for each semantic category (walls, ceiling, floor) per processed frame.                          | P0       |
| FR-MES-03 | Point clouds from multiple frames **SHALL** be registered and merged into a unified per-surface reconstruction using frame-to-frame correspondence.            | P0       |

#### 3.3.2 Geometric Processing & Area Calculation

| Req. ID   | Requirement                                                                                                                                                  | Priority |
| :-------- | :----------------------------------------------------------------------------------------------------------------------------------------------------------- | :------- |
| FR-MES-04 | The system **SHALL** apply the **RANSAC (Random Sample Consensus)** algorithm to each surface point cloud for **planar fitting**, aggressively rejecting outlier points caused by camera distortion or segmentation noise. | P0       |
| FR-MES-05 | **Mesh Triangulation** (Delaunay or Poisson, via **Open3D**) **SHALL** be applied to the cleaned point clouds to generate continuous triangulated surfaces.     | P0       |
| FR-MES-06 | **Gross Surface Area** **SHALL** be calculated by summing the areas of all constituent triangles in the mesh, reported in **square meters (m²)**.              | P0       |
| FR-MES-07 | Void areas corresponding to **Window Openings** and **Door Openings** (as classified by the segmentation mask) **SHALL** be projected into the parent wall plane, and their areas **SHALL** be subtracted from the gross wall area to yield the **Net Wall Area**. | P0       |
| FR-MES-08 | To handle **geometric occlusion** (surfaces partially hidden by temporary obstacles), the system **SHALL** apply **Convex Hull** or **Oriented Bounding Box (OBB)** algorithms via Open3D's "Coherent Structure" methodology to complete the geometric polygon of a detected planar surface before subtracting architectural voids. | P1       |

#### 3.3.3 Scale Calibration & Reference Anchoring

| Req. ID   | Requirement                                                                                                                                                  | Priority |
| :-------- | :----------------------------------------------------------------------------------------------------------------------------------------------------------- | :------- |
| FR-MES-09 | The system **SHALL** provide a **"Reference Gauge"** feature in the UI allowing the user to input a single known physical dimension visible in the video (e.g., standard door width = 0.90 m, or a known camera-to-wall distance).  | P0       |
| FR-MES-10 | The Reference Gauge value **SHALL** be used to compute a **Global Scaling Factor**, which is multiplied across the entire depth map to correct for monocular scale ambiguity and convert all distances to absolute metric values. | P0       |
| FR-MES-11 | The system **SHALL** display a **calibration confidence indicator** (High / Medium / Low) based on the consistency between the EXIF-derived intrinsic matrix and the user-provided reference gauge. | P2       |

#### 3.3.4 Wastage Calculation

| Req. ID   | Requirement                                                                                                                                                  | Priority |
| :-------- | :----------------------------------------------------------------------------------------------------------------------------------------------------------- | :------- |
| FR-MES-12 | The system **SHALL** apply a **configurable Waste Factor** (default range: **5%–15%**) to each material line item, adjustable per material type and room complexity. | P0       |
| FR-MES-13 | Default waste factor values **SHALL** be pre-populated from an embedded database but **SHALL** be user-overridable at both the project level and individual line-item level. | P0       |
| FR-MES-14 | The wastage calculation formula **SHALL** be: `Required Quantity = Net Area × Material Consumption Rate × (1 + Waste Factor%)`.                                | P0       |

---

### 3.4 Export Module (FR-EXP)

The Export Module generates the final deliverables: structured, editable BOQ documents and summary reports.

#### 3.4.1 BOQ Generation

| Req. ID   | Requirement                                                                                                                                                  | Priority |
| :-------- | :----------------------------------------------------------------------------------------------------------------------------------------------------------- | :------- |
| FR-EXP-01 | The system **SHALL** generate a **Bill of Quantities (BOQ)** table organized by **trade categories**: Plastering, Flooring, Painting, Ceiling Works, and any user-defined custom categories. | P0       |
| FR-EXP-02 | Each BOQ line item **SHALL** include the following columns: `Item No. | Trade | Description | Unit | Net Quantity | Waste Factor (%) | Gross Quantity | Unit Rate | Total Cost`. | P0       |
| FR-EXP-03 | The BOQ **SHALL** include a **Summary Section** with: Total Net Area per surface type (m²), Total Estimated Cost, a per-trade cost breakdown, and a project-level Grand Total. | P0       |

#### 3.4.2 Export Formats

| Req. ID   | Requirement                                                                                                                                                  | Priority |
| :-------- | :----------------------------------------------------------------------------------------------------------------------------------------------------------- | :------- |
| FR-EXP-04 | The system **SHALL** export the BOQ in **Microsoft Excel format (.xlsx)** with formatted cells, formulas for subtotals/totals, and a professional header containing project metadata (project name, date, location, prepared by). | P0       |
| FR-EXP-05 | The system **SHALL** export the BOQ in **PDF format (.pdf)** with a print-ready layout, company logo placeholder, and page numbering.                          | P0       |
| FR-EXP-06 | The system **SHALL** export raw measurement data in **CSV format (.csv)** for integration with external estimation tools.                                      | P2       |

#### 3.4.3 Manual Override & Editing

| Req. ID   | Requirement                                                                                                                                                  | Priority |
| :-------- | :----------------------------------------------------------------------------------------------------------------------------------------------------------- | :------- |
| FR-EXP-07 | The system **SHALL** provide an **in-app BOQ editor** allowing users to: add, delete, or modify individual line items; override auto-calculated quantities; and adjust unit rates and waste factors — **before** exporting. | P0       |
| FR-EXP-08 | All user overrides **SHALL** be visually distinguished (e.g., highlighted cells) and logged in an audit trail within the project file.                          | P1       |
| FR-EXP-09 | The system **SHALL** support **Undo/Redo** (minimum 20 levels) for all manual edits within the BOQ editor.                                                    | P1       |

#### 3.4.4 Pricing Database

| Req. ID   | Requirement                                                                                                                                                  | Priority |
| :-------- | :----------------------------------------------------------------------------------------------------------------------------------------------------------- | :------- |
| FR-EXP-10 | The system **SHALL** include an embedded, user-editable **Pricing & Consumption Rates database** containing default unit prices and material consumption rates per trade (e.g., paint coverage in m²/liter, plaster consumption in kg/m²). | P0       |
| FR-EXP-11 | Users **SHALL** be able to import/export the pricing database as a **CSV or JSON file** for portability and backup.                                            | P1       |

---

### 3.5 Project Management (FR-PRJ)

| Req. ID   | Requirement                                                                                                                                                  | Priority |
| :-------- | :----------------------------------------------------------------------------------------------------------------------------------------------------------- | :------- |
| FR-PRJ-01 | The system **SHALL** support a **project-based workflow** where each project encapsulates one or more video files (rooms), their measurements, and the resulting BOQ. | P0       |
| FR-PRJ-02 | Projects **SHALL** be saved to disk in a proprietary format (`.tibyan`) containing all intermediate data (point clouds, masks, measurements) to allow re-opening without re-processing. | P1       |
| FR-PRJ-03 | The system **SHALL** provide a **Project Dashboard** displaying recent projects with status indicators (New, Processing, Complete).                             | P1       |

---

## 4. Non-Functional Requirements

### 4.1 Performance (NFR-PERF)

| Req. ID    | Requirement                                                                                                                                                  | Target                 |
| :--------- | :----------------------------------------------------------------------------------------------------------------------------------------------------------- | :--------------------- |
| NFR-PERF-01 | End-to-end processing time for a **1-minute video (1080p, 30 FPS)** on a system with an Intel Core i7 (12th Gen+) CPU and 16 GB RAM, **without** a dedicated GPU. | **≤ 8 minutes**        |
| NFR-PERF-02 | End-to-end processing time for a **1-minute video (1080p, 30 FPS)** on a system with an Intel Core i7 (12th Gen+) CPU, 16 GB RAM, **and** an NVIDIA RTX 3060 6 GB GPU. | **≤ 3 minutes**        |
| NFR-PERF-03 | The **GUI shall remain fully responsive** (≤100 ms input latency) during all background processing operations. UI thread shall never be blocked by inference tasks. | 100% responsiveness    |
| NFR-PERF-04 | Peak **RAM consumption** during processing shall not exceed **6 GB** (excluding OS and application baseline), achieved via INT8 quantized models and streaming frame processing. | ≤ 6 GB peak overhead   |
| NFR-PERF-05 | Application **cold start** (launch to interactive UI) shall complete in **≤ 5 seconds** on the minimum supported hardware.                                    | ≤ 5 seconds            |

### 4.2 Accuracy (NFR-ACC)

| Req. ID    | Requirement                                                                                                                                                  | Target                 |
| :--------- | :----------------------------------------------------------------------------------------------------------------------------------------------------------- | :--------------------- |
| NFR-ACC-01 | Individual surface measurements (wall, ceiling, floor areas) **SHALL** achieve an accuracy of **±2%** relative to physical ground-truth measurements when a valid Reference Gauge is provided. | ±2% margin of error    |
| NFR-ACC-02 | Semantic segmentation **SHALL** achieve a **mean Intersection over Union (mIoU) ≥ 85%** on the target construction-environment validation dataset.             | mIoU ≥ 85%             |
| NFR-ACC-03 | Opening detection (doors/windows) **SHALL** correctly identify **≥ 90%** of openings visible in the video with an area estimation accuracy of **±5%**.          | ≥ 90% recall, ±5% area |
| NFR-ACC-04 | Without a Reference Gauge, the system **SHALL** still produce measurements with **±8%** accuracy using EXIF-derived intrinsics alone, with a clear UI warning. | ±8% uncalibrated       |

### 4.3 Compatibility & Platform Support (NFR-COMPAT)

| Req. ID      | Requirement                                                                                                                                                  | Target                 |
| :----------- | :----------------------------------------------------------------------------------------------------------------------------------------------------------- | :--------------------- |
| NFR-COMPAT-01 | The application **SHALL** support **Windows 10 (64-bit) and later** (Windows 11).                                                                            | Primary Platform       |
| NFR-COMPAT-02 | The application **SHALL** support **macOS 12 (Monterey) and later** (Apple Silicon and Intel).                                                                | Secondary Platform     |
| NFR-COMPAT-03 | Linux support is **deferred** to a future release and is not a requirement for v1.0.                                                                          | Deferred               |

### 4.4 Minimum Hardware Requirements

| Component     | Minimum Specification                                         | Recommended Specification                                        |
| :------------ | :------------------------------------------------------------ | :--------------------------------------------------------------- |
| **CPU**       | Intel Core i5 (10th Gen) / AMD Ryzen 5 3600 or equivalent     | Intel Core i7 (12th Gen+) / AMD Ryzen 7 5800X or equivalent      |
| **RAM**       | 8 GB DDR4                                                     | 16 GB DDR4 / DDR5                                                 |
| **GPU**       | None (CPU-only mode supported via OpenVINO)                   | NVIDIA RTX 3060 6 GB or higher (CUDA 11.8+, TensorRT compatible) |
| **Storage**   | 2 GB free disk space for application + models                 | SSD with ≥ 10 GB free space for projects and temporary data       |
| **Display**   | 1366 × 768 minimum                                           | 1920 × 1080 (Full HD) or higher                                  |

### 4.5 Security & Privacy (NFR-SEC)

| Req. ID    | Requirement                                                                                                                                                  | Priority |
| :--------- | :----------------------------------------------------------------------------------------------------------------------------------------------------------- | :------- |
| NFR-SEC-01 | **All processing SHALL occur locally** on the user's machine. No video, image, depth data, or measurement data shall be transmitted to any external server.    | P0       |
| NFR-SEC-02 | Project files (`.tibyan`) **SHALL** be stored unencrypted by default, with an optional password-protection feature for sensitive commercial projects.           | P2       |

### 4.6 Usability (NFR-USE)

| Req. ID    | Requirement                                                                                                                                                  | Priority |
| :--------- | :----------------------------------------------------------------------------------------------------------------------------------------------------------- | :------- |
| NFR-USE-01 | A first-time user with moderate technical proficiency **SHALL** be able to complete a full workflow (import → process → export) within **15 minutes** without external documentation, guided by the in-app wizard. | P0       |
| NFR-USE-02 | The application **SHALL** support **English and Arabic** UI localization in v1.0.                                                                              | P1       |
| NFR-USE-03 | The application **SHALL** include an **interactive onboarding tutorial** on first launch demonstrating the core workflow with a sample video.                   | P1       |

### 4.7 Reliability (NFR-REL)

| Req. ID    | Requirement                                                                                                                                                  | Priority |
| :--------- | :----------------------------------------------------------------------------------------------------------------------------------------------------------- | :------- |
| NFR-REL-01 | The application **SHALL** auto-save project state every **60 seconds** during processing to prevent data loss from crashes.                                    | P1       |
| NFR-REL-02 | In the event of a crash during AI processing, the application **SHALL** recover gracefully and offer to resume from the last completed frame batch.            | P1       |

---

## 5. Out of Scope

The following capabilities are **explicitly excluded** from the v1.0 release to prevent scope creep and maintain engineering focus:

| Exclusion                                          | Rationale                                                                                                   |
| :------------------------------------------------- | :---------------------------------------------------------------------------------------------------------- |
| **Structural Analysis & Load Calculations**        | Tibyan is a measurement and estimation tool, not a structural engineering platform. Structural analysis requires certified FEA solvers and licensed professional engineers. |
| **Finite Element Analysis (FEA) Integration**      | Integration with tools such as **ANSYS, SAP2000, ETABS, or ABAQUS** is entirely out of scope. These tools serve a fundamentally different engineering domain (structural/mechanical analysis). |
| **Cloud-Based Processing**                         | All inference is local. No cloud infrastructure, user accounts, or server-side processing will be developed for v1.0. |
| **Real-Time / Live Camera Processing**             | The system processes pre-recorded video files only. Live camera streaming and real-time AR overlays are excluded. |
| **BIM (Building Information Modeling) Export**      | Export to IFC, Revit (.rvt), or other BIM formats is deferred to a future release.                           |
| **Exterior Façade Measurement**                    | The AI models are trained exclusively on indoor construction environments. Exterior surface measurement is not supported. |
| **MEP (Mechanical, Electrical, Plumbing) Takeoff** | Identification and quantification of MEP elements (pipes, ducts, wiring runs) is excluded.                   |
| **Multi-Story / Full-Building Aggregation**        | Each video/room is processed independently. Cross-room aggregation and full-building summaries are deferred.  |
| **Mobile Application**                             | Tibyan v1.0 is a desktop-only application. A mobile companion app is a potential future product.             |

---

## 6. Success Metrics (KPIs)

### 6.1 Technical KPIs (Measured Pre-Launch via QA Testing)

| KPI                                    | Target                  | Measurement Method                                                                 |
| :------------------------------------- | :---------------------- | :--------------------------------------------------------------------------------- |
| **Measurement Accuracy (Calibrated)**  | ±2% vs. ground truth    | Benchmark suite of 50+ rooms with laser-measured ground truth; mean absolute error. |
| **Segmentation mIoU**                  | ≥ 85%                   | Evaluated on a held-out validation set of 500+ annotated construction images.       |
| **Processing Throughput (GPU)**        | ≤ 3 min / 1-min video   | Automated performance benchmark on RTX 3060 reference hardware.                    |
| **Processing Throughput (CPU-only)**   | ≤ 8 min / 1-min video   | Automated performance benchmark on Core i7-12700 reference hardware.               |
| **Application Crash Rate**            | < 0.5% of sessions      | Telemetry from internal beta testing (100+ hours of usage).                         |
| **GUI Responsiveness During Processing** | 0 UI-blocking events   | Automated UI thread monitoring during stress tests.                                |

### 6.2 Commercial KPIs (Measured Post-Launch — First 12 Months)

| KPI                                    | Target                  | Measurement Method                                                                 |
| :------------------------------------- | :---------------------- | :--------------------------------------------------------------------------------- |
| **Paid License Activations**           | ≥ 500 in Year 1         | License management server / activation records.                                    |
| **User Retention (Month-3)**           | ≥ 60%                   | Percentage of users active 90 days after first activation.                          |
| **Average Estimation Time Reduction**  | ≥ 80% vs. manual        | User survey comparing Tibyan workflow time vs. previous manual process.             |
| **Net Promoter Score (NPS)**           | ≥ 40                    | In-app survey at Day 30.                                                           |
| **Customer Support Ticket Volume**     | ≤ 5 tickets / 100 users / month | Support ticketing system metrics.                                           |
| **BOQ Accuracy User Satisfaction**     | ≥ 85% report "Accurate" or "Very Accurate" | In-app feedback prompt after each export.                           |

---

## 7. Glossary

| Term                        | Definition                                                                                                        |
| :-------------------------- | :---------------------------------------------------------------------------------------------------------------- |
| **AEC**                     | Architecture, Engineering, and Construction — the broad industry sector.                                          |
| **BOQ**                     | Bill of Quantities — an itemized list of materials, parts, and labor with associated costs for a construction project. |
| **Camera Intrinsic Matrix (K)** | A 3×3 matrix encoding the internal parameters of a camera (focal lengths, optical center) used for 3D projection.   |
| **CLAHE**                   | Contrast Limited Adaptive Histogram Equalization — an image enhancement technique.                                |
| **D-Former**                | A Vision Transformer-based architecture for semantic segmentation, adapted for construction environments.          |
| **Depth Anything V2**       | A state-of-the-art monocular depth estimation model producing metric (absolute) depth maps.                       |
| **EXIF**                    | Exchangeable Image File Format — metadata standard for images/videos containing camera and capture parameters.     |
| **FEA**                     | Finite Element Analysis — a computational method for structural/mechanical simulation (out of scope).             |
| **INT8 Quantization**       | A model compression technique reducing floating-point weights from 32-bit to 8-bit integers.                      |
| **mIoU**                    | Mean Intersection over Union — a standard metric for evaluating semantic segmentation accuracy.                    |
| **Monocular Depth Estimation** | The task of predicting depth (distance) from a single 2D camera image, without stereo or LiDAR input.           |
| **NNCF**                    | Neural Network Compression Framework — Intel's toolkit for model quantization and optimization.                   |
| **OBB**                     | Oriented Bounding Box — a minimum-area rotated rectangle enclosing a set of points.                               |
| **ONNX Runtime**            | An open-source inference engine for deploying pre-trained models across hardware platforms.                        |
| **OpenVINO**                | Intel's toolkit for optimizing and deploying AI inference on Intel hardware.                                       |
| **Pinhole Camera Model**    | A mathematical model describing the geometric relationship between 3D world points and their 2D image projections. |
| **PTQ**                     | Post-Training Quantization — compressing a trained model without re-training.                                     |
| **RANSAC**                  | Random Sample Consensus — an iterative algorithm for robust model fitting in the presence of outliers.            |
| **Scale Ambiguity**         | The inherent inability of a monocular camera to determine absolute distances without additional reference information. |
| **Semantic Segmentation**   | Pixel-level classification of an image into predefined categories (e.g., wall, floor, ceiling).                   |
| **TensorRT**                | NVIDIA's SDK for high-performance deep learning inference optimization on NVIDIA GPUs.                            |
| **Waste Factor**            | A percentage added to net material quantities to account for cutting losses, spillage, and installation inefficiencies. |
| **ZoeDepth**                | A technique for converting relative depth predictions into metric (absolute) depth using adaptive binning.         |

---

> **Document End** — This PRD shall be reviewed by the Engineering Lead, QA Lead, and Product Owner before transitioning to the Technical Design Document (TDD) phase.
