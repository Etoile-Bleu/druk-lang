#pragma once

#include "druk/ir/ir_instruction_base.h"

namespace druk::ir
{

class BuildArrayInst : public Instruction
{
   public:
    explicit BuildArrayInst(const std::vector<Value*>& elements);
    std::string           toString() const override;
    std::shared_ptr<Type> getType() const override;
};

class IndexGetInst : public Instruction
{
   public:
    IndexGetInst(Value* array_val, Value* index_val);
    std::string           toString() const override;
    std::shared_ptr<Type> getType() const override;
};

class IndexSetInst : public Instruction
{
   public:
    IndexSetInst(Value* array_val, Value* index_val, Value* value);
    std::string           toString() const override;
    std::shared_ptr<Type> getType() const override;
};

class LenInst : public Instruction
{
   public:
    explicit LenInst(Value* value);
    std::string           toString() const override;
    std::shared_ptr<Type> getType() const override;
};

}  // namespace druk::ir
