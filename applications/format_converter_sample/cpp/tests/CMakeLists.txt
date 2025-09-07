# Holohub provides holoscan_add_test() that wires gtest and handles pathing.

holoscan_add_test(
  NAME format_converter_test
  SOURCES test_format_converter.cpp
  DEPENDS format_converter_app   # ensures app binary builds before test runs
)
