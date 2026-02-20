#pragma once
#include <cstddef>
#include <utility>

#include "druk/gc/gc_config.h"
#include "druk/gc/gc_object.h"
#include "druk/gc/gc_roots.h"


namespace druk::gc
{

class GcHeap
{
   public:
    static GcHeap& get();

    template <typename T, typename... Args>
    T* alloc(Args&&... args)
    {
        maybeCollect();
        auto* obj = new T(std::forward<Args>(args)...);
        obj->next = head_;
        head_     = obj;
        ++count_;
        return obj;
    }

    void       collect();
    void       markObject(GcObject* obj);
    GcRootSet& roots();
    size_t     objectCount() const;

   private:
    GcHeap() = default;
    void maybeCollect();
    void markPhase();
    void sweepPhase();

    GcObject* head_      = nullptr;
    size_t    count_     = 0;
    size_t    threshold_ = kInitialThreshold;
    GcRootSet roots_;
};

}  // namespace druk::gc
