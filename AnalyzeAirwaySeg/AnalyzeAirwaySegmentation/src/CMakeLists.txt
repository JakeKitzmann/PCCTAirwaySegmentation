set(AnalyzeAirwaySegmentation_SRC AnalyzeAirwaySegmentation.cxx)

# Prefer the pixi-managed Python so nibabel/scikit-image are available even
# when the binary is launched from Slicer (which overrides PYTHONHOME/PYTHONPATH).
find_program(PIXI_PYTHON python3
    PATHS "${CMAKE_SOURCE_DIR}/../.pixi/envs/default/bin"
    NO_DEFAULT_PATH
)
if(NOT PIXI_PYTHON)
    find_program(PIXI_PYTHON python3)
    message(WARNING "pixi Python not found; falling back to system python3 (${PIXI_PYTHON}). nibabel may be missing.")
endif()
message(STATUS "AnalyzeAirwaySegmentation will use Python: ${PIXI_PYTHON}")

find_package(SlicerExecutionModel REQUIRED GenerateCLP)
include(${GenerateCLP_USE_FILE})
include(${SlicerExecutionModel_USE_FILE})
generateclp(AnalyzeAirwaySegmentation_SRC AnalyzeAirwaySegmentation.xml)

find_package(ITK 6.0.0 REQUIRED)
include(${ITK_USE_FILE})

add_executable(AnalyzeAirwaySegmentation ${AnalyzeAirwaySegmentation_SRC})
target_compile_definitions(AnalyzeAirwaySegmentation PRIVATE PYTHON_EXECUTABLE="${PIXI_PYTHON}")
target_link_libraries(AnalyzeAirwaySegmentation ${ITK_LIBRARIES})

add_custom_command(TARGET AnalyzeAirwaySegmentation POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_CURRENT_SOURCE_DIR}/AnalyzeAirwaySegmentation.xml
        $<TARGET_FILE_DIR:AnalyzeAirwaySegmentation>/AnalyzeAirwaySegmentation.xml
)

add_custom_command(TARGET AnalyzeAirwaySegmentation POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_CURRENT_SOURCE_DIR}/../evaluate.py
        $<TARGET_FILE_DIR:AnalyzeAirwaySegmentation>/evaluate.py
)

add_custom_command(TARGET AnalyzeAirwaySegmentation POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/../atm22
        $<TARGET_FILE_DIR:AnalyzeAirwaySegmentation>/atm22
)
