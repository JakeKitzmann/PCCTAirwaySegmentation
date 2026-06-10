set(RunRegistration_SRC RunRegistration.cxx)

find_package(SlicerExecutionModel REQUIRED GenerateCLP)
include(${GenerateCLP_USE_FILE})
include(${SlicerExecutionModel_USE_FILE})
generateclp(RunRegistration_SRC RunRegistration.xml)

find_package(ITK 6.0.0 REQUIRED)
include(${ITK_USE_FILE})

add_executable(RunRegistration ${RunRegistration_SRC})
target_link_libraries(RunRegistration ${ITK_LIBRARIES})