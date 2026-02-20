#pragma once

#include "druk/ir/ir_instruction_base.h"

namespace druk::ir
{

class BasicBlock;
class Function;

class BranchInst : public Instruction
{
   public:
    explicit BranchInst(BasicBlock* dest);
    std::string           toString() const override;
    std::shared_ptr<Type> getType() const override;

    BasicBlock* getDest() const
    {
        return dest_;
    }

   private:
    BasicBlock* dest_;
};

class CondBranchInst : public Instruction
{
   public:
    CondBranchInst(Value* cond, BasicBlock* t, BasicBlock* f);
    std::string           toString() const override;
    std::shared_ptr<Type> getType() const override;

    BasicBlock* getTrueDest() const
    {
        return trueDest_;
    }
    BasicBlock* getFalseDest() const
    {
        return falseDest_;
    }

   private:
    BasicBlock* trueDest_;
    BasicBlock* falseDest_;
};

class RetInst : public Instruction
{
   public:
    explicit RetInst(Value* val = nullptr);
    std::string           toString() const override;
    std::shared_ptr<Type> getType() const override;
};

class CallInst : public Instruction
{
   public:
    CallInst(Function* func, const std::vector<Value*>& args);
    std::string           toString() const override;
    std::shared_ptr<Type> getType() const override;

    Function* getCallee() const
    {
        return func_;
    }

   private:
    Function* func_;
};

class DynamicCallInst : public Instruction
{
   public:
    DynamicCallInst(Value* callee, const std::vector<Value*>& args);
    std::string           toString() const override;
    std::shared_ptr<Type> getType() const override;

    Value* getCallee() const
    {
        return getOperand(0);
    }
};

class PrintInst : public Instruction
{
   public:
    explicit PrintInst(Value* val);
    std::string           toString() const override;
    std::shared_ptr<Type> getType() const override;
};

}  // namespace druk::ir
