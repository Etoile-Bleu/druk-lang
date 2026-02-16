#pragma once

#ifdef DRUK_HAVE_LLVM

#include "druk/codegen/chunk.hpp"
#include "druk/codegen/jit_runtime.hpp"
#include "druk/codegen/obj.hpp"
#include "druk/codegen/opcode.hpp"
#include "druk/codegen/value.hpp"
#include <llvm/ExecutionEngine/Orc/LLJIT.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <memory>
#include <string>
#include <unordered_map>

namespace druk {

// Forward declarations
class VM;

/**
 * LLVM JIT backend for Druk bytecode.
 * Compiles bytecode to native machine code for maximum performance.
 */
class LLVMBackend {
public:
  explicit LLVMBackend(bool debug = false);
  ~LLVMBackend();

  // Disable copy and move
  LLVMBackend(const LLVMBackend &) = delete;
  LLVMBackend &operator=(const LLVMBackend &) = delete;

  /**
   * Compile a function's bytecode to native code.
   * Returns a function pointer that can be called directly.
   */
  using CompiledFunc = void (*)(PackedValue *out);
  CompiledFunc compile_function(ObjFunction *function);

  // Check if this function only uses JIT-supported opcodes.
  bool supports(ObjFunction *function) const;

  /**
   * Clear all compiled code and reset the JIT.
   */
  void reset();

private:
  bool debug_ = false;

  struct CompilationContext {
    llvm::LLVMContext context;
    std::unique_ptr<llvm::Module> module;
    llvm::IRBuilder<> builder;
    std::unique_ptr<llvm::orc::LLJIT> jit;

    // Current function being compiled
    ObjFunction *current_function = nullptr;
    llvm::Function *llvm_function = nullptr;
    llvm::Value *ret_out = nullptr;
    
    // Bytecode to basic block mapping
    std::unordered_map<size_t, llvm::BasicBlock *> basic_blocks;
    
    // Local variables (stack slots)
    std::vector<llvm::Value *> locals;
    
    // Value stack for expression evaluation
    std::vector<llvm::Value *> stack;

    // Stack state snapshot per basic block offset
    std::unordered_map<size_t, std::vector<llvm::Value *>> block_stack_states;
    
    // Track predecessors and their stack states for each block
    std::unordered_map<size_t, std::vector<std::pair<llvm::BasicBlock*, std::vector<llvm::Value*>>>> block_predecessors;
    
    // Global variables map
    std::unordered_map<std::string, llvm::GlobalVariable *> globals;

    CompilationContext();
  };

  std::unique_ptr<CompilationContext> ctx_;

  // Compilation stages
  void compile_function_body(ObjFunction *function);
  void analyze_control_flow(ObjFunction *function);
  llvm::BasicBlock *get_or_create_block(size_t offset);
  
  // Bytecode compilation
  void compile_instruction(const uint8_t *&ip, ObjFunction *function);
  
  // Stack manipulation helpers
  void push(llvm::Value *value);
  llvm::Value *pop();
  llvm::Value *peek(int distance);
  llvm::Value *create_entry_alloca(llvm::Type *type,
                                   const std::string &name = "");
  
  // Opcode implementations
  void emit_constant(llvm::Value *value);
  void emit_return();
  
  // Optimization passes
  void optimize_module();
};

} // namespace druk

#endif // DRUK_HAVE_LLVM
