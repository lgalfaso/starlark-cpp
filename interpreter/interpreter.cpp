// Copyright 2024-2025 Lucas Mirelmann

#include "interpreter/interpreter.hpp"

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
#include "runtime/starlark_function.hpp"
#include "runtime/starlark_integer.hpp"
#include "runtime/starlark_list.hpp"
#include "runtime/starlark_none.hpp"
#include "runtime/starlark_string.hpp"
#include "runtime/starlark_tuple.hpp"
#include "runtime/starlark_types.hpp"

using ::google::protobuf::Arena;
using ::google::protobuf::RepeatedPtrField;
using ::starlark::bigint::number;
using ::starlark::bigint::parse_number;
using ::starlark::bytecode::BlockType;
using ::starlark::bytecode::OpCode;
using ::starlark::bytecode::Program;
using ::starlark::compiler::compiler;
using ::starlark::grammar::options;
using ::starlark::grammar::predeclared_symbols;
using ::starlark::logging::logger;
using ::starlark::runtime::error_fn;
using ::starlark::runtime::starlark_bigint;
using ::starlark::runtime::starlark_bool;
using ::starlark::runtime::starlark_built_in_function;
using ::starlark::runtime::starlark_bytes;
using ::starlark::runtime::starlark_dictionary;
using ::starlark::runtime::starlark_float;
using ::starlark::runtime::starlark_fn_abs;
using ::starlark::runtime::starlark_fn_all;
using ::starlark::runtime::starlark_fn_any;
using ::starlark::runtime::starlark_fn_bool;
using ::starlark::runtime::starlark_fn_bytes;
using ::starlark::runtime::starlark_fn_chr;
using ::starlark::runtime::starlark_fn_dict;
using ::starlark::runtime::starlark_fn_dir;
using ::starlark::runtime::starlark_fn_enumerate;
using ::starlark::runtime::starlark_fn_fail;
using ::starlark::runtime::starlark_fn_float;
using ::starlark::runtime::starlark_fn_getattr;
using ::starlark::runtime::starlark_fn_hasattr;
using ::starlark::runtime::starlark_fn_hash;
using ::starlark::runtime::starlark_fn_int;
using ::starlark::runtime::starlark_fn_len;
using ::starlark::runtime::starlark_fn_list;
using ::starlark::runtime::starlark_fn_max;
using ::starlark::runtime::starlark_fn_min;
using ::starlark::runtime::starlark_fn_ord;
using ::starlark::runtime::starlark_fn_print;
using ::starlark::runtime::starlark_fn_range;
using ::starlark::runtime::starlark_fn_repr;
using ::starlark::runtime::starlark_fn_reversed;
using ::starlark::runtime::starlark_fn_set;
using ::starlark::runtime::starlark_fn_sorted;
using ::starlark::runtime::starlark_fn_str;
using ::starlark::runtime::starlark_fn_tuple;
using ::starlark::runtime::starlark_fn_type;
using ::starlark::runtime::starlark_fn_zip;
using ::starlark::runtime::starlark_integer;
using ::starlark::runtime::starlark_list;
using ::starlark::runtime::starlark_none;
using ::starlark::runtime::starlark_obj;
using ::starlark::runtime::starlark_string;
using ::starlark::runtime::starlark_tuple;
using ::starlark::runtime::starlark_types;

