// Copyright 2026 Lucas Mirelmann

#ifndef COMPILER_ANALYSIS_CALL_SITE_ANALYSIS_HPP_
#define COMPILER_ANALYSIS_CALL_SITE_ANALYSIS_HPP_

#include <map>

#include "proto/starlark_bytecode.pb.h"

#pragma GCC visibility push(default)

namespace starlark {
namespace compiler {
namespace analysis {

std::map<int, int> analyze_static_self_calls(const starlark::bytecode::Program& program, int block_idx);
void annotate_static_self_calls(starlark::bytecode::Program& program);

}  // namespace analysis
}  // namespace compiler
}  // namespace starlark

#pragma GCC visibility pop

#endif  // COMPILER_ANALYSIS_CALL_SITE_ANALYSIS_HPP_
