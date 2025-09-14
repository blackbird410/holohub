# SPDX-FileCopyrightText: Copyright (c) 2023-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
# SPDX-License-Identifier: Apache-2.0

find_package(holoscan 1.0 REQUIRED CONFIG
  PATHS "/opt/nvidia/holoscan" "/workspace/holoscan-sdk/install")

# Fetch GoogleTest
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/releases/download/v1.17.0/googletest-1.17.0.tar.gz
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(googletest)

# Create test executable
add_executable(format_converter_test
  test_format_converter.cpp
)

# Link libraries
target_link_libraries(format_converter_test
  holoscan::core
  holoscan::ops::format_converter
  GTest::gtest_main
  GTest::gmock
)

# Add test to CTest
add_test(NAME format_converter_test COMMAND format_converter_test)

# Set test properties for better output
set_tests_properties(format_converter_test PROPERTIES
  PASS_REGULAR_EXPRESSION "PASSED"
  FAIL_REGULAR_EXPRESSION "FAILED|ERROR|Exception"
)
