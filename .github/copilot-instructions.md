# GitHub Copilot Instructions - Druk Programming Language

## 🌏 Project Context

**Druk** (འབྲུག་) is a compiler for a programming language using **Dzongkha/Tibetan script syntax**. This is a serious compiler project with:

- Lexer (tokenization of Unicode Tibetan characters U+0F00-U+0FFF)
- Parser (recursive descent with full AST)
- Semantic analysis (type checking, name resolution)
- IR generation (custom intermediate representation)
- Multiple backends (Bytecode VM + LLVM JIT compiler)
- Runtime with garbage collection

**Language Example:**
```druk
// གྲངས་ཀ (Number type)
གྲངས་ཀ་ ན = ༡༠;  // n = 10 (Tibetan numerals ༠-༩)
གྲངས་ཀ་ ཨ = ༠;   // a = 0
གྲངས་ཀ་ བ = ༡;   // b = 1

// རིམ་པ (Loop)
རིམ་པ་ (ན > ༢) {
    གྲངས་ཀ་ ག = ཨ + བ;
    འབྲི་ ག;  // Print
    ཨ = བ;
    བ = ག;
    ན = ན - ༡;
}
```

---

## 🏗️ Architecture Rules - CRITICAL

### 1. MAXIMUM 100 LINES PER FILE - ABSOLUTE RULE

**Every single .cpp and .hpp file MUST be ≤100 lines.**

When suggesting code:
- ✅ Split into multiple focused files if needed
- ✅ Extract helper functions to separate files
- ❌ NEVER create files >100 lines
- ❌ NEVER suggest "adding" code that would exceed 100 lines

**Example of proper splitting:**
```
❌ BAD: backend_ir.cpp (406 lines) - REJECTED

✅ GOOD: Split into 5 files:
- backend_ir_constants.cpp (90 lines) - LLVM value constants
- backend_ir_compile.cpp (86 lines) - Function compilation
- backend_ir_instructions.cpp (92 lines) - Instruction dispatcher
- backend_ir_binary_ops.cpp (55 lines) - Binary operations
- backend_ir_control_flow.cpp (85 lines) - Branches & returns
```

### 2. Deep Module Hierarchy - Use Nested Folders

Prefer deep folder structures over flat ones:

```
✅ GOOD:
src/codegen/llvm/backend_ir_binary_ops.cpp
src/codegen/llvm/backend_ir_control_flow.cpp
src/parser/expr/primary.cpp
src/parser/expr/binary.cpp

❌ BAD:
src/codegen_llvm.cpp (monolithic)
src/parser_expressions.cpp (too broad)
```

### 3. Single Responsibility Per File

Each file should handle ONE specific concern:

```cpp
// ✅ backend_ir_binary_ops.cpp - ONLY binary operations
void compile_binary_op(ir::Instruction* inst, ...) {
    switch (inst->getOpcode()) {
        case ir::Opcode::Add: // ...
        case ir::Opcode::Sub: // ...
        case ir::Opcode::Mul: // ...
        case ir::Opcode::Div: // ...
    }
}

// ✅ backend_ir_control_flow.cpp - ONLY control flow
void compile_control_flow(ir::Instruction* inst, ...) {
    switch (inst->getOpcode()) {
        case ir::Opcode::Branch: // ...
        case ir::Opcode::ConditionalBranch: // ...
        case ir::Opcode::Return: // ...
    }
}
```

---

## 💻 C++ Code Style

### Naming Conventions - STRICT

| Element | Style | Example |
|---------|-------|---------|
| Classes/Structs | `PascalCase` | `LLVMBackend`, `CodeGenerator` |
| Functions/Methods | `snake_case` | `compile_instruction()`, `get_token()` |
| Variables | `snake_case` | `current_token`, `ir_values` |
| Constants | `kPascalCase` or `UPPER` | `kMaxLocals`, `MAX_STACK_SIZE` |
| Private members | `member_` | `builder_`, `context_`, `module_` |
| Namespaces | `snake_case` | `druk::codegen`, `druk::ir` |

### Indentation & Formatting

```cpp
// 4 spaces, NO TABS
namespace druk::codegen
{

void LLVMBackend::compile_binary_op(ir::Instruction* inst,
                                    llvm::StructType* packed_value_ty,
                                    llvm::PointerType* packed_ptr_ty)
{
    auto ops = inst->getOperands();
    if (ops.size() < 2)
        return;
    
    switch (inst->getOpcode())
    {
        case ir::Opcode::Add:
            fn = "druk_jit_add";
            break;
        default:
            return;
    }
}

}  // namespace druk::codegen
```

