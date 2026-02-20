# Druk Language: Future Features & Roadmap

Druk is designed to be a **Simple, High-Performance, GC-backed** language. We want developers to focus on logic without worrying about manual memory management, while still enjoying the speed of an LLVM-compiled binary.

This document outlines "gigantic" ideas for the future evolution of Druk.

---

## 1. Core Language & Productivity

### First-Class Functions & Closures
*   **The Goal**: Make functions treatable as values.
*   **Feature**: Lambda expressions and anonymous functions.
*   **Why**: Essential for modern programming patterns (map, filter, sort).

### Pattern Matching (Matching)
*   **The Goal**: Safe and expressive branching.
*   **Feature**: A `འགྲིག་པ་` (*'grig pa* - match) statement that supports deconstructing structs and checking values.
*   **Example**:
    ```dzongkha
    འགྲིག་པ་ x {
        ༡ -> བཀོད་ "One";
        ༢ -> བཀོད་ "Two";
        _ -> བཀོད་ "Other";
    }
    ```

### Built-in Dictionary/Map Type
*   **The Goal**: Easy data organization.
*   **Feature**: A native `Map<K, V>` type with syntax like `[ "key": "value" ]`.
*   **Why**: Crucial for almost any modern application (web, scripts, data).

### String Interpolation
*   **The Goal**: Readable string formatting.
*   **Feature**: Embedded expressions in strings. `བཀོད་ "Hello {name}, you are {age} years old!";`

### Null Safety (Option Types)
*   **The Goal**: "Don't care about memory" includes "Don't crash because of Null".
*   **Feature**: Non-nullable types by default. Optional types using `?`.
*   **Why**: High reliability with zero manual overhead.

---

## 2. Performance & Optimization

### Lightweight Concurrency (Green Threads)
*   **The Goal**: Go-like performance with simple syntax.
*   **Feature**: A `འགྲུལ་` (*'grul* - travel/process) keyword to spawn a concurrent task.
*   **Why**: Utilize multi-core CPUs without the complexity of threads/locks.

### Just-In-Time (JIT) Tiering
*   **The Goal**: Start fast, stay fast.
*   **Feature**: Multi-tier compilation. Quick JIT for startup, optimized LLVM for hot loops.

### SIMD Vectorization
*   **The Goal**: Extreme speed for math.
*   **Feature**: Auto-vectorization for array operations using LLVM's SIMD primitives.

### Low-Latency Garbage Collection
*   **The Goal**: "S'en fou de la mémoire" but without the "stop-the-world" pauses.
*   **Feature**: Implementation of a concurrent, generational GC (using the work already started in `gc_heap`).

---

## 3. Localization & Unique Identity

### Tibetan Date & Calendar Library
*   **The Goal**: Native support for the Lunar calendar.
*   **Feature**: Built-in functions to handle Tibetan dates, horoscopes, and time tracking.

### Native Numeral Operations
*   **The Goal**: Treat ༡ + ༢ as naturally as 1 + 2.
*   **Feature**: Full first-class support for Tibetan numerals in literal math, ensuring zero performance penalty compared to ASCII digits.

---

## 4. Ecosystem & Tooling

### Standard Library (Druk-Std)
*   **I/O**: Simple file reading/writing.
*   **Network**: High-performance HTTP client/server (built on top of the green thread system).
*   **JSON**: Native serialization/deserialization for structs.

### Interoperability (FFI)
*   **The Goal**: Access the world of C.
*   **Feature**: Easy foreign function interface to call existing C libraries (libuv, openssl, etc.).

### Druk LSP (Language Server Protocol)
*   **The Goal**: Modern IDE support.
*   **Feature**: Real-time error highlighting, autocompletion (in both Romanized and Tibetan script), and go-to-definition.

---

## 5. Design Philosophy: The "Simple & Fast" Rule

Every new feature in Druk must pass two tests:
1.  **Can a beginner understand it in 5 minutes?**
2.  **Does it have a path to O(1) or O(N) performance via LLVM?**

We ignore memory safety *ceremonies* (like lifetimes/manual free) in favor of a robust GC, allowing the developer's mind to stay on the problem, not the hardware.
