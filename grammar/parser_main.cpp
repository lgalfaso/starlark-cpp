// Copyright 2024-2025 Lucas Mirelmann

#include <sys/stat.h>
#include <fcntl.h>

#include <iostream>
#include <set>
#include <string>

#include "third-party/defer.hpp"
#include "grammar/options.hpp"
#include "grammar/parser.hpp"
#include "proto/starlark_ast.pb.h"

using starlark::ast::File;
using starlark::grammar::options;
using starlark::grammar::parser;
using starlark::logging::LogLevel;
using starlark::logging::logger;

namespace {

static const std::set<std::string, std::less<>> BUILD_symbols = {
    "depset",          "existing_rule", "existing_rules", "exports_files", "glob",                   "module_name",
    "module_version",  "package",       "package_group",  "package_name",  "package_relative_label", "repo_name",
    "repository_name", "select",        "subpackages",

    // For modules that expect these rules to be predefined.
    // General Rules
    "alias", "config_setting", "filegroup", "genquery", "genrule", "starlark_doc_extract", "test_suite",
    // Android Rules
    "android_binary", "aar_import", "android_library", "android_instrumentation_test", "android_local_test", "android_device",
    "android_ndk_repository", "android_sdk_repository",
    // C / C++ Rules
    "cc_binary",    "cc_import",          "cc_library",         "cc_shared_library", "cc_static_library", "cc_test",
    "cc_toolchain", "cc_toolchain_suite", "fdo_prefetch_hints", "fdo_profile",       "memprof_profile",   "propeller_optimize",
    // Java Rules
    "java_binary",  "java_import",     "java_library", "java_test", "java_package_configuration", "java_plugin",
    "java_runtime", "java_single_jar", "java_toolchain",
    // Objective-C Rules
    "objc_import", "objc_library",
    // Protocol Buffer Rules
    "cc_proto_library", "java_lite_proto_library", "java_proto_library", "proto_library", "py_proto_library", "proto_lang_toolchain",
    "proto_toolchain",
    // Python Rules
    "py_binary", "py_library", "py_test", "py_runtime",
    // Shell Rules
    "sh_binary", "sh_library", "sh_test",

    // Platforms and Toolchains Rules
    "constraint_setting", "constraint_value", "platform", "toolchain", "toolchain_type",

    // Deprecated.
    "licenses",

    // Unknown
    "cc_toolchain_alias", "cc_libc_top_alias",
    "xcode_version", "xcode_config", "available_xcodes",
    "label_flag", "label_setting",

    // Random
    "json",
};

static const std::set<std::string, std::less<>> WORKSPACE_symbols = {
    "bind", "register_execution_platforms", "register_toolchains", "workspace",

    // These should not be needed, but there are repositories that expect them to be predefined.
    "local_repository",

    // Deprecated
    "android_sdk_repository",
};

static const std::set<std::string, std::less<>> MODULE_symbols = {
    "archive_override", "bazel_dep",                 "git_override",  "include",                      "inject_repo",         "local_path_override",
    "module",           "multiple_version_override", "override_repo", "register_execution_platforms", "register_toolchains", "single_version_override",
    "use_extension",    "use_repo",                  "use_repo_rule",
};

static const std::set<std::string, std::less<>> bzl_symbols = {
    "analysis_test_transition", "aspect",           "configuration_field", "depset", "exec_group", "exec_transition",
    "macro",                    "module_extension", "provider",            "repository_rule", "rule", "select",
    "subrule",                  "tag_class",        "visibility",

    // Top-level Modules
    "apple_common", "attr",   "cc_common",       "config", "config_common", "coverage_common",
    "java_common",  "native", "platform_common", "proto",  "proto_common",  "testing",

    // Built-in Types
    "Action", "actions", "apple_platform", "Args", "Aspect", "Attribute",
    "bazel_module", "bazel_module_tags", "BuildSetting", "CcCompilationOutputs", "CcLinkingOutputs", "CompilationContext",
    "configuration", "ctx", "depset", "DirectoryExpander", "DottedVersion", "exec_result",
    "ExecGroupCollection", "ExecGroupContext", "ExecTransitionFactory", "extension_metadata", "FeatureConfiguration", "File",
    "fragments", "java_annotation_processing", "Label", "LateBoundDefault", "LibraryToLink", "License",
    "LinkerInput", "LinkingContext", "macro", "mapped_root", "module_ctx", "path",
    "Provider", "repository_ctx", "repository_os", "repository_rule", "root", "rule",
    "rule_attributes", "runfiles", "struct", "Subrule", "subrule_ctx", "SymlinkEntry",
    "tag_class", "Target", "TemplateDict", "toolchain_type", "ToolchainContext", "transition",

    // Providers
    "AnalysisTestResultInfo", "CcInfo", "CcToolchainConfigInfo", "CcToolchainInfo", "ConstraintCollection", "ConstraintSettingInfo",
    "ConstraintValueInfo", "DebugPackageInfo", "DefaultInfo", "ExecutionInfo", "FeatureFlagInfo", "file_provider",
    "FilesToRunProvider", "IncompatiblePlatformProvider", "InstrumentedFilesInfo", "java_compilation_info", "java_output_jars", "JavaRuntimeInfo",
    "JavaToolchainInfo", "ObjcProvider", "OutputGroupInfo", "PackageSpecificationInfo", "PlatformInfo", "ProguardSpecProvider",
    "ProtoRegistryProvider", "RunEnvironmentInfo", "TemplateVariableInfo", "ToolchainInfo", "ToolchainTypeInfo",
    // Deprecated providers.
    "JavaInfo",
    "PyRuntimeInfo", "PyInfo",
    "CcSharedLibraryInfo",

    // Unknown
    "py_internal", "PyCcLinkParamsProvider",
    "ProtoInfo",
    "cc_proto_aspect",
    "proto_common_do_not_use",
    "JavaPluginInfo",

    // Random
    "json",
};

}  // namespace

