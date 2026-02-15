#include "druk/common/allocator.hpp"

namespace druk {

// Implementation details if any needed in CPP.
// Currently ArenaAllocator is fully header-only/template based for `make`,
// but `allocate` could be moved here if we didn't want it inlined.
// For now, keeping logic in header for performance (inlining).

} // namespace druk
