#pragma once
#include <string>

#include "druk/gc/gc_object.h"

namespace druk::gc
{

class GcString final : public GcObject
{
   public:
    std::string data;

    explicit GcString(std::string s) : GcObject(GcType::String), data(std::move(s)) {}

    void trace() override;
};

}  // namespace druk::gc
