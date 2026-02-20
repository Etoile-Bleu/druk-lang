#include "druk/gc/types/gc_array.h"

#include "druk/codegen/core/value.h"


namespace druk::gc
{

void GcArray::trace()
{
    for (auto& elem : elements) elem.markGcRefs();
}

}  // namespace druk::gc
