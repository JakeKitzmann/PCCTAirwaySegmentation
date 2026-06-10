# AnalyzeAirwaySeg

A 3D Slicer scripted module that evaluates airway segmentation quality using the standard [ATM22 challenge](https://atm22.grand-challenge.org/) metrics — no external scripts or manual environment setup required.

![Module screenshot](AnalyzeAirwaySeg.png)

## Metrics

| Metric | Measures |
|--------|----------|
| **Dice Score (DSC)** | Voxel overlap — higher is better |
| **Tree Length Detected (TLD)** | Fraction of GT centerline covered by the prediction |
| **Branches Detected (BD)** | Fraction of GT branches ≥80% covered |
| **False Positive Rate (FPR)** | Over-segmentation penalty — lower is better |

## Quick start

1. Clone this repo.
2. In Slicer: **Edit → Application Settings → Modules** → add the **repo root** to *Additional module paths* → restart.
3. Load a ground truth and prediction `.nii.gz` into Slicer, select them in the module panel, click **Apply**.

On first load Slicer auto-installs the `itk` dependency (~1 min, once only).

## Documentation

See **[Handoff.md](Handoff.md)** for full setup instructions, usage guide, metric definitions, test data, and troubleshooting.

## Repository layout

```
AnalyzeAirwaySeg/
  AnalyzeAirwaySeg.py          # Module widget + logic
  atm22/
    tree_parse.py              # Airway tree parsing (vendored ATM22)
    evaluation_atm_22.py       # Metric calculations (vendored ATM22)
  Resources/UI/                # Qt .ui panel
  testdata/                    # Force.nii.gz + Force_pred.nii.gz sample data
```

## Reference

Zhang, M. et al. "Multi-site, Multi-domain Airway Tree Modeling." *MICCAI ATM22 Challenge*, 2022.
