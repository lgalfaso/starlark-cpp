// Copyright 2026 Lucas Mirelmann

#ifndef VM_SCOPED_TEMP_DIRECTORY_HPP_
#define VM_SCOPED_TEMP_DIRECTORY_HPP_

#include <gtest/gtest.h>

#include <filesystem>  // NOLINT(build/c++17)
#include <string_view>

namespace starlark {
namespace vm {

// Creates a subdirectory under ::testing::TempDir() (Bazel sets TEST_TMPDIR).
class scoped_temp_directory {
 public:
  explicit scoped_temp_directory(std::string_view name) : path_(::testing::TempDir()) {
    if (!name.empty()) {
      path_ /= name;
    }
    std::filesystem::remove_all(path_);
    std::filesystem::create_directories(path_);
  }

  ~scoped_temp_directory() { std::filesystem::remove_all(path_); }

  scoped_temp_directory(const scoped_temp_directory&) = delete;
  scoped_temp_directory& operator=(const scoped_temp_directory&) = delete;

  const std::filesystem::path& path() const { return path_; }

 private:
  std::filesystem::path path_;
};

}  // namespace vm
}  // namespace starlark

#endif  // VM_SCOPED_TEMP_DIRECTORY_HPP_
