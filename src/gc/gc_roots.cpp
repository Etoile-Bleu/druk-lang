#include "druk/gc/gc_roots.h"

namespace druk::gc
{

void GcRootSet::addSource(TraceCallback cb)
{
    sources_.push_back(std::move(cb));
}

void GcRootSet::removeSource(const TraceCallback* cb)
{
    (void)cb;
}

void GcRootSet::traceAll() const
{
    for (const auto& src : sources_) src(nullptr);
}

}  // namespace druk::gc
