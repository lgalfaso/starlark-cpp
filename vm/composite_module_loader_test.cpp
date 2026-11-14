// Copyright 2026 Lucas Mirelmann

#include <gtest/gtest.h>

#include <filesystem>  // NOLINT(build/c++17)
#include <fstream>
#include <map>
#include <memory>
#include <string>

#include "vm/scoped_temp_directory.hpp"
#include "vm/composite_module_loader.hpp"
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

TEST(CompositeModuleLoader, RoutesToChildRepository) {
  scoped_temp_directory main_dir{"main"};
  scoped_temp_directory dep_dir{"dep"};
  write_file(main_dir.path() / "main.bzl", "x = 1\n");
  write_file(dep_dir.path() / "lib.bzl", "y = 2\n");

  composite_module_loader composite;
  composite.add_repository("", std::make_unique<directory_module_loader>(main_dir.path()), {{"dep", "dep+1"}});
  composite.add_repository("dep+1", std::make_unique<directory_module_loader>(dep_dir.path(), "dep+1"), {});

  const std::string resolved = composite.resolve_canonical_name("@dep//:lib.bzl", "@@//:main.bzl");
  EXPECT_EQ(resolved, format_global_module_id("dep+1", "", "lib.bzl"));

  const auto source = composite.resolve_source(resolved);
  ASSERT_TRUE(source.ok());
  EXPECT_EQ(source->source, "y = 2\n");
}

TEST(CompositeModuleLoader, ContextDependentRepoMapping) {
  scoped_temp_directory repo_a{"a"};
  scoped_temp_directory repo_b{"b"};
  scoped_temp_directory dep_one{"d1"};
  scoped_temp_directory dep_two{"d2"};
  write_file(dep_one.path() / "x.bzl", "v = 1\n");
  write_file(dep_two.path() / "x.bzl", "v = 2\n");

  composite_module_loader composite;
  composite.add_repository("repo_a", std::make_unique<directory_module_loader>(repo_a.path(), "repo_a"), {{"dep", "shared+1"}});
  composite.add_repository("repo_b", std::make_unique<directory_module_loader>(repo_b.path(), "repo_b"), {{"dep", "shared+2"}});
  composite.add_repository("shared+1", std::make_unique<directory_module_loader>(dep_one.path(), "shared+1"), {});
  composite.add_repository("shared+2", std::make_unique<directory_module_loader>(dep_two.path(), "shared+2"), {});

  const std::string from_a = composite.resolve_canonical_name("@dep//:x.bzl", format_global_module_id("repo_a", "", "a.bzl"));
  const std::string from_b = composite.resolve_canonical_name("@dep//:x.bzl", format_global_module_id("repo_b", "", "b.bzl"));
  EXPECT_EQ(from_a, format_global_module_id("shared+1", "", "x.bzl"));
  EXPECT_EQ(from_b, format_global_module_id("shared+2", "", "x.bzl"));
}

TEST(CompositeModuleLoader, TransitiveMappingInDependencyRepo) {
  scoped_temp_directory main_dir{"main"};
  scoped_temp_directory dep_dir{"dep"};
  scoped_temp_directory other_dir{"other"};
  write_file(main_dir.path() / "main.bzl", "");
  write_file(dep_dir.path() / "use.bzl", "");
  write_file(other_dir.path() / "other.bzl", "z = 9\n");

  composite_module_loader composite;
  composite.add_repository("", std::make_unique<directory_module_loader>(main_dir.path()), {{"dep", "dep+1"}});
  composite.add_repository("dep+1",
      std::make_unique<directory_module_loader>(dep_dir.path(), "dep+1"),
      {{"other", "other+1"}});
  composite.add_repository("other+1", std::make_unique<directory_module_loader>(other_dir.path(), "other+1"), {});

  const std::string resolved = composite.resolve_canonical_name(
      "@other//:other.bzl", format_global_module_id("dep+1", "", "use.bzl"));
  EXPECT_EQ(resolved, format_global_module_id("other+1", "", "other.bzl"));
  const auto source = composite.resolve_source(resolved);
  ASSERT_TRUE(source.ok());
  EXPECT_EQ(source->source, "z = 9\n");
}

}  // namespace
}  // namespace vm
}  // namespace starlark