**Rules:**
- Opening brace on same line for namespaces/functions
- 4-space indentation (never tabs)
- Line length: prefer 100 cols, max 120
- Always use `{ }` for control flow (even single statements)

### Include Order

```cpp
// 1. Corresponding header
#include "druk/codegen/llvm/llvm_backend.h"

// 2. C++ standard library
#include <memory>
#include <vector>
#include <string>

// 3. Third-party (LLVM, etc.)
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>

// 4. Other project headers
#include "druk/ir/ir_instruction.h"
#include "druk/codegen/jit/jit_runtime.h"
```

### Header Guards

```cpp
#pragma once  // Always use #pragma once, NOT include guards
```

---

## 📁 File Organization

### Directory Structure

```
include/druk/          # Public API headers
├── lexer/
│   ├── lexer.hpp
│   └── token.hpp
├── parser/
│   ├── ast/           # AST node definitions
│   │   ├── expr.hpp
│   │   └── stmt.hpp
│   └── core/
│       └── parser.hpp
├── codegen/
│   ├── core/          # IR generation
│   ├── llvm/          # LLVM backend
│   └── jit/           # JIT runtime
├── ir/                # Intermediate representation
└── semantic/          # Type checking, resolution

src/                   # Implementation (.cpp files)
├── lexer/
├── parser/
│   ├── core/
│   └── expr/          # Split by expression type
│       ├── primary.cpp
│       ├── binary.cpp
│       └── unary.cpp
├── codegen/
│   ├── core/
│   └── llvm/          # SPLIT: 9+ small files
│       ├── backend_ir_constants.cpp      (90 lines)
│       ├── backend_ir_compile.cpp        (86 lines)
│       ├── backend_ir_instructions.cpp   (92 lines)
│       ├── backend_ir_binary_ops.cpp     (65 lines)
│       ├── backend_ir_control_flow.cpp   (85 lines)
│       ├── llvm_backend_init.cpp         (64 lines)
│       ├── llvm_backend_symbols.cpp      (106 lines)
│       ├── llvm_backend_symbols2.cpp     (97 lines)
│       └── llvm_backend_utils.cpp        (35 lines)
├── ir/
└── semantic/
```

### When to Create New Files

**Immediately split when:**
1. File reaches 80+ lines (proactive splitting)
2. Adding a new feature that doesn't fit existing file's focus
3. Function has distinct logical grouping (e.g., binary ops vs control flow)

**Naming:**
- Descriptive, not generic: `backend_ir_binary_ops.cpp` ✅ not `helpers.cpp` ❌
- Reflects functionality: `llvm_backend_symbols.cpp` ✅

---

## 🔧 Implementation Patterns

### Pattern 1: Feature Compilation with Optional Components

```cpp
#ifdef DRUK_HAVE_LLVM  // Always wrap LLVM code

#include "druk/codegen/llvm/llvm_backend.h"

namespace druk::codegen
{

void LLVMBackend::compile_binary_op(ir::Instruction* inst, ...)
{
    // Implementation
}

}  // namespace druk::codegen

#endif  // DRUK_HAVE_LLVM
```

### Pattern 2: Symbol Table / Variable Management

```cpp
class CodeGenerator
{
   private:
    // Always use descriptive containers
    std::unordered_map<std::string, ir::Value*> variables_;  // Name -> alloca
    ir::Value* lastValue_ = nullptr;  // Track last expression result
};
```

### Pattern 3: Unicode/Tibetan Handling

```cpp
// Tibetan digits: ༠-༩ (U+0F20-0F29)
// UTF-8 encoding: E0 BC A0 to E0 BC A9

std::string tibetanToAsciiNumber(std::string_view text)
{
    std::string result;
    for (size_t i = 0; i < text.size();)
    {
        if (i + 2 < text.size() &&
            static_cast<unsigned char>(text[i]) == 0xE0 &&
            static_cast<unsigned char>(text[i+1]) == 0xBC &&
            static_cast<unsigned char>(text[i+2]) >= 0xA0 &&
            static_cast<unsigned char>(text[i+2]) <= 0xA9)
        {
            result += '0' + (text[i+2] - 0xA0);  // Convert Tibetan to ASCII
            i += 3;  // Skip 3-byte UTF-8 sequence
        }
        else
        {
            ++i;
        }
    }
    return result;
}
```

### Pattern 4: Error Handling

