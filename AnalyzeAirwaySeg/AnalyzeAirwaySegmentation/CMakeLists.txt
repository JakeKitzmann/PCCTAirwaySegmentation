set(MODULE_NAME AnalyzeAirwaySegmentation)

add_custom_target(${MODULE_NAME} ALL
    COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_CURRENT_SOURCE_DIR}/${MODULE_NAME}.py
        ${CMAKE_CURRENT_BINARY_DIR}/${MODULE_NAME}.py
    COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_CURRENT_SOURCE_DIR}/${MODULE_NAME}.xml
        ${CMAKE_CURRENT_BINARY_DIR}/${MODULE_NAME}.xml
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/atm22
        ${CMAKE_CURRENT_BINARY_DIR}/atm22
    COMMENT "Staging AnalyzeAirwaySegmentation Python CLI"
)
