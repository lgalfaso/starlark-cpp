// Copyright 2026 Lucas Mirelmann

#include <gtest/gtest.h>

#include <filesystem>  // NOLINT(build/c++17)
#include <fstream>
#include <memory>
#include <string>

#include "vm/scoped_temp_directory.hpp"
#include "interpreter/interpreter.hpp"
#include "vm/composite_module_loader.hpp"
#include "vm/directory_module_loader.hpp"
#include "vm/module_label.hpp"

using ::starlark::grammar::grammar_options;
using ::starlark::interpreter::interpreter;
using ::starlark::logging::logger;
using ::starlark::runtime::runtime_options;
using ::starlark::runtime::starlark_obj;
using ::starlark::vm::composite_module_loader;
using ::starlark::vm::directory_module_loader;
using ::starlark::vm::format_global_module_id;
using ::starlark::vm::frame;

namespace {

void write_file(const std::filesystem::path& path, std::string_view content) {
  std::filesystem::create_directories(path.parent_path());
  std::ofstream out(path);
  ASSERT_TRUE(out.good());
  out << content;
}

using starlark::vm::scoped_temp_directory;

TEST(ModuleLoaderIntegration, DirectoryLoaderRunsLoadGraph) {
  scoped_temp_directory temp{"dir"};
  write_file(temp.path() / "main.bzl", R"starlark(
load("//:lib.bzl", "value")

result = value
)starlark");
  write_file(temp.path() / "lib.bzl", R"starlark(
value = "from-lib"
)starlark");

  directory_module_loader loader{temp.path()};
  interpreter runner;
  logger logging;
  const auto entry = format_global_module_id("", "", "main.bzl");
  const auto result = runner.run(loader, entry, grammar_options{}, runtime_options{}, logging);
  ASSERT_TRUE(result.ok()) << logging.size();
  ASSERT_EQ((*result)->elements.size(), 1u);
  ASSERT_NE(nullptr, (*result)->elements[0]);
  EXPECT_EQ((*result)->elements[0]->str(), "from-lib");
}

TEST(ModuleLoaderIntegration, CompositeLoaderCrossRepo) {
  scoped_temp_directory main_dir{"main"};
  scoped_temp_directory dep_dir{"dep"};
  write_file(main_dir.path() / "main.bzl", R"starlark(
load("@dep//:lib.bzl", "msg")

result = msg
)starlark");
  write_file(dep_dir.path() / "lib.bzl", R"starlark(
msg = "cross-repo"
)starlark");

  composite_module_loader composite;
  composite.add_repository("", std::make_unique<directory_module_loader>(main_dir.path()), {{"dep", "dep+1"}});
  composite.add_repository("dep+1", std::make_unique<directory_module_loader>(dep_dir.path(), "dep+1"), {});

  interpreter runner;
  logger logging;
  const auto entry = format_global_module_id("", "", "main.bzl");
  const auto result = runner.run(composite, entry, grammar_options{}, runtime_options{}, logging);
  ASSERT_TRUE(result.ok());
  ASSERT_EQ((*result)->elements.size(), 1u);
  EXPECT_EQ((*result)->elements[0]->str(), "cross-repo");
}

}  // namespace