```cpp
// Always validate inputs at function start
void compile_instruction(ir::Instruction* inst, ...)
{
    if (!inst)
        return;
        
    auto ops = inst->getOperands();
    if (ops.size() < 2)
        return;
    
    // ... proceed with valid data
}
```

---

## 🧪 Testing

### Test File Naming

```
tests/
├── lexer/
│   ├── lexer_test.cpp
│   └── lexer_tibetan_test.cpp  # Specific to Tibetan unicode
├── parser/
│   ├── expr_test.cpp
│   └── stmt_test.cpp
└── codegen/
    ├── ir_gen_test.cpp
    └── llvm_backend_test.cpp
```

### Test Naming Convention

```cpp
TEST(LexerTest, TokenizesTibetanNumerals) {
    // Test ༠-༩ recognition
}

TEST(ParserTest, ParsesLoopStatementWithTibetanCondition) {
    // Test རིམ་པ (loop)
}

TEST(CodegenTest, GeneratesBinaryOpIR) {
    // Test +, -, *, /
}
```

---

## 🌐 Tibetan/Dzongkha Specifics

### Keywords & Syntax

| Dzongkha | English | Unicode Codepoints | Usage |
|----------|---------|-------------------|-------|
| གྲངས་ཀ | Number | U+0F42... | Type declaration: `གྲངས་ཀ་ x = ༥;` |
| འབྲི | Print | U+0F60... | Statement: `འབྲི་ x;` |
| རིམ་པ | Loop | U+0F62... | Control: `རིམ་པ་ (x > ༠) { }` |
| གལ་ཏེ | If | U+0F42... | Conditional: `གལ་ཏེ (x) { }` |

### Numerals (CRITICAL)

```cpp
// Tibetan numerals MUST be converted before numeric parsing
'༠' (U+0F20) → '0'
'༡' (U+0F21) → '1'
'༢' (U+0F22) → '2'
'༣' (U+0F23) → '3'
'༤' (U+0F24) → '4'
'༥' (U+0F25) → '5'
'༦' (U+0F26) → '6'
'༧' (U+0F27) → '7'
'༨' (U+0F28) → '8'
'༩' (U+0F29) → '9'

// UTF-8 encoding: E0 BC A0-A9 (3 bytes)
```

### Variable Name Encoding

Variables use Tibetan script, stored as UTF-8:
```cpp
// Symbol table keys are UTF-8 encoded Tibetan strings
std::unordered_map<std::string, ir::Value*> variables_;

// Example: "ན" (na) → E0 BD 93 (3 bytes)
variables_["ན"] = alloca;  // Valid key
```

---

## 📝 Documentation Style

### Function Documentation

```cpp
/**
 * @brief Compiles binary arithmetic operations to LLVM IR.
 * 
 * Generates calls to JIT runtime functions (druk_jit_add, druk_jit_subtract, etc.)
 * that handle dynamic type checking and value operations.
 * 
 * @param inst IR instruction (must be Add/Sub/Mul/Div/comparison opcode)
 * @param packed_value_ty LLVM type for PackedValue struct
 * @param packed_ptr_ty LLVM pointer type for PackedValue*
 */
void compile_binary_op(ir::Instruction* inst,
                      llvm::StructType* packed_value_ty,
                      llvm::PointerType* packed_ptr_ty);
```

### Code Comments

```cpp
// ✅ GOOD: Explain WHY and non-obvious details
// Tibetan numerals are 3-byte UTF-8 sequences (E0 BC A0-A9)
// Must be converted before std::atoll() which only handles ASCII
if (static_cast<unsigned char>(text[i]) == 0xE0) {
    // ...
}

// ❌ BAD: State the obvious
// Check if character is E0
if (static_cast<unsigned char>(text[i]) == 0xE0) {
    // ...
}
```

### File Headers

```cpp
/**
 * @file backend_ir_binary_ops.cpp
 * @brief LLVM IR code generation for binary arithmetic operations.
 * 
 * Handles Add, Sub, Mul, Div, and comparison operators by generating
 * calls to JIT runtime functions (druk_jit_add, druk_jit_subtract, etc.).
 */
```

---

## 🚫 Anti-Patterns - DO NOT DO

### ❌ Large Monolithic Files

```cpp
// ❌ BAD: 500-line file with everything
void LLVMBackend::compile_function(ir::Function* func) {
    // 100 lines of setup
    // 150 lines of constant handling  
    // 200 lines of instruction compilation
    // 50 lines of optimization
}
```

### ❌ Generic Helper Files

