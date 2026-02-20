#pragma once
#include <cstddef>

namespace druk::gc
{

inline constexpr size_t kInitialThreshold = 64;
inline constexpr size_t kGrowthFactor     = 2;
inline constexpr size_t kMinThreshold     = 32;

}  // namespace druk::gc
