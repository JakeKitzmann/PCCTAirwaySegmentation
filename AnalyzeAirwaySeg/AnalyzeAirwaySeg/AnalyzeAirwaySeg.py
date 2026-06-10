import os
import sys

import numpy as np
import slicer
from slicer.ScriptedLoadableModule import *


def _ensure_dependencies():
    for pkg, pip_name in [
        ("skimage", "scikit-image"),
        ("scipy", "scipy"),
        ("itk", "itk"),
    ]:
        try:
            __import__(pkg)
        except ImportError:
            slicer.util.pip_install(pip_name)


def _atm22_dir():
    return os.path.join(os.path.dirname(__file__), "atm22")


def _ensure_atm22_on_path():
    d = _atm22_dir()
    if d not in sys.path:
        sys.path.insert(0, d)


class AnalyzeAirwaySeg(ScriptedLoadableModule):
    def __init__(self, parent):
        ScriptedLoadableModule.__init__(self, parent)
        self.parent.title = "AnalyzeAirwaySegmentation"
        self.parent.categories = ["APPIL"]
        self.parent.dependencies = []
        self.parent.contributors = ["Jacob J. Kitzmann"]
        self.parent.helpText = ""
        self.parent.acknowledgementText = ""
        slicer.app.connect("startupCompleted()", _ensure_dependencies)


class AnalyzeAirwaySegWidget(ScriptedLoadableModuleWidget):
    def setup(self) -> None:
        ScriptedLoadableModuleWidget.setup(self)

        uiWidget = slicer.util.loadUI(self.resourcePath("UI/AnalyzeAirwaySeg.ui"))
        self.layout.addWidget(uiWidget)
        self.ui = slicer.util.childWidgetVariables(uiWidget)
        uiWidget.setMRMLScene(slicer.mrmlScene)

        self.logic = AnalyzeAirwaySegLogic()

        self.ui.groundTruthSelector.connect("currentNodeChanged(vtkMRMLNode*)", self._checkCanApply)
        self.ui.predictionSelector.connect("currentNodeChanged(vtkMRMLNode*)", self._checkCanApply)
        self.ui.applyButton.connect("clicked(bool)", self.onApplyButton)

    def cleanup(self) -> None:
        pass

    def _checkCanApply(self, _node=None) -> None:
        ready = (
            self.ui.groundTruthSelector.currentNode() is not None
            and self.ui.predictionSelector.currentNode() is not None
        )
        self.ui.applyButton.enabled = ready
        self.ui.applyButton.toolTip = (
            "Run the evaluation." if ready else "Select both Ground Truth and Prediction volumes."
        )

    def onApplyButton(self) -> None:
        with slicer.util.tryWithErrorDisplay("Failed to run evaluation.", waitCursor=True):
            gt_node = self.ui.groundTruthSelector.currentNode()
            pred_node = self.ui.predictionSelector.currentNode()
            downsample_text = self.ui.downsampleComboBox.currentText
            downsample_target = {"128²": 128, "256²": 256, "512²": 512}.get(downsample_text, None)
            metrics = self.logic.process(gt_node, pred_node, downsample_target=downsample_target)
            col = max(len(k) for k in metrics) + 2
            lines = [f"{k:<{col}}{v}" for k, v in metrics.items()]
            self.ui.resultsText.setPlainText("\n".join(lines))

            csv_path = self.ui.csvPathEdit.text.strip()
            if csv_path:
                self.logic.append_csv(
                    csv_path,
                    gt_node.GetName(),
                    pred_node.GetName(),
                    downsample_text,
                    metrics,
                )