namespace starlark {
namespace interpreter {

namespace {

frame* create_frame(Arena& arena, std::size_t size, frame* parent_frame, const RepeatedPtrField<std::string>* names) {
  frame* result = Arena::Create<frame>(&arena, size, names);
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

frame* run_program(Program* starlark_program, std::map<std::string, starlark_obj*, std::less<>>& global_context, Arena& arena, logger& log) {
  std::vector<starlark_obj*> stack;
  std::vector<frame*> frame_stack;
  std::vector<std::pair<int, int>> call_stack;
  int instruction_ptr = 0;
  int block_ptr = 0;

  if (starlark_program == nullptr) {
    return nullptr;
  }
  // TODO(lmirelmann): This should go into a structure that keeps some of the constants.
  starlark_obj* bool_true = global_context["True"];
  starlark_obj* bool_false = global_context["False"];
  starlark_program->mutable_block(0)->add_op_code()->mutable_fail();
  error_handler error_callback(block_ptr, instruction_ptr, starlark_program->block(0).op_code_size() - 1, log);

  frame* result = nullptr;
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
        assert(stack.size() > op_code.add_to_list().number_of_elements());
        starlark_obj* candidate_list = stack[stack.size() - 1 - op_code.add_to_list().number_of_elements()];
        assert(candidate_list->type() == starlark_types::list_t);
        starlark_list* list = static_cast<starlark_list*>(candidate_list);
        assert(stack.size() >= op_code.add_to_list().number_of_elements());
        for (int i = 0; i < op_code.add_to_list().number_of_elements(); ++i) {
          list->add(stack[stack.size() - op_code.add_to_list().number_of_elements() + i], error_callback);
        }
        stack.resize(stack.size() - op_code.add_to_list().number_of_elements(), nullptr);
        break;
      }
      case OpCode::kMakeDictionary:
        // Note: The reserve size is not used.
        stack.push_back(Arena::Create<starlark_dictionary>(&arena));
        break;
      case OpCode::kAddToDictionary: {
        assert(stack.size() > op_code.add_to_dictionary().number_of_elements() * 2);
        starlark_obj* candidate_dict = stack[stack.size() - 1 - op_code.add_to_dictionary().number_of_elements() * 2];
        assert(candidate_dict->type() == starlark_types::dict_t);
        starlark_dictionary* dict = static_cast<starlark_dictionary*>(candidate_dict);
        assert(op_code.add_to_list().number_of_elements() < std::numeric_limits<decltype(op_code.add_to_list().number_of_elements())>::max() / 2);
        assert(stack.size() >= op_code.add_to_list().number_of_elements() * 2);
        for (int i = 0; i < op_code.add_to_dictionary().number_of_elements(); ++i) {
          auto* key = stack[stack.size() - 2 * op_code.add_to_dictionary().number_of_elements() + 2 * i];
          if (!dict->insert(key, stack[stack.size() - 2 * op_code.add_to_dictionary().number_of_elements() + 2 * i + 1], error_callback).first &&
              op_code.add_to_dictionary().number_of_elements() > 1) {
            // This makes use of the fact that dictionary comprehensions always add elements one at a time and
            // that dictionary literals add all the elements in one go.
            error_callback.add_error(std::format("Error: dictionary expression has duplicate key: {}", key->repr()));
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
          const auto& name = frame_stack[frame_stack.size() - 1 - op_code.load().frame()]->names->Get(op_code.load().pos_in_frame());
          error_callback.add_error(std::format("UnboundLocalError: cannot access local variable '{}' where it is not associated with a value", name));
          break;
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
        switch (op_code.create_frame().block_type()) {
          case BlockType::PREDECLARED_BLOCK: {
            assert(frame_stack.empty());
            auto* global_frame = create_frame(arena, op_code.create_frame().symbol().size(), nullptr, &op_code.create_frame().symbol());
            int count = 0;
            for (const auto& symbol : op_code.create_frame().symbol()) {
              auto pos = global_context.find(symbol);
              if (pos == global_context.end()) {
                error_callback.add_error(std::format("Error: Required symbol {} not avaible in the global context", symbol));
                return nullptr;
              } else {
                global_frame->elements[count] = pos->second;
                count++;
              }
            }
            frame_stack.push_back(global_frame);
            break;
          }
          case BlockType::MODULE_BLOCK:
            assert(result == nullptr);
            result = create_frame(arena, op_code.create_frame().symbol().size(), frame_stack.back(), &op_code.create_frame().symbol());
            frame_stack.push_back(result);
            break;
          default:
            frame_stack.push_back(create_frame(arena, op_code.create_frame().symbol().size(), frame_stack.back(), &op_code.create_frame().symbol()));
            break;
        }
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
      case OpCode::kCall: {
        int args_count = op_code.call().positional_arguments_count() +
            2 * op_code.call().named_arguments_count() +
            (op_code.call().has_variadic_positional_argument() ? 1 : 0) +
            (op_code.call().has_variadic_named_argument() ? 1 : 0);
        assert(stack.size() >= args_count + 1);
        /*
          int32 positional_arguments_count = 1;
          int32 named_arguments_count = 2;
          bool has_variadic_positional_argument = 3;
          bool has_variadic_named_argument = 4;
        */
        std::vector<starlark_obj*> pos_args;
        std::map<std::string, starlark_obj*> named_args;
        for (int i = 0; i < op_code.call().positional_arguments_count(); ++i) {
          pos_args.push_back(stack[stack.size() - args_count + i]);
        }
        for (int i = 0; i < op_code.call().named_arguments_count(); ++i) {
          auto* key = stack[stack.size() - args_count + op_code.call().positional_arguments_count() + 2 * i];
          auto* value = stack[stack.size() - args_count + op_code.call().positional_arguments_count() + 2 * i + 1];
          assert(key->type() == starlark_types::string_t);
          named_args.emplace(key->as_string(), value);
        }
        // TODO(lmirelmann): Get the variadic arguments.
        // TODO(lmirelmann): Get the named variadic arguments.
        stack.resize(stack.size() - args_count, nullptr);
        stack.back() = stack.back()->call(pos_args, named_args, arena, error_callback);
        break;
      }
      case OpCode::kGetIterator:
        assert(!frame_stack.empty());
        assert(!stack.empty());
        frame_stack.back()->iterators.push_back(stack.back()->get_iterator(true, arena, error_callback));
        stack.pop_back();
        break;
      case OpCode::kForIterator: {
        assert(!frame_stack.empty());
        assert(!frame_stack.back()->iterators.empty());
        auto* it = frame_stack.back()->iterators.back();
        if (it->has_next()) {
          stack.push_back(it->next());
        } else {
          instruction_ptr = op_code.for_iterator().address();
        }
        break;
      }
      case OpCode::kEndIterator:
        assert(!frame_stack.empty());
        assert(!frame_stack.back()->iterators.empty());
        frame_stack.back()->iterators.back()->end_iterator();
        frame_stack.back()->iterators.pop_back();
        break;
      case OpCode::kGoto:
        instruction_ptr = op_code.goto_().address();
        break;
      case OpCode::kIndexMember: {
        assert(stack.size() >= 2);
        auto* index = stack.back();
        stack.pop_back();
        stack.back() = stack.back()->index(*index, arena, error_callback);
        break;
      }
      case OpCode::kAssignIndexMember: {
        assert(stack.size() >= 3);
        auto* index = stack.back();
        stack.pop_back();
        auto* container = stack.back();
        stack.pop_back();
        auto* element = stack.back();
        stack.pop_back();
        container->index_assign(*index, *element, error_callback);
        break;
      }
      case OpCode::kEnd:
        assert(stack.empty());
        return result;
      case OpCode::kFail:
        return nullptr;
      case OpCode::kAssignDotMember:
      case OpCode::kAssignSliceRange:
      case OpCode::kAssignPlusEquals:
      case OpCode::kAssignMinusEquals:
      case OpCode::kAssignStarEquals:
      case OpCode::kAssignSlashEquals:
      case OpCode::kAssignSlashSlashEquals:
      case OpCode::kAssignPercentEquals:
      case OpCode::kAssignAmpersandEquals:
      case OpCode::kAssignPipeEquals:
      case OpCode::kAssignHatEquals:
      case OpCode::kAssignLessLessEquals:
      case OpCode::kAssignGreaterGreaterEquals:
      case OpCode::kDotMember:
      case OpCode::kLoadModule:
      case OpCode::kConstNone:
      case OpCode::kSliceRange:
      case OpCode::kReturn:
      case OpCode::kMakeFunction:
      case OpCode::kSetDefaultValues:
      case OpCode::kAssignDotMemberPlusEquals:
      case OpCode::kAssignDotMemberMinusEquals:
      case OpCode::kAssignDotMemberStarEquals:
      case OpCode::kAssignDotMemberSlashEquals:
      case OpCode::kAssignDotMemberSlashSlashEquals:
      case OpCode::kAssignDotMemberPercentEquals:
      case OpCode::kAssignDotMemberAmpersandEquals:
      case OpCode::kAssignDotMemberPipeEquals:
      case OpCode::kAssignDotMemberHatEquals:
      case OpCode::kAssignDotMemberLessLessEquals:
      case OpCode::kAssignDotMemberGreaterGreaterEquals:
      case OpCode::kAssignIndexMemberPlusEquals:
      case OpCode::kAssignIndexMemberMinusEquals:
      case OpCode::kAssignIndexMemberStarEquals:
      case OpCode::kAssignIndexMemberSlashEquals:
      case OpCode::kAssignIndexMemberSlashSlashEquals:
      case OpCode::kAssignIndexMemberPercentEquals:
      case OpCode::kAssignIndexMemberAmpersandEquals:
      case OpCode::kAssignIndexMemberPipeEquals:
      case OpCode::kAssignIndexMemberHatEquals:
      case OpCode::kAssignIndexMemberLessLessEquals:
      case OpCode::kAssignIndexMemberGreaterGreaterEquals:
      case OpCode::kAssignSliceRangePlusEquals:
      case OpCode::kAssignSliceRangeMinusEquals:
      case OpCode::kAssignSliceRangeStarEquals:
      case OpCode::kAssignSliceRangeSlashEquals:
      case OpCode::kAssignSliceRangeSlashSlashEquals:
      case OpCode::kAssignSliceRangePercentEquals:
      case OpCode::kAssignSliceRangeAmpersandEquals:
      case OpCode::kAssignSliceRangePipeEquals:
      case OpCode::kAssignSliceRangeHatEquals:
      case OpCode::kAssignSliceRangeLessLessEquals:
      case OpCode::kAssignSliceRangeGreaterGreaterEquals:

      case OpCode::OP_CODE_NOT_SET:
        // TODO(lmirelmann): Implement the other instructions.
        error_callback.add_error(std::format("Unknown op-code: {}", op_code.ShortDebugString()));
        return nullptr;
    }
  }
  return result;
}

}  // namespace

frame::frame(std::size_t size, const RepeatedPtrField<std::string>* names) : elements(size), names(names) {}

// TODO(lmirelmann): There has to be a way to define the loader.
interpreter::interpreter() {}

// TODO(lmirelmann): There has to be a way to define the parsing options.
// TODO(lmirelmann): There has to be a way to define the runtime options.
frame* interpreter::run(std::string_view starlark_code,
                        const std::map<std::string, starlark_obj*, std::less<>>& custom_binding,
                        Arena& arena, logger& logging) {
  std::set<std::string, std::less<>> binding;
  for (const auto& [key, value] : custom_binding) {
    binding.insert(key);
  }
  class compiler star_compiler(binding);
  Program* starlark_program = star_compiler.compile(starlark_code, options{}, logging, arena);
  if (starlark_program == nullptr) {
    return nullptr;
  }

  // TODO(lmirelmann): Add the other elements from the binding.
  std::map<std::string, starlark_obj*, std::less<>> global_context;
  global_context["True"] = Arena::Create<starlark_bool>(&arena, true);
  global_context["False"] = Arena::Create<starlark_bool>(&arena, false);
  global_context["None"] = Arena::Create<starlark_none>(&arena);
  global_context["abs"] = Arena::Create<starlark_built_in_function>(&arena, starlark_fn_abs, "abs");
  global_context["all"] = Arena::Create<starlark_built_in_function>(&arena, starlark_fn_all, "all");
  global_context["any"] = Arena::Create<starlark_built_in_function>(&arena, starlark_fn_any, "any");
  global_context["bool"] = Arena::Create<starlark_built_in_function>(&arena, starlark_fn_bool, "bool");
  global_context["bytes"] = Arena::Create<starlark_built_in_function>(&arena, starlark_fn_bytes, "bytes");
  global_context["chr"] = Arena::Create<starlark_built_in_function>(&arena, starlark_fn_chr, "chr");
  global_context["dict"] = Arena::Create<starlark_built_in_function>(&arena, starlark_fn_dict, "dict");
  global_context["dir"] = Arena::Create<starlark_built_in_function>(&arena, starlark_fn_dir, "dir");
  global_context["enumerate"] = Arena::Create<starlark_built_in_function>(&arena, starlark_fn_enumerate, "enumerate");
  global_context["fail"] = Arena::Create<starlark_built_in_function>(&arena, starlark_fn_fail, "fail");
  global_context["float"] = Arena::Create<starlark_built_in_function>(&arena, starlark_fn_float, "float");
  global_context["getattr"] = Arena::Create<starlark_built_in_function>(&arena, starlark_fn_getattr, "getattr");
  global_context["hasattr"] = Arena::Create<starlark_built_in_function>(&arena, starlark_fn_hasattr, "hasattr");
  global_context["hash"] = Arena::Create<starlark_built_in_function>(&arena, starlark_fn_hash, "hash");
  global_context["int"] = Arena::Create<starlark_built_in_function>(&arena, starlark_fn_int, "int");
  global_context["len"] = Arena::Create<starlark_built_in_function>(&arena, starlark_fn_len, "len");
  global_context["list"] = Arena::Create<starlark_built_in_function>(&arena, starlark_fn_list, "list");
  global_context["max"] = Arena::Create<starlark_built_in_function>(&arena, starlark_fn_max, "max");
  global_context["min"] = Arena::Create<starlark_built_in_function>(&arena, starlark_fn_min, "min");
  global_context["ord"] = Arena::Create<starlark_built_in_function>(&arena, starlark_fn_ord, "ord");
  global_context["print"] = Arena::Create<starlark_built_in_function>(&arena, starlark_fn_print, "print");
  global_context["range"] = Arena::Create<starlark_built_in_function>(&arena, starlark_fn_range, "range");
  global_context["repr"] = Arena::Create<starlark_built_in_function>(&arena, starlark_fn_repr, "repr");
  global_context["reversed"] = Arena::Create<starlark_built_in_function>(&arena, starlark_fn_reversed, "reversed");
  global_context["set"] = Arena::Create<starlark_built_in_function>(&arena, starlark_fn_set, "set");
  global_context["sorted"] = Arena::Create<starlark_built_in_function>(&arena, starlark_fn_sorted, "sorted");
  global_context["str"] = Arena::Create<starlark_built_in_function>(&arena, starlark_fn_str, "str");
  global_context["tuple"] = Arena::Create<starlark_built_in_function>(&arena, starlark_fn_tuple, "tuple");
  global_context["type"] = Arena::Create<starlark_built_in_function>(&arena, starlark_fn_type, "type");
  global_context["zip"] = Arena::Create<starlark_built_in_function>(&arena, starlark_fn_zip, "zip");
  for (const auto& kv : custom_binding) {
    global_context.insert(kv);
  }
  return run_program(starlark_program, global_context, arena, logging);
}

}  // namespace interpreter
}  // namespace starlark

