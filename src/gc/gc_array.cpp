#include "druk/gc/types/gc_array.h"

#include "druk/codegen/core/value.h"

namespace druk
{
namespace gc
{

GcArray::GcArray() : GcObject(GcType::Array) {}
GcArray::~GcArray() = default;

void GcArray::trace()
{
    for (auto& elem : elements) elem.markGcRefs();
}

}  // namespace gc
}  // namespace druk
