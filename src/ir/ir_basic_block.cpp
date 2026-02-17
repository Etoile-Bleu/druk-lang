#include "druk/ir/ir_basic_block.h"

namespace druk::ir
{

BasicBlock::BasicBlock(const std::string& name, Function* parent) : parent_(parent)
{
    setName(name);
}

std::string BasicBlock::toString() const
{
    return getName() + ":";
}

std::shared_ptr<Type> BasicBlock::getType() const
{
    return nullptr;  // Basic blocks don't really have a type in the usual sense
}

void BasicBlock::appendInstruction(std::unique_ptr<Instruction> inst)
{
    inst->setParent(this);
    instructions_.push_back(std::move(inst));
}

bool BasicBlock::hasTerminator() const
{
    if (instructions_.empty())
        return false;
    
    auto opcode = instructions_.back()->getOpcode();
    return opcode == Opcode::Return || 
           opcode == Opcode::Branch || 
           opcode == Opcode::ConditionalBranch;
}

}  // namespace druk::ir
