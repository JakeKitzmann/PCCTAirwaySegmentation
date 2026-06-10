# AnalyzeAirwaySeg — Lab Handoff

Evaluate airway segmentation quality directly inside 3D Slicer using the standard ATM22 challenge metrics (DSC, TD, BD, FPR). No external scripts, no manual environment setup.

---

## Setup

1. Clone or copy this repository to your machine.
2. Open 3D Slicer → **Edit → Application Settings → Modules**.
3. Under *Additional module paths*, add the **repo root** (the folder containing `AnalyzeAirwaySeg/`).
4. Restart Slicer. On first load, Slicer will automatically install the required `itk` dependency — this takes ~1 minute and only happens once.
5. Find the module under **Modules → Examples → AnalyzeAirwaySeg** (or search for it).

---

## How to use

1. Load your ground truth and prediction segmentation volumes into Slicer (File → Add Data, or drag-and-drop `.nii.gz` files).
2. In the module panel, set **Ground Truth** and **Prediction** to the corresponding volumes.
3. *(Optional)* Set **Downsample** if your volumes are large. This resamples the in-plane dimensions (x/y) to the chosen size while preserving slice thickness — useful if you run out of memory or computation is slow. Options: `128²`, `256²`, `512²`, or `None`.
4. *(Optional)* Enter a path in **CSV output** to save results to a file (e.g. `/Users/you/results.csv`). Leave blank to skip. If the file already exists, new rows are appended — so you can run multiple cases and collect all results in one file.
5. Click **Apply**.

Results appear in the **Results** panel. Three additional volumes are also added to the scene:
- **GT Skeleton** — centerline skeleton of the ground truth airway tree
- **Pred Skeleton** — centerline skeleton of the prediction
- **GT Tree Parsing** — ground truth skeleton labeled by branch

---

## What the metrics mean

| Metric | What it measures |
|--------|-----------------|
| **Dice Score (DSC)** | Voxel overlap between prediction and ground truth (higher = better; 100% = perfect) |
| **Tree Length Detected (TLD)** | Fraction of the GT centerline covered by the prediction — how much of the tree is detected at all |
| **Branches Detected (BD)** | Fraction of GT branches where ≥80% of the branch skeleton is covered — rewards completeness at the branch level |
| **False Positive Rate (FPR)** | Predicted voxels that fall outside the GT, relative to total background — penalizes over-segmentation |

These are the official metrics from the MICCAI 2022 ATM22 Airway Tree Modeling challenge. A good segmentation should have high DSC, TD, and BD, and low FPR.

---

## Test data

Sample volumes are in `AnalyzeAirwaySeg/testdata/`. Load `Force.nii.gz` as ground truth and `Force_pred.nii.gz` as prediction to verify the module is working correctly.

---

## Troubleshooting

**Module doesn't appear after restart** — double-check that you added the repo root (not the inner `AnalyzeAirwaySeg/` subfolder) to the module paths, and that Slicer fully restarted.

**pip install error on first load** — Slicer needs internet access to install `itk`. If you're on a restricted network, ask Jacob to provide a pre-installed environment.

**Out of memory / very slow** — enable Downsample at `256²` or `128²` before clicking Apply.

**Results look wrong** — confirm both volumes are binary (0/1 or 0/255) and are in the same voxel space. If they were segmented from the same CT they should already be registered.

---

## Reference

Zhang, M. et al. "Multi-site, Multi-domain Airway Tree Modeling." *MICCAI ATM22 Challenge*, 2022. Metric implementation by Minghui Zhang, Institute of Medical Robotics, Shanghai Jiao Tong University.
