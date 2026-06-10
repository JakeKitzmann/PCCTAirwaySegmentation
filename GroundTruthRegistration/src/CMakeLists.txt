set(RunRegistration_SRC RunRegistration.cxx)
find_package(SlicerExecutionModel REQUIRED GenerateCLP)
include(${GenerateCLP_USE_FILE})
include(${SlicerExecutionModel_USE_FILE})
generateclp(RunRegistration_SRC RunRegistration.xml)

find_package(ITK 6.0.0 REQUIRED)
include(${ITK_USE_FILE})

add_executable(RunRegistration ${RunRegistration_SRC})
target_link_libraries(RunRegistration ${ITK_LIBRARIES})

configure_file(RunRegistration.xml ${CMAKE_CURRENT_BINARY_DIR}/RunRegistration.xml COPYONLY)
install(TARGETS RunRegistration DESTINATION bin)
install(FILES RunRegistration.xml DESTINATION bin)