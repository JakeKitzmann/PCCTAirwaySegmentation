set(AirwayRegistration_SRC AirwayRegistration.cxx)
find_package(SlicerExecutionModel REQUIRED GenerateCLP)
include(${GenerateCLP_USE_FILE})
include(${SlicerExecutionModel_USE_FILE})
generateclp(AirwayRegistration_SRC AirwayRegistration.xml)

find_package(ITK 6.0.0 REQUIRED)
include(${ITK_USE_FILE})

add_executable(AirwayRegistration ${AirwayRegistration_SRC})
target_link_libraries(RunRegistration ${ITK_LIBRARIES})
