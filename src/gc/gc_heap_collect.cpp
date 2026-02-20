#include <iostream>

#include "druk/gc/gc_heap.h"


namespace druk::gc
{

void GcHeap::markObject(GcObject* obj)
{
    if (!obj || obj->marked)
        return;
    obj->marked = true;
    obj->trace();
}

void GcHeap::markPhase()
{
    roots_.traceAll();
}

void GcHeap::sweepPhase()
{
    GcObject** cursor = &head_;
    size_t     alive  = 0;
    while (*cursor)
    {
        if ((*cursor)->marked)
        {
            (*cursor)->marked = false;
            cursor            = &(*cursor)->next;
            ++alive;
        }
        else
        {
            GcObject* unreachable = *cursor;
            *cursor               = unreachable->next;
            delete unreachable;
        }
    }
    count_ = alive;
}

void GcHeap::collect()
{
    size_t before = count_;
    markPhase();
    sweepPhase();
    size_t freed = before - count_;

    std::cout << "[GC] Collected " << freed << " objects. " << count_ << " remaining. "
              << "New threshold: " << threshold_ << std::endl;

    threshold_ = (count_ < kMinThreshold) ? kInitialThreshold : count_ * kGrowthFactor;
}

}  // namespace druk::gc
