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

StringConcatInst::StringConcatInst(Value* l, Value* r) : Instruction(Opcode::StringConcat)
{
    addOperand(l);
    addOperand(r);
}

std::string StringConcatInst::toString() const
{
    return "string_concat";
}

std::shared_ptr<Type> StringConcatInst::getType() const
{
    return std::make_shared<PointerType>(Type::getInt8Ty()); // String object ptr
}

ToStringInst::ToStringInst(Value* val) : Instruction(Opcode::ToString)
{
    addOperand(val);
}

std::string ToStringInst::toString() const
{
    return "to_string";
}

std::shared_ptr<Type> ToStringInst::getType() const
{
    return std::make_shared<PointerType>(Type::getInt8Ty()); // String object ptr
}

UnwrapInst::UnwrapInst(Value* val) : Instruction(Opcode::Unwrap)
{
    addOperand(val);
}

std::string UnwrapInst::toString() const
{
    return "unwrap";
}

std::shared_ptr<Type> UnwrapInst::getType() const
{
    if (getOperands().empty()) return Type::getVoidTy();
    return getOperands()[0]->getType();
}

UnaryInst::UnaryInst(Opcode op, Value* val) : Instruction(op)
{
    addOperand(val);
}

std::string UnaryInst::toString() const
{
    return getOpcode() == Opcode::Neg ? "neg" : "not";
}

std::shared_ptr<Type> UnaryInst::getType() const
{
    if (getOperands().empty())
        return Type::getVoidTy();
    return getOperands()[0]->getType();
}

}  // namespace druk::ir
