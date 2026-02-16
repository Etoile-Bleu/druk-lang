#include "druk/codegen/chunk.hpp"
#include <gtest/gtest.h>

using namespace druk;

TEST(ChunkTest, WritesByte) {
  Chunk chunk;
  chunk.write(static_cast<uint8_t>(OpCode::Return), 123);

  EXPECT_EQ(chunk.code().size(), 1);
  EXPECT_EQ(static_cast<OpCode>(chunk.code()[0]), OpCode::Return);
  EXPECT_EQ(chunk.lines()[0], 123);
}

TEST(ChunkTest, AddsConstant) {
  Chunk chunk;
  chunk.add_constant(Value(int64_t{123}));
  int index = chunk.add_constant(Value(true));

  EXPECT_EQ(chunk.constants().size(), 2);
  EXPECT_EQ(index, 1);

  // Check values
  EXPECT_TRUE(chunk.constants()[0].is_int());
  // Verify deduplication
  int index2 = chunk.add_constant(Value(int64_t{123}));
  EXPECT_EQ(index2, 0);                   // Should reuse index 0
  EXPECT_EQ(chunk.constants().size(), 2); // Still 2 constants
  // Wait, Value is int64_t based on my definition.
  // Let's check value.hpp definition again.
}
