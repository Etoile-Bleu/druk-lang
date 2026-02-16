#ifdef DRUK_HAVE_LLVM

#include "druk/codegen/llvm_backend.hpp"
#include "druk/codegen/opcode.hpp"

#include <llvm/ExecutionEngine/Orc/LLJIT.h>
#include <llvm/ExecutionEngine/Orc/Core.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/CFG.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/PassManager.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Passes/PassBuilder.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/DynamicLibrary.h>
#include <llvm/Support/Error.h>
#include <llvm/Transforms/InstCombine/InstCombine.h>
#include <llvm/Transforms/Scalar.h>
#include <llvm/Transforms/Scalar/GVN.h>
#include <llvm/Transforms/Scalar/Reassociate.h>
#include <llvm/Transforms/Scalar/SimplifyCFG.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/TargetParser/Host.h>
#include <llvm/TargetParser/Triple.h>

#include <algorithm>
#include <iostream>
#include <stdexcept>

// Forward declarations for X86 target initialization and JIT runtime helpers
extern "C" {
  void LLVMInitializeX86TargetInfo();
  void LLVMInitializeX86Target();
  void LLVMInitializeX86TargetMC();
  void LLVMInitializeX86AsmPrinter();
  
  void druk_jit_value_nil(PackedValue *out);
  void druk_jit_value_bool(bool b, PackedValue *out);
  void druk_jit_value_int(int64_t i, PackedValue *out);
  void druk_jit_value_function(druk::ObjFunction *fn, PackedValue *out);
  void druk_jit_add(const PackedValue *a, const PackedValue *b, PackedValue *out);
  void druk_jit_subtract(const PackedValue *a, const PackedValue *b, PackedValue *out);
  void druk_jit_multiply(const PackedValue *a, const PackedValue *b, PackedValue *out);
  void druk_jit_divide(const PackedValue *a, const PackedValue *b, PackedValue *out);
  void druk_jit_negate(const PackedValue *a, PackedValue *out);
  void druk_jit_equal(const PackedValue *a, const PackedValue *b, PackedValue *out);
  void druk_jit_less(const PackedValue *a, const PackedValue *b, PackedValue *out);
  void druk_jit_greater(const PackedValue *a, const PackedValue *b, PackedValue *out);
  void druk_jit_not(const PackedValue *a, PackedValue *out);
  void druk_jit_get_global(const char *name, size_t name_len, PackedValue *out);
  void druk_jit_define_global(const char *name, size_t name_len, const PackedValue *val);
  void druk_jit_set_global(const char *name, size_t name_len, const PackedValue *val);
  void druk_jit_build_array(const PackedValue *elements, int32_t count, PackedValue *out);
  void druk_jit_index(const PackedValue *arr_val, const PackedValue *idx_val, PackedValue *out);
  void druk_jit_index_set(PackedValue *arr_val, const PackedValue *idx_val, const PackedValue *val);
  void druk_jit_build_struct(const PackedValue *keys, const PackedValue *values, int32_t count, PackedValue *out);
  void druk_jit_get_field(const PackedValue *struct_val, const char *field, size_t field_len, PackedValue *out);
  void druk_jit_set_field(PackedValue *struct_val, const char *field, size_t field_len, const PackedValue *val);
  void druk_jit_len(const PackedValue *val, PackedValue *out);
  void druk_jit_push(PackedValue *arr_val, const PackedValue *element);
  void druk_jit_pop_array(PackedValue *arr_val, PackedValue *out);
  void druk_jit_typeof(const PackedValue *val, PackedValue *out);
  void druk_jit_keys(const PackedValue *val, PackedValue *out);
  void druk_jit_values(const PackedValue *val, PackedValue *out);
  void druk_jit_contains(const PackedValue *container, const PackedValue *item, PackedValue *out);
  void druk_jit_input(PackedValue *out);
  void druk_jit_print(const PackedValue *val);
  void druk_jit_call(const PackedValue *callee, const PackedValue *args, int32_t arg_count, PackedValue *out);
  void druk_jit_get_arg(int32_t index, PackedValue *out);
  void druk_jit_register_function(druk::ObjFunction *function, void (*fn)(PackedValue *out));
}

