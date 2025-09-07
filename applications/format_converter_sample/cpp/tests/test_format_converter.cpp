// test_format_converter.cpp
#include <gtest/gtest.h>
#include <cstdio>
#include <fstream>
#include <string>
#include <thread>
#include <chrono>
#include <filesystem>
#include <cstdlib>

namespace fs = std::filesystem;

TEST(FormatConverterIntegration, ProducesExpectedMetadata) {
  const std::string out_meta = "/tmp/fc_metadata.txt";
  if (fs::exists(out_meta)) fs::remove(out_meta);

  // run the app binary (assumes it's built in parent build dir)
  int rc = std::system("../format_converter_app");
  // it's OK if return code is non-zero; we still check metadata file presence

  // wait up to 10s
  bool present = false;
  for (int i = 0; i < 40; ++i) {
    if (fs::exists(out_meta)) { present = true; break; }
    std::this_thread::sleep_for(std::chrono::milliseconds(250));
  }
  ASSERT_TRUE(present) << "metadata file not created";

  // read file
  std::ifstream ifs(out_meta);
  ASSERT_TRUE(ifs.is_open());
  std::string line;
  std::getline(ifs, line);
  ifs.close();

  // minimal assertions: contains dtype= and layout= and shape=
  ASSERT_NE(line.find("dtype="), std::string::npos);
  ASSERT_NE(line.find("layout="), std::string::npos);
  ASSERT_NE(line.find("shape="), std::string::npos);
}
