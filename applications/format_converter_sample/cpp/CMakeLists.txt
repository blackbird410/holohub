cmake_minimum_required(VERSION 3.22)

# Add the application (binary = format_converter_app)
holoscan_add_application(
  NAME format_converter_app
  SOURCES
    src/main.cpp
    # src/synthetic_source.cpp
    # src/saver_op.cpp
)

# Ensure binary lands where Holohub expects it
set(BIN_DIR ${CMAKE_CURRENT_BINARY_DIR}/applications/format_converter_sample/cpp)
file(MAKE_DIRECTORY ${BIN_DIR})

set_target_properties(format_converter_app PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${BIN_DIR}
)

# Add the test (reuses gtest from Holohub’s common infra)
if(HOLOHUB_BUILD_TESTS)
  add_subdirectory(tests)
endif()
