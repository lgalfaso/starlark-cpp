// Copyright 2026 Lucas Mirelmann

#include "native/engine/orc_engine.hpp"

#include <format>
#include <utility>
#include <memory>
#include <string>

#include "llvm/ExecutionEngine/Orc/LLJIT.h"
#include "llvm/ExecutionEngine/Orc/ThreadSafeModule.h"
#include "llvm/TargetParser/Triple.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/CodeGen.h"
#include "llvm/Support/DynamicLibrary.h"
#include "llvm/Support/Error.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/TargetParser/Host.h"

#include "native/engine/dylib_linker.hpp"

using ::starlark::result::status_or;
using status = ::starlark::result::status;
using status_code = ::starlark::result::status_code;

namespace starlark {
namespace native {

namespace {

template<typename T>
T unwrap_or(T fallback, llvm::Expected<T> value) {
  if (!value) {
    llvm::consumeError(value.takeError());
    return fallback;
  }
  return std::move(*value);
}

}  // namespace

struct orc_engine::impl {
  llvm::LLVMContext context;
  std::unique_ptr<llvm::orc::LLJIT> jit;
};

orc_engine::orc_engine() : impl_(std::make_unique<impl>()) {
  llvm::InitializeNativeTarget();
  llvm::InitializeNativeTargetAsmPrinter();
  llvm::InitializeNativeTargetAsmParser();

  auto jit_tmb = llvm::orc::JITTargetMachineBuilder::detectHost();
  if (!jit_tmb) {
    llvm::consumeError(jit_tmb.takeError());
    return;
  }
  jit_tmb->setCodeGenOptLevel(llvm::CodeGenOptLevel::Aggressive);
  auto jit = llvm::orc::LLJITBuilder().setJITTargetMachineBuilder(std::move(*jit_tmb)).create();
  if (!jit) {
    llvm::consumeError(jit.takeError());
    return;
  }
  impl_->jit = std::move(*jit);

  auto process_symbols = llvm::orc::DynamicLibrarySearchGenerator::GetForCurrentProcess(impl_->jit->getDataLayout().getGlobalPrefix());
  if (process_symbols) {
    (void)impl_->jit->getMainJITDylib().addGenerator(std::move(*process_symbols));
  } else {
    llvm::consumeError(process_symbols.takeError());
  }
}

orc_engine::~orc_engine() = default;

status_or<const starlark_module_descriptor*> orc_engine::lookup_descriptor_symbol(llvm::orc::JITDylib& dylib,
    std::string_view symbol_name) {
  auto addr = impl_->jit->lookup(dylib, symbol_name);
  if (!addr) {
    llvm::consumeError(addr.takeError());
    return status_or<const starlark_module_descriptor*>(status_code::kStaticError);
  }
  auto* descriptor = reinterpret_cast<const starlark_module_descriptor*>((*addr).toPtr<void*>());
  return status_or<const starlark_module_descriptor*>(descriptor);
}

status_or<const starlark_module_descriptor*> orc_engine::load_jit(uint64_t cache_key,
    std::unique_ptr<llvm::Module> module,
    std::unique_ptr<llvm::LLVMContext> context) {
  std::lock_guard lock(mutex_);
  auto it = loaded_.find(cache_key);
  if (it != loaded_.end()) {
    return status_or<const starlark_module_descriptor*>(it->second);
  }

  llvm::orc::ThreadSafeModule tsm(std::move(module), std::move(context));
  auto dylib_name = std::format("starlark_mod_{:x}", cache_key);
  auto jd = impl_->jit->createJITDylib(dylib_name);
  if (!jd) {
    llvm::consumeError(jd.takeError());
    return status_or<const starlark_module_descriptor*>(status_code::kStaticError);
  }

  auto process_symbols = llvm::orc::DynamicLibrarySearchGenerator::GetForCurrentProcess(impl_->jit->getDataLayout().getGlobalPrefix());
  if (process_symbols) {
    (void)jd->addGenerator(std::move(*process_symbols));
  } else {
    llvm::consumeError(process_symbols.takeError());
  }

  if (auto err = impl_->jit->addIRModule(*jd, std::move(tsm))) {
    llvm::consumeError(std::move(err));
    return status_or<const starlark_module_descriptor*>(status_code::kStaticError);
  }

  auto descriptor_symbol = std::format("starlark_module_descriptor_{:x}", cache_key);
  auto descriptor = lookup_descriptor_symbol(*jd, descriptor_symbol);
  if (!descriptor.ok()) {
    return descriptor;
  }
  loaded_[cache_key] = *descriptor;
  return descriptor;
}

status_or<const starlark_module_descriptor*> orc_engine::load_cached(uint64_t cache_key, std::filesystem::path dylib_path) {
  std::lock_guard lock(mutex_);
  auto it = loaded_.find(cache_key);
  if (it != loaded_.end()) {
    return status_or<const starlark_module_descriptor*>(it->second);
  }

  std::string path = dylib_path.string();
  auto lib = llvm::sys::DynamicLibrary::getPermanentLibrary(path.c_str());
  if (!lib.isValid()) {
    return status_or<const starlark_module_descriptor*>(status_code::kStaticError);
  }

  auto generator = llvm::orc::DynamicLibrarySearchGenerator::Load(path.c_str(), impl_->jit->getDataLayout().getGlobalPrefix());
  if (!generator) {
    llvm::consumeError(generator.takeError());
    return status_or<const starlark_module_descriptor*>(status_code::kStaticError);
  }

  auto dylib_name = std::to_string(cache_key) + "_cached";
  auto jd = impl_->jit->createJITDylib(dylib_name);
  if (!jd) {
    llvm::consumeError(jd.takeError());
    return status_or<const starlark_module_descriptor*>(status_code::kStaticError);
  }
  jd->addGenerator(std::move(*generator));

  auto descriptor = lookup_descriptor_symbol(*jd, std::format("starlark_module_descriptor_{:x}", cache_key));
  if (!descriptor.ok()) {
    return descriptor;
  }
  loaded_[cache_key] = *descriptor;
  return descriptor;
}

const starlark_module_descriptor* orc_engine::find_loaded(uint64_t cache_key) const {
  std::lock_guard lock(mutex_);
  auto it = loaded_.find(cache_key);
  if (it == loaded_.end()) {
    return nullptr;
  }
  return it->second;
}

status orc_engine::write_back(uint64_t cache_key, llvm::Module& module, std::filesystem::path dylib_path) {
  (void)cache_key;
  std::string error;
  llvm::Triple triple(llvm::sys::getDefaultTargetTriple());
  const llvm::Target* target = llvm::TargetRegistry::lookupTarget(triple, error);
  if (target == nullptr) {
    return status{status_code::kStaticError};
  }

  llvm::TargetOptions options;
  auto machine = target->createTargetMachine(triple,
      llvm::sys::getHostCPUName().str(),
      "",
      options,
      llvm::Reloc::PIC_,
      std::nullopt,
      llvm::CodeGenOptLevel::Aggressive);
  if (machine == nullptr) {
    return status{status_code::kStaticError};
  }

  module.setDataLayout(machine->createDataLayout());
  std::error_code ec;
  std::filesystem::create_directories(dylib_path.parent_path(), ec);
  auto object_path = dylib_path;
  object_path.replace_extension(".o");
  llvm::raw_fd_ostream out(object_path.string(), ec);
  if (ec) {
    return status{status_code::kStaticError};
  }

  llvm::legacy::PassManager pass;
  if (machine->addPassesToEmitFile(pass, out, nullptr, llvm::CodeGenFileType::ObjectFile)) {
    return status{status_code::kStaticError};
  }
  pass.run(module);
  out.flush();

  if (!link_object_to_dylib(triple, object_path, dylib_path)) {
    return status{status_code::kStaticError};
  }
  return starlark::result::ok_status();
}

}  // namespace native
}  // namespace starlark
