#include "druk/lexer/unicode.hpp"
#include <gtest/gtest.h>


TEST(InternerTest, Deduplication) {
  druk::ArenaAllocator arena;
  druk::StringInterner interner(arena);

  std::string_view s1 = interner.intern("foo");
  std::string_view s2 = interner.intern("foo");
  std::string_view s3 = interner.intern("bar");

  EXPECT_EQ(s1.data(), s2.data()); // Same address
  EXPECT_NE(s1.data(), s3.data()); // Different address
  EXPECT_EQ(s1, "foo");
}

TEST(UnicodeTest, Validation) {
  EXPECT_TRUE(druk::unicode::is_valid_utf8("hello"));
  EXPECT_TRUE(
      druk::unicode::is_valid_utf8("བཀྲ་ཤིས་བདེ་ལེགས།")); // Dzongkha/Tibetan

  // Invalid sequence (0xFF is never valid)
  char invalid[] = {(char)0xFF, 0};
  EXPECT_FALSE(druk::unicode::is_valid_utf8(invalid));
}
