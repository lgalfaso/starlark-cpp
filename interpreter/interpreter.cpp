// Copyright 2024-2025 Lucas Mirelmann

#include "interpreter/interpreter.hpp"

#include <iostream>

#include <functional>
#include <limits>
#include <map>
#include <set>
#include <string>
#include <string_view>
#include <vector>

#include "bigint/number.hpp"
#include "compiler/compiler.hpp"
#include "runtime/error_fn.hpp"
#include "runtime/starlark_bigint.hpp"
#include "runtime/starlark_bool.hpp"
#include "runtime/starlark_bytes.hpp"
#include "runtime/starlark_dictionary.hpp"
#include "runtime/starlark_float.hpp"
#include "runtime/starlark_integer.hpp"
#include "runtime/starlark_list.hpp"
#include "runtime/starlark_none.hpp"
#include "runtime/starlark_string.hpp"
#include "runtime/starlark_tuple.hpp"

using ::google::protobuf::Arena;
using ::starlark::bigint::number;
using ::starlark::bigint::parse_number;
using ::starlark::bytecode::OpCode;
using ::starlark::bytecode::Program;
using ::starlark::compiler::compiler;
using ::starlark::grammar::options;
using ::starlark::grammar::predeclared_symbols;
using ::starlark::logging::logger;
using ::starlark::runtime::error_fn;
using ::starlark::runtime::starlark_bigint;
using ::starlark::runtime::starlark_bool;
using ::starlark::runtime::starlark_bytes;
using ::starlark::runtime::starlark_dictionary;
using ::starlark::runtime::starlark_float;
using ::starlark::runtime::starlark_integer;
using ::starlark::runtime::starlark_list;
using ::starlark::runtime::starlark_none;
using ::starlark::runtime::starlark_obj;
using ::starlark::runtime::starlark_string;
using ::starlark::runtime::starlark_tuple;

namespace starlark {
namespace interpreter {

namespace {

frame* create_frame(Arena& arena, std::size_t size, frame* parent_frame) {
  // TODO(lmirelmann): Maybe the frame should allocate the vector using the arena.
  // TODO(lmirelmann): If we were to use our own arena, then we could allocate the elements in the same structure.
  frame* result = Arena::Create<frame>(&arena, size);
  result->parent_frame = parent_frame;
  return result;
}

class error_handler : public error_fn {
 public:
  error_handler(int& block_ptr, int& instruction_ptr, int instruction_ptr_value, logger& log) :
      block_ptr(block_ptr), instruction_ptr(instruction_ptr), instruction_ptr_value(instruction_ptr_value),
      log(log) {}

  void add_error(std::string_view error_msg) override {
    // TODO(lmirelmann): Do not have the module name nor the position.
    log.log(starlark::logging::LogLevel::LOG_LEVEL_ERROR, error_msg, "", starlark::logging::Position::default_instance());
    block_ptr = 0;
    instruction_ptr = instruction_ptr_value;
  }