```cpp
// ❌ BAD naming
helpers.cpp
utils.cpp
common.cpp

// ✅ GOOD naming
backend_ir_binary_ops.cpp
llvm_backend_symbols.cpp
tibetan_numeral_converter.cpp
```

### ❌ Deep Nesting

```cpp
// ❌ BAD: >3 levels of nesting
if (x) {
    if (y) {
        for (...) {
            if (z) {
                // Bad
            }
        }
    }
}

// ✅ GOOD: Early returns, extract functions
if (!x) return;
if (!y) return;

process_items();  // Extract nested logic
```

### ❌ Raw Pointers Without Documentation

```cpp
// ❌ BAD: Unclear ownership
Value* lastValue;

// ✅ GOOD: Document ownership/lifetime
ir::Value* lastValue_ = nullptr;  // Non-owning, valid until next expression
```

---

## 🔄 CMake Integration

When splitting files, **ALWAYS update CMakeLists.txt**:

```cmake
# Add ALL new source files
set(DRUK_LLVM_SOURCES
    src/codegen/llvm/backend_ir_constants.cpp
    src/codegen/llvm/backend_ir_compile.cpp
    src/codegen/llvm/backend_ir_instructions.cpp
    src/codegen/llvm/backend_ir_binary_ops.cpp
    src/codegen/llvm/backend_ir_control_flow.cpp
    src/codegen/llvm/llvm_backend_init.cpp
    src/codegen/llvm/llvm_backend_symbols.cpp
    src/codegen/llvm/llvm_backend_symbols2.cpp
    src/codegen/llvm/llvm_backend_utils.cpp
)

if(DRUK_HAVE_LLVM)
    target_sources(druk-core PRIVATE ${DRUK_LLVM_SOURCES})
endif()
```

---

## 🎯 Code Generation Priorities

When suggesting code, follow this priority:

1. **100-line limit** - Non-negotiable, split if needed
2. **Single responsibility** - One concern per file
3. **Descriptive names** - Function/file names explain purpose
4. **Tibetan support** - Handle UTF-8 encoded Dzongkha properly
5. **Modern C++17** - Use auto, range-for, smart pointers
6. **Documentation** - Brief comments explaining WHY

---

## 📊 Success Metrics

A good code suggestion:
- ✅ All files ≤100 lines
- ✅ Descriptive file/function names
- ✅ Properly handles Tibetan Unicode (if relevant)
- ✅ Includes CMakeLists.txt updates (for new files)
- ✅ Has brief documentation
- ✅ Follows project structure conventions
- ✅ Uses modern C++17 features

---

## 🔍 Common Tasks & Patterns

### Adding a New IR Instruction

1. Add opcode to `include/druk/ir/ir_instruction.h`
2. Create instruction class (following BinaryInst pattern)
3. Add IRBuilder method to `include/druk/ir/ir_builder.h`
4. Implement in `src/ir/ir_builder.cpp`
5. Add code generation in appropriate backend file:
   - If arithmetic: `backend_ir_binary_ops.cpp`
   - If control flow: `backend_ir_control_flow.cpp`
   - If new category: Create new file (e.g., `backend_ir_logical_ops.cpp`)

### Adding LLVM Runtime Function

1. Declare in `include/druk/codegen/jit/jit_runtime.h`
2. Implement in `src/codegen/jit/jit_runtime.cpp`
3. Register symbol:
   - If basic operation: `llvm_backend_symbols.cpp`
   - If extended: `llvm_backend_symbols2.cpp`
   - If new category: Create `llvm_backend_symbols3.cpp`

### Supporting New Tibetan Keyword

1. Add token type to `include/druk/lexer/token.hpp`
2. Update lexer to recognize keyword
3. Add parser support in appropriate file under `src/parser/`
4. Add semantic analysis in `src/semantic/`
5. Add code generation

---

## 💬 Communication Style

When suggesting code changes:

```
✅ GOOD:
"This function is 120 lines. Let's split it into:
- compile_arithmetic_ops() (45 lines) 
- compile_comparison_ops() (40 lines)
- compile_logical_ops() (35 lines)"

❌ BAD:
"Add this 150-line function to the file"
```

---

## 🌟 Final Reminder

**EVERY file must be ≤100 lines. No exceptions.**

This is not a guideline—it's a hard rule. When in doubt:
1. Split into multiple files
2. Use descriptive names
3. Update CMakeLists.txt
4. Document the split

**དགའ་པོ་བྱོས།** (Happy Coding!)
