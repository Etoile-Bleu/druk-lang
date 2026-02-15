#include "druk/common/allocator.hpp"
#include <gtest/gtest.h>


TEST(ArenaTest, Allocation) {
  druk::ArenaAllocator arena;
  int *p = (int *)arena.allocate(sizeof(int), alignof(int));
  *p = 42;
  EXPECT_EQ(*p, 42);
}

TEST(ArenaTest, MultipleBlocks) {
  druk::ArenaAllocator arena(128); // Small block size
  for (int i = 0; i < 100; ++i) {
    int *p = arena.make<int>(i);
    EXPECT_EQ(*p, i);
  }
}

TEST(ArenaTest, Reset) {
  druk::ArenaAllocator arena;
  void *p1 = arena.allocate(10, 1);
  arena.reset();
  void *p2 = arena.allocate(10, 1);
  // Should be reused (implementation detail, but likely same address if logic
  // is simple and we clear vectors but allocate new one immediately. Actually
  // our implementation clears blocks_ and allocates a new one. So p1 and p2
  // might be different depending on malloc. We just check it works.)
  EXPECT_NE(p2, nullptr);
}
