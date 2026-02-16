#include "druk/vm/vm.hpp"
#include "druk/codegen/opcode.hpp"
#include "druk/lexer/unicode.hpp"
#include <cmath>
#include <cstdarg>
#include <cstdio>
#include <iostream>
#include <memory>
#include <variant>
#include <string>

#ifdef __GNUC__
#pragma GCC diagnostic ignored "-Wpedantic"
#endif

// Define this to trace execution
#define DEBUG_TRACE_EXECUTION

namespace druk {

VM::VM() {
  stack_.resize(kStackMax);
  stack_base_ = stack_.data();
  stack_top_ = stack_base_;
  frames_.reserve(64);      // Pre-allocate frames
}

VM::~VM() {}

void VM::set_args(const std::vector<std::string>& args) {
  argv_storage_ = args;

  auto argv_array = std::make_shared<ObjArray>();
  argv_array->elements.reserve(argv_storage_.size());
  for (const auto& s : argv_storage_) {
    argv_array->elements.push_back(std::string_view(s));
  }

  static const std::string kArgv = "argv";
  static const std::string kArgc = "argc";
  static const std::string kArgvDz = "ནང་འཇུག་ཐོ་";
  static const std::string kArgcDz = "ནང་འཇུག་གྲངས་";

  auto set_global = [&](std::string_view name, Value value) {
    auto it = globals_.find(name);
    if (it == globals_.end()) {
      globals_.emplace(name, std::move(value));
      globals_version_++;
    } else {
      it->second = std::move(value);
    }
  };

  set_global(std::string_view(kArgv), argv_array);
  set_global(std::string_view(kArgc), static_cast<int64_t>(argv_storage_.size()));
  set_global(std::string_view(kArgvDz), argv_array);
  set_global(std::string_view(kArgcDz), static_cast<int64_t>(argv_storage_.size()));
}

InterpretResult VM::interpret(std::shared_ptr<ObjFunction> function) {
  stack_top_ = stack_base_;
  frames_.clear();

  // Create initial frame for the script/function
  CallFrame frame;
  frame.function = function.get();
  frame.ip = function->chunk.code().data();
  frame.slots_offset = 0;

  // Push the function itself onto stack (slot 0)
  push(function);

  frames_.push_back(frame);
  frame_ = &frames_.back();

  return run();
}

void VM::push(Value value) {
  *stack_top_ = std::move(value);
  ++stack_top_;
}

Value VM::pop() {
  --stack_top_;
  return std::move(*stack_top_);
}

const Value &VM::peek(int distance) const {
  return *(stack_top_ - 1 - static_cast<size_t>(distance));
}

void VM::runtime_error(const char *format, ...) {
  va_list args;
  va_start(args, format);
  vfprintf(stderr, format, args);
  va_end(args);
  fputs("\n", stderr);

  for (int i = static_cast<int>(frames_.size()) - 1; i >= 0; i--) {
    CallFrame *frame = &frames_[static_cast<size_t>(i)];
    ObjFunction *function = frame->function;
    size_t instruction =
        static_cast<size_t>(frame->ip - function->chunk.code().data() - 1);
    int line = function->chunk.lines()[instruction];
    fprintf(stderr, "[line %d] in ", line);
    if (function->name.empty()) {
      fprintf(stderr, "script\n");
    } else {
      fprintf(stderr, "%s()\n", function->name.c_str());
    }
  }

  frames_.clear();
  stack_top_ = stack_base_;
}

std::string_view VM::store_string(std::string value) {
  input_storage_.push_back(std::move(value));
  return std::string_view(input_storage_.back());
}

InterpretResult VM::run() {
  const uint8_t *ip = frame_->ip;

#define READ_BYTE() (*ip++)
#define READ_CONSTANT() (frame_->function->chunk.constants()[READ_BYTE()])
#define READ_SHORT() (ip += 2, (uint16_t)((ip[-2] << 8) | ip[-1]))

#define DISPATCH()                                                             \
  do {                                                                         \
    instruction = READ_BYTE();                                                 \
    goto *dispatch_table[instruction];                                         \
  } while (false)

#define BINARY_OP(type, op)                                                    \
  do {                                                                         \
    auto *bptr = std::get_if<int64_t>(stack_top_ - 1);                         \
    auto *aptr = std::get_if<int64_t>(stack_top_ - 2);                         \
    if (!aptr || !bptr) {                                                      \
      frame_->ip = ip;                                                         \
      runtime_error("Operands must be numbers.");                              \
      return InterpretResult::RuntimeError;                                    \
    }                                                                          \
    int64_t b = *bptr;                                                         \
    int64_t a = *aptr;                                                         \
    stack_top_ -= 2;                                                           \
    *stack_top_ = Value(type(a op b));                                         \
    ++stack_top_;                                                              \
  } while (false)

    static void *dispatch_table[] = {
      &&OP_RETURN,     &&OP_CONSTANT,      &&OP_NIL,           &&OP_TRUE,
      &&OP_FALSE,      &&OP_POP,           &&OP_GET_LOCAL,     &&OP_SET_LOCAL,
      &&OP_GET_GLOBAL, &&OP_DEFINE_GLOBAL, &&OP_SET_GLOBAL,    &&OP_EQUAL,
      &&OP_GREATER,    &&OP_LESS,          &&OP_ADD,           &&OP_SUBTRACT,
      &&OP_MULTIPLY,   &&OP_DIVIDE,        &&OP_NOT,           &&OP_NEGATE,
      &&OP_PRINT,      &&OP_JUMP,          &&OP_JUMP_IF_FALSE, &&OP_LOOP,
      &&OP_CALL,       &&OP_BUILD_ARRAY,   &&OP_INDEX,         &&OP_INDEX_SET,
      &&OP_BUILD_STRUCT, &&OP_GET_FIELD,   &&OP_SET_FIELD,
        &&OP_LEN,        &&OP_PUSH,          &&OP_POP_ARRAY,
        &&OP_TYPEOF,     &&OP_KEYS,          &&OP_VALUES,        &&OP_CONTAINS,
        &&OP_INPUT,
  };

  uint8_t instruction;
  DISPATCH();

OP_RETURN: {
  Value result = pop();
  size_t return_slots_offset = frame_->slots_offset;
  frames_.pop_back();
  if (frames_.empty()) {
    stack_top_ = stack_base_;
    push(std::move(result));
    return InterpretResult::Ok;
  }

  // Update frame_ BEFORE accessing it
  frame_ = &frames_.back();

  stack_top_ = stack_base_ + return_slots_offset;
  push(std::move(result));

  ip = frame_->ip;
}
  DISPATCH();

OP_CONSTANT: { push(READ_CONSTANT()); }
  DISPATCH();

OP_NIL: { push(std::monostate{}); }
  DISPATCH();

OP_TRUE: { push(true); }
  DISPATCH();

OP_FALSE: { push(false); }
  DISPATCH();

OP_POP: { pop(); }
  DISPATCH();

OP_GET_LOCAL: {
  uint8_t slot = READ_BYTE();
  push(stack_[frame_->slots_offset + slot]);
}
  DISPATCH();

OP_SET_LOCAL: {
  uint8_t slot = READ_BYTE();
  stack_[frame_->slots_offset + slot] = peek(0);
}
  DISPATCH();

OP_GET_GLOBAL: {
  std::string_view name = std::get<std::string_view>(READ_CONSTANT());
  if (global_cache_.slot && global_cache_.version == globals_version_ &&
      global_cache_.name == name) {
    push(*global_cache_.slot);
  } else {
    auto it = globals_.find(name);
    if (it == globals_.end()) {
      frame_->ip = ip;
      runtime_error("Undefined variable '%.*s'.", static_cast<int>(name.length()),
                    name.data());
      return InterpretResult::RuntimeError;
    }
    global_cache_.name = name;
    global_cache_.slot = &it->second;
    global_cache_.version = globals_version_;
    push(it->second);
  }
}
  DISPATCH();

OP_DEFINE_GLOBAL: {
  std::string_view name = std::get<std::string_view>(READ_CONSTANT());
  Value value = pop();
  auto it = globals_.find(name);
  if (it == globals_.end()) {
    auto [new_it, inserted] = globals_.emplace(name, std::move(value));
    it = new_it;
    if (inserted) {
      globals_version_++;
    }
  } else {
    it->second = std::move(value);
  }
  global_cache_.name = name;
  global_cache_.slot = &it->second;
  global_cache_.version = globals_version_;
}
  DISPATCH();

OP_SET_GLOBAL: {
  std::string_view name = std::get<std::string_view>(READ_CONSTANT());
  if (global_cache_.slot && global_cache_.version == globals_version_ &&
      global_cache_.name == name) {
    *global_cache_.slot = peek(0);
  } else {
    auto it = globals_.find(name);
    if (it == globals_.end()) {
      frame_->ip = ip;
      runtime_error("Undefined variable '%.*s'.", static_cast<int>(name.length()),
                    name.data());
      return InterpretResult::RuntimeError;
    }
    it->second = peek(0);
    global_cache_.name = name;
    global_cache_.slot = &it->second;
    global_cache_.version = globals_version_;
  }
}
  DISPATCH();

OP_EQUAL: {
  Value b = pop();
  Value a = pop();
  push(Value(a == b));
}
  DISPATCH();

OP_GREATER: { BINARY_OP(bool, >); }
  DISPATCH();

OP_LESS: { BINARY_OP(bool, <); }
  DISPATCH();

OP_ADD: { BINARY_OP(int64_t, +); }
  DISPATCH();

OP_SUBTRACT: { BINARY_OP(int64_t, -); }
  DISPATCH();

OP_MULTIPLY: { BINARY_OP(int64_t, *); }
  DISPATCH();

OP_DIVIDE: {
  auto *bptr = std::get_if<int64_t>(stack_top_ - 1);
  auto *aptr = std::get_if<int64_t>(stack_top_ - 2);
  if (!aptr || !bptr) {
    frame_->ip = ip;
    runtime_error("Operands must be numbers.");
    return InterpretResult::RuntimeError;
  }
  int64_t b = *bptr;
  int64_t a = *aptr;
  if (b == 0) {
    frame_->ip = ip;
    runtime_error("Division by zero.");
    return InterpretResult::RuntimeError;
  }
  stack_top_ -= 2;
  *stack_top_ = Value(a / b);
  ++stack_top_;
}
  DISPATCH();

OP_NOT: {
  Value v = pop();
  if (std::holds_alternative<bool>(v)) {
    push(Value(!std::get<bool>(v)));
  } else if (std::holds_alternative<std::monostate>(v)) {
    push(true);
  } else {
    push(false);
  }
}
  DISPATCH();

OP_NEGATE: {
  auto *vptr = std::get_if<int64_t>(stack_top_ - 1);
  if (!vptr) {
    frame_->ip = ip;
    runtime_error("Operand must be a number.");
    return InterpretResult::RuntimeError;
  }
  int64_t v = *vptr;
  --stack_top_;
  *stack_top_ = Value(-v);
  ++stack_top_;
}
  DISPATCH();

OP_PRINT: {
  Value val = pop();
  if (std::holds_alternative<int64_t>(val)) {
    std::cout << unicode::to_tibetan_numeral(std::get<int64_t>(val)) << "\n";
  } else if (std::holds_alternative<bool>(val)) {
    std::cout << (std::get<bool>(val) ? "བདེན" : "རྫུན") << "\n";
  } else if (std::holds_alternative<std::string_view>(val)) {
    std::cout << std::get<std::string_view>(val) << "\n";
  } else if (std::holds_alternative<std::monostate>(val)) {
    std::cout << "nil\n";
  } else if (std::holds_alternative<std::shared_ptr<ObjArray>>(val)) {
    auto arr = std::get<std::shared_ptr<ObjArray>>(val);
    std::cout << "[array:" << arr->elements.size() << "]\n";
  } else if (std::holds_alternative<std::shared_ptr<ObjStruct>>(val)) {
    auto obj = std::get<std::shared_ptr<ObjStruct>>(val);
    std::cout << "{struct:" << obj->fields.size() << "}\n";
  }
}
  DISPATCH();

OP_JUMP: {
  uint16_t offset = READ_SHORT();
  ip += offset;
}
  DISPATCH();

OP_JUMP_IF_FALSE: {
  uint16_t offset = READ_SHORT();
  const Value &v = peek(0);
  bool is_false = false;
  if (std::holds_alternative<bool>(v))
    is_false = !std::get<bool>(v);
  else if (std::holds_alternative<std::monostate>(v))
    is_false = true;

  if (is_false)
    ip += offset;
}
  DISPATCH();

OP_LOOP: {
  uint16_t offset = READ_SHORT();
  ip -= offset;
}
  DISPATCH();

#include "vm_collections.cpp"
#include "vm_fields.cpp"
#include "vm_builtins.cpp"

OP_CALL: {
  uint8_t arg_count = READ_BYTE();
  const Value &callee = peek(arg_count);
  if (!std::holds_alternative<std::shared_ptr<ObjFunction>>(callee)) {
    frame_->ip = ip;
    runtime_error("Can only call functions.");
    return InterpretResult::RuntimeError;
  }

  std::shared_ptr<ObjFunction> function =
      std::get<std::shared_ptr<ObjFunction>>(callee);

  if (static_cast<int>(arg_count) != function->arity) {
    frame_->ip = ip;
    runtime_error("Expected %d arguments but got %d.", function->arity,
                  static_cast<int>(arg_count));
    return InterpretResult::RuntimeError;
  }

  if (frames_.size() == 64) {
    frame_->ip = ip;
    runtime_error("Stack overflow.");
    return InterpretResult::RuntimeError;
  }

  frame_->ip = ip;

  CallFrame next_frame;
  next_frame.function = function.get();
  next_frame.ip = function->chunk.code().data();
  next_frame.slots_offset = stack_size() - static_cast<size_t>(arg_count) - 1;

  frames_.push_back(next_frame);
  frame_ = &frames_.back();
  ip = frame_->ip;
}
  DISPATCH();

#undef BINARY_OP
#undef READ_BYTE
#undef READ_CONSTANT
#undef READ_SHORT
#undef DISPATCH
}

} // namespace druk
