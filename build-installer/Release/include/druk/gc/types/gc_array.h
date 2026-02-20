#pragma once
#include <vector>

#include "druk/gc/gc_object.h"


namespace druk::codegen
{
class Value;
}

namespace druk::gc
{

class GcHeap;

class GcArray final : public GcObject
{
   public:
    std::vector<druk::codegen::Value> elements;

    GcArray() : GcObject(GcType::Array) {}

    void trace() override;
};

}  // namespace druk::gc
