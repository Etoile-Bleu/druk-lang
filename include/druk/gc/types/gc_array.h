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

    GcArray();
    ~GcArray() override;

    // Explicitly delete copy/move to avoid implicit instantiation of vector members
    GcArray(const GcArray&)            = delete;
    GcArray& operator=(const GcArray&) = delete;
    GcArray(GcArray&&)                 = delete;
    GcArray& operator=(GcArray&&)      = delete;

    void trace() override;
};

}  // namespace druk::gc
