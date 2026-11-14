// Copyright 2026 Lucas Mirelmann

#include <gtest/gtest.h>

#include <map>
#include <string>

#include "vm/module_label.hpp"

namespace starlark {
namespace vm {
namespace {

TEST(ModuleLabel, ParseGlobalMainRepo) {
  const auto parsed = parse_global_module_id("@@//:builtin.star");
  ASSERT_TRUE(parsed.has_value());
  EXPECT_EQ(parsed->canonical_repo, "");
  EXPECT_EQ(parsed->package, "");
  EXPECT_EQ(parsed->target, "builtin.star");
}

TEST(ModuleLabel, ApparentMainRepoBuiltinLabel) {
  EXPECT_FALSE(parse_global_module_id("@//:builtin.star").has_value());
  const std::map<std::string, std::string, std::less<>> main_repo{{"", ""}};
  const auto resolved = resolve_load_label("@//:builtin.star", "@@//:caller.bzl", main_repo);
  ASSERT_TRUE(resolved.has_value());
  EXPECT_EQ(*resolved, "@@//:builtin.star");
}

TEST(ModuleLabel, ParseGlobalExternalRepo) {
  const auto parsed = parse_global_module_id("@@rules_cc+1.0.3//pkg:defs.bzl");
  ASSERT_TRUE(parsed.has_value());
  EXPECT_EQ(parsed->canonical_repo, "rules_cc+1.0.3");
  EXPECT_EQ(parsed->package, "pkg");
  EXPECT_EQ(parsed->target, "defs.bzl");
}

TEST(ModuleLabel, FormatRoundTrip) {
  const std::string id = format_global_module_id("dep+1", "foo/bar", "baz.bzl");
  EXPECT_EQ(id, "@@dep+1//foo/bar:baz.bzl");
  const auto parsed = parse_global_module_id(id);
  ASSERT_TRUE(parsed.has_value());
  EXPECT_EQ(parsed->canonical_repo, "dep+1");
  EXPECT_EQ(parsed->package, "foo/bar");
  EXPECT_EQ(parsed->target, "baz.bzl");
}

TEST(ModuleLabel, ResolveAbsoluteLabel) {
  const std::map<std::string, std::string, std::less<>> no_mapping;
  const auto resolved = resolve_load_label("//pkg:file.bzl", "@@//caller:mod.bzl", no_mapping);
  ASSERT_TRUE(resolved.has_value());
  EXPECT_EQ(*resolved, "@@//pkg:file.bzl");
}

TEST(ModuleLabel, ResolvePackageOnlyLabel) {
  const std::map<std::string, std::string, std::less<>> no_mapping;
  const auto foo = resolve_load_label("//foo", "@@//:caller.bzl", no_mapping);
  ASSERT_TRUE(foo.has_value());
  EXPECT_EQ(*foo, "@@//foo:foo");

  const auto nested = resolve_load_label("//foo/bar", "@@//:caller.bzl", no_mapping);
  ASSERT_TRUE(nested.has_value());
  EXPECT_EQ(*nested, "@@//foo/bar:bar");
}

TEST(ModuleLabel, ResolveLoadLabelReturnsNullopt) {
  const std::map<std::string, std::string, std::less<>> no_mapping;
  const std::map<std::string, std::string, std::less<>> main_repo{{"", ""}};

  EXPECT_FALSE(resolve_load_label("", "@@//:caller.bzl", no_mapping).has_value());
  EXPECT_FALSE(resolve_load_label(":", "@@//:caller.bzl", no_mapping).has_value());
  EXPECT_FALSE(resolve_load_label("//", "@@//:caller.bzl", no_mapping).has_value());
  EXPECT_FALSE(resolve_load_label("//pkg:", "@@//:caller.bzl", no_mapping).has_value());
  EXPECT_FALSE(resolve_load_label("//foo/bar/", "@@//:caller.bzl", no_mapping).has_value());
  EXPECT_FALSE(resolve_load_label("//foo//bar", "@@//:caller.bzl", no_mapping).has_value());
  EXPECT_FALSE(resolve_load_label("//foo/../bar", "@@//:caller.bzl", no_mapping).has_value());
  EXPECT_FALSE(resolve_load_label("@@not-a-global-id", "@@//:caller.bzl", no_mapping).has_value());
  EXPECT_FALSE(resolve_load_label("//pkg:file.bzl", "not-a-global-id", no_mapping).has_value());
  EXPECT_FALSE(resolve_load_label("@unknown//:file.bzl", "@@//:caller.bzl", no_mapping).has_value());
  EXPECT_FALSE(resolve_load_label("@//:builtin.star", "@@//:caller.bzl", no_mapping).has_value());
  EXPECT_FALSE(resolve_load_label("@repo-without-slashes", "@@//:caller.bzl", main_repo).has_value());
}

TEST(ModuleLabel, ResolveRelativeLabel) {
  const std::map<std::string, std::string, std::less<>> no_mapping;
  const auto resolved = resolve_load_label(":sibling.bzl", "@@//foo/bar:mod.bzl", no_mapping);
  ASSERT_TRUE(resolved.has_value());
  EXPECT_EQ(*resolved, "@@//foo/bar:sibling.bzl");
}

TEST(ModuleLabel, ResolveApparentRepo) {
  const std::map<std::string, std::string, std::less<>> mapping{{"rules_cc", "rules_cc+1.0.3"}};
  const auto resolved = resolve_load_label("@rules_cc//cc:defs.bzl", "@@//:main.bzl", mapping);
  ASSERT_TRUE(resolved.has_value());
  EXPECT_EQ(*resolved, "@@rules_cc+1.0.3//cc:defs.bzl");
}

TEST(ModuleLabel, ResolveCanonicalRepoInSource) {
  const std::map<std::string, std::string, std::less<>> no_mapping;
  const auto resolved = resolve_load_label("@@rules_cc+1.0.3//cc:defs.bzl", "@@//:main.bzl", no_mapping);
  ASSERT_TRUE(resolved.has_value());
  EXPECT_EQ(*resolved, "@@rules_cc+1.0.3//cc:defs.bzl");
}

TEST(ModuleLabel, ResolveDifferentMappingPerContext) {
  const std::map<std::string, std::string, std::less<>> mapping_a{{"dep", "shared+1"}};
  const std::map<std::string, std::string, std::less<>> mapping_b{{"dep", "shared+2"}};
  const auto from_a = resolve_load_label("@dep//:x.bzl", "@@repo_a//:a.bzl", mapping_a);
  const auto from_b = resolve_load_label("@dep//:x.bzl", "@@repo_b//:b.bzl", mapping_b);
  ASSERT_TRUE(from_a.has_value());
  ASSERT_TRUE(from_b.has_value());
  EXPECT_EQ(*from_a, "@@shared+1//:x.bzl");
  EXPECT_EQ(*from_b, "@@shared+2//:x.bzl");
}

TEST(ModuleLabel, LabelToRelativePath) {
  EXPECT_EQ(*label_to_relative_path("", "root.bzl"), "root.bzl");
  EXPECT_EQ(*label_to_relative_path("pkg/sub", "file.bzl"), "pkg/sub/file.bzl");
}

}  // namespace
}  // namespace vm
}  // namespace starlark
