#pragma once
#include "druk/codegen/chunk.hpp"
#include "druk/codegen/obj.hpp"
#include "druk/codegen/value.hpp"
#include <memory>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace druk {

enum class InterpretResult { Ok, CompileError, RuntimeError };

struct CallFrame {
  ObjFunction *function;
  const uint8_t *ip;
  Value *slots;
};

class VM {
public:
  VM();
  ~VM();

  InterpretResult interpret(std::shared_ptr<ObjFunction> function);
  void set_args(const std::vector<std::string> &args);

private:
  InterpretResult run();

  void push(Value value);
  Value pop();
  const Value &peek(int distance) const;

  void runtime_error(const char *format, ...);
  std::string_view store_string(std::string value);

  size_t stack_size() const {
    return static_cast<size_t>(stack_top_ - stack_base_);
  }

  // Stack of call frames
  std::vector<CallFrame> frames_;
  CallFrame *frame_ = nullptr; // Current frame cache

  // Stack
  static constexpr size_t kStackMax = 256 * 64;
  std::vector<Value> stack_;
  Value *stack_base_ = nullptr;
  Value *stack_top_ = nullptr;

  // Globals
  std::unordered_map<std::string_view, Value> globals_;
  size_t globals_version_ = 0;
  struct GlobalCache {
    std::string_view name{};
    Value *slot = nullptr;
    size_t version = 0;
  } global_cache_;

  // argv/input string storage
  std::vector<std::string> argv_storage_;
  std::vector<std::string> input_storage_;

  friend class VMTest;
};

} // namespace druk