int main(int argc, char* argv[]) {
  for (int i = 1; i < argc; ++i) {
    std::string starlark_program;
    {
      int in_fd = open(argv[i], O_RDONLY);
      if (in_fd < 0) {
        return 2;
      }
      defer { close(in_fd); };
      struct stat sb;
      if (fstat(in_fd, &sb) != 0) {
        return 3;
      }
      starlark_program.resize(sb.st_size);
      read(in_fd, starlark_program.data(), sb.st_size);
    }

    std::string arg{argv[i]};
    logger logging;
    logging.set_level(LogLevel::LOG_LEVEL_ERROR);
    bool is_build_or_workspace =
        arg.ends_with("WORKSPACE") ||
        arg.ends_with("WORKSPACE.bazel") ||
        arg.ends_with("BUILD") ||
        arg.ends_with("BUILD.bazel");
    std::set<std::string, std::less<>> extra_symbols;
    if (arg.ends_with("BUILD") || arg.ends_with("BUILD.bazel")) {
      extra_symbols = BUILD_symbols;
    }
    if (arg.ends_with("WORKSPACE") || arg.ends_with("WORKSPACE.bazel")) {
      extra_symbols = WORKSPACE_symbols;
    }
    if (arg.ends_with("MODULE.bazel")) {
      extra_symbols = MODULE_symbols;
    }
    if (arg.ends_with(".bzl")) {
      extra_symbols = bzl_symbols;
    }
    parser star_parser(starlark_program,
                       options{
                           .escaped_octal_and_hex_char_are_ascii = false,
                           .require_load_statements_first = !is_build_or_workspace,
                           .allow_varadic_arguments = !is_build_or_workspace,
                           .allow_top_level_rebinding = is_build_or_workspace,
                       },
                       extra_symbols,
                       logging);
    google::protobuf::Arena arena;
    [[maybe_unused]] File* actual_starlark_file = star_parser.parse_file(arena);
    if (!logging.empty()) {
      std::cout << "Unable to parse: " << argv[i] << "\n";
    }
    for (const auto& entry : logging) {
      std::cout << "  " << entry.module() << ":" << entry.message() << ":" << entry.pos().row() << "," << entry.pos().column() << "\n";
    }
  }

  return 0;
}

