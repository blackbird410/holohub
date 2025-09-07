cmake_minimum_required(VERSION 3.22)

# Add the application (binary = format_converter_app)
holoscan_add_application(
  NAME format_converter_app
  SOURCES
    src/main.cpp
    src/synthetic_source.cpp
    src/saver_op.cpp
)

# Add the test (reuses gtest from Holohub’s common infra)
if(HOLOHUB_BUILD_TESTS)
  add_subdirectory(tests)
endif()
