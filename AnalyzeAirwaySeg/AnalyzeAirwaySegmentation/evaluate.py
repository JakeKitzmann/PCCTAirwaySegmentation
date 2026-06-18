#!/usr/bin/env python3
import sys
import os
import argparse
import numpy as np


def main():
    parser = argparse.ArgumentParser(description="ATM'22 airway segmentation evaluation")
    parser.add_argument('--segmentationGT', required=True)
    parser.add_argument('--segmentationPred', required=True)
    parser.add_argument('--skeletonGT', required=True)
    parser.add_argument('--skeletonPred', required=True)
    parser.add_argument('--labeledGT', required=True)
    parser.add_argument('--labeledPred', required=True)
    args = parser.parse_args()

    script_dir = os.path.dirname(os.path.abspath(__file__))
    sys.path.insert(0, os.path.join(script_dir, 'atm22'))

    import nibabel as nib
    from skimage.morphology import skeletonize
    from tree_parse import (
        large_connected_domain, skeleton_parsing, tree_parsing_func,
        loc_trachea, adjacent_map, parent_children_map,
        whether_refinement, tree_refinement,
    )
    from evaluation_atm_22 import (
        branch_detected_calculation, dice_coefficient_score_calculation,
        tree_length_calculation, false_positive_rate_calculation,
    )

    gt_nib = nib.load(args.segmentationGT)
    pred_nib = nib.load(args.segmentationPred)

    gt_orig = (gt_nib.get_fdata() > 0).astype(np.uint8)
    pred_orig = (pred_nib.get_fdata() > 0).astype(np.uint8)

    # GT tree parsing
    gt = large_connected_domain(gt_orig)
    skeleton = skeletonize(gt).astype(np.uint8)
    skel_parse, cd, num = skeleton_parsing(skeleton)
    tree_parsing = tree_parsing_func(skel_parse, gt, cd)
    trachea = loc_trachea(tree_parsing, num)
    ad = adjacent_map(tree_parsing, num)
    parent_map, children_map, _ = parent_children_map(ad, trachea, num)
    while whether_refinement(parent_map, children_map, tree_parsing, num, trachea):
        tree_parsing, num = tree_refinement(parent_map, children_map, tree_parsing, num, trachea)
        trachea = loc_trachea(tree_parsing, num)
        ad = adjacent_map(tree_parsing, num)
        parent_map, children_map, _ = parent_children_map(ad, trachea, num)

    # Prediction tree parsing
    pred = large_connected_domain(pred_orig)
    pred_skeleton = skeletonize(pred).astype(np.uint8)
    pred_skel_parse, pred_cd, pred_num = skeleton_parsing(pred_skeleton)
    pred_tree_parsing = tree_parsing_func(pred_skel_parse, pred, pred_cd)
    pred_trachea = loc_trachea(pred_tree_parsing, pred_num)
    pred_ad = adjacent_map(pred_tree_parsing, pred_num)
    pred_parent, pred_children, _ = parent_children_map(pred_ad, pred_trachea, pred_num)
    while whether_refinement(pred_parent, pred_children, pred_tree_parsing, pred_num, pred_trachea):
        pred_tree_parsing, pred_num = tree_refinement(pred_parent, pred_children, pred_tree_parsing, pred_num, pred_trachea)
        pred_trachea = loc_trachea(pred_tree_parsing, pred_num)
        pred_ad = adjacent_map(pred_tree_parsing, pred_num)
        pred_parent, pred_children, _ = parent_children_map(pred_ad, pred_trachea, pred_num)

    # Metrics (against original pred, matching Python module behavior)
    dsc = dice_coefficient_score_calculation(pred_orig, gt)
    td = tree_length_calculation(pred_orig, skeleton)
    total_b, det_b, bd = branch_detected_calculation(pred_orig, tree_parsing, skeleton)
    fpr = false_positive_rate_calculation(pred_orig, gt)

    # Save image outputs
    nib.save(nib.Nifti1Image(skeleton, gt_nib.affine), args.skeletonGT)
    nib.save(nib.Nifti1Image(pred_skeleton, pred_nib.affine), args.skeletonPred)
    nib.save(nib.Nifti1Image(tree_parsing.astype(np.uint16), gt_nib.affine), args.labeledGT)
    nib.save(nib.Nifti1Image(pred_tree_parsing.astype(np.uint16), pred_nib.affine), args.labeledPred)

    col = 28
    print(f"{'Dice Score (DSC)':<{col}}{dsc:.2f}%")
    print(f"{'Tree Length Detected (TLD)':<{col}}{td:.2f}%")
    print(f"{'Branches Detected (BD)':<{col}}{bd:.2f}%  ({det_b}/{total_b} branches)")
    print(f"{'False Positive Rate (FPR)':<{col}}{fpr:.3f}%")


if __name__ == '__main__':
    main()
