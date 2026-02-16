// C API wrapper for embedding Druk in standalone executables
#include "druk/codegen/chunk.hpp"
#include "druk/vm/vm.hpp"
#include <memory>
#include <cstring>
#include <vector>
#include <string>

extern "C" {

void* druk_chunk_create() {
  return new druk::Chunk();
}

void druk_chunk_destroy(void* chunk) {
  delete static_cast<druk::Chunk*>(chunk);
}

void druk_chunk_write(void* chunk, uint8_t byte, uint32_t line) {
  static_cast<druk::Chunk*>(chunk)->write(byte, line);
}

void* druk_vm_create() {
  return new druk::VM();
}

void druk_vm_destroy(void* vm) {
  delete static_cast<druk::VM*>(vm);
}

void druk_vm_set_args(void* vm, int argc, const char** argv) {
  auto vm_ptr = static_cast<druk::VM*>(vm);
  std::vector<std::string> args;
  args.reserve(static_cast<size_t>(argc));
  for (int i = 0; i < argc; ++i) {
    args.emplace_back(argv[i] ? argv[i] : "");
  }
  vm_ptr->set_args(args);
}

void* druk_function_create() {
  auto func = std::make_shared<druk::ObjFunction>();
  return new std::shared_ptr<druk::ObjFunction>(func);
}

void druk_function_set_chunk(void* func, void* chunk) {
  auto func_ptr = static_cast<std::shared_ptr<druk::ObjFunction>*>(func);
  auto chunk_ptr = static_cast<druk::Chunk*>(chunk);
  (*func_ptr)->chunk = *chunk_ptr; // Copy, don't move
}

void druk_function_set_name(void* func, const char* name) {
  auto func_ptr = static_cast<std::shared_ptr<druk::ObjFunction>*>(func);
  (*func_ptr)->name = name;
}

int druk_vm_interpret(void* vm, void* func) {
  try {
    auto vm_ptr = static_cast<druk::VM*>(vm);
    auto func_ptr = static_cast<std::shared_ptr<druk::ObjFunction>*>(func);
    auto result = vm_ptr->interpret(*func_ptr);
    return static_cast<int>(result);
  } catch (const std::exception& e) {
    fprintf(stderr, "Exception during interpret: %s\n", e.what());
    return 1;
  } catch (...) {
    fprintf(stderr, "Unknown exception during interpret\n");
    return 1;
  }
}

void druk_function_destroy(void* func) {
  delete static_cast<std::shared_ptr<druk::ObjFunction>*>(func);
}

// Sérialisation/désérialisation du chunk
size_t druk_chunk_serialize_size(void* chunk) {
  auto chunk_ptr = static_cast<druk::Chunk*>(chunk);
  const auto& code = chunk_ptr->code();
  const auto& constants = chunk_ptr->constants();
  const auto& lines = chunk_ptr->lines();
  
  size_t size = 0;
  // Header: magic(4) + code_size(4) + constants_size(4) + lines_size(4)
  size += 16;
  // Code
  size += code.size();
  // Lines
  size += lines.size() * sizeof(int);
  // Constants: pour chaque constant, type(1) + data
  for (const auto& constant : constants) {
    size += 1; // type byte
    if (constant.is_string()) {
      auto str = constant.as_string();
      size += 4 + str.size(); // size + data
    } else if (constant.is_int()) {
      size += 8;
    } else if (constant.is_bool()) {
      size += 1;
    } else if (constant.is_function()) {
      // Function objects - we'll skip these for now
      size += 0;
    } else {
      size += 1; // nil
    }
  }
  return size;
}

void druk_chunk_serialize(void* chunk, uint8_t* buffer) {
  auto chunk_ptr = static_cast<druk::Chunk*>(chunk);
  const auto& code = chunk_ptr->code();
  const auto& constants = chunk_ptr->constants();
  const auto& lines = chunk_ptr->lines();
  
  uint8_t* ptr = buffer;
  
  // Magic
  memcpy(ptr, "CHNK", 4);
  ptr += 4;
  
  // Sizes
  uint32_t code_size = (uint32_t)code.size();
  uint32_t constants_size = (uint32_t)constants.size();
  uint32_t lines_size = (uint32_t)lines.size();
  
  memcpy(ptr, &code_size, 4); ptr += 4;
  memcpy(ptr, &constants_size, 4); ptr += 4;
  memcpy(ptr, &lines_size, 4); ptr += 4;
  
  // Code
  memcpy(ptr, code.data(), code.size());
  ptr += code.size();
  
  // Lines
  memcpy(ptr, lines.data(), lines.size() * sizeof(int));
  ptr += lines.size() * sizeof(int);
  
  // Constants
  for (const auto& constant : constants) {
    if (constant.is_string()) {
      *ptr++ = 1; // string type
      auto str = constant.as_string();
      uint32_t str_size = (uint32_t)str.size();
      memcpy(ptr, &str_size, 4); ptr += 4;
      memcpy(ptr, str.data(), str.size()); ptr += str.size();
    } else if (constant.is_int()) {
      *ptr++ = 2; // int type
      int64_t val = constant.as_int();
      memcpy(ptr, &val, 8); ptr += 8;
    } else if (constant.is_bool()) {
      *ptr++ = 3; // bool type
      *ptr++ = constant.as_bool() ? 1 : 0;
    } else if (constant.is_function()) {
      // Skip function objects for now
      *ptr++ = 4; // function type (not handled yet)
    } else {
      *ptr++ = 0; // nil type
    }
  }
}

void druk_chunk_deserialize(void* chunk, const uint8_t* buffer, size_t /*size*/) {
  auto chunk_ptr = static_cast<druk::Chunk*>(chunk);
  const uint8_t* ptr = buffer;
  
  // Check magic
  if (memcmp(ptr, "CHNK", 4) != 0) {
    fprintf(stderr, "Invalid chunk magic\n");
    return;
  }
  ptr += 4;
  
  // Read sizes
  uint32_t code_size, constants_size, lines_size;
  memcpy(&code_size, ptr, 4); ptr += 4;
  memcpy(&constants_size, ptr, 4); ptr += 4;
  memcpy(&lines_size, ptr, 4); ptr += 4;
  
  // Read code
  for (uint32_t i = 0; i < code_size; i++) {
    chunk_ptr->write(*ptr++, 0); // line will be updated later
  }
  
  // Read lines
  auto& lines = const_cast<std::vector<int>&>(chunk_ptr->lines());
  lines.resize(lines_size);
  memcpy(lines.data(), ptr, lines_size * sizeof(int));
  ptr += lines_size * sizeof(int);
  
  // Read constants
  for (uint32_t i = 0; i < constants_size; i++) {
    uint8_t type = *ptr++;
    if (type == 1) { // string
      uint32_t str_size;
      memcpy(&str_size, ptr, 4); ptr += 4;
      // Store the string in the chunk's storage
      chunk_ptr->string_storage().emplace_back((const char*)ptr, str_size);
      ptr += str_size;
      // Add a string_view pointing to the stored string
      chunk_ptr->add_constant(druk::Value(std::string_view(chunk_ptr->string_storage().back())));
    } else if (type == 2) { // int
      int64_t val;
      memcpy(&val, ptr, 8); ptr += 8;
      chunk_ptr->add_constant(druk::Value(val));
    } else if (type == 3) { // bool
      bool val = (*ptr++) != 0;
      chunk_ptr->add_constant(druk::Value(val));
    } else if (type == 4) { // function (skip)
      // Skip function constants
    } else { // nil
      chunk_ptr->add_constant(druk::Value());
    }
  }
}

} // extern "C"