 private:
  int& block_ptr;
  int& instruction_ptr;
  const int instruction_ptr_value;
  logger& log;
};

frame* run_program(Program* starlark_program, frame* global_frame, Arena& arena, logger& log) {
  std::vector<starlark_obj*> stack;
  std::vector<frame*> frame_stack;
  std::vector<std::pair<int, int>> call_stack;
  int instruction_ptr = 0;
  int block_ptr = 0;
  // TODO(lmirelmann): This should go into a structure that keeps some of the constants.
  starlark_bool* bool_true = Arena::Create<starlark_bool>(&arena, true);
  starlark_bool* bool_false = Arena::Create<starlark_bool>(&arena, false);
  starlark_program->mutable_block(0)->add_op_code()->mutable_fail();
  error_handler error_callback(block_ptr, instruction_ptr, starlark_program->block(0).op_code_size() - 1, log);

  if (global_frame != nullptr) {
    frame_stack.push_back(global_frame);
  }
  if (starlark_program == nullptr) {
    return nullptr;
  }
  frame* result = nullptr;
  if (starlark_program->block(0).op_code(0).op_code_case() == OpCode::kCreateFrame) {
    instruction_ptr++;
    result = create_frame(arena, starlark_program->block(0).op_code(0).create_frame().slots(), global_frame);
    frame_stack.push_back(result);
  }

  while (true) {
    auto& op_code = starlark_program->block(block_ptr).op_code(instruction_ptr);
    instruction_ptr++;
    switch (op_code.op_code_case()) {
      case OpCode::kConstInt:
        stack.push_back(Arena::Create<starlark_integer>(&arena, op_code.const_int().value()));
        break;
      case OpCode::kConstBigInt:
        stack.push_back(Arena::Create<starlark_bigint>(&arena,
            parse_number(op_code.const_big_int().value(), nullptr)));
        break;
      case OpCode::kConstFloat:
        stack.push_back(Arena::Create<starlark_float>(&arena, op_code.const_float().value()));
        break;
      case OpCode::kConstString:
        stack.push_back(Arena::Create<starlark_string>(&arena, op_code.const_string().value()));
        break;
      case OpCode::kConstBytes:
        stack.push_back(Arena::Create<starlark_bytes>(&arena, op_code.const_bytes().value()));
        break;
      case OpCode::kMakeList:
        stack.push_back(Arena::Create<starlark_list>(&arena, op_code.make_list().reserve_size()));
        break;
      case OpCode::kAddToList: {
        assert(stack.size() > op_code.add_to_list().pos_in_stack());
        starlark_obj* candidate_list = stack[stack.size() - 1 - op_code.add_to_list().pos_in_stack()];
        assert(candidate_list->type() == "list");
        starlark_list* list = static_cast<starlark_list*>(candidate_list);
        assert(stack.size() >= op_code.add_to_list().number_of_elements());
        for (int i = 0; i < op_code.add_to_list().number_of_elements(); ++i) {
          list->add(stack[stack.size() - op_code.add_to_list().number_of_elements() + i], error_callback);
        }
        stack.resize(stack.size() - op_code.add_to_list().number_of_elements(), nullptr);
        break;
      }
      case OpCode::kMakeDictionary:
        // TODO(lmirelmann): The reserve size is ignored.
        stack.push_back(Arena::Create<starlark_dictionary>(&arena));
        break;
      case OpCode::kAddToDictionary: {
        assert(stack.size() > op_code.add_to_dictionary().pos_in_stack());
        starlark_obj* candidate_dict = stack[stack.size() - 1 - op_code.add_to_dictionary().pos_in_stack()];
        assert(candidate_dict->type() == "dict");
        starlark_dictionary* dict = static_cast<starlark_dictionary*>(candidate_dict);
        assert(op_code.add_to_list().number_of_elements() < std::numeric_limits<decltype(op_code.add_to_list().number_of_elements())>::max() / 2);
        assert(stack.size() >= op_code.add_to_list().number_of_elements() * 2);
        for (int i = 0; i < op_code.add_to_dictionary().number_of_elements(); ++i) {
          if (!dict->insert(stack[stack.size() - 2 * op_code.add_to_dictionary().number_of_elements() + 2 * i],
                            stack[stack.size() - 2 * op_code.add_to_dictionary().number_of_elements() + 2 * i + 1], error_callback) &&
              op_code.add_to_dictionary().number_of_elements() > 1) {
            // This makes use of the fact that dictionary comprehensions always add elements one at a time and
            // that dictionary literals add all the elements in one go.
            // TODO(lmirelmann): Report the error.
            return nullptr;
          }
        }
        stack.resize(stack.size() - 2 * op_code.add_to_dictionary().number_of_elements(), nullptr);
        break;
      }
      case OpCode::kMakeTuple: {
        assert(stack.size() >= op_code.make_tuple().number_of_elements());
        starlark_tuple* result = Arena::Create<starlark_tuple>(&arena, op_code.make_tuple().number_of_elements());
        for (int i = op_code.make_tuple().number_of_elements(); i > 0; --i) {
          result->add(stack[stack.size() - i]);
        }
        stack.resize(stack.size() - op_code.make_tuple().number_of_elements(), nullptr);
        stack.push_back(result);
        break;
      }
      case OpCode::kStore: {
        assert(!stack.empty());
        assert(frame_stack.size() > op_code.store().frame());
        assert(frame_stack[frame_stack.size() - 1 - op_code.store().frame()]->elements.size() > op_code.store().pos_in_frame());
        frame_stack[frame_stack.size() - 1 - op_code.store().frame()]->elements[op_code.store().pos_in_frame()] = stack.back();
        stack.pop_back();
        break;
      }
      case OpCode::kLoad: {
        assert(frame_stack.size() > op_code.load().frame());
        assert(frame_stack[frame_stack.size() - 1 - op_code.load().frame()]->elements.size() > op_code.load().pos_in_frame());
        auto* value = frame_stack[frame_stack.size() - 1 - op_code.load().frame()]->elements[op_code.load().pos_in_frame()];
        if (value == nullptr) {
std::cerr << "UnboundLocalError: cannot access local variable where it is not associated with a value\n";
          // TODO(lmirelmann): Add the error. The equivalent error from python is
          //                   `UnboundLocalError: cannot access local variable '{}' where it is not associated with a value`
          return nullptr;
        }
        stack.push_back(value);
        break;
      }
      case OpCode::kUnpack: {
        starlark_obj* element = stack.back();
        stack.pop_back();
        element->unpack(op_code.unpack().number_of_elements(), stack, error_callback);
        break;
      }
      case OpCode::kJumpIfFalse:
        if (!stack.back()->truthy()) {
          instruction_ptr = op_code.jump_if_false().address();
        }
        stack.pop_back();
        break;
      case OpCode::kJumpIfTrueOrPop:
        if (stack.back()->truthy()) {
          instruction_ptr = op_code.jump_if_true_or_pop().address();
        } else {
          stack.pop_back();
        }
        break;
      case OpCode::kJumpIfFalseOrPop:
        if (!stack.back()->truthy()) {
          instruction_ptr = op_code.jump_if_false_or_pop().address();
        } else {
          stack.pop_back();
        }
        break;
      case OpCode::kCreateFrame:
        frame_stack.push_back(create_frame(arena, op_code.create_frame().slots(), frame_stack.back()));
        break;
      case OpCode::kPopFrame:
        frame_stack.pop_back();
        break;
      case OpCode::kPop:
        stack.pop_back();
        break;
      case OpCode::kUnaryNot: {
        assert(!stack.empty());
        stack.back() = stack.back()->truthy() ? bool_false : bool_true;
        break;
      }
      case OpCode::kUnaryPlus:
        assert(!stack.empty());
        stack.back() = stack.back()->unary_plus(arena, error_callback);
        break;
      case OpCode::kUnaryMinus:
        stack.back() = stack.back()->unary_minus(arena, error_callback);
        break;
      case OpCode::kUnaryTilde:
        stack.back() = stack.back()->unary_tilde(arena, error_callback);
        break;
      case OpCode::kBinaryEqualsEquals: {
        assert(stack.size() >= 2);
        starlark_obj* element = stack.back();
        stack.pop_back();
        stack.back() = (stack.back()->equals(*element) ? bool_true : bool_false);
        break;
      }
      case OpCode::kBinaryBangEquals: {
        assert(stack.size() >= 2);
        starlark_obj* element = stack.back();
        stack.pop_back();
        stack.back() = (stack.back()->equals(*element) ? bool_false : bool_true);
        break;
      }
      case OpCode::kBinaryLessThan: {
        assert(stack.size() >= 2);
        starlark_obj* element = stack.back();
        stack.pop_back();
        stack.back() = (stack.back()->cmp(*element, "<", error_callback) < 0 ? bool_true : bool_false);
        break;
      }
      case OpCode::kBinaryLessThanEquals: {
        assert(stack.size() >= 2);
        starlark_obj* element = stack.back();
        stack.pop_back();
        stack.back() = (stack.back()->cmp(*element, "<=", error_callback) <= 0 ? bool_true : bool_false);
        break;
      }
      case OpCode::kBinaryGreaterThan: {
        assert(stack.size() >= 2);
        starlark_obj* element = stack.back();
        stack.pop_back();
        stack.back() = (stack.back()->cmp(*element, ">", error_callback) > 0 ? bool_true : bool_false);
        break;
      }
      case OpCode::kBinaryGreaterThanEquals: {
        assert(stack.size() >= 2);
        starlark_obj* element = stack.back();
        stack.pop_back();
        stack.back() = (stack.back()->cmp(*element, ">=", error_callback) >= 0 ? bool_true : bool_false);
        break;
      }
      case OpCode::kBinaryIn: {
        assert(stack.size() >= 2);
        starlark_obj* sequence = stack.back();
        stack.pop_back();
        stack.back() = sequence->binary_in(*stack.back(), error_callback) ? bool_true : bool_false;
        break;
      }
      case OpCode::kBinaryNotIn: {
        assert(stack.size() >= 2);
        starlark_obj* sequence = stack.back();
        stack.pop_back();
        stack.back() = sequence->binary_in(*stack.back(), error_callback) ? bool_false : bool_true;
        break;
      }
      case OpCode::kBinaryLessThanLessThan: {
        assert(stack.size() >= 2);
        starlark_obj* shift = stack.back();
        stack.pop_back();
        stack.back() = stack.back()->binary_lshift(*shift, arena, error_callback);
        break;
      }
      case OpCode::kBinaryGreaterThanGreaterThan: {
        assert(stack.size() >= 2);
        starlark_obj* shift = stack.back();
        stack.pop_back();
        stack.back() = stack.back()->binary_rshift(*shift, arena, error_callback);
        break;
      }
      case OpCode::kBinaryPipe: {
        assert(stack.size() >= 2);
        starlark_obj* shift = stack.back();
        stack.pop_back();
        stack.back() = stack.back()->binary_pipe(*shift, arena, error_callback);
        break;
      }
      case OpCode::kBinaryHat: {
        assert(stack.size() >= 2);
        starlark_obj* shift = stack.back();
        stack.pop_back();
        stack.back() = stack.back()->binary_hat(*shift, arena, error_callback);
        break;
      }
      case OpCode::kBinaryAmpersand: {
        assert(stack.size() >= 2);
        starlark_obj* shift = stack.back();
        stack.pop_back();
        stack.back() = stack.back()->binary_and(*shift, arena, error_callback);
        break;
      }
      case OpCode::kBinaryMinus: {
        assert(stack.size() >= 2);
        starlark_obj* shift = stack.back();
        stack.pop_back();
        stack.back() = stack.back()->binary_minus(*shift, arena, error_callback);
        break;
      }
      case OpCode::kBinaryPlus: {
        assert(stack.size() >= 2);
        starlark_obj* shift = stack.back();
        stack.pop_back();
        stack.back() = stack.back()->binary_plus(*shift, arena, error_callback);
        break;
      }
      case OpCode::kBinaryStar: {
        assert(stack.size() >= 2);
        starlark_obj* shift = stack.back();
        stack.pop_back();
        stack.back() = stack.back()->binary_star(*shift, arena, error_callback);
        break;
      }
      case OpCode::kBinaryPercent: {
        assert(stack.size() >= 2);
        starlark_obj* shift = stack.back();
        stack.pop_back();
        stack.back() = stack.back()->binary_percent(*shift, arena, error_callback);
        break;
      }
      case OpCode::kBinarySlash: {
        assert(stack.size() >= 2);
        starlark_obj* shift = stack.back();
        stack.pop_back();
        stack.back() = stack.back()->binary_slash(*shift, arena, error_callback);
        break;
      }
      case OpCode::kBinarySlashSlash: {
        assert(stack.size() >= 2);
        starlark_obj* shift = stack.back();
        stack.pop_back();
        stack.back() = stack.back()->binary_slash_slash(*shift, arena, error_callback);
        break;
      }
      case OpCode::kEnd:
        assert(stack.empty());
        return result;
      case OpCode::kFail:
        return nullptr;
      case OpCode::kGoto:
      case OpCode::kGetIterator:
      case OpCode::kForIterator:
      case OpCode::kEndIterator:
      case OpCode::kAssignDotMember:
      case OpCode::kAssignIndexMember:
      case OpCode::kAssignSliceRange:
      case OpCode::kPlusAssign:
      case OpCode::kMinusAssign:
      case OpCode::kStarAssign:
      case OpCode::kSlashAssign:
      case OpCode::kSlashSlashAssign:
      case OpCode::kPercentAssign:
      case OpCode::kAmpersandAssign:
      case OpCode::kPipeAssign:
      case OpCode::kHatAssign:
      case OpCode::kLessLessAssign:
      case OpCode::kGreaterGreaterAssign:
      case OpCode::kDotMember:
      case OpCode::kCall:
      case OpCode::kLoadModule:
      case OpCode::kConstNone:
      case OpCode::kIndexMember:
      case OpCode::kSliceRange:
      case OpCode::kReturn:
      case OpCode::kMakeFunction:
      case OpCode::kSetDefaultValues:
      case OpCode::OP_CODE_NOT_SET:
        // TODO(lmirelmann): Implement the other instructions.
        // TODO(lmirelmann): Log the error, but for this, we would need to know what the position of the operation is.
std::cerr << "Unknown op-code: " << op_code.op_code_case() << "\n";
        return nullptr;
    }
  }

  return result;
}

}  // namespace

frame::frame(std::size_t size) : elements(size) {}

// TODO(lmirelmann): There has to be a way to define the loader.
interpreter::interpreter() {}

// TODO(lmirelmann): There has to be a way to define the global context.
// TODO(lmirelmann): There has to be a way to define the parsing options.
// TODO(lmirelmann): The logger should be configurable.
frame* interpreter::run(std::string_view starlark_code, Arena& arena) {
  std::map<std::string, starlark_obj*, std::less<>> global_context;
  // TODO(lmirelmann): Add the other elements from the binding.
  for (const auto& symbol : predeclared_symbols) {
    global_context[symbol] = nullptr;
  }
  global_context["True"] = Arena::Create<starlark_bool>(&arena, true);
  global_context["False"] = Arena::Create<starlark_bool>(&arena, false);
  global_context["None"] = Arena::Create<starlark_none>(&arena);

  std::set<std::string, std::less<>> binding;
  for (const auto& [symbol, value] : global_context) {
    binding.insert(symbol);
  }

  class compiler star_compiler(binding);
  logger logging;
  Program* starlark_program = star_compiler.compile(starlark_code, options{}, logging, arena);
  if (starlark_program == nullptr) {
    return nullptr;
  }
  frame* global_frame = create_frame(arena, global_context.size(), nullptr);
  {
    int pos = 0;
    for (auto& [symbol, value] : global_context) {
      global_frame->elements[pos++] = value;
    }
  }
  return run_program(starlark_program, global_frame, arena, logging);
}

}  // namespace interpreter
}  // namespace starlark

