// Copyright 2026 Lucas Mirelmann

#include "io/read_file.hpp"

#include <gtest/gtest.h>

#include <fstream>
#include <string>

namespace starlark {
namespace io {
namespace {

TEST(ReadFile, ReadsExistingFile) {
  const std::string path = testing::TempDir() + "/read_file_test.txt";
  {
    std::ofstream out{path};
    ASSERT_TRUE(out.is_open());
    out << "hello, starlark";
  }

  const auto content = read_file(path);
  ASSERT_TRUE(content.has_value());
  EXPECT_EQ(*content, "hello, starlark");
}

TEST(ReadFile, MissingFileReturnsNullopt) {
  const auto content = read_file("/nonexistent/path/read_file_test.txt");
  EXPECT_FALSE(content.has_value());
}

}  // namespace
}  // namespace io
}  // namespace starlark
