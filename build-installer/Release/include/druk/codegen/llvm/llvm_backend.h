#pragma once

#ifdef DRUK_HAVE_LLVM

#include <llvm/ExecutionEngine/Orc/LLJIT.h>
#include <llvm/ExecutionEngine/Orc/ThreadSafeModule.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "druk/codegen/core/chunk.h"
#include "druk/codegen/core/obj.h"
#include "druk/codegen/core/value.h"
#include "druk/codegen/jit/jit_runtime.h"
#include "druk/ir/ir_function.h"

namespace druk::codegen
{

class LLVMBackend
{
   public:
    explicit LLVMBackend(bool debug = false);
    ~LLVMBackend();

    LLVMBackend(const LLVMBackend&)            = delete;
    LLVMBackend& operator=(const LLVMBackend&) = delete;

    using CompiledFunc = void (*)(PackedValue* out);
    CompiledFunc compileFunction(ir::Function* function);
    bool         emitObjectFile(ir::Module& module, const std::string& obj_path);

   private:
    bool debug_ = false;

    struct CompilationContext
    {
        std::unique_ptr<llvm::LLVMContext> context;
        std::unique_ptr<llvm::Module>      module;
        std::unique_ptr<llvm::IRBuilder<>> builder;
        std::unique_ptr<llvm::orc::LLJIT>  jit;

        ir::Function*   current_ir_function = nullptr;
        llvm::Function* llvm_function       = nullptr;
        llvm::Value*    ret_out             = nullptr;

        // New IR State
        std::unordered_map<ir::Value*, llvm::Value*>           ir_values;
        std::unordered_map<ir::BasicBlock*, llvm::BasicBlock*> ir_blocks;
        std::unordered_map<ir::Function*, llvm::Function*>     ir_functions;  // implementations
        std::unordered_map<ir::Function*, llvm::Function*>     ir_wrappers;   // void(out*) thunks

        std::unordered_map<std::string, llvm::GlobalVariable*> globals;

        CompilationContext();
    };

    std::unique_ptr<CompilationContext> ctx_;

    llvm::Value* get_llvm_value(ir::Value* value);
    llvm::Value* create_entry_alloca(llvm::Type* type, const std::string& name = "");

    void compile_instruction(ir::Instruction* inst, llvm::StructType* packed_value_ty,
                             llvm::PointerType* packed_ptr_ty, llvm::Type* i64_ty);
    void prepare_functions_and_wrappers(ir::Module& module, llvm::StructType* packed_value_ty,
                                        llvm::PointerType* packed_ptr_ty);

    void compile_single_function(ir::Function* function, llvm::StructType* packed_value_ty,
                                 llvm::PointerType* packed_ptr_ty, llvm::Type* i64_ty);
    void compile_binary_op(ir::Instruction* inst, llvm::StructType* packed_value_ty,
                           llvm::PointerType* packed_ptr_ty);
    void compile_memory_ops(ir::Instruction* inst, llvm::StructType* packed_value_ty,
                            llvm::Type* i64_ty);
    void compile_array_ops(ir::Instruction* inst, llvm::StructType* packed_value_ty,
                           llvm::PointerType* packed_ptr_ty, llvm::Type* i64_ty);
    void compile_array_build(ir::Instruction* inst, llvm::StructType* packed_value_ty,
                             llvm::PointerType* packed_ptr_ty, llvm::Type* i64_ty);
    void compile_array_index(ir::Instruction* inst, llvm::StructType* packed_value_ty,
                             llvm::PointerType* packed_ptr_ty);
    void compile_array_len(ir::Instruction* inst, llvm::StructType* packed_value_ty,
                           llvm::PointerType* packed_ptr_ty);
    void compile_control_flow(ir::Instruction* inst, llvm::StructType* packed_value_ty,
                              llvm::PointerType* packed_ptr_ty, llvm::Type* i64_ty);
    void compile_call_op(ir::Instruction* inst, llvm::PointerType* packed_ptr_ty);
    void compile_dynamic_call_op(ir::Instruction* inst, llvm::StructType* packed_value_ty,
                                 llvm::PointerType* packed_ptr_ty, llvm::Type* i64_ty);
    void compile_print_op(ir::Instruction* inst, llvm::PointerType* packed_ptr_ty);
    void compile_string_ops(ir::Instruction* inst, llvm::StructType* packed_value_ty,
                            llvm::PointerType* packed_ptr_ty);
    void compile_null_ops(ir::Instruction* inst, llvm::StructType* packed_value_ty,
                          llvm::PointerType* packed_ptr_ty);
    void compile_unary_op(ir::Instruction* inst, llvm::StructType* packed_value_ty,
                          llvm::PointerType* packed_ptr_ty);

    void register_runtime_symbols();
    void register_extended_symbols();
    void optimize_module();
};

}  // namespace druk::codegen

#endif  // DRUK_HAVE_LLVM
