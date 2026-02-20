#include "druk/gc/gc_heap.h"

namespace druk::gc
{

GcHeap& GcHeap::get()
{
    static GcHeap instance;
    return instance;
}

GcRootSet& GcHeap::roots()
{
    return roots_;
}

size_t GcHeap::objectCount() const
{
    return count_;
}

void GcHeap::maybeCollect()
{
    if (count_ >= threshold_)
        collect();
}

}  // namespace druk::gc
