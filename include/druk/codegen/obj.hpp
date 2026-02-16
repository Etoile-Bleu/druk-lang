#pragma once

#include "druk/codegen/chunk.hpp"
#include "druk/codegen/value.hpp"
#include <string>
#include <vector>
#include <unordered_map>

namespace druk {

struct ObjFunction {
  Chunk chunk;
  std::string name;
  int arity = 0;
};

struct ObjArray {
  std::vector<Value> elements;
};

struct ObjStruct {
  std::unordered_map<std::string, Value> fields;
};

} // namespace druk
