// Copyright 2026 Lucas Mirelmann

#include "native/ir/opcode_lowering.hpp"

#include <cassert>

#include <format>
#include <string_view>

#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Module.h"
#include "native/ir/ir_object_ops.hpp"

namespace starlark {
namespace native {

namespace {

using ::starlark::bytecode::BlockType;
using ::starlark::bytecode::OpCode;

llvm::Value* member_global(llvm::IRBuilderBase& builder, std::string_view member) {
  return builder.CreateGlobalString(member);
}

void call_obj(lowering_context& lowering, llvm::IRBuilderBase& builder, const char* name, llvm::ArrayRef<llvm::Value*> args) {
  builder.CreateCall(lowering.ir_exec.object_fn(name), args);
}

void call_obj_with_member(lowering_context& lowering,
    llvm::IRBuilderBase& builder,
    const char* name,
    llvm::Value* exec,
    std::string_view member,
    llvm::ArrayRef<llvm::Value*> tail) {
  auto* member_const = member_global(builder, member);
  std::vector<llvm::Value*> args = {exec, member_const, llvm::ConstantInt::get(lowering.ir_exec.i64_ty(), member.size())};
  args.insert(args.end(), tail.begin(), tail.end());
  builder.CreateCall(lowering.ir_exec.object_fn(name), args);
}

#define COMPOUND_FRAME(op_case, op_method, kind)                                                                                        \
  case op_case:                                                                                                                         \
    call_obj(lowering,                                                                                                                  \
        builder,                                                                                                                        \
        "starlark_obj_rt_frame_compound_assign",                                                                                        \
        {exec,                                                                                                                          \
            llvm::ConstantInt::get(i32, op.op_method().frame()),                                                                        \
            llvm::ConstantInt::get(i32, op.op_method().pos_in_frame()),                                                                 \
            llvm::ConstantInt::get(i32, kind),                                                                                          \
            ctx,                                                                                                                        \
            err});                                                                                                                      \
    break

#define COMPOUND_INDEX(op, kind)                                                                                                        \
  case op:                                                                                                                              \
    call_obj(lowering, builder, "starlark_obj_rt_index_compound_assign", {exec, llvm::ConstantInt::get(i32, kind), ctx, err});          \
    break

#define COMPOUND_DOT(op_case, op_method, kind)                                                                                          \
  case op_case:                                                                                                                         \
    call_obj_with_member(lowering,                                                                                                      \
        builder,                                                                                                                        \
        "starlark_obj_rt_dot_compound_assign",                                                                                          \
        exec,                                                                                                                           \
        op.op_method().member(),                                                                                                        \
        {llvm::ConstantInt::get(i32, kind), ctx, err});                                                                                 \
    break

#define SLICE_COMPOUND(kind)                                                                                                            \
  call_obj(lowering, builder, "starlark_obj_rt_assign_slice_range_op", {exec, llvm::ConstantInt::get(i32, kind), ctx, err});            \
  break

}  // namespace

void lower_opcode(lowering_context& lowering,
    llvm::IRBuilderBase& builder,
    llvm::Function* fn,
    llvm::Value* exec,
    llvm::Value* ctx,
    llvm::Value* err,
    llvm::BasicBlock* error_bb,
    llvm::BasicBlock* next_bb,
    int block_idx,
    int ip,
    const starlark::bytecode::OpCode& op,
    const starlark::bytecode::Program& program) {
  auto& ir_exec = lowering.ir_exec;
  auto* i32 = ir_exec.i32_ty();
  (void)error_bb;
  (void)next_bb;
  const starlark::vm::module_metadata* metadata = lowering.options.metadata;

  switch (op.op_code_case()) {
    case OpCode::kConstNone:
      ir_object_ops::emit_push_none(lowering, builder, exec, ctx);
      break;
    case OpCode::kConstInt: {
      auto* value = ir_exec.emit_box_int_value(builder, ctx, llvm::ConstantInt::get(ir_exec.i64_ty(), op.const_int().value()));
      ir_exec.emit_push(builder, exec, value);
      break;
    }
    case OpCode::kConstFloat: {
      auto* value = builder.CreateCall(ir_exec.object_fn("starlark_obj_rt_create_float"),
          {ctx, llvm::ConstantFP::get(llvm::Type::getDoubleTy(builder.getContext()), op.const_float().value())});
      ir_exec.emit_push(builder, exec, value);
      break;
    }
    case OpCode::kConstString: {
      auto* value = builder.CreateCall(ir_exec.object_fn("starlark_obj_rt_get_const_string"),
          {exec, llvm::ConstantInt::get(ir_exec.i64_ty(), op.const_string().const_string_pos())});
      ir_exec.emit_push(builder, exec, value);
      break;
    }
    case OpCode::kConstStringView: {
      auto* value = builder.CreateCall(ir_exec.object_fn("starlark_obj_rt_get_const_string"),
          {exec, llvm::ConstantInt::get(ir_exec.i64_ty(), op.const_string_view().const_string_pos())});
      ir_exec.emit_push(builder, exec, value);
      break;
    }
    case OpCode::kConstBigInt: {
      auto* digits = builder.CreateGlobalString(op.const_big_int().value());
      call_obj(lowering, builder, "starlark_obj_rt_make_bigint", {exec, digits, ctx});
      break;
    }
    case OpCode::kConstBytes: {
      const auto& bytes = op.const_bytes().value();
      auto* data = builder.CreateGlobalString(bytes);
      call_obj(lowering, builder, "starlark_obj_rt_make_bytes", {exec, data, llvm::ConstantInt::get(ir_exec.i64_ty(), bytes.size()), ctx});
      break;
    }
    case OpCode::kLoad: {
      auto* value = ir_exec.emit_load_slot_inline(builder,
          fn,
          exec,
          llvm::ConstantInt::get(i32, op.load().frame()),
          llvm::ConstantInt::get(i32, op.load().pos_in_frame()),
          err);
      ir_exec.emit_push(builder, exec, value);
      break;
    }
    case OpCode::kStore: {
      auto* value = ir_exec.emit_pop(builder, exec);
      ir_exec.emit_store_slot_inline(builder,
          exec,
          llvm::ConstantInt::get(i32, op.store().frame()),
          llvm::ConstantInt::get(i32, op.store().pos_in_frame()),
          value);
      break;
    }
    case OpCode::kPop:
      ir_exec.emit_pop(builder, exec);
      break;
    case OpCode::kPopFrame:
      ir_exec.emit_pop_frame(builder, exec);
      break;
    case OpCode::kCreateFrame:
      if (op.create_frame().block_type() == BlockType::PREDECLARED_BLOCK) {
        builder.CreateCall(lowering.module.getFunction("starlark_rt_exec_create_predeclared"), {exec, fn->getArg(0), ctx, err});
      } else if (op.create_frame().block_type() == BlockType::MODULE_BLOCK) {
        assert(metadata != nullptr);
        ir_object_ops::emit_create_frame_from_meta(lowering,
            builder,
            exec,
            starlark::vm::frame_meta_index(*metadata, BlockType::MODULE_BLOCK));
      } else if (op.create_frame().block_type() == BlockType::FILE_BLOCK && block_idx == 0) {
        assert(metadata != nullptr);
        ir_object_ops::emit_create_frame_from_meta(lowering,
            builder,
            exec,
            starlark::vm::frame_meta_index(*metadata, BlockType::FILE_BLOCK));
      } else if (op.create_frame().block_type() != BlockType::FUNCTION_BLOCK) {
        assert(metadata != nullptr);
        const int meta_index = starlark::vm::frame_meta_index_at(*metadata, block_idx, ip);
        assert(meta_index >= 0);
        ir_object_ops::emit_create_frame_from_meta(lowering, builder, exec, meta_index);
      }
      break;
    case OpCode::kMakeFunction: {
      const int entry = op.make_function().entrypoint();
      auto it = lowering.function_blocks.find(entry);
      assert(it != lowering.function_blocks.end() && it->second != nullptr);
      assert(metadata != nullptr);
      assert(entry > 0 && static_cast<std::size_t>(entry - 1) < metadata->functions.size());
      llvm::Value* fn_ptr = llvm::ConstantExpr::getBitCast(it->second, llvm::PointerType::getUnqual(builder.getContext()));
      const auto& fn_meta = metadata->functions[static_cast<std::size_t>(entry - 1)];
      auto* name_const = builder.CreateGlobalString(fn_meta.fn_name);
      builder.CreateCall(lowering.ir_exec.object_fn("starlark_obj_rt_make_native_function_meta"),
          {exec,
              fn_ptr,
              name_const,
              llvm::ConstantInt::get(lowering.ir_exec.i64_ty(), fn_meta.fn_name.size()),
              llvm::ConstantInt::get(i32, entry),
              llvm::ConstantInt::get(i32, op.make_function().default_values_count())});
      break;
    }
    case OpCode::kMakeTuple:
      ir_object_ops::emit_make_tuple_inline(lowering, builder, exec, ctx, op.make_tuple().number_of_elements());
      break;
    case OpCode::kMakeList:
      call_obj(lowering, builder, "starlark_obj_rt_make_list", {exec, llvm::ConstantInt::get(i32, op.make_list().reserve_size()), ctx});
      break;
    case OpCode::kAddToList:
      call_obj(lowering, builder, "starlark_obj_rt_add_to_list", {exec, llvm::ConstantInt::get(i32, op.add_to_list().number_of_elements()), ctx, err});
      break;
    case OpCode::kMakeDictionary:
      call_obj(lowering, builder, "starlark_obj_rt_make_dict", {exec, ctx});
      break;
    case OpCode::kAddToDictionary:
      call_obj(lowering, builder, "starlark_obj_rt_add_to_dict", {exec, llvm::ConstantInt::get(i32, op.add_to_dictionary().number_of_elements()), ctx, err});
      break;
    case OpCode::kUnaryNot:
      call_obj(lowering, builder, "starlark_obj_rt_unary_not", {exec, ctx, err});
      break;
    case OpCode::kUnaryPlus:
      call_obj(lowering, builder, "starlark_obj_rt_unary_plus", {exec, ctx, err});
      break;
    case OpCode::kUnaryMinus:
      call_obj(lowering, builder, "starlark_obj_rt_unary_minus", {exec, ctx, err});
      break;
    case OpCode::kUnaryTilde:
      call_obj(lowering, builder, "starlark_obj_rt_unary_tilde", {exec, ctx, err});
      break;
    case OpCode::kBinaryEqualsEquals:
      ir_object_ops::emit_cmp_with_int_fastpath(lowering, builder, fn, exec, ctx, err, ir_object_ops::int_cmp_kind::kEq, "starlark_obj_rt_cmp_eq");
      break;
    case OpCode::kBinaryBangEquals:
      ir_object_ops::emit_cmp_with_int_fastpath(lowering, builder, fn, exec, ctx, err, ir_object_ops::int_cmp_kind::kNe, "starlark_obj_rt_cmp_ne");
      break;
    case OpCode::kBinaryLessThan:
      ir_object_ops::emit_cmp_with_int_fastpath(lowering, builder, fn, exec, ctx, err, ir_object_ops::int_cmp_kind::kLt, "starlark_obj_rt_cmp_lt");
      break;
    case OpCode::kBinaryLessThanEquals:
      ir_object_ops::emit_cmp_with_int_fastpath(lowering, builder, fn, exec, ctx, err, ir_object_ops::int_cmp_kind::kLe, "starlark_obj_rt_cmp_le");
      break;
    case OpCode::kBinaryGreaterThan:
      ir_object_ops::emit_cmp_with_int_fastpath(lowering, builder, fn, exec, ctx, err, ir_object_ops::int_cmp_kind::kGt, "starlark_obj_rt_cmp_gt");
      break;
    case OpCode::kBinaryGreaterThanEquals:
      ir_object_ops::emit_cmp_with_int_fastpath(lowering, builder, fn, exec, ctx, err, ir_object_ops::int_cmp_kind::kGe, "starlark_obj_rt_cmp_ge");
      break;
    case OpCode::kBinaryIn:
      call_obj(lowering, builder, "starlark_obj_rt_cmp_in", {exec, ctx, err});
      break;
    case OpCode::kBinaryNotIn:
      call_obj(lowering, builder, "starlark_obj_rt_cmp_not_in", {exec, ctx, err});
      break;
    case OpCode::kBinaryLessThanLessThan:
      call_obj(lowering, builder, "starlark_obj_rt_binary_lshift", {exec, ctx, err});
      break;
    case OpCode::kBinaryGreaterThanGreaterThan:
      call_obj(lowering, builder, "starlark_obj_rt_binary_rshift", {exec, ctx, err});
      break;
    case OpCode::kBinaryPipe:
      ir_object_ops::emit_binary_with_int_fastpath(lowering, builder, fn, exec, ctx, err, ir_object_ops::int_binop_kind::kOr, "starlark_obj_rt_binary_pipe");
      break;
    case OpCode::kBinaryHat:
      ir_object_ops::emit_binary_with_int_fastpath(lowering, builder, fn, exec, ctx, err, ir_object_ops::int_binop_kind::kXor, "starlark_obj_rt_binary_hat");
      break;
    case OpCode::kBinaryAmpersand:
      ir_object_ops::emit_binary_with_int_fastpath(lowering, builder, fn, exec, ctx, err, ir_object_ops::int_binop_kind::kAnd, "starlark_obj_rt_binary_ampersand");
      break;
    case OpCode::kBinaryStar:
      ir_object_ops::emit_binary_with_int_fastpath(lowering, builder, fn, exec, ctx, err, ir_object_ops::int_binop_kind::kMul, "starlark_obj_rt_binary_star");
      break;
    case OpCode::kBinaryPercent:
      call_obj(lowering, builder, "starlark_obj_rt_binary_percent", {exec, ctx, err});
      break;
    case OpCode::kBinarySlash:
      call_obj(lowering, builder, "starlark_obj_rt_binary_slash", {exec, ctx, err});
      break;
    case OpCode::kBinarySlashSlash:
      call_obj(lowering, builder, "starlark_obj_rt_binary_slash_slash", {exec, ctx, err});
      break;
    case OpCode::kBinaryPlus:
      ir_object_ops::emit_binary_with_int_fastpath(lowering, builder, fn, exec, ctx, err, ir_object_ops::int_binop_kind::kAdd, "starlark_obj_rt_binary_plus");
      break;
    case OpCode::kBinaryMinus:
      ir_object_ops::emit_binary_with_int_fastpath(lowering, builder, fn, exec, ctx, err, ir_object_ops::int_binop_kind::kSub, "starlark_obj_rt_binary_minus");
      break;
    case OpCode::kCallPos:
      ir_object_ops::emit_call_pos_inline(lowering, builder, fn, exec, ctx, err, op.call_pos().positional_count());
      break;
    case OpCode::kCallMethodPos0:
      call_obj_with_member(lowering, builder, "starlark_obj_rt_call_method_pos0", exec, op.call_method_pos0().member(), {ctx, err});
      break;
    case OpCode::kCallMethodPos1:
      call_obj_with_member(lowering, builder, "starlark_obj_rt_call_method_pos1", exec, op.call_method_pos1().member(), {ctx, err});
      break;
    case OpCode::kCallMethodPos2:
      call_obj_with_member(lowering, builder, "starlark_obj_rt_call_method_pos2", exec, op.call_method_pos2().member(), {ctx, err});
      break;
    case OpCode::kCallMethodPos3:
      call_obj_with_member(lowering, builder, "starlark_obj_rt_call_method_pos3", exec, op.call_method_pos3().member(), {ctx, err});
      break;
    case OpCode::kCallMethodPos:
      call_obj_with_member(lowering,
          builder,
          "starlark_obj_rt_call_method_pos",
          exec,
          op.call_method_pos().member(),
          {llvm::ConstantInt::get(i32, op.call_method_pos().positional_count()), ctx, err});
      break;
    case OpCode::kCallNamed:
      call_obj(lowering,
          builder,
          "starlark_obj_rt_call_named",
          {exec,
              llvm::ConstantInt::get(i32, op.call_named().positional_arguments_count()),
              llvm::ConstantInt::get(i32, op.call_named().named_arguments_count()),
              ctx,
              err});
      break;
    case OpCode::kCallPosStar:
      call_obj(lowering, builder, "starlark_obj_rt_call_pos_star", {exec, llvm::ConstantInt::get(i32, op.call_pos_star().positional_arguments_count()), ctx, err});
      break;
    case OpCode::kCall:
      call_obj(lowering,
          builder,
          "starlark_obj_rt_call_full",
          {exec,
              llvm::ConstantInt::get(i32, op.call().positional_arguments_count()),
              llvm::ConstantInt::get(i32, op.call().named_arguments_count()),
              llvm::ConstantInt::get(ir_exec.i1_ty(), op.call().has_variadic_positional_argument()),
              llvm::ConstantInt::get(ir_exec.i1_ty(), op.call().has_variadic_named_argument()),
              ctx,
              err});
      break;
    case OpCode::kCallPos0:
      ir_object_ops::emit_call_pos_inline(lowering, builder, fn, exec, ctx, err, 0);
      break;
    case OpCode::kCallPos1:
      ir_object_ops::emit_call_pos_inline(lowering, builder, fn, exec, ctx, err, 1);
      break;
    case OpCode::kCallPos2:
      ir_object_ops::emit_call_pos_inline(lowering, builder, fn, exec, ctx, err, 2);
      break;
    case OpCode::kCallPos3:
      ir_object_ops::emit_call_pos_inline(lowering, builder, fn, exec, ctx, err, 3);
      break;
    case OpCode::kIndexMember:
      call_obj(lowering, builder, "starlark_obj_rt_index", {exec, ctx, err});
      break;
    case OpCode::kAssignIndexMember:
      call_obj(lowering, builder, "starlark_obj_rt_assign_index_member", {exec, ctx, err});
      break;
    case OpCode::kDotMember:
      call_obj_with_member(lowering, builder, "starlark_obj_rt_dot", exec, op.dot_member().member(), {ctx, err});
      break;
    case OpCode::kAssignDotMember:
      call_obj_with_member(lowering, builder, "starlark_obj_rt_assign_dot_member", exec, op.assign_dot_member().member(), {ctx, err});
      break;
    case OpCode::kSliceRange:
      call_obj(lowering, builder, "starlark_obj_rt_slice_range", {exec, ctx, err});
      break;
    case OpCode::kAssignSliceRange:
      call_obj(lowering, builder, "starlark_obj_rt_assign_slice_range", {exec, ctx, err});
      break;
    case OpCode::kAssignSliceRangePlusEquals:
      SLICE_COMPOUND(1);
    case OpCode::kAssignSliceRangeMinusEquals:
      SLICE_COMPOUND(2);
    case OpCode::kAssignSliceRangeStarEquals:
      SLICE_COMPOUND(3);
    case OpCode::kAssignSliceRangeSlashEquals:
      SLICE_COMPOUND(4);
    case OpCode::kAssignSliceRangeSlashSlashEquals:
      SLICE_COMPOUND(5);
    case OpCode::kAssignSliceRangePercentEquals:
      SLICE_COMPOUND(6);
    case OpCode::kAssignSliceRangeAmpersandEquals:
      SLICE_COMPOUND(7);
    case OpCode::kAssignSliceRangePipeEquals:
      SLICE_COMPOUND(8);
    case OpCode::kAssignSliceRangeHatEquals:
      SLICE_COMPOUND(9);
    case OpCode::kAssignSliceRangeLessLessEquals:
      SLICE_COMPOUND(10);
    case OpCode::kAssignSliceRangeGreaterGreaterEquals:
      SLICE_COMPOUND(11);
    COMPOUND_FRAME(OpCode::kAssignPlusEquals, assign_plus_equals, 0);
    COMPOUND_FRAME(OpCode::kAssignMinusEquals, assign_minus_equals, 1);
    COMPOUND_FRAME(OpCode::kAssignStarEquals, assign_star_equals, 2);
    COMPOUND_FRAME(OpCode::kAssignSlashEquals, assign_slash_equals, 3);
    COMPOUND_FRAME(OpCode::kAssignSlashSlashEquals, assign_slash_slash_equals, 4);
    COMPOUND_FRAME(OpCode::kAssignPercentEquals, assign_percent_equals, 5);
    COMPOUND_FRAME(OpCode::kAssignAmpersandEquals, assign_ampersand_equals, 6);
    COMPOUND_FRAME(OpCode::kAssignPipeEquals, assign_pipe_equals, 7);
    COMPOUND_FRAME(OpCode::kAssignHatEquals, assign_hat_equals, 8);
    COMPOUND_FRAME(OpCode::kAssignLessLessEquals, assign_less_less_equals, 9);
    COMPOUND_FRAME(OpCode::kAssignGreaterGreaterEquals, assign_greater_greater_equals, 10);
    COMPOUND_INDEX(OpCode::kAssignIndexMemberPlusEquals, 0);
    COMPOUND_INDEX(OpCode::kAssignIndexMemberMinusEquals, 1);
    COMPOUND_INDEX(OpCode::kAssignIndexMemberStarEquals, 2);
    COMPOUND_INDEX(OpCode::kAssignIndexMemberSlashEquals, 3);
    COMPOUND_INDEX(OpCode::kAssignIndexMemberSlashSlashEquals, 4);
    COMPOUND_INDEX(OpCode::kAssignIndexMemberPercentEquals, 5);
    COMPOUND_INDEX(OpCode::kAssignIndexMemberAmpersandEquals, 6);
    COMPOUND_INDEX(OpCode::kAssignIndexMemberPipeEquals, 7);
    COMPOUND_INDEX(OpCode::kAssignIndexMemberHatEquals, 8);
    COMPOUND_INDEX(OpCode::kAssignIndexMemberLessLessEquals, 9);
    COMPOUND_INDEX(OpCode::kAssignIndexMemberGreaterGreaterEquals, 10);
    COMPOUND_DOT(OpCode::kAssignDotMemberPlusEquals, assign_dot_member_plus_equals, 0);
    COMPOUND_DOT(OpCode::kAssignDotMemberMinusEquals, assign_dot_member_minus_equals, 1);
    COMPOUND_DOT(OpCode::kAssignDotMemberStarEquals, assign_dot_member_star_equals, 2);
    COMPOUND_DOT(OpCode::kAssignDotMemberSlashEquals, assign_dot_member_slash_equals, 3);
    COMPOUND_DOT(OpCode::kAssignDotMemberSlashSlashEquals, assign_dot_member_slash_slash_equals, 4);
    COMPOUND_DOT(OpCode::kAssignDotMemberPercentEquals, assign_dot_member_percent_equals, 5);
    COMPOUND_DOT(OpCode::kAssignDotMemberAmpersandEquals, assign_dot_member_ampersand_equals, 6);
    COMPOUND_DOT(OpCode::kAssignDotMemberPipeEquals, assign_dot_member_pipe_equals, 7);
    COMPOUND_DOT(OpCode::kAssignDotMemberHatEquals, assign_dot_member_hat_equals, 8);
    COMPOUND_DOT(OpCode::kAssignDotMemberLessLessEquals, assign_dot_member_less_less_equals, 9);
    COMPOUND_DOT(OpCode::kAssignDotMemberGreaterGreaterEquals, assign_dot_member_greater_greater_equals, 10);
    case OpCode::kLoadModule: {
      const auto& module_name = op.load_module().module();
      auto* module_const = builder.CreateGlobalString(module_name);
      for (const auto& value : op.load_module().value()) {
        auto* symbol_const = builder.CreateGlobalString(value.remote_symbol());
        builder.CreateCall(ir_exec.object_fn("starlark_obj_rt_load_module_symbol"),
            {exec,
                module_const,
                llvm::ConstantInt::get(ir_exec.i64_ty(), module_name.size()),
                symbol_const,
                llvm::ConstantInt::get(ir_exec.i64_ty(), value.remote_symbol().size()),
                llvm::ConstantInt::get(i32, value.pos().frame()),
                llvm::ConstantInt::get(i32, value.pos().pos_in_frame()),
                err});
      }
      break;
    }
    case OpCode::kUnpack:
      ir_object_ops::emit_unpack_inline(lowering, builder, exec, ctx, err, op.unpack().number_of_elements());
      break;
    case OpCode::kGetIterator:
      call_obj(lowering, builder, "starlark_obj_rt_get_iterator", {exec, ctx, err});
      break;
    case OpCode::kEndIterator:
      call_obj(lowering, builder, "starlark_obj_rt_end_iterator", {exec});
      break;
    case OpCode::kFail:
      builder.CreateBr(error_bb);
      break;
    case OpCode::kNop:
      break;
    default:
      break;
  }
}

}  // namespace native
}  // namespace starlark
