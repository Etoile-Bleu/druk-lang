#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "druk/codegen/core/chunk.h"
#include "druk/codegen/core/value.h"


namespace druk::codegen
{

struct ObjFunction
{
    Chunk       chunk;
    std::string name;
    int         arity = 0;
};

struct ObjArray
{
    std::vector<Value> elements;
};

struct ObjStruct
{
    std::unordered_map<std::string, Value> fields;
};

}  // namespace druk::codegen