namespace druk {

LLVMBackend::CompilationContext::CompilationContext()
    : context(), module(nullptr), builder(context) {}

LLVMBackend::LLVMBackend(bool debug)
    : ctx_(std::make_unique<CompilationContext>()), debug_(debug) {
  // Initialize X86 target RIGHT BEFORE creating JIT
  if (debug_) {
    std::cerr << "[LLVMBackend] Initializing X86 target in constructor..." << std::endl;
  }
  LLVMInitializeX86TargetInfo();
  LLVMInitializeX86Target();
  LLVMInitializeX86TargetMC();
  LLVMInitializeX86AsmPrinter();
  if (debug_) {
    std::cerr << "[LLVMBackend] X86 target initialized in constructor" << std::endl;
  }
  
  // Get target triple
  llvm::Triple triple(llvm::sys::getDefaultTargetTriple());
  if (debug_) {
    std::cerr << "[LLVMBackend] Target triple: " << triple.str() << std::endl;
  }
  
  // Try to find the target
  std::string error;
  const llvm::Target *target = llvm::TargetRegistry::lookupTarget(triple, error);
  if (!target) {
    if (debug_) {
      std::cerr << "[LLVMBackend] Failed to lookup target: " << error << std::endl;
    }
    throw std::runtime_error("Failed to lookup target: " + error);
  }
  if (debug_) {
    std::cerr << "[LLVMBackend] Target found: " << target->getName() << std::endl;
  }
  
  // Create JIT
  if (debug_) {
    std::cerr << "[LLVMBackend] Creating LLJIT..." << std::endl;
  }
  llvm::sys::DynamicLibrary::LoadLibraryPermanently(nullptr);
  auto jit = llvm::orc::LLJITBuilder()
                 .setLinkProcessSymbolsByDefault(true)
                 .create();
  if (!jit) {
    throw std::runtime_error("Failed to create LLJIT: " + 
                           llvm::toString(jit.takeError()));
  }
  ctx_->jit = std::move(*jit);

  ctx_->module = std::make_unique<llvm::Module>("druk", ctx_->context);
  ctx_->module->setDataLayout(ctx_->jit->getDataLayout());
  ctx_->module->setTargetTriple(triple);
    // Expose runtime helpers explicitly (Windows requires exporting symbols)
    auto &jd = ctx_->jit->getMainJITDylib();
    llvm::orc::MangleAndInterner mangle(ctx_->jit->getExecutionSession(),
                      ctx_->jit->getDataLayout());
    llvm::orc::SymbolMap symbols;
    symbols[mangle("druk_jit_value_nil")] =
      {llvm::orc::ExecutorAddr::fromPtr(&druk_jit_value_nil), llvm::JITSymbolFlags::Exported};
    symbols[mangle("druk_jit_value_bool")] =
      {llvm::orc::ExecutorAddr::fromPtr(&druk_jit_value_bool), llvm::JITSymbolFlags::Exported};
    symbols[mangle("druk_jit_value_int")] =
      {llvm::orc::ExecutorAddr::fromPtr(&druk_jit_value_int), llvm::JITSymbolFlags::Exported};
    symbols[mangle("druk_jit_value_function")] =
      {llvm::orc::ExecutorAddr::fromPtr(&druk_jit_value_function), llvm::JITSymbolFlags::Exported};
    symbols[mangle("druk_jit_add")] =
      {llvm::orc::ExecutorAddr::fromPtr(&druk_jit_add), llvm::JITSymbolFlags::Exported};
    symbols[mangle("druk_jit_subtract")] =
      {llvm::orc::ExecutorAddr::fromPtr(&druk_jit_subtract), llvm::JITSymbolFlags::Exported};
    symbols[mangle("druk_jit_multiply")] =
      {llvm::orc::ExecutorAddr::fromPtr(&druk_jit_multiply), llvm::JITSymbolFlags::Exported};
    symbols[mangle("druk_jit_divide")] =
      {llvm::orc::ExecutorAddr::fromPtr(&druk_jit_divide), llvm::JITSymbolFlags::Exported};
    symbols[mangle("druk_jit_negate")] =
      {llvm::orc::ExecutorAddr::fromPtr(&druk_jit_negate), llvm::JITSymbolFlags::Exported};
    symbols[mangle("druk_jit_equal")] =
      {llvm::orc::ExecutorAddr::fromPtr(&druk_jit_equal), llvm::JITSymbolFlags::Exported};
    symbols[mangle("druk_jit_less")] =
      {llvm::orc::ExecutorAddr::fromPtr(&druk_jit_less), llvm::JITSymbolFlags::Exported};
    symbols[mangle("druk_jit_greater")] =
      {llvm::orc::ExecutorAddr::fromPtr(&druk_jit_greater), llvm::JITSymbolFlags::Exported};
    symbols[mangle("druk_jit_not")] =
      {llvm::orc::ExecutorAddr::fromPtr(&druk_jit_not), llvm::JITSymbolFlags::Exported};
    symbols[mangle("druk_jit_get_global")] =
      {llvm::orc::ExecutorAddr::fromPtr(&druk_jit_get_global), llvm::JITSymbolFlags::Exported};
    symbols[mangle("druk_jit_define_global")] =
      {llvm::orc::ExecutorAddr::fromPtr(&druk_jit_define_global), llvm::JITSymbolFlags::Exported};
    symbols[mangle("druk_jit_set_global")] =
      {llvm::orc::ExecutorAddr::fromPtr(&druk_jit_set_global), llvm::JITSymbolFlags::Exported};
    symbols[mangle("druk_jit_build_array")] =
      {llvm::orc::ExecutorAddr::fromPtr(&druk_jit_build_array), llvm::JITSymbolFlags::Exported};
    symbols[mangle("druk_jit_index")] =
      {llvm::orc::ExecutorAddr::fromPtr(&druk_jit_index), llvm::JITSymbolFlags::Exported};
    symbols[mangle("druk_jit_index_set")] =
      {llvm::orc::ExecutorAddr::fromPtr(&druk_jit_index_set), llvm::JITSymbolFlags::Exported};
    symbols[mangle("druk_jit_build_struct")] =
      {llvm::orc::ExecutorAddr::fromPtr(&druk_jit_build_struct), llvm::JITSymbolFlags::Exported};
    symbols[mangle("druk_jit_get_field")] =
      {llvm::orc::ExecutorAddr::fromPtr(&druk_jit_get_field), llvm::JITSymbolFlags::Exported};
    symbols[mangle("druk_jit_set_field")] =
      {llvm::orc::ExecutorAddr::fromPtr(&druk_jit_set_field), llvm::JITSymbolFlags::Exported};
    symbols[mangle("druk_jit_len")] =
      {llvm::orc::ExecutorAddr::fromPtr(&druk_jit_len), llvm::JITSymbolFlags::Exported};
    symbols[mangle("druk_jit_push")] =
      {llvm::orc::ExecutorAddr::fromPtr(&druk_jit_push), llvm::JITSymbolFlags::Exported};
    symbols[mangle("druk_jit_pop_array")] =
      {llvm::orc::ExecutorAddr::fromPtr(&druk_jit_pop_array), llvm::JITSymbolFlags::Exported};
    symbols[mangle("druk_jit_typeof")] =
      {llvm::orc::ExecutorAddr::fromPtr(&druk_jit_typeof), llvm::JITSymbolFlags::Exported};
    symbols[mangle("druk_jit_keys")] =
      {llvm::orc::ExecutorAddr::fromPtr(&druk_jit_keys), llvm::JITSymbolFlags::Exported};
    symbols[mangle("druk_jit_values")] =
      {llvm::orc::ExecutorAddr::fromPtr(&druk_jit_values), llvm::JITSymbolFlags::Exported};
    symbols[mangle("druk_jit_contains")] =
      {llvm::orc::ExecutorAddr::fromPtr(&druk_jit_contains), llvm::JITSymbolFlags::Exported};
    symbols[mangle("druk_jit_input")] =
      {llvm::orc::ExecutorAddr::fromPtr(&druk_jit_input), llvm::JITSymbolFlags::Exported};
    symbols[mangle("druk_jit_print")] =
      {llvm::orc::ExecutorAddr::fromPtr(&druk_jit_print), llvm::JITSymbolFlags::Exported};
    symbols[mangle("druk_jit_call")] =
      {llvm::orc::ExecutorAddr::fromPtr(&druk_jit_call), llvm::JITSymbolFlags::Exported};
    symbols[mangle("druk_jit_get_arg")] =
      {llvm::orc::ExecutorAddr::fromPtr(&druk_jit_get_arg), llvm::JITSymbolFlags::Exported};
    symbols[mangle("druk_jit_register_function")] =
      {llvm::orc::ExecutorAddr::fromPtr(&druk_jit_register_function), llvm::JITSymbolFlags::Exported};
    symbols[mangle("druk_jit_set_compile_handler")] =
      {llvm::orc::ExecutorAddr::fromPtr(&druk_jit_set_compile_handler), llvm::JITSymbolFlags::Exported};
    symbols[mangle("druk_jit_value_as_int")] =
      {llvm::orc::ExecutorAddr::fromPtr(&druk_jit_value_as_int), llvm::JITSymbolFlags::Exported};

    llvm::cantFail(jd.define(llvm::orc::absoluteSymbols(std::move(symbols))));
  if (debug_) {
    std::cerr << "[LLVMBackend] LLJIT created successfully!" << std::endl;
  }
}

LLVMBackend::~LLVMBackend() = default;

bool LLVMBackend::supports(ObjFunction *function) const {
  // We now support ALL opcodes via helpers
  return true;
}

void LLVMBackend::reset() {
  ctx_ = std::make_unique<CompilationContext>();
  ctx_->module = std::make_unique<llvm::Module>("druk", ctx_->context);
  ctx_->module->setDataLayout(ctx_->jit->getDataLayout());
  ctx_->module->setTargetTriple(llvm::Triple(llvm::sys::getDefaultTargetTriple()));
  
  auto jit = llvm::orc::LLJITBuilder().create();
  if (jit) {
    ctx_->jit = std::move(*jit);
  }
}

LLVMBackend::CompiledFunc
LLVMBackend::compile_function(ObjFunction *function) {
  if (!function) {
    return nullptr;
  }

  ctx_->current_function = function;
  ctx_->basic_blocks.clear();
  ctx_->locals.clear();
  ctx_->stack.clear();
  ctx_->block_stack_states.clear();
  ctx_->block_predecessors.clear();

    // Create function signature: void func(PackedValue* out)
    llvm::Type *i8_ty = llvm::Type::getInt8Ty(ctx_->context);
    llvm::Type *i64_ty = llvm::Type::getInt64Ty(ctx_->context);
    llvm::ArrayType *padding_ty = llvm::ArrayType::get(i8_ty, 7);
    llvm::StructType *packed_value_ty = llvm::StructType::get(
      ctx_->context, {i8_ty, padding_ty, i64_ty, i64_ty}, false);
    llvm::PointerType *packed_ptr_ty = llvm::PointerType::getUnqual(packed_value_ty);
  
    llvm::FunctionType *func_type = llvm::FunctionType::get(
      llvm::Type::getVoidTy(ctx_->context), {packed_ptr_ty}, false);

  std::string func_name = function->name.empty() ? "main" : function->name;
    ctx_->llvm_function = llvm::Function::Create(
      func_type, llvm::Function::ExternalLinkage, func_name, ctx_->module.get());
    ctx_->ret_out = ctx_->llvm_function->getArg(0);

  // Analyze control flow to create basic blocks
  analyze_control_flow(function);

  // Compile function body
  compile_function_body(function);

  // Verify the generated function
  if (llvm::verifyFunction(*ctx_->llvm_function, &llvm::errs())) {
    llvm::errs() << "Function verification failed!\n";
    ctx_->llvm_function->print(llvm::errs());
    return nullptr;
  }

  // Optimize
  optimize_module();

  if (std::getenv("DRUK_JIT_DUMP_IR")) {
    ctx_->module->print(llvm::errs(), nullptr);
  }

  // Add module to JIT
  auto tsm = llvm::orc::ThreadSafeModule(std::move(ctx_->module),
                                         std::make_unique<llvm::LLVMContext>());
  
  auto err = ctx_->jit->addIRModule(std::move(tsm));
  if (err) {
    llvm::errs() << "Failed to add module to JIT: " 
                 << llvm::toString(std::move(err)) << "\n";
    return nullptr;
  }

  // Look up the compiled function
  auto sym = ctx_->jit->lookup(func_name);
  if (!sym) {
    llvm::errs() << "Failed to look up function: " 
                 << llvm::toString(sym.takeError()) << "\n";
    return nullptr;
  }

  // Recreate module for next compilation
  ctx_->module = std::make_unique<llvm::Module>("druk", ctx_->context);
  ctx_->module->setDataLayout(ctx_->jit->getDataLayout());
  ctx_->module->setTargetTriple(llvm::Triple(llvm::sys::getDefaultTargetTriple()));

  auto compiled = reinterpret_cast<CompiledFunc>(sym->getValue());
  druk_jit_register_function(function, compiled);
  return compiled;
}

void LLVMBackend::analyze_control_flow(ObjFunction *function) {
  const auto &code = function->chunk.code();
  
  // Entry block
  get_or_create_block(0);
  
  // Scan for jump targets
  for (size_t i = 0; i < code.size();) {
    OpCode op = static_cast<OpCode>(code[i]);
    i++;
    
    switch (op) {
    case OpCode::Jump: {
      uint16_t offset = (code[i] << 8) | code[i + 1];
      get_or_create_block(i + 2 + offset);
      i += 2;
      break;
    }
    case OpCode::JumpIfFalse: {
      uint16_t offset = (code[i] << 8) | code[i + 1];
      get_or_create_block(i + 2 + offset); // Jump target
      get_or_create_block(i + 2);          // Fall-through
      i += 2;
      break;
    }
    case OpCode::Return: {
      // Code after return is reachable from other branches; ensure a block exists.
      get_or_create_block(i);
      break;
    }
    case OpCode::Loop: {
      uint16_t offset = (code[i] << 8) | code[i + 1];
      get_or_create_block(i + 2 - offset);
      i += 2;
      break;
    }
    case OpCode::Constant:
    case OpCode::GetLocal:
    case OpCode::SetLocal:
    case OpCode::Call:
    case OpCode::BuildArray:
    case OpCode::BuildStruct:
    case OpCode::GetField:
    case OpCode::SetField:
      i++; // 1-byte operand
      break;
    default:
      break;
    }
  }
}

llvm::BasicBlock *LLVMBackend::get_or_create_block(size_t offset) {
  auto it = ctx_->basic_blocks.find(offset);
  if (it != ctx_->basic_blocks.end()) {
    return it->second;
  }
  
  std::string name = "bb_" + std::to_string(offset);
  llvm::BasicBlock *bb = llvm::BasicBlock::Create(
      ctx_->context, name, ctx_->llvm_function);
  ctx_->basic_blocks[offset] = bb;
  return bb;
}

llvm::Value *LLVMBackend::create_entry_alloca(llvm::Type *type,
                                              const std::string &name) {
  llvm::BasicBlock &entry = ctx_->llvm_function->getEntryBlock();
  llvm::IRBuilder<> tmp_builder(&entry, entry.begin());
  if (name.empty()) {
    return tmp_builder.CreateAlloca(type);
  }
  return tmp_builder.CreateAlloca(type, nullptr, name);
}

void LLVMBackend::compile_function_body(ObjFunction *function) {
  const auto &code = function->chunk.code();
  const auto &constants = function->chunk.constants();
  
  // Start with entry block
  llvm::BasicBlock *entry = get_or_create_block(0);
  ctx_->builder.SetInsertPoint(entry);
  ctx_->block_stack_states.emplace(0, ctx_->stack);
  
  // Define PackedValue struct type: {i8, [7 x i8], {i64}, i64}
  // This matches the C struct layout
  llvm::Type *i8_ty = llvm::Type::getInt8Ty(ctx_->context);
  llvm::Type *i64_ty = llvm::Type::getInt64Ty(ctx_->context);
  llvm::ArrayType *padding_ty = llvm::ArrayType::get(i8_ty, 7);
  
  llvm::StructType *packed_value_ty = llvm::StructType::get(
      ctx_->context,
      {i8_ty, padding_ty, i64_ty, i64_ty}, // type, padding, data union (as i64), extra
      /*isPacked=*/false);
  
  // Allocate locals as PackedValue structs
  const size_t max_locals = 256;
  ctx_->locals.resize(max_locals);
  auto nil_helper_type = llvm::FunctionType::get(
      llvm::Type::getVoidTy(ctx_->context),
      {llvm::PointerType::getUnqual(packed_value_ty)},
      false);
  auto nil_helper_fn = ctx_->module->getOrInsertFunction("druk_jit_value_nil", nil_helper_type);
  
  auto get_arg_type = llvm::FunctionType::get(
      llvm::Type::getVoidTy(ctx_->context),
      {llvm::Type::getInt32Ty(ctx_->context), llvm::PointerType::getUnqual(packed_value_ty)},
      false);
  auto get_arg_fn = ctx_->module->getOrInsertFunction("druk_jit_get_arg", get_arg_type);
  
  const int arity = function ? function->arity : 0;
  for (size_t i = 0; i < max_locals; i++) {
    ctx_->locals[i] = create_entry_alloca(
      packed_value_ty, "local_" + std::to_string(i));
    
    if (i <= static_cast<size_t>(arity)) {
      auto idx = llvm::ConstantInt::get(llvm::Type::getInt32Ty(ctx_->context), static_cast<int32_t>(i));
      ctx_->builder.CreateCall(get_arg_fn, {idx, ctx_->locals[i]});
    } else {
      ctx_->builder.CreateCall(nil_helper_fn, {ctx_->locals[i]});
    }
  }

  // Expression stack starts empty; locals are stored in dedicated slots.
  ctx_->stack.clear();
  
  // Compile bytecode
  const uint8_t *ip = code.data();
  const uint8_t *end = code.data() + code.size();
  std::vector<size_t> block_offsets;
  block_offsets.reserve(ctx_->basic_blocks.size());
  for (const auto &entry : ctx_->basic_blocks) {
    block_offsets.push_back(entry.first);
  }
  std::sort(block_offsets.begin(), block_offsets.end());
  
  while (ip < end) {
    size_t offset = ip - code.data();
    
    // Check if we need to start a new basic block
    auto bb_it = ctx_->basic_blocks.find(offset);
    if (bb_it != ctx_->basic_blocks.end() && bb_it->second != ctx_->builder.GetInsertBlock()) {
      // Need to branch to new block if current block is not terminated
      if (!ctx_->builder.GetInsertBlock()->getTerminator()) {
        ctx_->builder.CreateBr(bb_it->second);
        // Record this predecessor and its stack state
        ctx_->block_predecessors[offset].push_back({ctx_->builder.GetInsertBlock(), ctx_->stack});
      }
      if (std::getenv("DRUK_JIT_DEBUG")) {
        std::cerr << "[JIT DEBUG] Block transition at offset=" << offset 
                  << " stack_size=" << ctx_->stack.size() << "\n";
      }
      
      ctx_->builder.SetInsertPoint(bb_it->second);

      // Restore stack for this block from predecessors or saved state.
      auto pred_it = ctx_->block_predecessors.find(offset);
      if (pred_it != ctx_->block_predecessors.end()) {
        if (pred_it->second.size() == 1) {
          ctx_->stack = pred_it->second[0].second;
        } else if (pred_it->second.size() > 1) {
          // Create PHI nodes when multiple predecessors merge.
          size_t min_size = pred_it->second[0].second.size();
          size_t max_size = min_size;
          for (const auto& [pred_bb, pred_stack] : pred_it->second) {
            min_size = std::min(min_size, pred_stack.size());
            max_size = std::max(max_size, pred_stack.size());
          }

          ctx_->stack.clear();
          if (min_size > 0) {
            llvm::Type *i8_ty = llvm::Type::getInt8Ty(ctx_->context);
            llvm::Type *i64_ty = llvm::Type::getInt64Ty(ctx_->context);
            llvm::ArrayType *padding_ty = llvm::ArrayType::get(i8_ty, 7);
            llvm::StructType *packed_value_ty = llvm::StructType::get(
                ctx_->context, {i8_ty, padding_ty, i64_ty, i64_ty}, false);
            llvm::PointerType *packed_ptr_ty = llvm::PointerType::getUnqual(packed_value_ty);

            for (size_t i = 0; i < min_size; i++) {
              llvm::PHINode *phi = ctx_->builder.CreatePHI(
                  packed_ptr_ty, pred_it->second.size(),
                  "stack_phi_" + std::to_string(i));
              for (const auto& [pred_bb, pred_stack] : pred_it->second) {
                phi->addIncoming(pred_stack[i], pred_bb);
              }
              ctx_->stack.push_back(phi);
            }
          }

          if (max_size != min_size && std::getenv("DRUK_JIT_DEBUG")) {
            std::cerr << "[JIT DEBUG] Stack size mismatch at block " << offset
                      << " min=" << min_size << " max=" << max_size << "\n";
          }
        }
      } else {
        auto state_it = ctx_->block_stack_states.find(offset);
        if (state_it != ctx_->block_stack_states.end()) {
          ctx_->stack = state_it->second;
        }
      }
    }
    
    // Skip if block is already terminated
    if (ctx_->builder.GetInsertBlock()->getTerminator()) {
      auto next_it = std::upper_bound(block_offsets.begin(), block_offsets.end(), offset);
      if (next_it == block_offsets.end()) {
        break;
      }
      ip = code.data() + *next_it;
      continue;
    }
    
    compile_instruction(ip, function);
  }
  
  // Ensure function returns
  if (!ctx_->builder.GetInsertBlock()->getTerminator()) {
    if (ctx_->stack.empty()) {
      ctx_->builder.CreateCall(nil_helper_fn, {ctx_->ret_out});
    } else {
      llvm::Value *top = pop();
      llvm::Value *size = llvm::ConstantInt::get(i64_ty, 24);
      ctx_->builder.CreateMemCpy(ctx_->ret_out, llvm::MaybeAlign(8), top, llvm::MaybeAlign(8), size);
    }
    ctx_->builder.CreateRetVoid();
  }

  // Ensure all basic blocks are properly terminated
  for (const auto &entry : ctx_->basic_blocks) {
    llvm::BasicBlock *bb = entry.second;
    if (bb->getTerminator()) {
      continue;
    }
    ctx_->builder.SetInsertPoint(bb);
    if (llvm::pred_empty(bb)) {
      ctx_->builder.CreateUnreachable();
    } else {
      ctx_->builder.CreateCall(nil_helper_fn, {ctx_->ret_out});
      ctx_->builder.CreateRetVoid();
    }
  }
}

void LLVMBackend::compile_instruction(const uint8_t *&ip,
                                     ObjFunction *function) {
  const auto &constants = function->chunk.constants();
  OpCode op = static_cast<OpCode>(*ip++);
  
  auto &ctx = ctx_->context;
  auto &builder = ctx_->builder;
  
  // PackedValue type
  llvm::Type *i8_ty = llvm::Type::getInt8Ty(ctx);
  llvm::Type *i64_ty = llvm::Type::getInt64Ty(ctx);
  llvm::ArrayType *padding_ty = llvm::ArrayType::get(i8_ty, 7);
  llvm::StructType *packed_value_ty = llvm::StructType::get(
      ctx, {i8_ty, padding_ty, i64_ty, i64_ty}, false);
  llvm::PointerType *packed_ptr_ty = llvm::PointerType::getUnqual(packed_value_ty);
  
  switch (op) {
  case OpCode::Return: {
    emit_return();
    break;
  }
  
  case OpCode::Constant: {
    uint8_t idx = *ip++;
    const Value &val = constants[idx];
    
    // Allocate temp PackedValue and call helper to initialize it
    llvm::Value *temp = create_entry_alloca(packed_value_ty, "const_temp");
    
    if (val.is_nil()) {
      auto helper = ctx_->module->getOrInsertFunction("druk_jit_value_nil",
          llvm::FunctionType::get(llvm::Type::getVoidTy(ctx), {packed_ptr_ty}, false));
      builder.CreateCall(helper, {temp});
    } else if (val.is_int()) {
      auto helper = ctx_->module->getOrInsertFunction("druk_jit_value_int",
          llvm::FunctionType::get(llvm::Type::getVoidTy(ctx), {i64_ty, packed_ptr_ty}, false));
      builder.CreateCall(helper, {llvm::ConstantInt::get(i64_ty, val.as_int()), temp});
    } else if (val.is_bool()) {
      auto helper = ctx_->module->getOrInsertFunction("druk_jit_value_bool",
          llvm::FunctionType::get(llvm::Type::getVoidTy(ctx), {llvm::Type::getInt1Ty(ctx), packed_ptr_ty}, false));
      builder.CreateCall(helper, {llvm::ConstantInt::get(llvm::Type::getInt1Ty(ctx), val.as_bool()), temp});
    } else if (val.is_string()) {
      // For string constants, we need to embed them as global strings
      auto str_val = val.as_string();
      llvm::Constant *str_const = llvm::ConstantDataArray::getString(ctx, str_val, false);
      llvm::GlobalVariable *str_global = new llvm::GlobalVariable(
          *ctx_->module, str_const->getType(), true,
          llvm::GlobalValue::PrivateLinkage, str_const, "str_const");
      
      // Create PackedValue manually for string
      llvm::Value *type_ptr = builder.CreateStructGEP(packed_value_ty, temp, 0);
      builder.CreateStore(llvm::ConstantInt::get(i8_ty, 3), type_ptr); // String type = 3
      
      llvm::Value *data_ptr = builder.CreateStructGEP(packed_value_ty, temp, 2);
      llvm::Value *str_ptr = builder.CreateBitCast(str_global, llvm::PointerType::getUnqual(i8_ty));
      builder.CreateStore(builder.CreatePtrToInt(str_ptr, i64_ty), data_ptr);
      
      llvm::Value *extra_ptr = builder.CreateStructGEP(packed_value_ty, temp, 3);
      builder.CreateStore(llvm::ConstantInt::get(i64_ty, str_val.size()), extra_ptr);
      } else if (val.is_function()) {
        auto helper = ctx_->module->getOrInsertFunction("druk_jit_value_function",
          llvm::FunctionType::get(llvm::Type::getVoidTy(ctx),
            {llvm::PointerType::getUnqual(llvm::Type::getInt8Ty(ctx)), packed_ptr_ty}, false));
        auto fn_ptr = llvm::ConstantInt::get(
          llvm::Type::getInt64Ty(ctx),
          static_cast<uint64_t>(reinterpret_cast<uintptr_t>(val.as_function().get())));
        auto fn_ptr_cast = builder.CreateIntToPtr(
          fn_ptr, llvm::PointerType::getUnqual(llvm::Type::getInt8Ty(ctx)));
        builder.CreateCall(helper, {fn_ptr_cast, temp});
    } else {
      // Unsupported constant - create nil
      auto helper = ctx_->module->getOrInsertFunction("druk_jit_value_nil",
          llvm::FunctionType::get(llvm::Type::getVoidTy(ctx), {packed_ptr_ty}, false));
      builder.CreateCall(helper, {temp});
    }
    
    push(temp);
    break;
  }
  
  case OpCode::Nil: {
    llvm::Value *temp = create_entry_alloca(packed_value_ty);
    auto helper = ctx_->module->getOrInsertFunction("druk_jit_value_nil",
        llvm::FunctionType::get(llvm::Type::getVoidTy(ctx), {packed_ptr_ty}, false));
    builder.CreateCall(helper, {temp});
    push(temp);
    break;
  }
  
  case OpCode::True: {
    llvm::Value *temp = create_entry_alloca(packed_value_ty);
    auto helper = ctx_->module->getOrInsertFunction("druk_jit_value_bool",
        llvm::FunctionType::get(llvm::Type::getVoidTy(ctx), {llvm::Type::getInt1Ty(ctx), packed_ptr_ty}, false));
    builder.CreateCall(helper, {llvm::ConstantInt::get(llvm::Type::getInt1Ty(ctx), true), temp});
    push(temp);
    break;
  }
  
  case OpCode::False: {
    llvm::Value *temp = create_entry_alloca(packed_value_ty);
    auto helper = ctx_->module->getOrInsertFunction("druk_jit_value_bool",
        llvm::FunctionType::get(llvm::Type::getVoidTy(ctx), {llvm::Type::getInt1Ty(ctx), packed_ptr_ty}, false));
    builder.CreateCall(helper, {llvm::ConstantInt::get(llvm::Type::getInt1Ty(ctx), false), temp});
    push(temp);
    break;
  }
  
  case OpCode::GetLocal: {
    uint8_t slot = *ip++;
    if (slot < ctx_->locals.size()) {
      llvm::Value *temp = create_entry_alloca(packed_value_ty);
      llvm::Value *src = ctx_->locals[slot];
      // Memcpy slot to temp
      llvm::Value *size = llvm::ConstantInt::get(i64_ty, 24); // sizeof(PackedValue)
      builder.CreateMemCpy(temp, llvm::MaybeAlign(8), src, llvm::MaybeAlign(8), size);
      push(temp);
    }
    break;
  }
  
  case OpCode::SetLocal: {
    uint8_t slot = *ip++;
    if (slot < ctx_->locals.size() && !ctx_->stack.empty()) {
      llvm::Value *val = peek(0);
      llvm::Value *size = llvm::ConstantInt::get(i64_ty, 24);
      builder.CreateMemCpy(ctx_->locals[slot], llvm::MaybeAlign(8), val, llvm::MaybeAlign(8), size);
    }
    break;
  }
  
  case OpCode::GetGlobal: {
    uint8_t idx = *ip++;
    const Value &name_val = constants[idx];
    if (name_val.is_string()) {
      auto name = name_val.as_string();
      llvm::Constant *name_str = llvm::ConstantDataArray::getString(ctx, name, false);
      llvm::GlobalVariable *name_global = new llvm::GlobalVariable(
          *ctx_->module, name_str->getType(), true,
          llvm::GlobalValue::PrivateLinkage, name_str, "global_name");
      
      llvm::Value *temp = create_entry_alloca(packed_value_ty);
      auto helper = ctx_->module->getOrInsertFunction("druk_jit_get_global",
          llvm::FunctionType::get(llvm::Type::getVoidTy(ctx),
              {llvm::PointerType::getUnqual(i8_ty), i64_ty, packed_ptr_ty}, false));
      
      llvm::Value *name_ptr = builder.CreateBitCast(name_global, llvm::PointerType::getUnqual(i8_ty));
      builder.CreateCall(helper, {name_ptr, llvm::ConstantInt::get(i64_ty, name.size()), temp});
      push(temp);
    }
    break;
  }
  
  case OpCode::DefineGlobal: {
    uint8_t idx = *ip++;
    const Value &name_val = constants[idx];
    if (name_val.is_string() && !ctx_->stack.empty()) {
      auto name = name_val.as_string();
      llvm::Constant *name_str = llvm::ConstantDataArray::getString(ctx, name, false);
      llvm::GlobalVariable *name_global = new llvm::GlobalVariable(
          *ctx_->module, name_str->getType(), true,
          llvm::GlobalValue::PrivateLinkage, name_str, "global_name");
      
      llvm::Value *val = pop();
      auto helper = ctx_->module->getOrInsertFunction("druk_jit_define_global",
          llvm::FunctionType::get(llvm::Type::getVoidTy(ctx),
              {llvm::PointerType::getUnqual(i8_ty), i64_ty, packed_ptr_ty}, false));
      
      llvm::Value *name_ptr = builder.CreateBitCast(name_global, llvm::PointerType::getUnqual(i8_ty));
      builder.CreateCall(helper, {name_ptr, llvm::ConstantInt::get(i64_ty, name.size()), val});
    }
    break;
  }
  
  case OpCode::SetGlobal: {
    uint8_t idx = *ip++;
    const Value &name_val = constants[idx];
    if (name_val.is_string() && !ctx_->stack.empty()) {
      auto name = name_val.as_string();
      llvm::Constant *name_str = llvm::ConstantDataArray::getString(ctx, name, false);
      llvm::GlobalVariable *name_global = new llvm::GlobalVariable(
          *ctx_->module, name_str->getType(), true,
          llvm::GlobalValue::PrivateLinkage, name_str, "global_name");
      
      llvm::Value *val = peek(0);
      auto helper = ctx_->module->getOrInsertFunction("druk_jit_set_global",
          llvm::FunctionType::get(llvm::Type::getVoidTy(ctx),
              {llvm::PointerType::getUnqual(i8_ty), i64_ty, packed_ptr_ty}, false));
      
      llvm::Value *name_ptr = builder.CreateBitCast(name_global, llvm::PointerType::getUnqual(i8_ty));
      builder.CreateCall(helper, {name_ptr, llvm::ConstantInt::get(i64_ty, name.size()), val});
    }
    break;
  }
  
  case OpCode::Add: {
    if (ctx_->stack.size() >= 2) {
      llvm::Value *b = pop();
      llvm::Value *a = pop();
      llvm::Value *result = create_entry_alloca(packed_value_ty);
      
      auto helper = ctx_->module->getOrInsertFunction("druk_jit_add",
          llvm::FunctionType::get(llvm::Type::getVoidTy(ctx),
              {packed_ptr_ty, packed_ptr_ty, packed_ptr_ty}, false));
      builder.CreateCall(helper, {a, b, result});
      push(result);
    }
    break;
  }
  
  case OpCode::Subtract: {
    if (ctx_->stack.size() >= 2) {
      llvm::Value *b = pop();
      llvm::Value *a = pop();
      llvm::Value *result = create_entry_alloca(packed_value_ty);
      
      auto helper = ctx_->module->getOrInsertFunction("druk_jit_subtract",
          llvm::FunctionType::get(llvm::Type::getVoidTy(ctx),
              {packed_ptr_ty, packed_ptr_ty, packed_ptr_ty}, false));
      builder.CreateCall(helper, {a, b, result});
      push(result);
    }
    break;
  }
  
  case OpCode::Multiply: {
    if (ctx_->stack.size() >= 2) {
      llvm::Value *b = pop();
      llvm::Value *a = pop();
      llvm::Value *result = create_entry_alloca(packed_value_ty);
      
      auto helper = ctx_->module->getOrInsertFunction("druk_jit_multiply",
          llvm::FunctionType::get(llvm::Type::getVoidTy(ctx),
              {packed_ptr_ty, packed_ptr_ty, packed_ptr_ty}, false));
      builder.CreateCall(helper, {a, b, result});
      push(result);
    }
    break;
  }
  
  case OpCode::Divide: {
    if (ctx_->stack.size() >= 2) {
      llvm::Value *b = pop();
      llvm::Value *a = pop();
      llvm::Value *result = create_entry_alloca(packed_value_ty);
      
      auto helper = ctx_->module->getOrInsertFunction("druk_jit_divide",
          llvm::FunctionType::get(llvm::Type::getVoidTy(ctx),
              {packed_ptr_ty, packed_ptr_ty, packed_ptr_ty}, false));
      builder.CreateCall(helper, {a, b, result});
      push(result);
    }
    break;
  }
  
  case OpCode::Negate: {
    if (!ctx_->stack.empty()) {
      llvm::Value *a = pop();
      llvm::Value *result = create_entry_alloca(packed_value_ty);
      
      auto helper = ctx_->module->getOrInsertFunction("druk_jit_negate",
          llvm::FunctionType::get(llvm::Type::getVoidTy(ctx),
              {packed_ptr_ty, packed_ptr_ty}, false));
      builder.CreateCall(helper, {a, result});
      push(result);
    }
    break;
  }
  
  case OpCode::Equal: {
    if (ctx_->stack.size() >= 2) {
      llvm::Value *b = pop();
      llvm::Value *a = pop();
      llvm::Value *result = create_entry_alloca(packed_value_ty);
      
      auto helper = ctx_->module->getOrInsertFunction("druk_jit_equal",
          llvm::FunctionType::get(llvm::Type::getVoidTy(ctx),
              {packed_ptr_ty, packed_ptr_ty, packed_ptr_ty}, false));
      builder.CreateCall(helper, {a, b, result});
      push(result);
    }
    break;
  }
  
  case OpCode::Less: {
    if (ctx_->stack.size() >= 2) {
      llvm::Value *b = pop();
      llvm::Value *a = pop();
      llvm::Value *result = create_entry_alloca(packed_value_ty);
      
      auto helper = ctx_->module->getOrInsertFunction("druk_jit_less",
          llvm::FunctionType::get(llvm::Type::getVoidTy(ctx),
              {packed_ptr_ty, packed_ptr_ty, packed_ptr_ty}, false));
      builder.CreateCall(helper, {a, b, result});
      push(result);
    }
    break;
  }
  
  case OpCode::Greater: {
    if (ctx_->stack.size() >= 2) {
      llvm::Value *b = pop();
      llvm::Value *a = pop();
      llvm::Value *result = create_entry_alloca(packed_value_ty);
      
      auto helper = ctx_->module->getOrInsertFunction("druk_jit_greater",
          llvm::FunctionType::get(llvm::Type::getVoidTy(ctx),
              {packed_ptr_ty, packed_ptr_ty, packed_ptr_ty}, false));
      builder.CreateCall(helper, {a, b, result});
      push(result);
    }
    break;
  }
  
  case OpCode::Not: {
    if (!ctx_->stack.empty()) {
      llvm::Value *a = pop();
      llvm::Value *result = create_entry_alloca(packed_value_ty);
      
      auto helper = ctx_->module->getOrInsertFunction("druk_jit_not",
          llvm::FunctionType::get(llvm::Type::getVoidTy(ctx),
              {packed_ptr_ty, packed_ptr_ty}, false));
      builder.CreateCall(helper, {a, result});
      push(result);
    }
    break;
  }
  
  case OpCode::Jump: {
    uint16_t offset = (*ip << 8) | *(ip + 1);
    ip += 2;
    
    size_t target = (ip - function->chunk.code().data()) + offset;
    ctx_->block_stack_states[target] = ctx_->stack;
    llvm::BasicBlock *target_bb = get_or_create_block(target);
    // Record this predecessor
    ctx_->block_predecessors[target].push_back({builder.GetInsertBlock(), ctx_->stack});
    builder.CreateBr(target_bb);
    break;
  }
  
  case OpCode::JumpIfFalse: {
    uint16_t offset = (*ip << 8) | *(ip + 1);
    ip += 2;
    
    size_t target = (ip - function->chunk.code().data()) + offset;
    size_t next = ip - function->chunk.code().data();

    if (std::getenv("DRUK_JIT_DEBUG")) {
      std::cerr << "[JIT DEBUG] JumpIfFalse: current=" << (ip - function->chunk.code().data() - 3)
                << " target=" << target << " next=" << next << " stack_size=" << ctx_->stack.size() << "\n";
    }

    ctx_->block_stack_states[target] = ctx_->stack;
    ctx_->block_stack_states[next] = ctx_->stack;
    
    llvm::BasicBlock *true_bb = get_or_create_block(next);
    llvm::BasicBlock *false_bb = get_or_create_block(target);
    
    llvm::Value *val = peek(0);
    // Check if value is falsey (nil or false bool)
    llvm::Value *type_ptr = builder.CreateStructGEP(packed_value_ty, val, 0);
    llvm::Value *type_val = builder.CreateLoad(i8_ty, type_ptr);
    llvm::Value *is_nil = builder.CreateICmpEQ(type_val, llvm::ConstantInt::get(i8_ty, 0));
    
    llvm::Value *data_ptr = builder.CreateStructGEP(packed_value_ty, val, 2);
    llvm::Value *data_val = builder.CreateLoad(i64_ty, data_ptr);
    llvm::Value *is_false_bool = builder.CreateAnd(
        builder.CreateICmpEQ(type_val, llvm::ConstantInt::get(i8_ty, 2)), // Bool type
        builder.CreateICmpEQ(data_val, llvm::ConstantInt::get(i64_ty, 0))); // false value
    
    llvm::Value *is_falsey = builder.CreateOr(is_nil, is_false_bool);
    
    // Record predecessors for both paths
    llvm::BasicBlock *current_bb = builder.GetInsertBlock();
    ctx_->block_predecessors[target].push_back({current_bb, ctx_->stack});
    ctx_->block_predecessors[next].push_back({current_bb, ctx_->stack});
    
    builder.CreateCondBr(is_falsey, false_bb, true_bb);
    break;
  }
  
  case OpCode::Loop: {
    uint16_t offset = (*ip << 8) | *(ip + 1);
    ip += 2;
    
    size_t target = (ip - function->chunk.code().data()) - offset;
    ctx_->block_stack_states[target] = ctx_->stack;
    llvm::BasicBlock *target_bb = get_or_create_block(target);
    // Record this predecessor
    ctx_->block_predecessors[target].push_back({builder.GetInsertBlock(), ctx_->stack});
    builder.CreateBr(target_bb);
    break;
  }
  
  case OpCode::Pop: {
    if (std::getenv("DRUK_JIT_DEBUG")) {
      std::cerr << "[JIT DEBUG] Pop at offset=" << (ip - function->chunk.code().data() - 1) 
                << " stack_size_before=" << ctx_->stack.size() << "\n";
    }
    if (!ctx_->stack.empty()) {
      pop();
    }
    break;
  }

  case OpCode::Print: {
    if (std::getenv("DRUK_JIT_DEBUG")) {
      std::cerr << "[JIT DEBUG] Print at offset=" << (ip - function->chunk.code().data() - 1)
                << " stack_size=" << ctx_->stack.size() << "\n";
    }
    llvm::Value *val = pop();
    auto helper = ctx_->module->getOrInsertFunction("druk_jit_print",
        llvm::FunctionType::get(llvm::Type::getVoidTy(ctx), {packed_ptr_ty}, false));
    builder.CreateCall(helper, {val});
    break;
  }
  
  case OpCode::BuildArray: {
    uint8_t count = *ip++;
    if (ctx_->stack.size() >= count) {
      // Pop elements in reverse order
      std::vector<llvm::Value *> elements;
      for (uint8_t i = 0; i < count; ++i) {
        elements.push_back(pop());
      }
      std::reverse(elements.begin(), elements.end());
      
      llvm::Value *array_size = llvm::ConstantInt::get(llvm::Type::getInt32Ty(ctx), count);
      llvm::Value *array_ptr = llvm::ConstantPointerNull::get(packed_ptr_ty);
      if (count > 0) {
        llvm::Type *array_type = llvm::ArrayType::get(packed_value_ty, count);
        llvm::Value *array_alloc = create_entry_alloca(array_type);
        for (uint8_t i = 0; i < count; ++i) {
          llvm::Value *elem_ptr = builder.CreateConstGEP2_32(array_type, array_alloc, 0, i);
          llvm::Value *size = llvm::ConstantInt::get(i64_ty, 24);
          builder.CreateMemCpy(elem_ptr, llvm::MaybeAlign(8), elements[i], llvm::MaybeAlign(8), size);
        }
        array_ptr = builder.CreateBitCast(array_alloc, packed_ptr_ty);
      }
      
      llvm::Value *result = create_entry_alloca(packed_value_ty);
      auto helper = ctx_->module->getOrInsertFunction("druk_jit_build_array",
          llvm::FunctionType::get(llvm::Type::getVoidTy(ctx),
              {packed_ptr_ty, llvm::Type::getInt32Ty(ctx), packed_ptr_ty}, false));
      builder.CreateCall(helper, {array_ptr, array_size, result});
      push(result);
    }
    break;
  }
  
  case OpCode::Index: {
    if (ctx_->stack.size() >= 2) {
      llvm::Value *idx = pop();
      llvm::Value *arr = pop();
      llvm::Value *result = create_entry_alloca(packed_value_ty);
      
      auto helper = ctx_->module->getOrInsertFunction("druk_jit_index",
          llvm::FunctionType::get(llvm::Type::getVoidTy(ctx),
              {packed_ptr_ty, packed_ptr_ty, packed_ptr_ty}, false));
      builder.CreateCall(helper, {arr, idx, result});
      push(result);
    }
    break;
  }
  
  case OpCode::IndexSet: {
    if (ctx_->stack.size() >= 3) {
      llvm::Value *val = pop();
      llvm::Value *idx = pop();
      llvm::Value *arr = pop();
      
      auto helper = ctx_->module->getOrInsertFunction("druk_jit_index_set",
          llvm::FunctionType::get(llvm::Type::getVoidTy(ctx),
              {packed_ptr_ty, packed_ptr_ty, packed_ptr_ty}, false));
      builder.CreateCall(helper, {arr, idx, val});
      push(val);
    }
    break;
  }
  
  case OpCode::Len: {
    if (!ctx_->stack.empty()) {
      llvm::Value *val = pop();
      llvm::Value *result = create_entry_alloca(packed_value_ty);
      
      auto helper = ctx_->module->getOrInsertFunction("druk_jit_len",
          llvm::FunctionType::get(llvm::Type::getVoidTy(ctx),
              {packed_ptr_ty, packed_ptr_ty}, false));
      builder.CreateCall(helper, {val, result});
      push(result);
    }
    break;
  }
  
  case OpCode::Push: {
    if (ctx_->stack.size() >= 2) {
      llvm::Value *elem = pop();
      llvm::Value *arr = pop();
      
      auto helper = ctx_->module->getOrInsertFunction("druk_jit_push",
          llvm::FunctionType::get(llvm::Type::getVoidTy(ctx),
              {packed_ptr_ty, packed_ptr_ty}, false));
      builder.CreateCall(helper, {arr, elem});
      llvm::Value *result = create_entry_alloca(packed_value_ty);
      auto nil_helper = ctx_->module->getOrInsertFunction("druk_jit_value_nil",
          llvm::FunctionType::get(llvm::Type::getVoidTy(ctx), {packed_ptr_ty}, false));
      builder.CreateCall(nil_helper, {result});
      push(result);
    }
    break;
  }
  
  case OpCode::PopArray: {
    if (!ctx_->stack.empty()) {
      llvm::Value *arr = pop();
      llvm::Value *result = create_entry_alloca(packed_value_ty);
      
      auto helper = ctx_->module->getOrInsertFunction("druk_jit_pop_array",
          llvm::FunctionType::get(llvm::Type::getVoidTy(ctx),
              {packed_ptr_ty, packed_ptr_ty}, false));
      builder.CreateCall(helper, {arr, result});
      push(result);
    }
    break;
  }
  
  case OpCode::TypeOf: {
    if (!ctx_->stack.empty()) {
      llvm::Value *val = pop();
      llvm::Value *result = create_entry_alloca(packed_value_ty);
      
      auto helper = ctx_->module->getOrInsertFunction("druk_jit_typeof",
          llvm::FunctionType::get(llvm::Type::getVoidTy(ctx),
              {packed_ptr_ty, packed_ptr_ty}, false));
      builder.CreateCall(helper, {val, result});
      push(result);
    }
    break;
  }
  
  case OpCode::Keys: {
    if (!ctx_->stack.empty()) {
      llvm::Value *val = pop();
      llvm::Value *result = create_entry_alloca(packed_value_ty);
      
      auto helper = ctx_->module->getOrInsertFunction("druk_jit_keys",
          llvm::FunctionType::get(llvm::Type::getVoidTy(ctx),
              {packed_ptr_ty, packed_ptr_ty}, false));
      builder.CreateCall(helper, {val, result});
      push(result);
    }
    break;
  }
  
  case OpCode::Values: {
    if (!ctx_->stack.empty()) {
      llvm::Value *val = pop();
      llvm::Value *result = create_entry_alloca(packed_value_ty);
      
      auto helper = ctx_->module->getOrInsertFunction("druk_jit_values",
          llvm::FunctionType::get(llvm::Type::getVoidTy(ctx),
              {packed_ptr_ty, packed_ptr_ty}, false));
      builder.CreateCall(helper, {val, result});
      push(result);
    }
    break;
  }
  
  case OpCode::Contains: {
    if (ctx_->stack.size() >= 2) {
      llvm::Value *item = pop();
      llvm::Value *container = pop();
      llvm::Value *result = create_entry_alloca(packed_value_ty);
      
      auto helper = ctx_->module->getOrInsertFunction("druk_jit_contains",
          llvm::FunctionType::get(llvm::Type::getVoidTy(ctx),
              {packed_ptr_ty, packed_ptr_ty, packed_ptr_ty}, false));
      builder.CreateCall(helper, {container, item, result});
      push(result);
    }
    break;
  }
  
  case OpCode::Input: {
    llvm::Value *result = create_entry_alloca(packed_value_ty);
    
    auto helper = ctx_->module->getOrInsertFunction("druk_jit_input",
        llvm::FunctionType::get(llvm::Type::getVoidTy(ctx), {packed_ptr_ty}, false));
    builder.CreateCall(helper, {result});
    push(result);
    break;
  }
  
  case OpCode::BuildStruct: {
    uint8_t count = *ip++;
    if (ctx_->stack.size() >= static_cast<size_t>(count) * 2) {
      llvm::Value *keys_ptr = llvm::ConstantPointerNull::get(packed_ptr_ty);
      llvm::Value *vals_ptr = llvm::ConstantPointerNull::get(packed_ptr_ty);
      if (count > 0) {
        llvm::Type *key_array_ty = llvm::ArrayType::get(packed_value_ty, count);
        llvm::Type *val_array_ty = llvm::ArrayType::get(packed_value_ty, count);
        llvm::Value *keys_alloc = create_entry_alloca(key_array_ty);
        llvm::Value *vals_alloc = create_entry_alloca(val_array_ty);
        
        for (int i = static_cast<int>(count) - 1; i >= 0; --i) {
          llvm::Value *value = pop();
          llvm::Value *key = pop();
          llvm::Value *key_ptr = builder.CreateConstGEP2_32(key_array_ty, keys_alloc, 0, i);
          llvm::Value *val_ptr = builder.CreateConstGEP2_32(val_array_ty, vals_alloc, 0, i);
          llvm::Value *size = llvm::ConstantInt::get(i64_ty, 24);
          builder.CreateMemCpy(key_ptr, llvm::MaybeAlign(8), key, llvm::MaybeAlign(8), size);
          builder.CreateMemCpy(val_ptr, llvm::MaybeAlign(8), value, llvm::MaybeAlign(8), size);
        }
        
        keys_ptr = builder.CreateBitCast(keys_alloc, packed_ptr_ty);
        vals_ptr = builder.CreateBitCast(vals_alloc, packed_ptr_ty);
      }
      
      llvm::Value *result = create_entry_alloca(packed_value_ty);
      auto helper = ctx_->module->getOrInsertFunction("druk_jit_build_struct",
          llvm::FunctionType::get(llvm::Type::getVoidTy(ctx),
              {packed_ptr_ty, packed_ptr_ty, llvm::Type::getInt32Ty(ctx), packed_ptr_ty}, false));
      builder.CreateCall(helper, {keys_ptr, vals_ptr,
          llvm::ConstantInt::get(llvm::Type::getInt32Ty(ctx), count), result});
      push(result);
    }
    break;
  }
  
  case OpCode::GetField: {
    uint8_t name_index = *ip++;
    const Value &name_val = constants[name_index];
    if (!ctx_->stack.empty() && name_val.is_string()) {
      llvm::Value *obj = pop();
      auto name = name_val.as_string();
      llvm::Constant *name_str = llvm::ConstantDataArray::getString(ctx, name, false);
      llvm::GlobalVariable *name_global = new llvm::GlobalVariable(
          *ctx_->module, name_str->getType(), true,
          llvm::GlobalValue::PrivateLinkage, name_str, "field_name");
      
      llvm::Value *result = create_entry_alloca(packed_value_ty);
      auto helper = ctx_->module->getOrInsertFunction("druk_jit_get_field",
          llvm::FunctionType::get(llvm::Type::getVoidTy(ctx),
              {packed_ptr_ty, llvm::PointerType::getUnqual(i8_ty), i64_ty, packed_ptr_ty}, false));
      llvm::Value *name_ptr = builder.CreateBitCast(name_global, llvm::PointerType::getUnqual(i8_ty));
      builder.CreateCall(helper, {obj, name_ptr, llvm::ConstantInt::get(i64_ty, name.size()), result});
      push(result);
    }
    break;
  }
  
  case OpCode::SetField: {
    uint8_t name_index = *ip++;
    const Value &name_val = constants[name_index];
    if (ctx_->stack.size() >= 2 && name_val.is_string()) {
      llvm::Value *value = pop();
      llvm::Value *obj = pop();
      auto name = name_val.as_string();
      llvm::Constant *name_str = llvm::ConstantDataArray::getString(ctx, name, false);
      llvm::GlobalVariable *name_global = new llvm::GlobalVariable(
          *ctx_->module, name_str->getType(), true,
          llvm::GlobalValue::PrivateLinkage, name_str, "field_name");
      
      auto helper = ctx_->module->getOrInsertFunction("druk_jit_set_field",
          llvm::FunctionType::get(llvm::Type::getVoidTy(ctx),
              {packed_ptr_ty, llvm::PointerType::getUnqual(i8_ty), i64_ty, packed_ptr_ty}, false));
      llvm::Value *name_ptr = builder.CreateBitCast(name_global, llvm::PointerType::getUnqual(i8_ty));
      builder.CreateCall(helper, {obj, name_ptr, llvm::ConstantInt::get(i64_ty, name.size()), value});
      push(value);
    }
    break;
  }
  
  case OpCode::Call: {
    uint8_t arg_count = *ip++;
    if (ctx_->stack.size() >= static_cast<size_t>(arg_count) + 1) {
      std::vector<llvm::Value *> args;
      args.reserve(arg_count);
      for (uint8_t i = 0; i < arg_count; ++i) {
        args.push_back(pop());
      }
      std::reverse(args.begin(), args.end());
      llvm::Value *callee = pop();
      
      llvm::Value *args_ptr = llvm::ConstantPointerNull::get(packed_ptr_ty);
      if (arg_count > 0) {
        llvm::Type *args_array_ty = llvm::ArrayType::get(packed_value_ty, arg_count);
        llvm::Value *args_alloc = create_entry_alloca(args_array_ty);
        for (uint8_t i = 0; i < arg_count; ++i) {
          llvm::Value *arg_ptr = builder.CreateConstGEP2_32(args_array_ty, args_alloc, 0, i);
          llvm::Value *size = llvm::ConstantInt::get(i64_ty, 24);
          builder.CreateMemCpy(arg_ptr, llvm::MaybeAlign(8), args[i], llvm::MaybeAlign(8), size);
        }
        args_ptr = builder.CreateBitCast(args_alloc, packed_ptr_ty);
      }
      
      llvm::Value *result = create_entry_alloca(packed_value_ty);
      auto helper = ctx_->module->getOrInsertFunction("druk_jit_call",
          llvm::FunctionType::get(llvm::Type::getVoidTy(ctx),
              {packed_ptr_ty, packed_ptr_ty, llvm::Type::getInt32Ty(ctx), packed_ptr_ty}, false));
      builder.CreateCall(helper, {callee, args_ptr,
          llvm::ConstantInt::get(llvm::Type::getInt32Ty(ctx), arg_count), result});
      push(result);
    }
    break;
  }
  
  default:
    break;
  }
}

void LLVMBackend::push(llvm::Value *value) { ctx_->stack.push_back(value); }

llvm::Value *LLVMBackend::pop() {
  if (ctx_->stack.empty()) {
    // Return a nil PackedValue
    llvm::Type *i8_ty = llvm::Type::getInt8Ty(ctx_->context);
    llvm::Type *i64_ty = llvm::Type::getInt64Ty(ctx_->context);
    llvm::ArrayType *padding_ty = llvm::ArrayType::get(i8_ty, 7);
    llvm::StructType *packed_value_ty = llvm::StructType::get(
        ctx_->context, {i8_ty, padding_ty, i64_ty, i64_ty}, false);
    
    llvm::Value *temp = create_entry_alloca(packed_value_ty);
    auto helper = ctx_->module->getOrInsertFunction("druk_jit_value_nil",
        llvm::FunctionType::get(llvm::Type::getVoidTy(ctx_->context),
            {llvm::PointerType::getUnqual(packed_value_ty)}, false));
    ctx_->builder.CreateCall(helper, {temp});
    return temp;
  }
  llvm::Value *val = ctx_->stack.back();
  ctx_->stack.pop_back();
  return val;
}

llvm::Value *LLVMBackend::peek(int distance) {
  size_t idx = ctx_->stack.size() - 1 - distance;
  if (idx >= ctx_->stack.size()) {
    // Return nil
    llvm::Type *i8_ty = llvm::Type::getInt8Ty(ctx_->context);
    llvm::Type *i64_ty = llvm::Type::getInt64Ty(ctx_->context);
    llvm::ArrayType *padding_ty = llvm::ArrayType::get(i8_ty, 7);
    llvm::StructType *packed_value_ty = llvm::StructType::get(
        ctx_->context, {i8_ty, padding_ty, i64_ty, i64_ty}, false);
    
    llvm::Value *temp = create_entry_alloca(packed_value_ty);
    auto helper = ctx_->module->getOrInsertFunction("druk_jit_value_nil",
        llvm::FunctionType::get(llvm::Type::getVoidTy(ctx_->context),
            {llvm::PointerType::getUnqual(packed_value_ty)}, false));
    ctx_->builder.CreateCall(helper, {temp});
    return temp;
  }
  return ctx_->stack[idx];
}

void LLVMBackend::emit_return() {
  llvm::Type *i64_ty = llvm::Type::getInt64Ty(ctx_->context);
  llvm::Type *i8_ty = llvm::Type::getInt8Ty(ctx_->context);
  llvm::ArrayType *padding_ty = llvm::ArrayType::get(i8_ty, 7);
  llvm::StructType *packed_value_ty = llvm::StructType::get(
      ctx_->context, {i8_ty, padding_ty, i64_ty, i64_ty}, false);
  
  auto nil_helper = ctx_->module->getOrInsertFunction("druk_jit_value_nil",
      llvm::FunctionType::get(llvm::Type::getVoidTy(ctx_->context),
          {llvm::PointerType::getUnqual(packed_value_ty)}, false));
  
  if (ctx_->stack.empty()) {
    ctx_->builder.CreateCall(nil_helper, {ctx_->ret_out});
  } else {
    llvm::Value *top = ctx_->stack.back();
    llvm::Value *size = llvm::ConstantInt::get(i64_ty, 24);
    ctx_->builder.CreateMemCpy(ctx_->ret_out, llvm::MaybeAlign(8), top, llvm::MaybeAlign(8), size);
  }
  ctx_->builder.CreateRetVoid();
}

void LLVMBackend::optimize_module() {
  // Create the analysis managers
  llvm::LoopAnalysisManager LAM;
  llvm::FunctionAnalysisManager FAM;
  llvm::CGSCCAnalysisManager CGAM;
  llvm::ModuleAnalysisManager MAM;

  // Create the pass builder
  llvm::PassBuilder PB;

  // Register all the analyses
  PB.registerModuleAnalyses(MAM);
  PB.registerCGSCCAnalyses(CGAM);
  PB.registerFunctionAnalyses(FAM);
  PB.registerLoopAnalyses(LAM);
  PB.crossRegisterProxies(LAM, FAM, CGAM, MAM);

  // Create the pass manager and add optimization passes
  llvm::ModulePassManager MPM =
      PB.buildPerModuleDefaultPipeline(llvm::OptimizationLevel::O3);

  // Run the optimizations
  MPM.run(*ctx_->module, MAM);
}

} // namespace druk

#endif // DRUK_HAVE_LLVM
