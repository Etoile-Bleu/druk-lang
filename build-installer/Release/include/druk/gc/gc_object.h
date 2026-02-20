#pragma once
#include <cstdint>

namespace druk::gc
{

enum class GcType : uint8_t
{
    Array,
    String,
    Struct,
    Function,
};

class GcObject
{
   public:
    GcObject* next   = nullptr;
    bool      marked = false;
    GcType    kind;

    explicit GcObject(GcType t) : kind(t) {}
    virtual ~GcObject() = default;

    virtual void trace() = 0;
};

}  // namespace druk::gc
