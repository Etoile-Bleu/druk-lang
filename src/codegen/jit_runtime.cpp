#include "druk/codegen/jit_runtime.hpp"
#include "druk/codegen/obj.hpp"
#include "druk/codegen/value.hpp"
#include "druk/lexer/unicode.hpp"

#include <cstdint>
#include <cstring>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace {
// Global state for JIT runtime
std::vector<std::string> g_jit_args;
std::unordered_map<std::string, druk::Value> g_globals;
std::vector<std::string> g_string_storage;

bool jit_debug_enabled() {
  static int enabled = -1;
  if (enabled != -1) {
    return enabled != 0;
  }
  const char *env = std::getenv("DRUK_JIT_DEBUG");
  enabled = (env && *env) ? 1 : 0;
  return enabled != 0;
}

const char *value_type_name(uint8_t type) {
  switch (static_cast<druk::ValueType>(type)) {
    case druk::ValueType::Nil: return "nil";
    case druk::ValueType::Int: return "int";
    case druk::ValueType::Bool: return "bool";
    case druk::ValueType::String: return "string";
    case druk::ValueType::Function: return "function";
    case druk::ValueType::Array: return "array";
    case druk::ValueType::Struct: return "struct";
  }
  return "unknown";
}

void log_type_on_error(const char *fn_name, const PackedValue *val) {
  if (!jit_debug_enabled()) {
    return;
  }
  if (!val) {
    std::cerr << "[JIT DEBUG] " << fn_name << " got <null>\n";
    return;
  }
  std::cerr << "[JIT DEBUG] " << fn_name << " got type="
            << static_cast<int>(val->type) << " ("
            << value_type_name(val->type) << ")\n";
}

void log_type_pair_on_error(const char *fn_name,
                            const PackedValue *left,
                            const PackedValue *right) {
  if (!jit_debug_enabled()) {
    return;
  }
  std::cerr << "[JIT DEBUG] " << fn_name << " left=";
  if (left) {
    std::cerr << static_cast<int>(left->type) << " ("
              << value_type_name(left->type) << ")";
  } else {
    std::cerr << "<null>";
  }
  std::cerr << " right=";
  if (right) {
    std::cerr << static_cast<int>(right->type) << " ("
              << value_type_name(right->type) << ")";
  } else {
    std::cerr << "<null>";
  }
  std::cerr << "\n";
}

std::string_view store_string(std::string str) {
  g_string_storage.push_back(std::move(str));
  return g_string_storage.back();
}

using DrukFunctionPtr = DrukJitFunc;

void pack_value(const druk::Value &v, PackedValue *out) {
  out->type = static_cast<uint8_t>(v.type());
  out->extra = 0;
  
  switch (v.type()) {
    case druk::ValueType::Nil:
      out->data.i = 0;
      break;
    case druk::ValueType::Int:
      out->data.i = v.as_int();
      break;
    case druk::ValueType::Bool:
      out->data.i = v.as_bool() ? 1 : 0;
      break;
    case druk::ValueType::String: {
      auto sv = v.as_string();
      out->data.s = sv.data();
      out->extra = static_cast<int64_t>(sv.size());
      break;
    }
    case druk::ValueType::Function:
      out->data.ptr = new std::shared_ptr<druk::ObjFunction>(v.as_function());
      break;
    case druk::ValueType::Array:
      out->data.ptr = new std::shared_ptr<druk::ObjArray>(v.as_array());
      break;
    case druk::ValueType::Struct:
      out->data.ptr = new std::shared_ptr<druk::ObjStruct>(v.as_struct());
      break;
  }
}

druk::Value unpack_value(const PackedValue *p) {
  auto type = static_cast<druk::ValueType>(p->type);
  switch (type) {
    case druk::ValueType::Nil:
      return druk::Value();
    case druk::ValueType::Int:
      return druk::Value(p->data.i);
    case druk::ValueType::Bool:
      return druk::Value(p->data.i != 0);
    case druk::ValueType::String:
      return druk::Value(std::string_view(p->data.s, static_cast<size_t>(p->extra)));
    case druk::ValueType::Function: {
      auto ptr = static_cast<std::shared_ptr<druk::ObjFunction>*>(p->data.ptr);
      return druk::Value(*ptr);
    }
    case druk::ValueType::Array: {
      auto ptr = static_cast<std::shared_ptr<druk::ObjArray>*>(p->data.ptr);
      return druk::Value(*ptr);
    }
    case druk::ValueType::Struct: {
      auto ptr = static_cast<std::shared_ptr<druk::ObjStruct>*>(p->data.ptr);
      return druk::Value(*ptr);
    }
  }
  return druk::Value();
}

