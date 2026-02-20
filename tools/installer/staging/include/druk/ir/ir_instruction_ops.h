#pragma once

#include "druk/ir/ir_instruction_base.h"

namespace druk::ir
{

class BinaryInst : public Instruction
{
   public:
    BinaryInst(Opcode op, Value* l, Value* r);
    std::string           toString() const override;
    std::shared_ptr<Type> getType() const override;
};

class StringConcatInst : public Instruction
{
   public:
    StringConcatInst(Value* l, Value* r);
    std::string           toString() const override;
    std::shared_ptr<Type> getType() const override;
};

class ToStringInst : public Instruction
{
   public:
    ToStringInst(Value* val);
    std::string           toString() const override;
    std::shared_ptr<Type> getType() const override;
};

class UnwrapInst : public Instruction
{
   public:
    UnwrapInst(Value* val);
    std::string           toString() const override;
    std::shared_ptr<Type> getType() const override;
};

class UnaryInst : public Instruction
{
   public:
    UnaryInst(Opcode op, Value* val);
    std::string           toString() const override;
    std::shared_ptr<Type> getType() const override;
};

}  // namespace druk::ir
