// Copyright 2026 Lucas Mirelmann

#include <gtest/gtest.h>

#include <filesystem>  // NOLINT(build/c++17)
#include <fstream>
#include <string>

#include "vm/scoped_temp_directory.hpp"
#include "vm/directory_module_loader.hpp"
#include "vm/module_label.hpp"

namespace starlark {
namespace vm {
namespace {

void write_file(const std::filesystem::path& path, std::string_view content) {
  std::filesystem::create_directories(path.parent_path());
  std::ofstream out(path);
  ASSERT_TRUE(out.good());
  out << content;
}

using starlark::vm::scoped_temp_directory;

TEST(DirectoryModuleLoader, LoadsFileByGlobalId) {
  scoped_temp_directory temp{"root"};
  write_file(temp.path() / "foo.bzl", "x = 1\n");
  directory_module_loader loader{temp.path()};
  const auto source = loader.resolve_source("@@//:foo.bzl");
  ASSERT_TRUE(source.ok());
  EXPECT_EQ(source->source, "x = 1\n");
}

TEST(DirectoryModuleLoader, LoadsNestedPackage) {
  scoped_temp_directory temp{"root"};
  write_file(temp.path() / "pkg" / "lib.bzl", "v = 2\n");
  directory_module_loader loader{temp.path()};
  const auto source = loader.resolve_source(format_global_module_id("", "pkg", "lib.bzl"));
  ASSERT_TRUE(source.ok());
  EXPECT_EQ(source->source, "v = 2\n");
}

TEST(DirectoryModuleLoader, RejectsParentTraversalInLabel) {
  scoped_temp_directory temp{"root"};
  write_file(temp.path() / "foo.bzl", "x = 1\n");
  write_file(temp.path().parent_path() / "outside.bzl", "x = 1\n");
  directory_module_loader loader{temp.path()};
  const auto resolved = loader.resolve_canonical_name("../outside.bzl", "@@//:foo.bzl");
  const auto source = loader.resolve_source(resolved);
  EXPECT_FALSE(source.ok());
}

TEST(DirectoryModuleLoader, RejectsSymlinkEscape) {
  scoped_temp_directory temp{"root"};
  const std::filesystem::path outside = temp.path().parent_path() / "dir_loader_outside.bzl";
  write_file(outside, "x = 1\n");
  write_file(temp.path() / "link.bzl", "placeholder\n");
  std::filesystem::remove(temp.path() / "link.bzl");
  std::filesystem::create_symlink(outside, temp.path() / "link.bzl");

  directory_module_loader loader{temp.path()};
  const auto source = loader.resolve_source("@@//:link.bzl");
  EXPECT_FALSE(source.ok());
}

TEST(DirectoryModuleLoader, CanonicalNameRelativeLoad) {
  scoped_temp_directory temp{"root"};
  write_file(temp.path() / "pkg" / "a.bzl", "");
  write_file(temp.path() / "pkg" / "b.bzl", "");
  directory_module_loader loader{temp.path()};
  const std::string resolved = loader.resolve_canonical_name(":b.bzl", format_global_module_id("", "pkg", "a.bzl"));
  EXPECT_EQ(resolved, format_global_module_id("", "pkg", "b.bzl"));
}

TEST(DirectoryModuleLoader, FixedBindingsForAllTargets) {
  scoped_temp_directory temp{"root"};
  write_file(temp.path() / "a.bzl", "a = 1\n");
  write_file(temp.path() / "b.bzl", "b = 2\n");

  module_info::bindings_t bindings;
  bindings.emplace("shared", nullptr);
  fixed_bindings_directory_module_loader loader{temp.path(), "", std::move(bindings)};

  const auto a = loader.resolve_source(format_global_module_id("", "", "a.bzl"));
  const auto b = loader.resolve_source(format_global_module_id("", "", "b.bzl"));
  ASSERT_TRUE(a.ok());
  ASSERT_TRUE(b.ok());
  EXPECT_EQ(a->bindings.count("shared"), 1u);
  EXPECT_EQ(b->bindings.count("shared"), 1u);
}

}  // namespace
}  // namespace vm
}  // namespace starlark