void cleanup_packed(PackedValue *p) {
  auto type = static_cast<druk::ValueType>(p->type);
  if (type == druk::ValueType::Function) {
    delete static_cast<std::shared_ptr<druk::ObjFunction>*>(p->data.ptr);
  } else if (type == druk::ValueType::Array) {
    delete static_cast<std::shared_ptr<druk::ObjArray>*>(p->data.ptr);
  } else if (type == druk::ValueType::Struct) {
    delete static_cast<std::shared_ptr<druk::ObjStruct>*>(p->data.ptr);
  }
}

struct CallFrame {
  std::vector<PackedValue> args;
};

std::vector<CallFrame> g_call_frames;
std::unordered_map<druk::ObjFunction *, DrukFunctionPtr> g_compiled_functions;
DrukJitCompileFn g_compile_handler = nullptr;

} // namespace

extern "C" {

// Argument setup
void druk_jit_set_args(const char **argv, int32_t argc) {
  g_jit_args.clear();
  if (!argv || argc <= 0) return;
  g_jit_args.reserve(static_cast<size_t>(argc));
  for (int32_t i = 0; i < argc; ++i) {
    g_jit_args.emplace_back(argv[i] ? argv[i] : "");
  }

  auto argv_array = std::make_shared<druk::ObjArray>();
  argv_array->elements.reserve(g_jit_args.size());
  for (const auto &arg : g_jit_args) {
    argv_array->elements.push_back(druk::Value(store_string(arg)));
  }

  static const std::string kArgv = "argv";
  static const std::string kArgc = "argc";
  static const std::string kArgvDz = "ནང་འཇུག་ཐོ";
  static const std::string kArgcDz = "ནང་འཇུག་གྲངས་";

  g_globals[kArgv] = druk::Value(argv_array);
  g_globals[kArgc] = druk::Value(static_cast<int64_t>(g_jit_args.size()));
  g_globals[kArgvDz] = druk::Value(argv_array);
  g_globals[kArgcDz] = druk::Value(static_cast<int64_t>(g_jit_args.size()));
}

// Value creation helpers - return as i64 pair (type, data)
void druk_jit_value_nil(PackedValue *out) {
  out->type = static_cast<uint8_t>(druk::ValueType::Nil);
  out->data.i = 0;
  out->extra = 0;
}

void druk_jit_value_bool(bool b, PackedValue *out) {
  out->type = static_cast<uint8_t>(druk::ValueType::Bool);
  out->data.i = b ? 1 : 0;
  out->extra = 0;
}

void druk_jit_value_int(int64_t i, PackedValue *out) {
  out->type = static_cast<uint8_t>(druk::ValueType::Int);
  out->data.i = i;
  out->extra = 0;
}

void druk_jit_value_function(druk::ObjFunction *fn, PackedValue *out) {
  if (!fn) {
    out->type = static_cast<uint8_t>(druk::ValueType::Nil);
    out->data.i = 0;
    out->extra = 0;
    return;
  }
  out->type = static_cast<uint8_t>(druk::ValueType::Function);
  out->data.ptr = new std::shared_ptr<druk::ObjFunction>(
      fn, [](druk::ObjFunction *) {});
  out->extra = 0;
}

// Arithmetic operations
void druk_jit_add(const PackedValue *a, const PackedValue *b, PackedValue *out) {
  druk::Value va = unpack_value(a);
  druk::Value vb = unpack_value(b);
  
  if (va.is_int() && vb.is_int()) {
    pack_value(druk::Value(va.as_int() + vb.as_int()), out);
  } else {
    druk_jit_value_nil(out); // nil on error
  }
}

void druk_jit_subtract(const PackedValue *a, const PackedValue *b, PackedValue *out) {
  druk::Value va = unpack_value(a);
  druk::Value vb = unpack_value(b);
  
  if (va.is_int() && vb.is_int()) {
    pack_value(druk::Value(va.as_int() - vb.as_int()), out);
  } else {
    druk_jit_value_nil(out);
  }
}

void druk_jit_multiply(const PackedValue *a, const PackedValue *b, PackedValue *out) {
  druk::Value va = unpack_value(a);
  druk::Value vb = unpack_value(b);
  
  if (va.is_int() && vb.is_int()) {
    pack_value(druk::Value(va.as_int() * vb.as_int()), out);
  } else {
    druk_jit_value_nil(out);
  }
}

void druk_jit_divide(const PackedValue *a, const PackedValue *b, PackedValue *out) {
  druk::Value va = unpack_value(a);
  druk::Value vb = unpack_value(b);
  
  if (va.is_int() && vb.is_int() && vb.as_int() != 0) {
    pack_value(druk::Value(va.as_int() / vb.as_int()), out);
  } else {
    druk_jit_value_nil(out);
  }
}

void druk_jit_negate(const PackedValue *a, PackedValue *out) {
  druk::Value va = unpack_value(a);
  if (va.is_int()) {
    pack_value(druk::Value(-va.as_int()), out);
  } else {
    druk_jit_value_nil(out);
  }
}

// Comparison operations
void druk_jit_equal(const PackedValue *a, const PackedValue *b, PackedValue *out) {
  druk::Value va = unpack_value(a);
  druk::Value vb = unpack_value(b);
  bool eq = (va == vb);
  if (jit_debug_enabled()) {
    std::cerr << "[JIT DEBUG] equal ";
    if (va.is_int() && vb.is_int()) {
      std::cerr << va.as_int() << " == " << vb.as_int();
    } else {
      std::cerr << static_cast<int>(va.type()) << " == " << static_cast<int>(vb.type());
    }
    std::cerr << " -> " << (eq ? "true" : "false") << "\n";
  }
  pack_value(druk::Value(eq), out);
}

void druk_jit_less(const PackedValue *a, const PackedValue *b, PackedValue *out) {
  druk::Value va = unpack_value(a);
  druk::Value vb = unpack_value(b);
  
  if (va.is_int() && vb.is_int()) {
    pack_value(druk::Value(va.as_int() < vb.as_int()), out);
  } else {
    pack_value(druk::Value(false), out);
  }
}

void druk_jit_greater(const PackedValue *a, const PackedValue *b, PackedValue *out) {
  druk::Value va = unpack_value(a);
  druk::Value vb = unpack_value(b);
  
  if (va.is_int() && vb.is_int()) {
    pack_value(druk::Value(va.as_int() > vb.as_int()), out);
  } else {
    pack_value(druk::Value(false), out);
  }
}

void druk_jit_not(const PackedValue *a, PackedValue *out) {
  druk::Value va = unpack_value(a);
  bool is_falsey = va.is_nil() || (va.is_bool() && !va.as_bool());
  if (jit_debug_enabled()) {
    std::cerr << "[JIT DEBUG] not: input=";
    if (va.is_bool()) {
      std::cerr << (va.as_bool() ? "true" : "false");
    } else if (va.is_nil()) {
      std::cerr << "nil";
    } else if (va.is_int()) {
      std::cerr << va.as_int();
    } else {
      std::cerr << "type=" << static_cast<int>(va.type());
    }
    std::cerr << " output=" << (is_falsey ? "true" : "false") << "\n";
  }
  pack_value(druk::Value(is_falsey), out);
}

// Globals
void druk_jit_get_global(const char *name, size_t name_len, PackedValue *out) {
  std::string_view key(name, name_len);
  auto it = g_globals.find(std::string(key));
  if (it != g_globals.end()) {
    pack_value(it->second, out);
  } else {
    std::cerr << "Runtime Error: Undefined variable '" << key << "'\n";
    druk_jit_value_nil(out);
  }
}

void druk_jit_define_global(const char *name, size_t name_len, const PackedValue *val) {
  std::string key(name, name_len);
  g_globals[key] = unpack_value(val);
}

void druk_jit_set_global(const char *name, size_t name_len, const PackedValue *val) {
  std::string key(name, name_len);
  auto it = g_globals.find(key);
  if (it != g_globals.end()) {
    it->second = unpack_value(val);
  } else {
    std::cerr << "Runtime Error: Undefined variable '" << key << "'\n";
  }
}

// Array operations
void druk_jit_build_array(const PackedValue *elements, int32_t count, PackedValue *out) {
  auto arr = std::make_shared<druk::ObjArray>();
  arr->elements.reserve(static_cast<size_t>(count));

  if (jit_debug_enabled()) {
    std::cerr << "[JIT DEBUG] build_array count=" << count;
    for (int32_t i = 0; i < count; ++i) {
      const auto &pv = elements[i];
      std::cerr << " elem" << i << "=" << static_cast<int>(pv.type)
                << "(" << value_type_name(pv.type) << ")";
      if (pv.type == static_cast<uint8_t>(druk::ValueType::Int)) {
        std::cerr << ":" << pv.data.i;
      }
    }
    std::cerr << "\n";
  }
  
  for (int32_t i = 0; i < count; ++i) {
    arr->elements.push_back(unpack_value(&elements[i]));
  }
  
  pack_value(druk::Value(arr), out);
}

void druk_jit_index(const PackedValue *arr_val, const PackedValue *idx_val, PackedValue *out) {
  druk::Value arr = unpack_value(arr_val);
  druk::Value idx = unpack_value(idx_val);
  
  if (arr.is_array() && idx.is_int()) {
    auto arr_ptr = arr.as_array();
    int64_t i = idx.as_int();
    if (i >= 0 && static_cast<size_t>(i) < arr_ptr->elements.size()) {
      const auto &elem = arr_ptr->elements[static_cast<size_t>(i)];
      pack_value(elem, out);
      if (jit_debug_enabled()) {
        std::cerr << "[JIT DEBUG] index idx=" << i << " value type="
                  << static_cast<int>(elem.type()) << " ("
                  << value_type_name(static_cast<uint8_t>(elem.type())) << ")";
        if (elem.is_int()) {
          std::cerr << " value=" << elem.as_int();
        }
        std::cerr << "\n";
      }
      return;
    }
  }
  log_type_pair_on_error("index", arr_val, idx_val);
  druk_jit_value_nil(out);
}

void druk_jit_index_set(PackedValue *arr_val, const PackedValue *idx_val, const PackedValue *val) {
  druk::Value arr = unpack_value(arr_val);
  druk::Value idx = unpack_value(idx_val);
  druk::Value value = unpack_value(val);
  
  if (arr.is_array() && idx.is_int()) {
    auto arr_ptr = arr.as_array();
    int64_t i = idx.as_int();
    if (i >= 0 && static_cast<size_t>(i) < arr_ptr->elements.size()) {
      if (jit_debug_enabled()) {
        std::cerr << "[JIT DEBUG] index_set idx=" << i;
        if (value.is_int()) {
          std::cerr << " val=" << value.as_int();
        }
        std::cerr << "\n";
      }
      arr_ptr->elements[static_cast<size_t>(i)] = value;
    }
  } else {
    log_type_pair_on_error("index_set", arr_val, idx_val);
  }
}

// Struct operations
void druk_jit_build_struct(const PackedValue *keys, const PackedValue *values, int32_t count, PackedValue *out) {
  auto s = std::make_shared<druk::ObjStruct>();
  
  for (int32_t i = 0; i < count; ++i) {
    druk::Value key_val = unpack_value(&keys[i]);
    if (!key_val.is_string()) {
      std::cerr << "Runtime Error: Struct field name must be a string.\n";
      continue;
    }
    std::string key(key_val.as_string());
    s->fields[key] = unpack_value(&values[i]);
  }
  
  pack_value(druk::Value(s), out);
}

void druk_jit_get_field(const PackedValue *struct_val, const char *field, size_t field_len, PackedValue *out) {
  druk::Value s = unpack_value(struct_val);
  
  if (s.is_struct()) {
    auto s_ptr = s.as_struct();
    std::string key(field, field_len);
    auto it = s_ptr->fields.find(key);
    if (it != s_ptr->fields.end()) {
      pack_value(it->second, out);
      return;
    }
  }
  
  druk_jit_value_nil(out);
}

void druk_jit_set_field(PackedValue *struct_val, const char *field, size_t field_len, const PackedValue *val) {
  druk::Value s = unpack_value(struct_val);
  
  if (s.is_struct()) {
    auto s_ptr = s.as_struct();
    std::string key(field, field_len);
    s_ptr->fields[key] = unpack_value(val);
  }
}

// Builtins
void druk_jit_len(const PackedValue *val, PackedValue *out) {
  druk::Value v = unpack_value(val);
  
  if (v.is_array()) {
    pack_value(druk::Value(static_cast<int64_t>(v.as_array()->elements.size())), out);
  } else if (v.is_struct()) {
    pack_value(druk::Value(static_cast<int64_t>(v.as_struct()->fields.size())), out);
  } else {
    log_type_on_error("len", val);
    std::cerr << "Runtime Error: len() requires array or struct.\n";
    druk_jit_value_nil(out);
  }
}

void druk_jit_push(PackedValue *arr_val, const PackedValue *element) {
  druk::Value arr = unpack_value(arr_val);
  
  if (arr.is_array()) {
    auto arr_ptr = arr.as_array();
    arr_ptr->elements.push_back(unpack_value(element));
  } else {
    log_type_on_error("push", arr_val);
    std::cerr << "Runtime Error: push() requires array as first argument.\n";
  }
}

void druk_jit_pop_array(PackedValue *arr_val, PackedValue *out) {
  druk::Value arr = unpack_value(arr_val);
  
  if (arr.is_array()) {
    auto arr_ptr = arr.as_array();
    if (!arr_ptr->elements.empty()) {
      pack_value(arr_ptr->elements.back(), out);
      arr_ptr->elements.pop_back();
      return;
    }
    std::cerr << "Runtime Error: Cannot pop from empty array.\n";
  } else {
    log_type_on_error("pop", arr_val);
    std::cerr << "Runtime Error: pop() requires array.\n";
  }
  
  druk_jit_value_nil(out);
}

void druk_jit_typeof(const PackedValue *val, PackedValue *out) {
  druk::Value v = unpack_value(val);
  std::string_view type_name;
  
  switch (v.type()) {
    case druk::ValueType::Nil: type_name = "nil"; break;
    case druk::ValueType::Int: type_name = "int"; break;
    case druk::ValueType::Bool: type_name = "bool"; break;
    case druk::ValueType::String: type_name = "string"; break;
    case druk::ValueType::Function: type_name = "function"; break;
    case druk::ValueType::Array: type_name = "array"; break;
    case druk::ValueType::Struct: type_name = "struct"; break;
  }
  
  pack_value(druk::Value(store_string(std::string(type_name))), out);
}

void druk_jit_keys(const PackedValue *val, PackedValue *out) {
  druk::Value v = unpack_value(val);
  
  if (v.is_struct()) {
    auto s = v.as_struct();
    auto arr = std::make_shared<druk::ObjArray>();
    arr->elements.reserve(s->fields.size());
    
    for (const auto &[key, _] : s->fields) {
      arr->elements.push_back(druk::Value(store_string(key)));
    }
    
    pack_value(druk::Value(arr), out);
  } else {
    log_type_on_error("keys", val);
    std::cerr << "Runtime Error: keys() requires a struct.\n";
    druk_jit_value_nil(out);
  }
}

void druk_jit_values(const PackedValue *val, PackedValue *out) {
  druk::Value v = unpack_value(val);
  
  if (v.is_struct()) {
    auto s = v.as_struct();
    auto arr = std::make_shared<druk::ObjArray>();
    arr->elements.reserve(s->fields.size());
    
    for (const auto &[_, value] : s->fields) {
      arr->elements.push_back(value);
    }
    
    pack_value(druk::Value(arr), out);
  } else {
    log_type_on_error("values", val);
    std::cerr << "Runtime Error: values() requires a struct.\n";
    druk_jit_value_nil(out);
  }
}

void druk_jit_contains(const PackedValue *container, const PackedValue *item, PackedValue *out) {
  druk::Value cont = unpack_value(container);
  druk::Value it = unpack_value(item);
  
  if (cont.is_array()) {
    auto arr = cont.as_array();
    for (const auto &elem : arr->elements) {
      if (elem == it) {
        pack_value(druk::Value(true), out);
        return;
      }
    }
    pack_value(druk::Value(false), out);
  } else if (cont.is_struct()) {
    if (it.is_string()) {
      auto s = cont.as_struct();
      std::string key(it.as_string());
      pack_value(druk::Value(s->fields.find(key) != s->fields.end()), out);
    } else {
      pack_value(druk::Value(false), out);
    }
  } else {
    log_type_on_error("contains", container);
    std::cerr << "Runtime Error: contains() requires array or struct.\n";
    druk_jit_value_nil(out);
  }
}

void druk_jit_input(PackedValue *out) {
  std::string line;
  if (std::getline(std::cin, line)) {
    pack_value(druk::Value(store_string(std::move(line))), out);
  } else {
    pack_value(druk::Value(store_string("")), out);
  }
}

// Print
void druk_jit_print(const PackedValue *val) {
  druk::Value v = unpack_value(val);
  if (jit_debug_enabled()) {
    std::cerr << "[JIT DEBUG] print type=" << static_cast<int>(v.type()) << "\n";
  }
  
  switch (v.type()) {
    case druk::ValueType::Nil:
      std::cout << "nil\n";
      break;
    case druk::ValueType::Int:
      std::cout << druk::unicode::to_tibetan_numeral(v.as_int()) << "\n";
      break;
    case druk::ValueType::Bool:
      std::cout << (v.as_bool() ? "བདེན" : "རྫུན") << "\n";
      break;
    case druk::ValueType::String:
      std::cout << v.as_string() << "\n";
      break;
    case druk::ValueType::Function:
      std::cout << "<function>\n";
      break;
    case druk::ValueType::Array:
      std::cout << "[array:" << v.as_array()->elements.size() << "]\n";
      break;
    case druk::ValueType::Struct:
      std::cout << "{struct:" << v.as_struct()->fields.size() << "}\n";
      break;
  }
}

void druk_jit_register_function(druk::ObjFunction *function, DrukFunctionPtr fn) {
  if (!function || !fn) {
    return;
  }
  g_compiled_functions[function] = fn;
}

void druk_jit_set_compile_handler(DrukJitCompileFn fn) {
  g_compile_handler = fn;
}

void druk_jit_call(const PackedValue *callee,
                   const PackedValue *args,
                   int32_t arg_count,
                   PackedValue *out) {
  if (!callee || !out) {
    return;
  }

  druk::Value callee_val = unpack_value(callee);
  if (!callee_val.is_function()) {
    std::cerr << "Runtime Error: Can only call functions.\n";
    druk_jit_value_nil(out);
    return;
  }

  auto function = callee_val.as_function();
  if (!function) {
    druk_jit_value_nil(out);
    return;
  }

  if (arg_count != function->arity) {
    std::cerr << "Runtime Error: Expected " << function->arity
              << " arguments but got " << arg_count << ".\n";
    druk_jit_value_nil(out);
    return;
  }

  auto it = g_compiled_functions.find(function.get());
  if (it == g_compiled_functions.end()) {
    if (g_compile_handler) {
      DrukFunctionPtr compiled = g_compile_handler(function.get());
      if (compiled) {
        g_compiled_functions[function.get()] = compiled;
      }
    }
    it = g_compiled_functions.find(function.get());
  }
  if (it == g_compiled_functions.end()) {
    std::cerr << "Runtime Error: JIT function not compiled.\n";
    druk_jit_value_nil(out);
    return;
  }

  CallFrame frame;
  frame.args.reserve(static_cast<size_t>(arg_count + 1));
  frame.args.push_back(*callee);
  for (int32_t i = 0; i < arg_count; ++i) {
    frame.args.push_back(args[i]);
  }

  if (jit_debug_enabled()) {
    std::cerr << "[JIT DEBUG] call argc=" << arg_count;
    for (int32_t i = 0; i <= arg_count; ++i) {
      const auto &pv = frame.args[static_cast<size_t>(i)];
      std::cerr << " arg" << i << "=" << static_cast<int>(pv.type)
                << "(" << value_type_name(pv.type) << ")";
    }
    std::cerr << "\n";
  }

  g_call_frames.push_back(std::move(frame));
  it->second(out);
  if (jit_debug_enabled()) {
    std::cerr << "[JIT DEBUG] call return type=" << static_cast<int>(out->type)
              << " (" << value_type_name(out->type) << ")\n";
  }
  g_call_frames.pop_back();
}

void druk_jit_get_arg(int32_t index, PackedValue *out) {
  if (!out) {
    return;
  }

  if (g_call_frames.empty()) {
    druk_jit_value_nil(out);
    return;
  }

  const auto &frame = g_call_frames.back();
  if (index < 0 || static_cast<size_t>(index) >= frame.args.size()) {
    druk_jit_value_nil(out);
    return;
  }

  *out = frame.args[static_cast<size_t>(index)];

  if (jit_debug_enabled()) {
    std::cerr << "[JIT DEBUG] get_arg " << index << " -> "
              << static_cast<int>(out->type) << " ("
              << value_type_name(out->type) << ")\n";
  }
}

int64_t druk_jit_value_as_int(const PackedValue *value) {
  if (!value) {
    return 0;
  }
  if (value->type == static_cast<uint8_t>(druk::ValueType::Int)) {
    return value->data.i;
  }
  return 0;
}

} // extern "C"
