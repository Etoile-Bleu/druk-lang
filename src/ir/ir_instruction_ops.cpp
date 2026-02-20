#include "druk/ir/ir_instruction_ops.h"

namespace druk::ir
{

BinaryInst::BinaryInst(Opcode op, Value* l, Value* r) : Instruction(op)
{
    addOperand(l);
    addOperand(r);
}

std::string BinaryInst::toString() const
{
    return "binop";
}

std::shared_ptr<Type> BinaryInst::getType() const
{
    if (getOperands().empty())
        return Type::getVoidTy();
    return getOperands()[0]->getType();
}

}  // namespace druk::ir