class AnalyzeAirwaySegLogic(ScriptedLoadableModuleLogic):
    def __init__(self) -> None:
        ScriptedLoadableModuleLogic.__init__(self)

    def process(self, ground_truth_node, prediction_node, downsample_target=None) -> dict:
        _ensure_atm22_on_path()

        from skimage.morphology import skeletonize
        from tree_parse import (
            large_connected_domain, skeleton_parsing, tree_parsing_func,
            loc_trachea, adjacent_map, parent_children_map,
            whether_refinement, tree_refinement,
        )
        from evaluation_atm_22 import (
            branch_detected_calculation, dice_coefficient_score_calculation,
            tree_length_calculation, sensitivity_calculation, precision_calculation,
            false_positive_rate_calculation,
        )

        gt = (slicer.util.arrayFromVolume(ground_truth_node) > 0).astype(np.uint8)
        pred = (slicer.util.arrayFromVolume(prediction_node) > 0).astype(np.uint8)

        if downsample_target is not None:
            gt = self._downsample(gt, target=downsample_target)
            pred = self._downsample(pred, target=downsample_target)

        gt = large_connected_domain(gt)
        skeleton = skeletonize(gt).astype(np.uint8)
        skeleton_parse, cd, num = skeleton_parsing(skeleton)
        tree_parsing = tree_parsing_func(skeleton_parse, gt, cd)
        trachea = loc_trachea(tree_parsing, num)
        ad_matric = adjacent_map(tree_parsing, num)
        parent_map, children_map, _ = parent_children_map(ad_matric, trachea, num)
        while whether_refinement(parent_map, children_map, tree_parsing, num, trachea):
            tree_parsing, num = tree_refinement(parent_map, children_map, tree_parsing, num, trachea)
            trachea = loc_trachea(tree_parsing, num)
            ad_matric = adjacent_map(tree_parsing, num)
            parent_map, children_map, _ = parent_children_map(ad_matric, trachea, num)

        pred_skeleton = skeletonize(pred).astype(np.uint8)

        dsc = dice_coefficient_score_calculation(pred, gt)
        td = tree_length_calculation(pred, skeleton)
        total_b, det_b, bd = branch_detected_calculation(pred, tree_parsing, skeleton)
        fpr = false_positive_rate_calculation(pred, gt)

        self._add_volume(skeleton.astype(np.float32), "GT Skeleton", ground_truth_node)
        self._add_volume(pred_skeleton.astype(np.float32), "Pred Skeleton", prediction_node)
        self._add_volume(tree_parsing.astype(np.float32), "GT Tree Parsing", ground_truth_node)

        return {
            "Dice Score (DSC)":            f"{dsc:.2f}%",
            "Tree Length Detected (TLD)":  f"{td:.2f}%",
            "Branches Detected (BD)":      f"{bd:.2f}%  ({det_b}/{total_b} branches)",
            "False Positive Rate (FPR)":   f"{fpr:.3f}%",
        }

    def _downsample(self, array, target=256):
        import itk
        ImageType = itk.Image[itk.UC, 3]
        img = itk.image_from_array(array, ttype=ImageType)
        input_size = img.GetLargestPossibleRegion().GetSize()
        input_spacing = img.GetSpacing()

        output_size = itk.Size[3]()
        output_spacing = [
            input_spacing[i] * input_size[i] / target if i < 2 else input_spacing[i]
            for i in range(3)
        ]
        for i in range(3):
            output_size[i] = target if i < 2 else input_size[i]

        interpolator = itk.NearestNeighborInterpolateImageFunction[ImageType, itk.D].New()
        resampler = itk.ResampleImageFilter[ImageType, ImageType].New()
        resampler.SetInput(img)
        resampler.SetTransform(itk.IdentityTransform[itk.D, 3].New())
        resampler.SetInterpolator(interpolator)
        resampler.SetSize(output_size)
        resampler.SetOutputSpacing(output_spacing)
        resampler.SetOutputOrigin(img.GetOrigin())
        resampler.SetOutputDirection(img.GetDirection())
        resampler.Update()
        return itk.array_from_image(resampler.GetOutput()).astype(array.dtype)

    def append_csv(self, path, gt_name, pred_name, downsample, metrics):
        import csv
        import os
        abbrev = {
            "Dice Score (DSC)":           "DSC",
            "Tree Length Detected (TLD)": "TLD",
            "Branches Detected (BD)":     "BD",
            "False Positive Rate (FPR)":  "FPR",
        }
        fieldnames = ["gt", "prediction", "downsample", "TLD", "BD", "FPR", "DSC"]
        write_header = not os.path.exists(path)
        with open(path, "a", newline="") as f:
            writer = csv.DictWriter(f, fieldnames=fieldnames)
            if write_header:
                writer.writeheader()
            row = {"gt": gt_name, "prediction": pred_name, "downsample": downsample}
            row.update({abbrev[k]: v for k, v in metrics.items()})
            writer.writerow(row)

    def _add_volume(self, array, name, reference_node):
        node = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLScalarVolumeNode", name)
        slicer.util.updateVolumeFromArray(node, array)
        node.CopyOrientation(reference_node)
        slicer.util.setSliceViewerLayers(background=node, fit=True)
        return node


class AnalyzeAirwaySegTest(ScriptedLoadableModuleTest):
    def setUp(self):
        slicer.mrmlScene.Clear()

    def runTest(self):
        self.setUp()
