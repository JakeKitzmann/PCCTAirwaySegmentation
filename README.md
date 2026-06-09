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
cpp/        ITK-based preprocessing (CMake)
python/     Deep learning training and inference
scripts/    Cluster job scripts (SGE)
```

## Getting Started

### Prerequisites

- [ITK](https://itk.org/) built at `~/dev/ITK-build`
- [SlicerExecutionModel](https://github.com/Slicer/SlicerExecutionModel) built at `~/dev/SEM-build`
- [pixi](https://pixi.sh) for Python environment management
- CMake ≥ 3.16

### Build C++ Preprocessing

```bash
cd cpp
mkdir build && cd build
cmake .. -DITK_DIR=~/dev/ITK-build -DSlicerExecutionModel_DIR=~/dev/SEM-build
cmake --build .
```

### Python Environment

```bash
pixi install
pixi run <task>
```

## Data

Data is stored on the LSS cluster. NIfTI (`.nii.gz`) is the standard format throughout the pipeline. Do not commit data files to this repo.

Cluster jobs are submitted via **SGE** (`qsub`).

## Pipeline Overview

1. **Preprocessing** (C++ / ITK) — resampling, normalization, patch extraction from raw PCCT volumes
2. **Training** (Python / DL) — train axial, coronal, and sagittal U-Nets independently, then train the VFN
3. **Inference** — run all three U-Nets and the VFN on new volumes
4. **Evaluation** — metrics computed via [`AirwaySegmentationEvaluation`](../AirwaySegmentationEvaluation/)

## Evaluation Metrics

| Metric | Description |
|---|---|
| Dice coefficient | Volumetric overlap accuracy |
| Tree Length Detected (TLD) | Airway tree completeness |
| False Discovery Rate (FDR) | Over-segmentation measure |
| Centerline / branch detection | Graph-based airway topology accuracy |

Evaluation tooling lives in the sibling repo `../AirwaySegmentationEvaluation/`.
