set(AnalyzeAirwaySegmentation_SRC AnalyzeAirwaySegmentation.cxx)
find_package(SlicerExecutionModel REQUIRED GenerateCLP)
include(${GenerateCLP_USE_FILE})
include(${SlicerExecutionModel_USE_FILE})
generateclp(AnalyzeAirwaySegmentation_SRC AnalyzeAirwaySegmentation.xml)

find_package(ITK 6.0.0 REQUIRED)
include(${ITK_USE_FILE})

add_executable(AnalyzeAirwaySegmentation ${AnalyzeAirwaySegmentation_SRC})
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
