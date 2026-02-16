# Druk Performance Optimization Guide

## Current Status

After implementing a basic tree-walking interpreter, here are the benchmark results:

```
BM_CPP_Sum        0.184 ns    (~5.4T items/s)
BM_Druk_Sum    114,276 ns    (~8.75M items/s)
```

**Performance Gap**: Druk is approximately **621,000x slower** than C++.

## Completed Optimizations

### 1. Implemented Basic Interpreter ✅
- Created tree-walking interpreter for expressions and statements
- Fixed parser bugs (loop/if parsing, operator precedence)
- Optimized benchmark to only measure interpretation time (parsing done once)

### 2. Fast Number Parsing ✅
- Used `std::from_chars` for optimal number parsing performance
- Direct conversion without string manipulation

## Recommended Optimizations (Priority Order)

### HIGH PRIORITY - Expected 10-100x Improvement

#### 1. **Bytecode Compilation**
Instead of walking the AST, compile to bytecode and use a VM:
```cpp
// Current (slow): Walk AST for every iteration
for (auto _ : state) {
  interpreter.execute(ast);  // Visits every node
}

// Optimized: Compile once, execute bytecode
auto bytecode = compiler.compile(ast);  // Once
for (auto _ : state) {
  vm.execute(bytecode);  // Simple instruction fetch-decode-execute loop
}
```
**Expected improvement**: 20-50x faster

#### 2. **Inline Caching for Variables**
Cache variable locations after first lookup:
```cpp
struct CachedVariable {
  std::string_view name;
  size_t slot;  // Cache the index
  bool valid;
};

// First access: O(n) search + cache
// Subsequent: O(1) direct array access
```
**Expected improvement**: 5-10x for variable-heavy code

#### 3. **Tagged Value Representation**
Replace `std::variant` with compact tagged values:
```cpp
// Current: std::variant<double, bool, string_view> = 32+ bytes
// Optimized: Single 64-bit value with type tag
struct Value {
  uint64_t bits;  // 8 bytes total
  // Use NaN-boxing or pointer tagging
};
```
**Expected improvement**: 2-3x (less memory, better cache)

### MEDIUM PRIORITY - Expected 2-10x Improvement

#### 4. **Direct-Threaded Interpreter**
Use computed goto for dispatch:
```cpp
// Current: switch statement (branch prediction issues)
switch (expr->kind) {
  case Binary: ...
  case Unary: ...
}

// Optimized: Direct jumps
static void* dispatch_table[] = {&&op_binary, &&op_unary, ...};
goto *dispatch_table[instr];
```
**Expected improvement**: 2-5x

#### 5. **Stack-Based Execution**
Replace recursive evaluate() with explicit stack:
```cpp
// Current: Recursive calls have overhead
Value evaluate(Expr* expr);

// Optimized: Explicit value stack
std::array<Value, 256> stack;
int sp = 0;
```
**Expected improvement**: 2-3x

#### 6. **Specialized Fast Paths**
Add fast paths for common patterns:
```cpp
// Detect: sum = sum + i (very common in loops)
if (is_accumulator_pattern(expr)) {
  // Direct: *slot += value (no AST walking)
  return fast_accumulate(slot, value);
}
```
**Expected improvement**: 3-5x for loops

### LOW PRIORITY - Expected <2x Improvement

#### 7. **String Interning**
Ensure variable names are interned once:
- Already using `StringInterner` ✅
- Could add perfect hashing for known identifiers

#### 8. **Arena Allocation**
Already using arena allocator ✅
- Could add object pooling for hot types

#### 9. **Compiler Optimizations**
```bash
# Already using:
-O3 -march=native -flto

# Could add:
-fprofile-generate  # Profile-guided optimization
# Run benchmarks
-fprofile-use
```
**Expected improvement**: 1.2-1.5x

## Implementation Roadmap

### Phase 1: Quick Wins (1-2 hours)
1. Add inline caching for variables
2. Optimize Value representation (remove std::variant)
3. Use explicit stack instead of recursion

**Expected**: 5-10x improvement → ~10-20µs

### Phase 2: Bytecode VM (4-8 hours)
1. Design bytecode instruction set
2. Implement compiler (AST → bytecode)
3. Implement bytecode VM
4. Add direct threading

**Expected**: Additional 10-20x → ~0.5-2µs

### Phase 3: JIT Compilation (Advanced)
1. Use LLVM or custom JIT
2. Compile hot loops to native code
3. Add type specialization

**Expected**: Additional 100-1000x → approach C++ performance

## Benchmark Comparison

| Implementation | Time | Relative to C++ |
|---|---|---|
| Current Druk | 114µs | 621,000x slower |
| After Phase 1 | ~15µs | ~80,000x slower |
| After Phase 2 | ~1µs | ~5,000x slower |
| After Phase 3 (JIT) | ~10ns | ~50x slower |
| Theoretical limit | ~1ns | ~5x slower |

## Example: Lua Performance

Lua's interpreter achieves similar performance to Phase 2:
- Uses bytecode VM with register-based execution
- Simple loop sum is ~1-5µs on similar hardware
- With LuaJIT: ~10-50ns (near-C++ performance)

## Conclusion

The current 621,000x slowdown is expected for a basic tree-walking interpreter. With the recommended optimizations, Druk can achieve:
- **Phase 1** (simple changes): ~50,000-100,000x slower (~realistic for interpreted languages)
- **Phase 2** (bytecode VM): ~5,000-10,000x slower (~competitive with Python/Ruby)
- **Phase 3** (JIT): ~10-100x slower (~competitive with LuaJIT/V8)

The most impactful single change would be implementing a bytecode compiler + VM (Phase 2).
