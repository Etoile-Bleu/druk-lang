#pragma once

#include <list>

#include "druk/ir/ir_instruction.h"
#include "druk/ir/ir_value.h"

namespace druk::ir
{

class Function;

/**
 * @brief Container for a sequence of IR instructions.
 */
class BasicBlock : public Value
{
   public:
    explicit BasicBlock(const std::string& name = "", Function* parent = nullptr);

    std::string           toString() const override;
    std::shared_ptr<Type> getType() const override;

    void appendInstruction(std::unique_ptr<Instruction> inst);

    auto begin()
    {
        return instructions_.begin();
    }
    auto end()
    {
        return instructions_.end();
    }

    Function* getParent() const
    {
        return parent_;
    }
    void setParent(Function* parent)
    {
        parent_ = parent;
    }

    bool hasTerminator() const;

   private:
    std::list<std::unique_ptr<Instruction>> instructions_;
    Function*                               parent_;
};

}  // namespace druk::ir
