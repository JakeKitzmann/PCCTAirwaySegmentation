# PCCTAirwaySegmentation

Airway segmentation for photon-counting CT (PCCT) using a 2.5D Multi-View U-Net architecture.

## Architecture

Three 2D U-Nets — axial, coronal, sagittal — whose outputs are fused by a Virtual Fusion Network (VFN). This 2.5D approach captures volumetric context without full 3D convolutions.

## Repo Layout

```
cpp/        ITK-based preprocessing (CMake)
python/     Deep learning training and inference
```

## C++ / ITK Preprocessing

- Build system: **CMake**
- ITK built at `~/dev/ITK-build`
- SlicerExecutionModel at `~/dev/SEM-build`
- Standard configure + build pattern (see `AirwaySegmentationEvaluation` sibling repo for reference pixi tasks):
  ```
  cmake .. -DITK_DIR=~/dev/ITK-build -DSlicerExecutionModel_DIR=~/dev/SEM-build
  cmake --build .
  ```

## Python / Deep Learning

- Environment manager: **pixi** (`pixi.toml` at repo root, `pixi run <task>` to execute tasks)
- DL framework: PyTorch or Keras (TBD — update this when decided)
- Key dependencies to add: deep learning framework, nibabel, scikit-image, numpy

## Data

- Stored on the **LSS cluster** — do not commit data files
- Job scheduler: **SGE** (`qsub` for cluster jobs)
- Format: NIfTI (`.nii.gz`)

## Evaluation

Metrics used: Dice coefficient, Tree Length Detected (TLD), False Discovery Rate (FDR), centerline/branch detection.

The evaluation tooling lives in the sibling repo `../AirwaySegmentationEvaluation/` (a 3D Slicer extension). Run evaluations from there, not from this repo.

## Coding Conventions

- No enforced formatter yet — update this when one is chosen
- Keep C++ and Python cleanly separated in their respective subdirectories
- Cluster job scripts go in `scripts/` or alongside the code that uses them

## Workflow Notes

- Preprocessing (C++) outputs NIfTI files consumed by the Python training pipeline
- LSS cluster is the compute environment; develop locally, train remotely via SGE
