#pragma once

#include <string>

#include "druk/codegen/core/chunk.h"
#include "druk/gc/gc_object.h"


namespace druk::codegen
{

struct ObjFunction : public gc::GcObject
{
    Chunk       chunk;
    std::string name;
    int         arity = 0;

    ObjFunction() : gc::GcObject(gc::GcType::Function) {}
    ~ObjFunction() override = default;

    void trace() override;
};

}  // namespace druk::codegen
