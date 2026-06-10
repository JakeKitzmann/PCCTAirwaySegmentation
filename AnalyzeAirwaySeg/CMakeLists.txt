cmake_minimum_required(VERSION 3.20.6...3.22.6 FATAL_ERROR)

project(AnalyzeAirwaySeg)

#-----------------------------------------------------------------------------
# Extension meta-information
set(EXTENSION_HOMEPAGE "https://www.slicer.org/wiki/Documentation/Nightly/Extensions/AnalyzeAirwaySeg")
set(EXTENSION_CONTRIBUTORS "Jacob J. Kitzmann (The University of Iowa)")
set(EXTENSION_DESCRIPTION "TLD, BD, and FPR of existing airway segmentation to ground truth")
set(EXTENSION_ICONURL "https://www.example.com/Slicer/Extensions/AnalyzeAirwaySeg.png")
set(EXTENSION_SCREENSHOTURLS "https://www.example.com/Slicer/Extensions/AnalyzeAirwaySeg/Screenshots/1.png")
set(EXTENSION_DEPENDS "NA") # Specified as a list or "NA" if no dependencies

#-----------------------------------------------------------------------------
# Extension dependencies
find_package(Slicer REQUIRED)
include(${Slicer_USE_FILE})

#-----------------------------------------------------------------------------
# Extension modules
add_subdirectory(AnalyzeAirwaySeg)
## NEXT_MODULE

#-----------------------------------------------------------------------------
include(${Slicer_EXTENSION_GENERATE_CONFIG})
include(${Slicer_EXTENSION_CPACK})
