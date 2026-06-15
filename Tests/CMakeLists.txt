find_package(ITK 6.0.0 REQUIRED)
include(${ITK_USE_FILE})

enable_testing()

add_executable(
	RegistrationTest 
	GroundTruthRegistrationTest.cxx	  
)
target_link_libraries(
  RegistrationTest
  GTest::gtest_main
  ${ITK_LIBRARIES}
)

include(GoogleTest)
gtest_discover_tests(RegistrationTest)
