// Copyright 2026 Lucas Mirelmann

#include "native/engine/dylib_linker.hpp"

#include <cstdlib>
#include <filesystem>  // NOLINT(build/c++17)
#include <string>
#include <vector>

#include "lld/Common/Driver.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/raw_ostream.h"

#if defined(__APPLE__)
LLD_HAS_DRIVER(macho)
#else
LLD_HAS_DRIVER(elf)
#endif

namespace starlark {
namespace native {

namespace {

#if defined(__APPLE__)
constexpr ::lld::DriverDef kLldDrivers[] = {{::lld::Darwin, &::lld::macho::link}};
constexpr char kLldArgv0[] = "ld64.lld";
#else
constexpr ::lld::DriverDef kLldDrivers[] = {{::lld::Gnu, &::lld::elf::link}};
constexpr char kLldArgv0[] = "ld.lld";
#endif

std::string lld_arch_name(const llvm::Triple& triple) {
  switch (triple.getArch()) {
    case llvm::Triple::aarch64:
    case llvm::Triple::aarch64_32:
      return "arm64";
    case llvm::Triple::x86_64:
      return "x86_64";
    case llvm::Triple::arm:
      return "arm";
    default:
      return triple.getArchName().str();
  }
}

}  // namespace

bool link_object_to_dylib(const llvm::Triple& triple,
    const std::filesystem::path& object_path,
    const std::filesystem::path& dylib_path) {
  const std::string object = object_path.string();
  const std::string dylib = dylib_path.string();

  std::vector<std::string> args;
  args.push_back(kLldArgv0);
#if defined(__APPLE__)
  args.push_back("-dylib");
  args.push_back("-undefined");
  args.push_back("dynamic_lookup");
  args.push_back("-arch");
  args.push_back(lld_arch_name(triple));

  llvm::VersionTuple macos_version;
  if (!triple.getMacOSXVersion(macos_version)) {
    macos_version = llvm::VersionTuple(11, 0);
  }
  std::string version = std::to_string(macos_version.getMajor());
  if (macos_version.getMinor().has_value()) {
    version += ".";
    version += std::to_string(*macos_version.getMinor());
  }
  args.push_back("-platform_version");
  args.push_back("macos");
  args.push_back(version);
  args.push_back(version);
#else
  args.push_back("-shared");
#endif
  args.push_back("-o");
  args.push_back(dylib);
  args.push_back(object);

  llvm::SmallVector<const char*, 16> argv;
  for (const std::string& arg : args) {
    argv.push_back(arg.c_str());
  }

  const ::lld::Result result = ::lld::lldMain(argv, llvm::nulls(), llvm::errs(), kLldDrivers);
  if (result.retCode != 0 || !result.canRunAgain) {
    if (result.retCode != 0) {
      llvm::errs() << "lld link failed (exit " << result.retCode << ")\n";
    } else {
      llvm::errs() << "lld link succeeded but cannot run again in-process\n";
    }
    llvm::errs() << "command:";
    for (const std::string& arg : args) {
      llvm::errs() << " " << arg;
    }
    llvm::errs() << "\n";
    return false;
  }
  return true;
}

}  // namespace native
}  // namespace starlark
