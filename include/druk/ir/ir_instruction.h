#pragma once

#include <vector>

// #include "druk/ir/ir_basic_block.h"
// #include "druk/ir/ir_function.h"
#include "druk/ir/ir_type.h"
#include "druk/ir/ir_value.h"

namespace druk::ir
{

class BasicBlock;

enum class Opcode
{
    // Arithmetic
    Add,
    Sub,
    Mul,
    Div,
    Mod,

    // Comparison
    Equal,
    NotEqual,
    LessThan,
    LessEqual,
    GreaterThan,
    GreaterEqual,

    // Logical
    And,
    Or,
    Not,

    // Memory
    Load,
    Store,
    Alloca,

    // Control flow
    Branch,
    ConditionalBranch,
    Return,
    Call,
    Print,

    // Type conversion
    IntToFloat,
    FloatToInt,
    Bitcast
};

/**
 * @brief Base class for all IR instructions.
 */
class Instruction : public Value
{
   public:
    virtual ~Instruction() = default;

    Opcode getOpcode() const
    {
        return opcode_;
    }

    const std::vector<Value*>& getOperands() const
    {
        return operands_;
    }
    void addOperand(Value* operand)
    {
        operands_.push_back(operand);
    }

    BasicBlock* getParent() const
    {
        return parent_;
    }
    void setParent(BasicBlock* block)
    {
        parent_ = block;
    }

   protected:
    explicit Instruction(Opcode opcode) : opcode_(opcode), parent_(nullptr) {}

   private:
    Opcode              opcode_;
    std::vector<Value*> operands_;
    BasicBlock*         parent_;
};

class BinaryInst : public Instruction
{
   public:
    BinaryInst(Opcode op, Value* l, Value* r);
    std::string           toString() const override;
    std::shared_ptr<Type> getType() const override;
};

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

class Function;

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

class PrintInst : public Instruction
{
   public:
    explicit PrintInst(Value* val);
    std::string           toString() const override;
    std::shared_ptr<Type> getType() const override;
};

}  // namespace druk::ir
