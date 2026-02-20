#pragma once
#include <string>
#include <unordered_map>

#include "druk/gc/gc_object.h"


namespace druk::codegen
{
class Value;
}

namespace druk::gc
{

class GcHeap;

class GcStruct final : public GcObject
{
   public:
    std::unordered_map<std::string, druk::codegen::Value> fields;

    GcStruct() : GcObject(GcType::Struct) {}

    void trace() override;
};

}  // namespace druk::gc
