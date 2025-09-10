cmake_minimum_required(VERSION 3.20)
project(format_converter_sample CXX)

find_package(holoscan 1.0 REQUIRED CONFIG
  PATHS "/opt/nvidia/holoscan" "/workspace/holoscan-sdk/install")

add_executable(${PROJECT_NAME}
  main.cpp
)

target_link_libraries(${PROJECT_NAME}
  PRIVATE
  holoscan::core
  holoscan::ops::format_converter
)

if(BUILD_TESTING)
  enable_testing()
  add_subdirector(tests)
endif()

# Install application into the install/ directory for packaging
install(
  TARGETS ${PROJECT_NAME}
  DESTINATION bin/${PROJECT_NAME}/cpp
)
