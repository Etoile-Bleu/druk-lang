#pragma once

#include "druk/ir/ir_instruction_base.h"

namespace druk::ir
{

class AllocaInst : public Instruction
{
   public:
    explicit AllocaInst(std::shared_ptr<Type> type);
    std::string           toString() const override;
    std::shared_ptr<Type> getType() const override;

    std::shared_ptr<Type> getAllocatedType() const
    {
        return allocatedType_;
    }

   private:
    std::shared_ptr<Type> allocatedType_;
};

class LoadInst : public Instruction
{
   public:
    explicit LoadInst(Value* ptr);
    std::string           toString() const override;
    std::shared_ptr<Type> getType() const override;
};

class StoreInst : public Instruction
{
   public:
    StoreInst(Value* val, Value* ptr);
    std::string           toString() const override;
    std::shared_ptr<Type> getType() const override;
};

}  // namespace druk::ir
