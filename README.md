# PCCT Airway Segmentation

Airway segmentation for photon-counting CT (PCCT) using a **2.5D Multi-View U-Net** architecture. Three 2D U-Nets — trained on axial, coronal, and sagittal planes — are fused by a Virtual Fusion Network (VFN) to produce volumetric airway segmentations without the memory cost of full 3D convolutions.

## Architecture

```
Axial slices    → U-Net A ─┐
Coronal slices  → U-Net C ──→ Virtual Fusion Network → Segmentation
Sagittal slices → U-Net S ─┘
```

Each U-Net learns plane-specific features. The VFN learns to combine the three prediction volumes into a final airway mask.

## Repo Layout

```
GroundTruthRegistration/   ITK-based registration of low/high-res CT ground truth masks (C++/CMake)
GetAeroPathData/           Notebook to download the AeroPath public dataset
AnalyzeAirwaySeg/          3D Slicer extension for airway segmentation analysis
TestData/                  Small NIfTI test volumes (256³ and 512³)
QualifyingExam/            Qualifying exam writeup
build/                     CMake out-of-source build directory (git-ignored)
pixi.toml                  Pixi environment and task definitions
SetupDevelopment.sh        One-shot setup script (pixi install + test data download)
CMakeLists.txt             Root CMake (adds GroundTruthRegistration)
```

## Getting Started

### Prerequisites

- [pixi](https://pixi.sh) for environment and task management
- [ITK](https://itk.org/) built at `~/dev/ITK-build`
- [SlicerExecutionModel](https://github.com/Slicer/SlicerExecutionModel) built at `~/dev/SEM-build`
- CMake ≥ 3.22

### Setup

```bash
./SetupDevelopment.sh
```

This runs `pixi install` and downloads test data from Google Drive if not already present.

### Build C++ (GroundTruthRegistration)

```bash
pixi run build
```

This runs CMake configure + build from the `build/` directory using the ITK and SlicerExecutionModel paths above. Equivalent to:

```bash
cd build
cmake .. -DITK_DIR=~/dev/ITK-build -DSlicerExecutionModel_DIR=~/dev/SEM-build
cmake --build .
```

### Test Registration

```bash
pixi run test
```

Runs `RunRegistration` on the 256³ and 512³ test volumes in `TestData/`.

### Python / Jupyter

```bash
pixi run jupyterlab
```

Key Python dependencies (managed by pixi): `numpy`, `matplotlib`, `pandas`, `simpleitk`, `itk`, `gdown`.

## Data

Data is stored on the LSS cluster. NIfTI (`.nii.gz`) is the standard format throughout the pipeline. Do not commit data files to this repo.

Cluster jobs are submitted via **SGE** (`qsub`).

## Pipeline Overview

1. **Ground Truth Registration** (C++ / ITK) — registers low-res airway masks to high-res PCCT volumes (`GroundTruthRegistration/`)
2. **Training** (Python / DL) — train axial, coronal, and sagittal U-Nets independently, then train the VFN *(in development)*
3. **Inference** — run all three U-Nets and the VFN on new volumes *(in development)*
4. **Evaluation** — metrics computed via [`AirwaySegmentationEvaluation`](../AirwaySegmentationEvaluation/) or the `AnalyzeAirwaySeg` Slicer extension

## Evaluation Metrics

| Metric | Description |
|---|---|
| Dice coefficient | Volumetric overlap accuracy |
| Tree Length Detected (TLD) | Airway tree completeness |
| False Discovery Rate (FDR) | Over-segmentation measure |
| Centerline / branch detection | Graph-based airway topology accuracy |

