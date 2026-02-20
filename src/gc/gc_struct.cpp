#include "druk/gc/types/gc_struct.h"

#include "druk/codegen/core/value.h"


namespace druk::gc
{

void GcStruct::trace()
{
    for (auto& [key, val] : fields) val.markGcRefs();
}

}  // namespace druk::gc
