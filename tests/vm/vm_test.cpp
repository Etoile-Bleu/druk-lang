#include "druk/codegen/chunk.hpp"
#include "druk/codegen/opcode.hpp"
#include "druk/codegen/value.hpp"
#include "druk/vm/vm.hpp"
#include <gtest/gtest.h>

namespace druk {

class VMTest : public ::testing::Test {
protected:
  VM vm;
  Chunk chunk;

  const Value &peek(int distance) const { return vm.peek(distance); }

  // Helper to interpret the test chunk
  InterpretResult interpret() {
    auto func = std::make_shared<ObjFunction>();
    func->chunk = chunk; // Copy content
    return vm.interpret(func);
  }
};

TEST_F(VMTest, BasicArithmetic) {
  // 1 + 2
  int c1 = chunk.add_constant(int64_t{1});
  int c2 = chunk.add_constant(int64_t{2});

  chunk.write(OpCode::Constant, 1);
  chunk.write(static_cast<uint8_t>(c1), 1);
  chunk.write(OpCode::Constant, 1);
  chunk.write(static_cast<uint8_t>(c2), 1);
  chunk.write(OpCode::Add, 1);
  chunk.write(OpCode::Return, 1);

  EXPECT_EQ(interpret(), InterpretResult::Ok);
}

TEST_F(VMTest, Negate) {
  int c1 = chunk.add_constant(int64_t{10});
  chunk.write(OpCode::Constant, 1);
  chunk.write(static_cast<uint8_t>(c1), 1);
  chunk.write(OpCode::Negate, 1);
  chunk.write(OpCode::Return, 1);

  EXPECT_EQ(interpret(), InterpretResult::Ok);
}

TEST_F(VMTest, Comparison) {
  // 1 < 2 -> true
  int c1 = chunk.add_constant(int64_t{1});
  int c2 = chunk.add_constant(int64_t{2});

  chunk.write(OpCode::Constant, 1);
  chunk.write(static_cast<uint8_t>(c1), 1);
  chunk.write(OpCode::Constant, 1);
  chunk.write(static_cast<uint8_t>(c2), 1);
  chunk.write(OpCode::Less, 1);

  chunk.write(OpCode::Return, 1);

  EXPECT_EQ(interpret(), InterpretResult::Ok);
  Value res = peek(0);
  EXPECT_TRUE(std::holds_alternative<bool>(res));
  EXPECT_TRUE(std::get<bool>(res));
}

TEST_F(VMTest, GlobalVariables) {
  int name_idx = chunk.add_constant(std::string_view("a"));
  int val_idx = chunk.add_constant(int64_t{10});

  chunk.write(OpCode::Constant, 1);
  chunk.write(static_cast<uint8_t>(val_idx), 1);
  chunk.write(OpCode::DefineGlobal, 1);
  chunk.write(static_cast<uint8_t>(name_idx), 1);

  chunk.write(OpCode::GetGlobal, 1);
  chunk.write(static_cast<uint8_t>(name_idx), 1);

  chunk.write(OpCode::Return, 1);

  EXPECT_EQ(interpret(), InterpretResult::Ok);

  Value res = peek(0);
  EXPECT_TRUE(std::holds_alternative<int64_t>(res));
  EXPECT_EQ(std::get<int64_t>(res), 10);
}

TEST_F(VMTest, SetGlobal) {
  int name_idx = chunk.add_constant(std::string_view("a"));
  int val1 = chunk.add_constant(int64_t{10});
  int val2 = chunk.add_constant(int64_t{20});

  chunk.write(OpCode::Constant, 1);
  chunk.write(static_cast<uint8_t>(val1), 1);
  chunk.write(OpCode::DefineGlobal, 1);
  chunk.write(static_cast<uint8_t>(name_idx), 1);

  chunk.write(OpCode::Constant, 1);
  chunk.write(static_cast<uint8_t>(val2), 1);
  chunk.write(OpCode::SetGlobal, 1);
  chunk.write(static_cast<uint8_t>(name_idx), 1);

  chunk.write(OpCode::Return, 1);

  EXPECT_EQ(interpret(), InterpretResult::Ok);

  Value res = peek(0);
  EXPECT_TRUE(std::holds_alternative<int64_t>(res));
  EXPECT_EQ(std::get<int64_t>(res), 20);
}

TEST_F(VMTest, LocalVariables) {
  // Push 1 (slot 1)
  int c1 = chunk.add_constant(int64_t{1});
  chunk.write(OpCode::Constant, 1);
  chunk.write(static_cast<uint8_t>(c1), 1);

  // Push 2 (slot 2)
  int c2 = chunk.add_constant(int64_t{2});
  chunk.write(OpCode::Constant, 1);
  chunk.write(static_cast<uint8_t>(c2), 1);

  // Get local 1 (a)
  chunk.write(OpCode::GetLocal, 1);
  chunk.write(1, 1);

  // Get local 2 (b)
  chunk.write(OpCode::GetLocal, 1);
  chunk.write(2, 1);

  // Add
  chunk.write(OpCode::Add, 1);

  // Result 3 is on stack
  chunk.write(OpCode::Return, 1);

  EXPECT_EQ(interpret(), InterpretResult::Ok);
  EXPECT_EQ(std::get<int64_t>(peek(0)), 3);
}

TEST_F(VMTest, SetLocal) {
  int c1 = chunk.add_constant(int64_t{1});
  chunk.write(OpCode::Constant, 1);
  chunk.write(static_cast<uint8_t>(c1), 1);

  // Set local 1 = 2
  int c2 = chunk.add_constant(int64_t{2});
  chunk.write(OpCode::Constant, 1);
  chunk.write(static_cast<uint8_t>(c2), 1);

  chunk.write(OpCode::SetLocal, 1);
  chunk.write(1, 1);

  chunk.write(OpCode::Return, 1);

  EXPECT_EQ(interpret(), InterpretResult::Ok);

  EXPECT_EQ(std::get<int64_t>(peek(0)), 2);
}

TEST_F(VMTest, FunctionCall) {
  auto add_func = std::make_shared<ObjFunction>();
  add_func->name = "add";
  add_func->arity = 2;

  // arg a is slot 1, arg b is slot 2
  add_func->chunk.write(OpCode::GetLocal, 1);
  add_func->chunk.write(1, 1);

  add_func->chunk.write(OpCode::GetLocal, 1);
  add_func->chunk.write(2, 1);

  add_func->chunk.write(OpCode::Add, 1);
  add_func->chunk.write(OpCode::Return, 1);

  int func_idx = chunk.add_constant(add_func);
  chunk.write(OpCode::Constant, 1);
  chunk.write(static_cast<uint8_t>(func_idx), 1);

  int c1 = chunk.add_constant(int64_t{1});
  chunk.write(OpCode::Constant, 1);
  chunk.write(static_cast<uint8_t>(c1), 1);

  int c2 = chunk.add_constant(int64_t{2});
  chunk.write(OpCode::Constant, 1);
  chunk.write(static_cast<uint8_t>(c2), 1);

  chunk.write(OpCode::Call, 1);
  chunk.write(2, 1);

  chunk.write(OpCode::Return, 1);

  EXPECT_EQ(interpret(), InterpretResult::Ok);

  Value res = peek(0);
  EXPECT_TRUE(std::holds_alternative<int64_t>(res));
  EXPECT_EQ(std::get<int64_t>(res), 3);
}

} // namespace druk
