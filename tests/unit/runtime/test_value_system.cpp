// test_value_system.cpp — druk::codegen::Value type safety and operations
#include <gtest/gtest.h>

#include "druk/codegen/core/value.h"


using namespace druk::codegen;

class ValueSystemTest : public ::testing::Test
{
};

// ─── Nil ──────────────────────────────────────────────────────────────────────
TEST_F(ValueSystemTest, DefaultIsNil)
{
    Value v;
    EXPECT_TRUE(v.isNil());
    EXPECT_FALSE(v.isInt());
    EXPECT_FALSE(v.isBool());
    EXPECT_EQ(v.type(), ValueType::Nil);
}

// ─── Int ──────────────────────────────────────────────────────────────────────
TEST_F(ValueSystemTest, IntValue)
{
    Value v(int64_t{42});
    EXPECT_TRUE(v.isInt());
    EXPECT_FALSE(v.isNil());
    EXPECT_EQ(v.asInt(), 42);
    EXPECT_EQ(v.type(), ValueType::Int);
}
TEST_F(ValueSystemTest, IntZero)
{
    Value v(int64_t{0});
    EXPECT_TRUE(v.isInt());
    EXPECT_EQ(v.asInt(), 0);
}
TEST_F(ValueSystemTest, NegativeInt)
{
    Value v(int64_t{-100});
    EXPECT_EQ(v.asInt(), -100);
}
TEST_F(ValueSystemTest, LargeInt)
{
    Value v(int64_t{9'999'999'999LL});
    EXPECT_EQ(v.asInt(), 9'999'999'999LL);
}

// ─── Bool ─────────────────────────────────────────────────────────────────────
TEST_F(ValueSystemTest, TrueValue)
{
    Value v(true);
    EXPECT_TRUE(v.isBool());
    EXPECT_TRUE(v.asBool());
    EXPECT_EQ(v.type(), ValueType::Bool);
}
TEST_F(ValueSystemTest, FalseValue)
{
    Value v(false);
    EXPECT_TRUE(v.isBool());
    EXPECT_FALSE(v.asBool());
}

// ─── Equality ─────────────────────────────────────────────────────────────────
TEST_F(ValueSystemTest, IntEquality)
{
    Value a(int64_t{5});
    Value b(int64_t{5});
    Value c(int64_t{6});
    EXPECT_EQ(a, b);
    EXPECT_NE(a, c);
}
TEST_F(ValueSystemTest, BoolEquality)
{
    Value t1(true), t2(true), f1(false);
    EXPECT_EQ(t1, t2);
    EXPECT_NE(t1, f1);
}
TEST_F(ValueSystemTest, NilEquality)
{
    Value a, b;
    EXPECT_EQ(a, b);
}
TEST_F(ValueSystemTest, MixedTypesNotEqual)
{
    Value i(int64_t{1});
    Value b(true);
    // Different types should not compare equal
    EXPECT_NE(i, b);
}

// ─── Type predicates are exclusive ───────────────────────────────────────────
TEST_F(ValueSystemTest, IntIsNotBool)
{
    Value v(int64_t{1});
    EXPECT_FALSE(v.isBool());
    EXPECT_FALSE(v.isString());
    EXPECT_FALSE(v.isArray());
    EXPECT_FALSE(v.isNil());
}
TEST_F(ValueSystemTest, BoolIsNotInt)
{
    Value v(true);
    EXPECT_FALSE(v.isInt());
    EXPECT_FALSE(v.isString());
    EXPECT_FALSE(v.isNil());
}
