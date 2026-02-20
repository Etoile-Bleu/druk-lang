#pragma once

#include <vector>

#include "druk/ir/ir_opcode.h"
#include "druk/ir/ir_type.h"
#include "druk/ir/ir_value.h"

namespace druk::ir
{

class BasicBlock;

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
    Value* getOperand(uint32_t index) const
    {
        return operands_.at(index);
    }
    uint32_t getOperandCount() const
    {
        return static_cast<uint32_t>(operands_.size());
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

}  // namespace druk::ir
