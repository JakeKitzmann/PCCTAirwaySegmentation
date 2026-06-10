#!/bin/bash
set -e

pixi install

# ── 4. Download atlas data (MIATT_ATLAS/) if not already present ──────────────
if [ ! -f "testingdata/Mask256.nii.gz" ]; then
    echo "Downloading testing data from Google Drive..."
    pixi run gdown --folder "https://drive.google.com/drive/folders/1sUJKmk5CPf6RAoGge5N0PPPEHwSTmJZY?usp=sharing"
else
    echo "Data already present, skipping download."
fi
