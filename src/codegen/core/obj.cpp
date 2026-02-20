#include "druk/codegen/core/obj.h"

#include "druk/gc/gc_heap.h"


namespace druk::codegen
{

void ObjFunction::trace()
{
    for (const auto& constant : chunk.constants())
    {
        constant.markGcRefs();
    }
}

}  // namespace druk::codegen
