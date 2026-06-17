set(GroundTruthRegistration_SRC GroundTruthRegistration.cxx)
find_package(SlicerExecutionModel REQUIRED GenerateCLP)
include(${GenerateCLP_USE_FILE})
include(${SlicerExecutionModel_USE_FILE})
generateclp(GroundTruthRegistration_SRC GroundTruthRegistration.xml)

find_package(ITK 6.0.0 REQUIRED)
include(${ITK_USE_FILE})

add_executable(GroundTruthRegistration ${GroundTruthRegistration_SRC})
target_link_libraries(GroundTruthRegistration ${ITK_LIBRARIES})