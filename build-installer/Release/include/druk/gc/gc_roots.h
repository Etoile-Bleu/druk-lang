#pragma once
#include <functional>
#include <vector>

namespace druk::gc
{

class GcObject;

using TraceCallback = std::function<void(GcObject*)>;

class GcRootSet
{
   public:
    void addSource(TraceCallback cb);
    void removeSource(const TraceCallback* cb);
    void traceAll() const;

   private:
    std::vector<TraceCallback> sources_;
};

}  // namespace druk::gc
