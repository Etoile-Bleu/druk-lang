#include "druk/ir/ir_instruction_memory.h"

namespace druk::ir
{

AllocaInst::AllocaInst(std::shared_ptr<Type> type)
    : Instruction(Opcode::Alloca), allocatedType_(type)
{
}

std::string AllocaInst::toString() const
{
    return "alloca";
}

std::shared_ptr<Type> AllocaInst::getType() const
{
    return allocatedType_;
}

LoadInst::LoadInst(Value* ptr) : Instruction(Opcode::Load)
{
    addOperand(ptr);
}

std::string LoadInst::toString() const
{
    return "load";
}

std::shared_ptr<Type> LoadInst::getType() const
{
    return Type::getVoidTy();
}

StoreInst::StoreInst(Value* val, Value* ptr) : Instruction(Opcode::Store)
{
    addOperand(val);
    addOperand(ptr);
}

std::string StoreInst::toString() const
{
    return "store";
}

std::shared_ptr<Type> StoreInst::getType() const
{
    return Type::getVoidTy();
}

}  // namespace druk::ir
