# Druk Programming Language

A high-performance compiler for a programming language with Dzongkha syntax.

## Requirements

- C++20 compiler (GCC 13.2+ recommended)
- CMake 3.27+
- ICU (International Components for Unicode)
- Ninja (optional, recommended)

## Building

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

## Running

```bash
./build/druk examples/hello.druk
```

## Testing

```bash
cd build
ctest
```
