# Druk Programming Language

A high-performance compiler for a programming language with Dzongkha syntax.

## Features

- 🚀 **LLVM JIT Compilation** - Native code generation for maximum performance
- 📊 **Bytecode VM** - Fast interpreted execution with computed goto dispatch
- 🔤 **Unicode Support** - Full Tibetan/Dzongkha script support via ICU
- ⚡ **Optimizations** - LLVM O3 optimizations, constant folding, loop unrolling

## Performance

With LLVM JIT enabled:
- **1000-10000x faster** than VM interpretation
- Near-native C++ performance for compute-intensive loops
- Sub-nanosecond overhead per operation

See [LLVM_JIT_GUIDE.md](LLVM_JIT_GUIDE.md) for benchmarks and optimization details.

## Requirements

- C++20 compiler (GCC 13.2+ / MSVC 2022+ / Clang 15+)
- CMake 3.27+
- ICU (International Components for Unicode)
- **LLVM 17+** (optional, for JIT compilation)
- Ninja (optional, recommended)

## Quick Start

### Install LLVM (Optional but Recommended)

**Windows:**
```powershell
choco install llvm
```

**Linux:**
```bash
sudo apt-get install llvm-17 llvm-17-dev  # Ubuntu/Debian
```

**macOS:**
```bash
brew install llvm@17
```

## Building

### With LLVM JIT (Recommended)

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

### Without LLVM (VM only)

If LLVM is not found, the project automatically builds with VM-only mode.

## Running

```bash
./build/druk examples/hello.druk
```

## Benchmarks

Run performance comparisons:

```powershell
# Windows
.\run_jit_benchmark.ps1

# Linux/macOS
cd build && ./bin/druk_compare_jit_bench
```

## Testing

```bash
cd build
ctest
```
